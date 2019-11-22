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

void MachFile::loadSegmentCommand64()
{
  segment_command_64* seg64 = reinterpret_cast<segment_command_64*>(temp + ptr);

  std::cout << "Segment: " << seg64->segname << std::endl;
  std::cout << MAGENTA"\tnum sections: " << seg64->nsects << std::endl;
  std::cout << BLUE"\tVM address: " RESET << "0x"  << std::hex << seg64->vmaddr << std::endl;
  std::cout << GREEN"\tVM segment size: " RESET << "0x"  << seg64->vmsize << std::endl;
  std::cout << RED"\tFile Offset: " RESET << "0x"  << seg64->fileoff << std::endl;
  std::cout << GREEN"\tFile size: " RESET << "0x"  << seg64->filesize << std::endl;
  std::cout << "\tFlags: " RESET << "0x"  << std::hex << seg64->flags << std::endl;
  
  if (seg64->nsects == 0 ) return;

  // enumerate sections
  for(size_t i = 0; i < seg64->nsects; ++i){
    section_64* sec64 = reinterpret_cast<section_64*>(temp + ptr + sizeof(segment_command_64) + i*sizeof(section_64));
    
    std::cout << "\tSection: " << sec64->sectname << std::endl;
    std::cout << BLUE"\t\tVM address: " RESET << "0x"  << sec64->addr << std::endl;
    std::cout << GREEN"\t\tVM section size: " RESET << "0x"  << sec64->size << std::endl;
    std::cout << RED"\t\tFile Offset: " RESET << "0x"  << sec64->offset << std::endl;
    std::cout << "\t\tSection Alignment: " RESET << "0x"  << (1 << sec64->align) << std::endl;
    std::cout << RED"\t\tFile Offset Relocation Entries: " RESET << "0x"  << sec64->reloff << std::endl;
    std::cout << MAGENTA"\t\tNumber of Relocation Entries: " << std::dec << sec64->nreloc << std::endl;
    std::cout << "\t\tFlags: " RESET << "0x"  << std::hex << sec64->flags << std::endl;
  }
  // do something
}

