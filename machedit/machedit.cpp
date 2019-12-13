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

void MachEdit::writeFile(const char* data, uint8_t* fileOffset)
{
  uint64_t fileLen = machFile->basicInfo.fileSize; 
  uint64_t dataLen = strlen(data);
  // Do not write past the end of the file
  if (fileOffset + dataLen > reinterpret_cast<uint8_t*>(machFile->machfile) + fileLen)
    return;
  
  memcpy(fileOffset, data, strlen(data));
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
    fileOffset = m64->sizeofcmds + sizeof(mach_header_64) - 8;
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
  memcpy(machFile->machfile + machFile->ptr, reinterpret_cast<char*>(pLoadCmd), cmdSize);

  // Edit header
  mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machFile->machfile);
  m64->ncmds++;
  m64->sizeofcmds += cmdSize;
  return false;
} 