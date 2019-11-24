#include <iostream>
#include <fstream>
#include "machdump.h"
#include "tostringloader.h"

#define DEBUG(x) do { std::cerr << x; } while (0)


void handleError(bool err)
{
  if (!err)
    return;
  std::cout << "Failed with error: " << strerror(errno) << std::endl;
  exit(EXIT_FAILURE);
}

// prints X.Y.Z which is encoded in nibbles xxxx.yy.zz
void parseVersions(uint32_t version)
{
    std::cout << std::dec
    << ((version & 0xFFFF0000) >> 16) << "." 
    << ((version & 0x0000FF00) >> 8) << "." 
    << (version & 0xFF);
}

MachFile::MachFile(const char* fileName) : ptr(0)
{
  // get the file size
  std::ifstream fs (fileName, std::ios::in | std::ios::binary);
  fs.seekg(0, std::ios_base::end);
  basicInfo.fileSize = fs.tellg(); // record the file size
  fs.clear();
  fs.seekg(0, std::ios::beg);

  // read file into local buffer
  machfile = new char[basicInfo.fileSize];
  fs.read(machfile, basicInfo.fileSize);
  handleError(fs.fail());
  fs.close();
  
  mach_header_64* pMachHeader = reinterpret_cast<mach_header_64*>(machfile);
#ifdef VERBOSE
  printHeader(pMachHeader);
#endif
  parseHeader(pMachHeader);

  if (basicInfo.magic != MH_MAGIC_64)
  {
    std::cout << "Mach-O 64 files only" << std::endl;
    exit(1);
  }

  // parse each LOAD command
  load_command* pLoadCommand;
  for (int i = 0; i < pMachHeader->ncmds; ++i)
  {
    pLoadCommand = reinterpret_cast<load_command*>(machfile + ptr);
#ifdef VERBOSE
    printLC(pLoadCommand, i);
#endif
    parseLC(pLoadCommand);
    // Go to the next segment
    ptr += pLoadCommand->cmdsize;
  }
}

MachFile::~MachFile() 
{
  delete machfile;
}

// ---------------------------------------------------------------------------
// Parse and Print Commands
// ---------------------------------------------------------------------------

void MachFile::parseHeader(mach_header_64* pMachHeader)
{
  symbolsInfo.isTwoLevel = pMachHeader->flags & MH_TWOLEVEL;
  basicInfo.magic = pMachHeader->magic;
  basicInfo.numberOfLoadCommands = pMachHeader->ncmds;
  ptr += sizeof(mach_header_64);
}

void MachFile::printHeader(mach_header_64* pMachHeader) 
{
  std::cout << "MACH HEADER" << std::endl;
  std::cout << std::hex << "\tmagic: " RESET << "0x"  << pMachHeader->magic << std::endl;
  std::cout << "\tcputype: " RESET << "0x"  << pMachHeader->cputype << std::endl;
  std::cout << "\tcpusubtype: " RESET << "0x"  << pMachHeader->cpusubtype << std::endl;
  std::cout << "\tfiletype: " <<toStringMH(pMachHeader->filetype) << 
    " "  RESET << "(0x"  << std::hex << pMachHeader->filetype << ")" << std::endl;
  std::cout << std::dec << MAGENTA "\tncmds: " RESET << pMachHeader->ncmds << std::endl;
  std::cout << GREEN"\tsizeofcmds: " RESET<< pMachHeader->sizeofcmds << std::endl;
  std::cout << std:: hex << "\tflags: " RESET << "0x"  << pMachHeader->flags << std::endl;
  if (symbolsInfo.isTwoLevel)
    std::cout << "This file is TWOLEVEL" << std::endl;
}

