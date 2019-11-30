#include "machedit.h"
#include <fstream>
#include <vector>
#include <iostream>

MachEdit::MachEdit(const char* fileName):
machFile(new MachFile(fileName))
{
}

MachEdit::~MachEdit()
{
  delete machFile;
}

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

bool MachEdit::redefineEntry(const char* stub)
{

// Find original entrypoint
  uintptr_t og_offset = machFile->basicInfo.entrypointOffset;

// Find readable and executable section

// Edit header to change virtual size of the section

// Copy new code to the executable
            // 0x10000d524      6802000048     push 0x48000002
            // 0x10000d529      83c420         add esp, 0x20
            // 0x10000d52c      5d             pop rbp
            // 0x10000d52d      c3             ret

// Copy push ret instruction to go back to OEP
  entry_point_command* epc = reinterpret_cast<entry_point_command*>(machFile->loaderInfo.entryPointPtr);
  // lol how to jump?
  // push rip
  // jump rip - sizeof code? 

  // push ret opcodes
}