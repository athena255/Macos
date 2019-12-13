#pragma once
#define RED     "\x1B[31m" // file offsets
#define GREEN   "\x1B[32m" // size
#define YELLOW  "\x1B[33m"  // index
#define BLUE    "\x1B[34m" // vm addresses
#define MAGENTA "\x1B[35m" // counts
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"
#define RESET   "\x1B[00m"

#ifdef DEBUG_EN
#define DEBUG(x) do { std::cerr << x; } while (0)
#else
#define DEBUG(x)
#endif
#include <mach/vm_prot.h>
#include <iostream>
/**
 * Parses a xxxx.yy.zz into a X.Y.Z
 */
inline uint32_t unparseVersion(const char* strVersion)
{
  int vArr [3];
  memset(vArr, 0, 3);
  sscanf(strVersion, "%d.%d.%d", vArr, vArr + 1, vArr + 2);
  uint32_t version = 0;
  version |= ((vArr[0] & 0xFFFF) << 16);
  version |= ((vArr[1] & 0xFF) << 8);
  version |= (vArr[2] & 0xFF);
  return version;
}

/* prints X.Y.Z which is encoded in nibbles xxxx.yy.zz */
inline void parseVersions(uint32_t version)
{
    std::cout << std::dec
    << ((version & 0xFFFF0000) >> 16) << "." 
    << ((version & 0x0000FF00) >> 8) << "." 
    << (version & 0xFF);
}