void MachFile::parseLC(load_command* pLoadCommand)
{
    switch(pLoadCommand->cmd)
    {
      case LC_SEGMENT_64:
        return loadSegmentCommand64();
      case LC_DYLD_INFO_ONLY:
        return loadDyldInfoCommand();
      case LC_SYMTAB:
        return loadSymtabCommand();
      case LC_DYSYMTAB:
        return loadDysymtabCommand();
      case LC_ID_DYLINKER: 
      case LC_LOAD_DYLINKER: 
      case LC_DYLD_ENVIRONMENT:
        return loadDylinkerCommand();
      case LC_UUID:
        return loadUUIDCommand();
      case LC_BUILD_VERSION:
        return loadBuildVersion();
      case LC_MAIN:
        return loadEntryPointCommand();
      case LC_LOAD_DYLIB: // added by the static linker
      case LC_LOAD_WEAK_DYLIB:
      case LC_REEXPORT_DYLIB:
        return loadDylibCommand();
      case LC_CODE_SIGNATURE:
      case LC_SEGMENT_SPLIT_INFO:
      case LC_FUNCTION_STARTS:
      case LC_DATA_IN_CODE:
      case LC_DYLIB_CODE_SIGN_DRS:
      case LC_LINKER_OPTIMIZATION_HINT:
        return loadLinkeditDataCommand();
      default:
        std::cout << "No action taken for Cmd ID: " << std::hex << "0x" << pLoadCommand->cmd << std::endl;
    }
}

void MachFile::printLC(load_command* pLoadCommand, uint32_t lcNum)
{
    std::cout << "\nLC Command " << std::dec << lcNum << ": " << toStringLC(pLoadCommand->cmd) << std::endl;
    std::cout << GREEN"Header Size: " RESET << "0x"  << std::hex << pLoadCommand->cmdsize << std::endl;
}

void MachFile::parseSegmentCommand64(segment_command_64* seg64)
{
  if ( !strncmp(seg64->segname, "__LINKEDIT", 10))
  {
    loaderInfo.linkedItSegPtr = reinterpret_cast<uintptr_t>(seg64);
  }
}

void MachFile::printSegmentCommand64(segment_command_64* seg64)
{
  std::cout << "Segment: " << seg64->segname << std::endl;
  std::cout << MAGENTA"\tnum sections: " << seg64->nsects << std::endl;
  std::cout << BLUE"\tVM address: " RESET << "0x"  << std::hex << seg64->vmaddr << std::endl;
  std::cout << GREEN"\tVM segment size: " RESET << "0x"  << seg64->vmsize << std::endl;
  std::cout << RED"\tFile Offset: " RESET << "0x"  << seg64->fileoff << std::endl;
  std::cout << GREEN"\tFile size: " RESET << "0x"  << seg64->filesize << std::endl;
  std::cout << "\tFlags: " RESET << "0x"  << std::hex << seg64->flags << std::endl;
}

void MachFile::parseSection64(section_64* sec64)
{
  if ( !strncmp(sec64->sectname, "__text", 6))
  {
    loaderInfo.textPtr = reinterpret_cast<uintptr_t>(sec64);
  }
  else if ( !strncmp(sec64->sectname, "__nl_symbol_ptr", 15))
  {
    loaderInfo.nlSymbolPtr = reinterpret_cast<uintptr_t>(sec64);
  }
  else if ( !strncmp(sec64->sectname, "__la_symbol_ptr", 15)) // pointer to imported functions
  {
    loaderInfo.laSymbolPtr = reinterpret_cast<uintptr_t>(sec64);
  }
  else if ( !strncmp(sec64->sectname, "__data", 6))
  {
    loaderInfo.dataPtr = reinterpret_cast<uintptr_t>(sec64);
  }
}

