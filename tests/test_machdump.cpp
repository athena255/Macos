
#include <iostream>
#include "../machdump.h"
using namespace std;

void runTest(int (*testFn)(), string testName ) {
    if (testFn()) {
         cout << GREEN "[PASSED] " << RESET;
    }else{
        cout << RED "[FAILED] " << RESET;
    }
    cout << testName << endl;
}

bool assertEqual(uint64_t got, uint64_t expect)
{
    if (expect == got) 
        return true;

    cout << RED "Expected " << expect 
        << " got " << got << RESET << endl;
    return false;
}
int testSectionInfo()
{
    MachFile testAout("testVectors/a.out");
    uint8_t bOk = 1;
    section_64* sec64 = reinterpret_cast<section_64*>(testAout.loaderInfo.nlSymbolPtr);
    bOk &= assertEqual(sec64->addr, 0x100016000);
    sec64 = reinterpret_cast<section_64*>(testAout.loaderInfo.laSymbolPtr);
    bOk &= assertEqual(sec64->addr, 0x100016020);
    sec64 = reinterpret_cast<section_64*>(testAout.loaderInfo.textPtr);
    bOk &= assertEqual(sec64->addr, 0x100000f40);
    sec64 = reinterpret_cast<section_64*>(testAout.loaderInfo.dataPtr);
    bOk &= assertEqual(sec64->addr, 0x1000161c8);
    segment_command_64* seg64 = reinterpret_cast<segment_command_64*>(testAout.loaderInfo.linkedItSegPtr);
    bOk &= assertEqual(seg64->vmaddr, 0x101364000);
    return bOk;
}

void testSymbolsInfo()
{
    MachFile testAout("testVectors/a.out");
    uint8_t bOk = 1;
    bOk &= assertEqual(testAout.symbolsInfo.numIndirEntries, 53);

}

int lazytest()
{
    MachFile testHeaplib("testVectors/a.out");
    return 0;
}

void testLib()
{
    MachFile testLib("/usr/lib/libSystem.B.dylib");
}


int main() {
    // runTest(testExecutable, "testExecutable");
     lazytest();
    // runTest(testSectionInfo, "testSectionInfo");
     testLib();
    // runTest(testEntry, "testEntry");
    // runTest(testBase, "TestBase");
}
