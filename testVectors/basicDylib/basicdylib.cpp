#include <stdio.h>
__attribute__((constructor))
static void init(void)
{
  printf("[%s] initializer()\n", __FILE__);
}

__attribute__((destructor))
static void detach(void)
{
  printf("[%s] finalizer()\n", __FILE__);
}