void MachFile::printSection64(section_64* sec64)
{
    std::cout << "\tSection: " << sec64->sectname << std::endl;
    std::cout << BLUE"\t\tVM address: " RESET << "0x"  << sec64->addr << std::endl;
    std::cout << GREEN"\t\tVM section size: " RESET << "0x"  << sec64->size << std::endl;
    std::cout << RED"\t\tFile Offset: " RESET << "0x"  << sec64->offset << std::endl;
    std::cout << "\t\tSection Alignment: " RESET << "0x"  << (1 << sec64->align) << std::endl;
    std::cout << RED"\t\tFile Offset Relocation Entries: " RESET << "0x"  << sec64->reloff << std::endl;
    std::cout << MAGENTA"\t\tNumber of Relocation Entries: " RESET << std::dec << sec64->nreloc << std::endl;
    std::cout << "\t\tFlags: " RESET << "0x"  << std::hex << sec64->flags << std::endl;
    std::cout << RED "\t\tReserved 1 (offset or index): " RESET << "0x" << std::hex << sec64->reserved1 << std::endl;
    std::cout << MAGENTA "\t\tReserved 2 (count or sizeof): " RESET << "0x" << std::hex << sec64->reserved2 << std::endl;
    std::cout << "\t\tReserved 3 (reserved): " RESET << "0x" << std::hex << sec64->reserved3 << std::endl; 
}

void MachFile::parseDyldInfoCommand(dyld_info_command* dic)
{

}

void MachFile::printDyldInfoCommand(dyld_info_command* dic)
{
  std::cout << RED"\trebase info at offset: " RESET << "0x"  << std::hex << 
    dic->rebase_off << GREEN" size "  RESET << "(0x"  << dic->rebase_size << ")" << std::endl ;

  std::cout << RED"\tbind info at offset: " RESET << "0x"  
    << dic->bind_off << GREEN" size "  RESET << "(0x"  << dic->bind_size << ")" << std::endl ;

  std::cout << RED"\tweak_bind info at offset: " RESET << "0x" 
    << dic->weak_bind_off << GREEN" size "  RESET << "(0x" 
    << dic->weak_bind_size << ")" << std::endl ;

  std::cout << RED"\tlazy_bind info at offset: " RESET << "0x" 
    << dic->lazy_bind_off << GREEN" size "  RESET << "(0x"  << dic->lazy_bind_size << ")" << std::endl ;

  std::cout << RED"\texport info at offset: " RESET << "0x" 
    << dic->export_off << GREEN" size "  RESET << "(0x"  << dic->export_size << ")" << std::endl ;
}

void MachFile::parseSymtabCommand(symtab_command* sc)
{
  symbolsInfo.symTablePtr = reinterpret_cast<uintptr_t>((reinterpret_cast<uintptr_t>(machfile) + sc->symoff));
  symbolsInfo.strTablePtr = reinterpret_cast<uintptr_t>((reinterpret_cast<uintptr_t>(machfile) + sc->stroff));
}

void MachFile::printSymtabCommand(symtab_command* sc)
{
  std::cout << RED"\tSymbols table starts at offset: " RESET << "0x"  << std::hex << sc->symoff << std::endl;
  std::cout << MAGENTA"\t\tNum Entries: "  RESET << std::dec << sc->nsyms << std::endl;
  std::cout << RED"\tStrings table at offset: " RESET << "0x" << std::hex << sc->stroff << std::endl;
  std::cout << GREEN"\t\tSize: "  RESET << "0x"  << sc->strsize << std::endl;
  // printSymbolTable(symbolsInfo.symTablePtr, sc->nsyms);
}

void MachFile::parseDsymtabCommand(dysymtab_command* dc)
{
  symbolsInfo.indirSymTable = reinterpret_cast<uintptr_t>(machfile + dc->indirectsymoff);
  if (dc->ntoc) parseToC(dc->tocoff, dc->ntoc);
  if (dc->nmodtab) parseModuleTable(dc->modtaboff, dc->nmodtab);
  if (dc->nextrefsyms) parseModuleTable(dc->extrefsymoff, dc->nextrefsyms);
  parseIndirtab((symbolsInfo.indirSymTable) , dc->nindirectsyms);
}