void MachFile::loadDyldInfoCommand()
{
  dyld_info_command* dic = reinterpret_cast<dyld_info_command*>(temp + ptr);
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

void MachFile::loadSymtabCommand()
{
  symtab_command* sc = reinterpret_cast<symtab_command*>(temp + ptr);
  std::cout << RED"\tSymbol table at offset: " RESET << "0x"  
    << std::hex << sc->symoff << MAGENTA" count "  RESET << "(0x"  << sc->nsyms << ")" << std::endl;
  // parseSymbolTable(sc->symoff, sc->nsyms);
  std::cout << RED"\tStrings table at offset: " RESET << "0x"  
    << std::hex << sc->stroff << GREEN" size "  RESET << "(0x"  << sc->strsize << ")" << std::endl;
}

void MachFile::loadDylinkerCommand()
{
  dylinker_command* dc = reinterpret_cast<dylinker_command*>(temp + ptr);
  char* nameAddr = temp + ptr + dc->name.offset;
  std::cout << "\tDynamic Linker: " << nameAddr << std::endl;
}

void MachFile::loadUUIDCommand()
{
  uuid_command* uuid = reinterpret_cast<uuid_command*>(temp + ptr);
  uint8_t* id = uuid->uuid;

  std::cout << "\tUUID: \n\t\t";
  
  for (int i = 0; i < 16; ++i)
    std::cout << (uint16_t)id[i] << " ";
  
  std::cout << std::endl;
}

void MachFile::loadBuildVersion()
{
  build_version_command* b = reinterpret_cast<build_version_command*>(temp + ptr);
  uint32_t minos = b->minos;
  std::cout << std::dec << "\tMin OS: " 
    << ((minos & 0xFFFF0000) >> 16) << "." 
    << ((minos & 0x0000FF00) >> 8) << "." 
    << (minos & 0xFF) << std::endl;

  std::cout << "\tplatform: " << toStringPLATFORM(b->platform) << std::endl;

  uint32_t sdk = b->sdk;
  std::cout << "\tSDK: " 
    << ((sdk & 0xFFFF0000) >> 16) << "." 
    << ((sdk & 0x0000FF00) >> 8) << "." 
    << (sdk & 0xFF) << std::endl;

  char* btvAddr = temp + ptr + sizeof(build_version_command);
  build_tool_version* btv;
  for (int i = 0; i < b->ntools; i++)
  {
    btv = reinterpret_cast<build_tool_version*>(btvAddr + i*sizeof(build_tool_version));
    std::cout << "\tTools: " << toStringTOOL(btv->tool) << " v" 
      << std::hex << (uint32_t) btv->version << std::endl;
  }
}

void MachFile::loadSourceVersion()
{
  source_version_command* s = reinterpret_cast<source_version_command*>(temp + ptr);
  std::cout << "\tSource Version: " << s->version << std::endl;
}

void MachFile::loadEntryPointCommand()
{
  entry_point_command* ep = reinterpret_cast<entry_point_command*>(temp + ptr);
  std::cout << RED"Entrypoint offset: " RESET << "0x"  << std::hex << ep->entryoff << std::endl;
}

void MachFile::loadDylibCommand()
{
  dylib_command* dc = reinterpret_cast<dylib_command*>(temp + ptr);
  char* nameAddr = temp + ptr + dc->dylib.name.offset;
  std::cout << "\tDylib pathname: " << nameAddr << std::endl;

  std::cout << std::hex << "\tCurrent Version: " 
    << dc->dylib.current_version << std::endl;

  std::cout << "\tCompatability Version: " 
    << dc->dylib.compatibility_version << std::endl;
}

void MachFile::loadLinkeditDataCommand()
{
  linkedit_data_command* ldc = reinterpret_cast<linkedit_data_command*>(temp + ptr);
  std::cout << RED"\tOffset of data in __LINKEDIT segment: " RESET << "0x"  
    << std::hex << ldc->dataoff << std::endl;

  std::cout << GREEN"\tData size: " RESET << "0x"  << ldc->datasize << std::endl;
}

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

void MachFile::parseSymbolTable(uint32_t offset, uint32_t count)
{
  nlist_64* symEntry;
  for (int i = 0; i < count; ++i)
  {
    symEntry = reinterpret_cast<nlist_64*>(temp + ptr + offset + i*sizeof(nlist_64));
    std::cout << "\t\ttype: " << toStringN((uint16_t)symEntry->n_type) 
      << " section number: " << std::hex << (uint16_t) symEntry->n_sect
      << " desc: " << toStringN(symEntry->n_desc) 
      << " "  RESET << "(0x"  << std::hex << symEntry->n_desc << ") "
      << " value: " << toStringN(symEntry->n_value) 
      << " "  RESET << "(0x"  << std::hex << symEntry->n_value << ") "<< std::endl;
  }
}

void MachFile::loadDysymtabCommand()
{
  dysymtab_command* dc = reinterpret_cast<dysymtab_command*>(temp + ptr);
  // These symbols used for dynamic binding

  std::cout << "\texternal symbols at index: " RESET << "0x"  
    << std::hex << dc->iextdefsym << std::dec << MAGENTA" (count: "  RESET
    << dc->nextdefsym << ")" << std::endl;

  std::cout << "\tundefined symbols at index: " RESET << "0x" 
    << std::hex << dc->iundefsym << std::dec << MAGENTA" (count: "  RESET
    << dc->nundefsym << ")" << std::endl;
  // If this is a dynamic linked shared library, then binding is indirectly through module and reference table

  // This is for dynamically liked shared library files only
  std::cout << RED"\ttable of contents at offset: " RESET << "0x" 
    << std::hex << dc->tocoff << std::dec << MAGENTA" (count: "  RESET
    << dc->ntoc << ")"<< std::endl;
  if (dc->ntoc)
    parseToC(dc->tocoff, dc->ntoc);

  // This is for dynamic binding of object files ("modules")
  std::cout << RED"\tmodule table at offset: " RESET << "0x" 
    << std::hex << dc->modtaboff << std::dec << MAGENTA" (count: " RESET
    << dc->nmodtab << ")"<< std::endl;
  if (dc->nmodtab)
    parseModuleTable(dc->modtaboff, dc->nmodtab);

  // The external references that each module makes (dynamically linked shared library files only)
  std::cout << RED"\texternal reference symbol table at offset: " RESET << "0x"  
    << std::hex << dc->extrefsymoff << std::dec << MAGENTA" (count: " RESET 
    << dc->nextrefsyms << ")"<< std::endl;
  if (dc->nextrefsyms)
    parseModuleTable(dc->extrefsymoff, dc->nextdefsym);
  
  // The indirect references "symbol pointers" and "routine stubs"
  std::cout << RED"\tindirect symbol table at offset: " RESET << "0x" 
    << std::hex << dc->indirectsymoff << std::dec << MAGENTA" (count: " RESET 
    << dc->nindirectsyms << ")"<< std::endl;
  // if (dc->nindirectsyms)
  //   parseSymbolTable(dc->nindirectsyms, dc->nindirectsyms);

  // External relocation entries
  std::cout << RED"\texternal relocation entries at offset: " RESET << "0x" 
    << std::hex << dc->extreloff << std::dec << MAGENTA" (count: "  RESET
    << dc->nextrel << ")"<< std::endl;

  // Local relocation entries (only used if module is moved from statically linked edited address)
  std::cout << RED"\tlocal relocation entries at offset: " RESET << "0x" 
    << std::hex << dc->locreloff << std::dec << MAGENTA" (count: "  RESET
    << dc->nlocrel << ")"<< std::endl;

}

void MachFile::parseLC(size_t i)
{
    std::cout << "\nLC Header: " << toStringLC(pLoadCommands[i]->cmd) << std::endl;
    std::cout << GREEN"Header Size: " RESET << "0x"  << std::hex << pLoadCommands[i]->cmdsize << std::endl;
    switch(pLoadCommands[i]->cmd){
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
      case LC_SOURCE_VERSION:
        return loadSourceVersion();
      case LC_MAIN:
        return loadEntryPointCommand();
      case LC_LOAD_DYLIB:
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
        std::cout << "idk what to do for: " << std::hex << "0x" << pLoadCommands[i]->cmd << std::endl;
    }
}

void MachFile::parseHeader()
{
  pMachHeader = reinterpret_cast<mach_header_64*>(temp);
  ptr += sizeof(mach_header_64);
}

MachFile::MachFile(const char* fileName) : fileName(fileName), ptr(0)
{
  std::ifstream machFile (fileName, std::ios::in | std::ios::binary);
  machFile.read(temp, 10000);
  handleError(machFile.fail());

  parseHeader();
  pLoadCommands = new load_command*[pMachHeader->ncmds];

  for (int i = 0; i < pMachHeader->ncmds; ++i)
  {
    // Save the address of the headers of each segment
    pLoadCommands[i] = reinterpret_cast<load_command*>(temp + ptr);
    parseLC(i);
    // Go to the next segment
    ptr += pLoadCommands[i]->cmdsize;
  }
  machFile.close();
}

void MachFile::printHeader() {
  std::cout << std::hex << "magic: " RESET << "0x"  << pMachHeader->magic << std::endl;
  std::cout << "cputype: " RESET << "0x"  << pMachHeader->cputype << std::endl;
  std::cout << "cpusubtype: " RESET << "0x"  << pMachHeader->cpusubtype << std::endl;
  std::cout << "filetype: " <<toStringMH(pMachHeader->filetype) << 
    " "  RESET << "(0x"  << std::hex << pMachHeader->filetype << ")" << std::endl;
  std::cout << std::dec << MAGENTA"ncmds: "RESET << pMachHeader->ncmds << std::endl;
  std::cout << GREEN"sizeofcmds: " RESET<< pMachHeader->sizeofcmds << std::endl;
  std::cout << std:: hex << "flags: " RESET << "0x"  << pMachHeader->flags << std::endl;
}

void MachFile::printLoadCommands() {
  for (int i = 0; i < pMachHeader->ncmds; ++i)
  {
    std::cout << toStringLC(pLoadCommands[i]->cmd) << " "  RESET << "(0x"  <<
     std::hex << pLoadCommands[i]->cmd << ") " <<  std::endl;
    std::cout << std::dec << pLoadCommands[i]->cmdsize << std::endl;
  }
}


MachFile::~MachFile() 
{
  // delete [] pLoadCommands;
  // delete pMachHeader;
}
