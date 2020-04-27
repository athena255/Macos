#include "../machfnmap.h"
#include "includes/mach_common.h"
#include "includes/test_common.h"


int test_init()
{
    MachFnMap("testVectors/hello");
    return 1;
}

int main()
{
    runTest(test_init, "test_init");
}