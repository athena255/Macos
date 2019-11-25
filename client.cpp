#include <stdlib.h>
#include "machdump.h"

int main(int argc, char* argv[])
{
  if (argc != 2) 
    return 1;
  MachFile m(argv[1]);
  return 0;
}