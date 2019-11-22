
#include <iostream>
#include "machdump.h"
using namespace std;

/* 
 * FUNCTIONS BEING TESTED: init & alloc
 * SPECIFICATION BEING TESTED:

 *
 * MANIFESTATION OF ERROR:

 */

void runTest(int (*testFn)(), string testName ) {
    if (testFn()) {
         cout << GREEN "[PASSED] " << RESET;
    }else{
        cout << RED "[FAILED] " << RESET;
    }
    cout << testName << endl;
}

bool assertEqual(uint32_t expect, uint32_t got)
{
    if (expect == got) 
        return true;

    cout << RED "Expected " << expect 
        << " got " << got << RESET << endl;
    return false;
}

int testGetMagic()
{
    MachFile testHeaplib("testVectors/test_heaplib");
    return assertEqual(testHeaplib.pMachHeader->magic, MH_MAGIC_64);
}

int testExecutable()
{
    MachFile testHeaplib("testVectors/test_heaplib");
    return assertEqual(testHeaplib.pMachHeader->filetype, MH_EXECUTE);
}

// int testBase()
// {
//         MachFile testHeaplib("testVectors/test_heaplib");
//         return assertEqual(testHeaplib.basicInfo.baseOfCode, 0x100000000);
// }

int lazytest()
{
    MachFile testHeaplib("testVectors/a.out");
    testHeaplib.printHeader();
    // testHeaplib.printLoadCommands();
    return 0;
}


int main() {
    // runTest(testGetMagic, "testGetMagic");
    // runTest(testExecutable, "testExecutable");
    lazytest();
    // runTest(testEntry, "testEntry");
    // runTest(testBase, "TestBase");
}
