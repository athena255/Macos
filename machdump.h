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
void parseSymbolTable(uint32_t, uint32_t);

GenInfo genInfo;
BasicInfo basicInfo;
DirectoryInfo dirInfo;

const char* fileName;
mach_header_64* pMachHeader;
load_command** pLoadCommands;
char temp[10000];
size_t ptr;

};