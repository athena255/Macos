#pragma once

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>

#define RED     "\x1B[31m" // file offsets
#define GREEN   "\x1B[32m" // size
#define YELLOW  "\x1B[33m" 
#define BLUE    "\x1B[34m" // vm addresses
#define MAGENTA "\x1B[35m" // counts
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"
#define RESET   "\x1B[00m"

struct GenInfo {
  uintptr_t entryPoint;
  uint32_t fileOffset;
  uint32_t linkerInfo;
};

struct BasicInfo {
  uintptr_t imageBase;
  uint32_t sizeOfImage;
  uint32_t baseOfCode;
  uint32_t baseOfData;
  uint32_t fileAlignment;
  uint32_t magic;
  uint32_t numberOfLoadCommands;
  uint32_t timeDateStamp;
};

struct DirectoryInfo {
  uintptr_t exportTable;
  uintptr_t importTable;
  uintptr_t resource;
  uintptr_t tlvTable;
  uintptr_t debug;
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
  uintptr_t indirSymTable; // location of indirect symbol table in file
};

#define MAX_FILE_SIZE 0x20000

class MachFile{

public:
MachFile(const char* fileName);
~MachFile();

void printHeader();
void printLoadCommands();

void parseLC(size_t i);
void parseHeader();

void loadSegmentCommand64();
void loadDyldInfoCommand();
void loadSymtabCommand();
void loadDysymtabCommand();
void loadDylinkerCommand();
void loadUUIDCommand();
void loadBuildVersion();
void loadSourceVersion();
void loadEntryPointCommand();
void loadDylibCommand();
void loadLinkeditDataCommand();

// part of Dsymtab
void parseToC(uint32_t offset, uint32_t count);
void parseModuleTable(uint32_t offset, uint32_t count);
void parseRefTable(uint32_t offset, uint32_t count);
void parseSymbolTable(uintptr_t tableStart, uint32_t count);
void parseTwoLevel(uint16_t n_desc);

// symbol tables handling
void parseSection(section_64* sec64);
void parseSegment(segment_command_64* seg64);
void parseDysymTable(uintptr_t);
void parseIndirTable(uintptr_t tableStart, uint32_t count);

GenInfo genInfo;
BasicInfo basicInfo;
DirectoryInfo dirInfo;
LoaderInfo loaderInfo;
SymbolsInfo symbolsInfo;

const char* fileName;
mach_header_64* pMachHeader;
load_command** pLoadCommands;
char machfile[MAX_FILE_SIZE];
size_t ptr;

};