void MachFile::printDsymtabCommand(dysymtab_command* dc)
{
  std::cout << YELLOW"\texternal symbols at index: " RESET << "0x"  << std::hex << dc->iextdefsym;
  std::cout << std::dec << MAGENTA" (count: "  RESET << dc->nextdefsym << ")" << std::endl;
  // printSymbolTable(symbolsInfo.symTablePtr + dc->iextdefsym * (sizeof (nlist_64)), dc->nextdefsym);

  std::cout << YELLOW"\tundefined symbols at index: " RESET << "0x" << std::hex << dc->iundefsym;
  std::cout << std::dec << MAGENTA" (count: "  RESET << dc->nundefsym << ")" << std::endl;
  // printSymbolTable(symbolsInfo.symTablePtr +  dc->iundefsym * (sizeof (nlist_64)), dc->nundefsym);

  // This is for dynamically liked shared library files only
  std::cout << RED"\ttable of contents at offset: " RESET << "0x" << std::hex << dc->tocoff;
  std::cout << std::dec << MAGENTA" (count: "  RESET << dc->ntoc << ")"<< std::endl;

  // This is for dynamic binding of object files ("modules")
  std::cout << RED"\tmodule table at offset: " RESET << "0x" << std::hex << dc->modtaboff;
  std::cout << std::dec << MAGENTA" (count: " RESET << dc->nmodtab << ")"<< std::endl;

  // The external references that each module makes (dynamically linked shared library files only)
  std::cout << RED"\texternal reference symbol table at offset: " RESET << "0x"  << std::hex << dc->extrefsymoff;
  std::cout << std::dec << MAGENTA" (count: " RESET << dc->nextrefsyms << ")"<< std::endl;
  
  // The indirect references "symbol pointers" and "routine stubs"
  std::cout << RED"\tindirect symbol table at offset: " RESET << "0x" << std::hex << dc->indirectsymoff;
  std::cout << std::dec << MAGENTA" (count: " RESET << dc->nindirectsyms << ")"<< std::endl;
  // printIndirtab((symbolsInfo.indirSymTable) , dc->nindirectsyms);

  // External relocation entries
  std::cout << RED"\texternal relocation entries at offset: " RESET << "0x" << std::hex << dc->extreloff; 
  std::cout << std::dec << MAGENTA" (count: "  RESET << dc->nextrel << ")"<< std::endl;

  // Local relocation entries (only used if module is moved from statically linked edited address)
  std::cout << RED"\tlocal relocation entries at offset: " RESET << "0x" << std::hex << dc->locreloff;
  std::cout << std::dec << MAGENTA" (count: "  RESET << dc->nlocrel << ")"<< std::endl;
}

void MachFile::parseEntrypointCommand(entry_point_command* ep)
{
  basicInfo.entrypointOffset = ep->entryoff;
}

void MachFile::printEntrypointCommand(entry_point_command* ep)
{
  std::cout << RED"Entrypoint offset: " RESET << "0x"  << std::hex << ep->entryoff << std::endl;
}

void MachFile::parseDylibCommand(dylib_command* dc)
{
}

void MachFile::printDylibCommand(dylib_command* dc)
{
  char* nameAddr = machfile + ptr + dc->dylib.name.offset;

  std::cout << "\tDylib pathname: " << nameAddr << std::endl;

  std::cout << "\tCompatibility Version: ";
  parseVersions(dc->dylib.current_version);
  std::cout << std::endl;

  std::cout << "\tCompatibility Version: ";
  parseVersions(dc->dylib.compatibility_version);
  std::cout << std::endl;
}

void MachFile::parseLinkeditDataCommand(linkedit_data_command* ldc)
{
}

void MachFile::printLinkeditDataCommand(linkedit_data_command* ldc)
{
  std::cout << RED"\tOffset of data in __LINKEDIT segment: " RESET << "0x"  
    << std::hex << ldc->dataoff << std::endl;
  std::cout << GREEN"\tData size: " RESET << "0x"  << ldc->datasize << std::endl;
}

void MachFile::printDylinkerCommand(dylinker_command* dc)
{
  char* nameAddr = machfile + ptr + dc->name.offset;
  std::cout << "\tDynamic Linker: " << nameAddr << std::endl;
}

void MachFile::printUUIDCommand(uuid_command* uuid)
{
  uint8_t* id = uuid->uuid;
  std::cout << "\tUUID: \n\t\t";
  for (int i = 0; i < 16; ++i)
    std::cout << (uint16_t)id[i] << " ";
  
  std::cout << std::endl;
}

