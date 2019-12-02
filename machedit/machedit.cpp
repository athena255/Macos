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

// Convert number to little endian
// Requires that result is size len+1
void convertToLE(char* result, uint32_t len, uint64_t number)
{
  memset(result, 0, len + 1);
  for (int i = 0; i < 4; ++i)
  {
    result[i] = ((number >> sizeof(uintptr_t)*i) & 0xFF);
  }
}

bool MachEdit::redefineEntry(const char* stub)
{
  segment_command_64* textSeg = reinterpret_cast<segment_command_64*>(machFile->loaderInfo.textSegPtr);
  section_64* textSec = reinterpret_cast<section_64*>(machFile->loaderInfo.textPtr);
  uint64_t sizeAvailable = textSeg->vmsize - textSec->size;
  if (sizeAvailable < 5) // 5 because 32 bit relative jmp instruction is 5 bytes
    return false;

  // Write to the first line after the textSection
  uint32_t amtAlign = (1<< textSec->align) - (textSec->size % (1 << textSec->align));

  // Offset to start of __text + text size + offset to next alignment
  uint64_t fileEditOffset = textSec->offset + textSec->size + amtAlign;

  // Relative jmp to negative of this amount
  int64_t jmpBackAmt = -(fileEditOffset - machFile->basicInfo.entrypointOffset);
  DEBUG("jmpBackAmt " << std::hex << -jmpBackAmt << std::endl);
  char opcodes [5];
  if (jmpBackAmt <= -0x7f) // use the e9 opcode
  {
    // Write the jmp opcode at fileEditOffset
    uintptr_t fileEditAddr = reinterpret_cast<uintptr_t>(machFile->machfile) + fileEditOffset;
    *(uintptr_t*)(fileEditAddr) = 0xe9;
    
    // Write the relative offset of the jmp: (1<<32) + jmpBackAmt - 5
    *(uintptr_t*) (fileEditAddr + 1) = jmpBackAmt - 5;
  }
  else // use the eb opcode
  {
    printf("not yet implemented");
    return false;
  }
  DEBUG("oldsize " << textSec->size << std::endl);
  // Write the new size in section64 of __text
  //*(uintptr_t*) ( &textSec->size) = textSec->size + 5;
  DEBUG("newsize " << textSec->size << std::endl);
  entry_point_command* epc = reinterpret_cast<entry_point_command*>(machFile->loaderInfo.entryPointPtr);
  // Update entry point segment to point to our opcodes
  *(uintptr_t*)(&epc->entryoff) = fileEditOffset;

  return true;
}