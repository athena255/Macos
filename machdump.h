#pragma once
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>

#define RED     "\x1B[31m" // file offsets
#define GREEN   "\x1B[32m" // size
#define YELLOW  "\x1B[33m"  // index
#define BLUE    "\x1B[34m" // vm addresses
#define MAGENTA "\x1B[35m" // counts
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"
#define RESET   "\x1B[00m"

struct BasicInfo {
  uint32_t magic;
  uint32_t numberOfLoadCommands;
  size_t fileSize;
  uint32_t entrypointOffset; // entrypoint offset in the file
};

struct LoaderInfo {
  uintptr_t nlSymbolPtr; // pointer to section64 describing __nl_symbol_ptr
  uintptr_t laSymbolPtr; 
  uintptr_t textPtr; // pointer to section64 describing __text
  uintptr_t dataPtr; 
  uintptr_t linkedItSegPtr; // pointer to __LINKEDIT
};

struct SymbolsInfo {
  uint8_t isTwoLevel; // MH_TWOLEVEL flag of mach_header is set
  uintptr_t symTablePtr; // location of symtable in file
  size_t numIndirEntries; 
  uint32_t indirOffset;
  uintptr_t indirSymTable; // location of indirect symbol table in file
  uintptr_t strTablePtr; // location of string table in file
};

class MachFile{

public:
MachFile(const char* fileName);
~MachFile();

void printHeader(mach_header_64* pMachHeader);
void parseHeader(mach_header_64* pMachHeader);

void parseLC(load_command* pLoadCommand);
void printLC(load_command* pLoadCommand, uint32_t lcNum);

void parseSegmentCommand64(segment_command_64* seg64);
void printSegmentCommand64(segment_command_64* seg64);

void parseSection64(section_64* sec64);
void printSection64(section_64* sec64);

void parseDyldInfoCommand(dyld_info_command* dic); // does nothing
void printDyldInfoCommand(dyld_info_command* dic);

void disasRebase(uint32_t offset, uint32_t size);
void disasBind(uint32_t offset, uint32_t size);

void parseSymtabCommand(symtab_command* sc);
void printSymtabCommand(symtab_command* sc);

void parseDsymtabCommand(dysymtab_command* dc);
void printDsymtabCommand(dysymtab_command* dc);

void parseEntrypointCommand(entry_point_command* ep);
void printEntrypointCommand(entry_point_command* ep);

void parseDylibCommand(dylib_command* dc); // does nothing
void printDylibCommand(dylib_command* dc);

void parseLinkeditDataCommand(linkedit_data_command* ldc); // does nothing
void printLinkeditDataCommand(linkedit_data_command* ldc);

// fileoffset is offset from the front of the file
void parseDataInCode(uint32_t dataoffset, uint32_t datasize);
void printDataInCode(data_in_code_entry* dce);

void printDylinkerCommand(dylinker_command* dc);

void printUUIDCommand(uuid_command* uuid);

void printBuildVersionCommand(build_version_command* b);

// Load Commands
void loadSegmentCommand64();
void loadDyldInfoCommand();
void loadSymtabCommand();
void loadDysymtabCommand();
void loadDylinkerCommand();
void loadUUIDCommand();
void loadBuildVersion();
void loadEntryPointCommand();
void loadDylibCommand();
void loadLinkeditDataCommand();

// part of Dsymtab
void parseToC(uint32_t offset, uint32_t count);
void parseModuleTable(uint32_t offset, uint32_t count);
void parseRefTable(uint32_t offset, uint32_t count);
void parseTwoLevel(uint16_t n_desc);

// part of Symbols
void printNlist(nlist_64* symEntry); // print symbol table entries
// Print [numEntries] amount of entries from the symbol table starting at
// index [symtabStartIdx]
void printSymbolTable(uintptr_t symtabStartIdx, uint32_t numEntries);
void parseIndirtab(uintptr_t tableStartAddr, uint32_t numEntries); // does nothing
void printIndirtab(); // print the entire indirect table
void printSymEntry(uint32_t symtabIndex); // print the symbol at symtabIndex

BasicInfo basicInfo;
LoaderInfo loaderInfo;
SymbolsInfo symbolsInfo;

char* machfile;
size_t ptr;

};