void MachFile::printBuildVersionCommand(build_version_command* b)
{
  std::cout << "\tMin OS: ";
  parseVersions(b->minos);
  std::cout << std::endl;

  std::cout << "\tplatform: " << toStringPLATFORM(b->platform) << std::endl;

  uint32_t sdk = b->sdk;
  std::cout << "\tSDK: ";
  parseVersions(b->sdk);
  std::cout << std::endl;

  char* btvAddr = machfile + ptr + sizeof(build_version_command);
  build_tool_version* btv;
  for (int i = 0; i < b->ntools; i++)
  {
    btv = reinterpret_cast<build_tool_version*>(btvAddr + i*sizeof(build_tool_version));
    std::cout << "\tTools: " << toStringTOOL(btv->tool);
    parseVersions(btv->version); 
    std::cout << std::endl;
  }
}

// ---------------------------------------------------------------------------
// Load Commands
// ---------------------------------------------------------------------------

void MachFile::loadSegmentCommand64()
{
  segment_command_64* seg64 = reinterpret_cast<segment_command_64*>(machfile + ptr);
  parseSegmentCommand64(seg64);
#ifdef VERBOSE
  printSegmentCommand64(seg64);
#endif
  if (seg64->nsects == 0 ) return;

  for(size_t i = 0; i < seg64->nsects; ++i){
    section_64* sec64 = reinterpret_cast<section_64*>(machfile + ptr + sizeof(segment_command_64) + i*sizeof(section_64));
    parseSection64(sec64);
#ifdef VERBOSE
    printSection64(sec64);
#endif
  }
}

void MachFile::loadDyldInfoCommand()
{
  dyld_info_command* dic = reinterpret_cast<dyld_info_command*>(machfile + ptr);
  parseDyldInfoCommand(dic);
#ifdef VERBOSE
  printDyldInfoCommand(dic);
#endif
}

void MachFile::loadSymtabCommand()
{
  symtab_command* sc = reinterpret_cast<symtab_command*>(machfile + ptr);
  parseSymtabCommand(sc);
#ifdef VERBOSE
  printSymtabCommand(sc);
#endif
}

// describes the dynamic linking table (symbols used for dynamic binding)
void MachFile::loadDysymtabCommand()
{
  dysymtab_command* dc = reinterpret_cast<dysymtab_command*>(machfile + ptr);
  parseDsymtabCommand(dc);
#ifdef VERBOSE
  printDsymtabCommand(dc);
#endif
}

// For dynamically linked executables, this command specifies the name of the dynamic linker
// that the kernel must load in order to execute the file
// Dylinker itself specifies its name usign LC_ID_DYLINKER load command
void MachFile::loadDylinkerCommand()
{
  dylinker_command* dc = reinterpret_cast<dylinker_command*>(machfile + ptr);
#ifdef VERBOSE
  printDylinkerCommand(dc);
#endif
}

void MachFile::loadUUIDCommand()
{
  uuid_command* uuid = reinterpret_cast<uuid_command*>(machfile + ptr);
#ifdef VERBOSE
  printUUIDCommand(uuid);
#endif
}

void MachFile::loadBuildVersion()
{
  build_version_command* b = reinterpret_cast<build_version_command*>(machfile + ptr);
#ifdef VERBOSE
  printBuildVersionCommand(b);
#endif
}

void MachFile::loadEntryPointCommand()
{
  entry_point_command* ep = reinterpret_cast<entry_point_command*>(machfile + ptr);
  parseEntrypointCommand(ep);
#ifdef VERBOSE
  printEntrypointCommand(ep);
#endif
}
// dylib struct is data used by dynamic linker to match shared library 
// against files that have linked to it
void MachFile::loadDylibCommand()
{
  dylib_command* dc = reinterpret_cast<dylib_command*>(machfile + ptr);
  parseDylibCommand(dc);
#ifdef VERBOSE
  printDylibCommand(dc);
#endif
}

