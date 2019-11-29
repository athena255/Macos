#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <mach/mach_time.h> // struct mach_timebase_info
#include <mach/mach_init.h> // struct mach_thread_self
#include <mach/shared_region.h>
#include <mach-o/loader.h> 
#include <mach-o/nlist.h> 
#include <stdint.h>
#include <stdlib.h>
#include <TargetConditionals.h>
#include <vector>
#include <new>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <mach-o/fat.h> 
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <libkern/OSAtomic.h>
#include <string.h>

const char* fPath = "lol/";

const char* getShortName()
{
	// try to return leaf name
	if ( fPath != NULL ) {
		const char* s = strrchr(fPath, '/');
		if ( s != NULL ) 
			return &s[1]; // is this null if our string is "blah/"
	}
	return fPath; 
}

int main()
{
  const char* res = getShortName();
  if (res == NULL)
  {
    printf("problem\n");
  }
  else
  {
    printf("%s", res);
  }
  
  return 0;
}