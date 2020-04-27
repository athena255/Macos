#include <fstream>
#include <vector>
#include <iostream>
#include <string>

#include "machedit.h"
#include "../includes/mach_common.h"

MachEdit::MachEdit(const char* fileName):
machFile(new MachFile(fileName))
{
}

MachEdit::~MachEdit()
{
  delete machFile;
}

// Slow search but we don't really use this function
uint8_t* MachEdit::searchSig(const char* signature, uint32_t offset)
{
		static auto pattern_to_bytes = [](const char* pattern) 
    {
			auto bytes = std::vector<int>{};
			char* start = const_cast<char*>(pattern);
			char* end = start + strlen(pattern);

			for (char* cur = start; cur < end; ++cur) 
      {
				if (*cur == '?') 
        {
          ++cur;
					bytes.push_back(-1);
				}
				else 
        {
				  bytes.push_back(strtoul(cur, &cur, 16)); // convert to bytes
				}
			}
			return bytes;
		};

		uint64_t fileLen = machFile->basicInfo.fileSize;
		auto sigBytes = pattern_to_bytes(signature);
		uint8_t* fileData = reinterpret_cast<std::uint8_t*>(machFile->machfile);
		uint64_t sigLen = sigBytes.size();

    uint64_t i = offset, j = 0;
    for (; i < fileLen - sigLen; ++i)
    {
      if (j >= sigLen)
        return &fileData[i];
      if (fileData[i+j] == sigBytes[j] || sigBytes[j] == -1)
      {
        ++j; 
        --i; // i should be in the same spot when checking signature
      }
      else
      {
        i -= j; // scroll all the way back
        j = 0;
      }
    }
    return nullptr;
}

bool MachEdit::writeFile(const char* data, uintptr_t fileOffset, size_t dataLen)
{
  size_t fileLen = machFile->basicInfo.fileSize; 
  // Do not write past the end of the file
  if (fileOffset + dataLen > fileLen)
    return false;
  memcpy(machFile->machfile + fileOffset, data, dataLen);
  return true;
}

void MachEdit::commit(const char* newfileName)
{
  std::ofstream newFile (newfileName, std::ios::out | std::ios::binary);
  newFile.write(machFile->machfile, machFile->basicInfo.fileSize);
  newFile.close();
}

void MachEdit::redefineEntry(uint64_t fileOffset, uint64_t newEntry)
{
  if (fileOffset == 0)
  {
    mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machFile->machfile);
    fileOffset = m64->sizeofcmds + sizeof(mach_header_64);
  }
  if (newEntry == 0)
    newEntry = fileOffset;

  int64_t jmpBackAmt = machFile->basicInfo.entrypointOffset - fileOffset;
  uintptr_t fileEditAddr = reinterpret_cast<uintptr_t>( machFile->machfile) + fileOffset;
 
  if (jmpBackAmt <= -0x7f || jmpBackAmt >= 0x82)
  { // write the long jmp opcode (5 bytes)
    *(uintptr_t*)(fileEditAddr) = 0xe9;
    *(uintptr_t*)(fileEditAddr + 1) = jmpBackAmt - 5;
  }
  else
  { // write the short jmp opcode (2 bytes)
    *(uintptr_t*)fileEditAddr = 0xeb;
    *reinterpret_cast<uint8_t*>(fileEditAddr + 1) = (uint8_t)(jmpBackAmt - 2);
  }
  
  entry_point_command* epc = reinterpret_cast<entry_point_command*>(machFile->loaderInfo.entryPointPtr);
  *(uintptr_t*)(&epc->entryoff) = newEntry;
}


bool MachEdit::addLC(uintptr_t pLoadCmd, uint32_t cmdSize)
{
  // Start at the end of the current commands
  writeFile(reinterpret_cast<char*>(pLoadCmd), machFile->ptr, cmdSize);
  // Edit header
  mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machFile->machfile);
  m64->ncmds++;
  m64->sizeofcmds += cmdSize;
  return false;
} 