void MachFile::loadLinkeditDataCommand()
{
  linkedit_data_command* ldc = reinterpret_cast<linkedit_data_command*>(machfile + ptr);
  parseLinkeditDataCommand(ldc);
#ifdef VERBOSE
  printLinkeditDataCommand(ldc);
#endif
}

// ---------------------------------------------------------------------------
// Dysym
// ---------------------------------------------------------------------------

// defined external symbols sorted by name
void MachFile::parseToC(uint32_t offset, uint32_t count)
{
  std::cout << "This is for dynamically linked shared library files only" << std::endl;
}

void MachFile::parseModuleTable(uint32_t offset, uint32_t count)
{
  std::cout << "This is for dynamically linked shared library files only" << std::endl;
}
// has the defined and undefined external symbols
void MachFile::parseRefTable(uint32_t offset, uint32_t count)
{
  std::cout << "This is for dynamically linked shared library files only" << std::endl;
}

void MachFile::parseTwoLevel(uint16_t n_desc)
{
  if (symbolsInfo.isTwoLevel)
  {
    uint8_t ordinal = GET_LIBRARY_ORDINAL(n_desc);
    std::cout << "\t\tlibrary ordinal: " << std::dec << (uint16_t)ordinal << " " << toStringREFERENCE(REFERENCE_TYPE & n_desc);
  }
  else
  {
    std::cout << "\t\tdesc: " << toStringREFERENCE(REFERENCE_TYPE & n_desc) << "(0x"  << std::hex << (uint16_t) n_desc << ") ";
  } 
}

// ---------------------------------------------------------------------------
// Symbols
// ---------------------------------------------------------------------------

// prints an Nlist_64
void MachFile::printNlist(nlist_64* symEntry)
{
    std::string stype = toStringN(N_TYPE & symEntry->n_type);
    if (N_STAB & symEntry->n_type)
      std::cout << "\t\tN_STAB entry (debug): " << toStringSTAB(symEntry->n_type);
    else if (N_PEXT & symEntry->n_type)
      std::cout << "\t\tN_PEXT (private extern) " << stype;
    else if (N_EXT & symEntry->n_type)
      std::cout << "\t\t N_EXT (external symbol) "<< stype;
    else
      std::cout << "\t\t" << stype;
    std::cout << " section number: " << std::dec << (uint16_t) symEntry->n_sect;
    parseTwoLevel(symEntry->n_desc);
    std::cout << " value:"  << "(0x"  << std::hex << symEntry->n_value << ") " 
      << toStringN(symEntry->n_value) << std::endl;
}

void MachFile::printSymbolTable(uintptr_t tableStartAddr, uint32_t numEntries)
{
   nlist_64* symEntry;
  for (int i = 0; i < numEntries; ++i)
  {
    symEntry = reinterpret_cast<nlist_64*>(tableStartAddr+ i*sizeof(nlist_64));
    printNlist(symEntry);
  } 
}

void MachFile::parseIndirtab(uintptr_t tableStartAddr, uint32_t numEntries)
{
  // indirect symbol table entry is 32 bit index into the symbol table 
}

void MachFile::printIndirtab(uintptr_t tableStartAddr, uint32_t numEntries)
{
  uint32_t* indirEntry; // pointer to an index into the sym table
  nlist_64* symEntry;
  std::cout << YELLOW "\t\tSymtab Index \t\tStrTab Index\t\tValue " RESET << std::endl;
  for (int i = 0; i < numEntries; ++i)
  {
    indirEntry = reinterpret_cast<uint32_t*>(tableStartAddr + i*sizeof(uint32_t));
    std::cout << "\t\t" << std::dec << *indirEntry;
    
    symEntry = reinterpret_cast<nlist_64*>((nlist_64*)symbolsInfo.symTablePtr + *indirEntry);
    uint32_t strIdx = symEntry->n_un.n_strx; 
    std::cout << "\t\t\t" << symEntry->n_un.n_strx;
    std::cout <<  "\t\t\t" <<reinterpret_cast<char*>(symbolsInfo.strTablePtr + strIdx) << std::endl;
#ifdef VERBOSE
    printNlist(symEntry);
#endif
  }
}