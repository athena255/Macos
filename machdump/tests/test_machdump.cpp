#include "../machdump.h"
#include "includes/mach_common.h"
#include "includes/test_common.h"

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

int testSymbolsInfo()
{
    MachFile testAout("testVectors/a.out");
    uint8_t bOk = 1;
    bOk &= assertEqual(testAout.symbolsInfo.numIndirEntries, 53);
    return bOk;
}

int testBasicInfo()
{
    MachFile testAout("testVectors/a.out");
    uint8_t bOk = 1;
    bOk &= assertEqual(testAout.basicInfo.magic,  0xfeedfacf);
    bOk &= assertEqual(testAout.basicInfo.numberOfLoadCommands, 15);
    bOk &= assertEqual(testAout.basicInfo.entrypointOffset, 0x7100);
    return bOk;
}

int lazytest()
{
    MachFile testHeaplib("testVectors/a.out");
    return 1;
}

void testLib()
{
    MachFile testLib("/usr/lib/libSystem.B.dylib");
}


int main() {
    lazytest();
    runTest(testBasicInfo, "testSymbolsInfo");
    runTest(testSectionInfo, "testSectionInfo");
    runTest(testSymbolsInfo, "testSymbolsInfo");
    runTest(lazytest, "testVectors/a.out");
    testLib(); // should complain
}