void replaceOrdinalHelp(MachEdit* m, uint32_t oldOrdinal, uint32_t newOrdinal, uintptr_t startIndex, size_t numSyms)
{
  uint32_t symIndex;
  nlist_64* symEntry;
  uint16_t ordinal;
  for (int i = 0; i < numSyms; ++i)
  {
    symIndex = i + startIndex;
    symEntry = reinterpret_cast<nlist_64*>(m->machFile->symbolsInfo.symTablePtr + symIndex*sizeof(nlist_64));
    ordinal = GET_LIBRARY_ORDINAL(symEntry->n_desc);
    if (ordinal == oldOrdinal)
      SET_LIBRARY_ORDINAL(symEntry->n_desc, newOrdinal);
  }
}

void MachEdit::replaceOrdinal(uint32_t oldOrdinal, uint32_t newOrdinal, size_t dsymIndex)
{

  dysymtab_command* dc = reinterpret_cast<dysymtab_command*>(machFile->lcVec[dsymIndex]);
  replaceOrdinalHelp(this, oldOrdinal, newOrdinal, dc->iundefsym, dc->nundefsym);

}

void MachEdit::addDylib(const char* dylibPath)
{
  size_t pathLen = strnlen(dylibPath, 255);
  dylib_command dc;
  memset(&dc, 0, sizeof(dc));
  dc.cmd = LC_LOAD_DYLIB;
  dc.cmdsize = sizeof(dc) + pathLen + 8;
  dc.dylib.name.offset = sizeof(dc);
  // I don't think compatibility version matters...
  addLC(reinterpret_cast<uintptr_t>(&dc), dc.cmdsize);
  writeFile(dylibPath, machFile->ptr + sizeof(dc), pathLen+1);
  machFile->lcVec.push_back(machFile->ptr);
  machFile->ptr += dc.cmdsize;
}

void shiftSection64(uintptr_t lc, size_t amt){
  section_64* s = reinterpret_cast<section_64*>(lc);
  s->addr += amt;
  s->offset += amt;
}

void shiftSegment64(uintptr_t lc, size_t amt){
  segment_command_64* sc = reinterpret_cast<segment_command_64*>(lc);
  sc->fileoff += amt;
  for (size_t i = 0; i < sc->nsects; ++i){
    shiftSection64(
      lc + sizeof(segment_command_64) + i*sizeof(section_64),
      amt
    );
  }
}

void fixSegmentCommands(uintptr_t lc, size_t amt)
{
  switch(reinterpret_cast<load_command*>(lc)->cmd)
  {
    case LC_SEGMENT_64:
      shiftSegment64(lc, amt);
      break;
    default:
      break;
  }
}

uintptr_t shiftDown(uintptr_t lc, size_t amt)
{
  
  size_t cmdsize = reinterpret_cast<load_command*>(lc)->cmdsize;
  uint8_t* tmp = new uint8_t[cmdsize];
  memcpy(tmp, (void*)lc, cmdsize);
  // Copy tmp back to lc but at an offset
  memcpy((void*)(lc + amt), tmp, cmdsize);
  fixSegmentCommands(lc+amt, amt);
  return lc + amt;
}

void extendSegment64(segment_command_64* sc, size_t amt)
{
  sc->filesize += amt;
}


void MachEdit::extendLC(size_t lcIndex, size_t amt)
{
  if (lcIndex >= machFile->pMachHeader->ncmds)
    return;
  machFile->basicInfo.fileSize += amt;
  load_command* pFirstLC = reinterpret_cast<load_command*>(machFile->lcVec[lcIndex]);
  
  // adjust lc cmdsize
  pFirstLC->cmdsize += amt;

  if (pFirstLC->cmd == LC_SEGMENT_64)
    extendSegment64(reinterpret_cast<segment_command_64*>(pFirstLC), amt);
  
  // TODO: handle case where current file buffer is not large enough
  // iterate
  for (size_t i = machFile->pMachHeader->ncmds - 1; i > lcIndex; --i)
  {
    shiftDown(machFile->lcVec[i], amt);
    machFile->lcVec[i] += amt; // shift these pointers
  }
  // Fix headers
  machFile->pMachHeader->sizeofcmds += amt;

}

void MachEdit::embedBin(char const *exePathName)
{
  // Idea: Copy contents of file into the end of the file
  // Write shellcode to load that into a temporary file
}