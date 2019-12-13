#include "../machedit.h"
#include "includes/mach_common.h"
#include "includes/test_common.h"


int test_init()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  return 1; // we good if we don't abort trap or seg fault
}

int test_search_head_no_wild()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  const char* pattern = "cf fa ed fe";// cffa edfe 0700
  uint8_t* actual = reinterpret_cast<uint8_t*>(machEdit.machFile->machfile);
  uint8_t* got = machEdit.searchSig(pattern, 0);
  return assertEqual((uint64_t)got, (uint64_t)actual);
}

int test_search_head()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  const char* pattern = "cf ? ed fe";// cffa edfe 0700
  uint8_t* actual = reinterpret_cast<uint8_t*>(machEdit.machFile->machfile);
  uint8_t* got = machEdit.searchSig(pattern, 0);
  return assertEqual((uint64_t)got, (uint64_t)actual);
}

int test_search_sig()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
// 100000f40:      55      pushq   %rbp
// 100000f41:      48 89 e5        movq    %rsp, %rbp
// 100000f44:      48 83 ec 20     subq    $32, %rsp
// 100000f48:      31 c0   xorl    %eax, %eax
// 100000f4a:      89 c6   movl    %eax, %esi
// 100000f4c:      48 8d 7d f0     leaq    -16(%rbp), %rdi
// 100000f50:      e8 85 28 01 00  callq   75909 <wtclockdisk.c+0x1000137da>
// this would be __text section
  const char* pattern = "e5 ? ? ? 20 31 c0";
  section_64* textSeg = reinterpret_cast<section_64*>(machEdit.machFile->loaderInfo.textPtr);
  uint8_t* actual = reinterpret_cast<uint8_t*>(machEdit.machFile->machfile + textSeg->offset + 3);
  uint8_t* got = machEdit.searchSig(pattern, 0);
  return assertEqual((uint64_t)got, (uint64_t)actual);
}

int test_do_not_write_past_file()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  uint64_t fileLen = machEdit.machFile->basicInfo.fileSize;
  char oldData [0x10];
  memset(oldData, 0, 0x10);
  memcpy(oldData, machEdit.machFile->machfile+fileLen - 8, 8);
  machEdit.writeFile("blah blah", reinterpret_cast<uint8_t*>(machEdit.machFile->machfile) + fileLen - 4);
  int res = strncmp(oldData, machEdit.machFile->machfile+fileLen - 8, 8);
 return assertEqual(res, 0);
}

int test_edit_file()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  char oldData [0x10];
  memset(oldData, 0, 0x10);
  uint8_t offset = 69;
  // save old data
  memcpy(oldData, machEdit.machFile->machfile + offset, 0x10);

  // transform data
  char newData[0x10];
  for (uint8_t i = 0; i < 0x10; ++i)
    newData[i] = oldData[i] + 1;

  machEdit.writeFile(newData, reinterpret_cast<uint8_t*> (machEdit.machFile->machfile) + offset);
  int res = strncmp(oldData, machEdit.machFile->machfile + offset, 0x10);
 return res != 0;
}

int test_write_unchanged_to_file()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  const char* newFileName = "newfile";
  machEdit.commit(newFileName);
  return compareFiles(newFileName, testFileName); 
}

int test_write_to_file()
{
  const char* testFileName = "testVectors/a.out";
  MachEdit machEdit(testFileName);
  const char* newFileName = "changedFile";
  machEdit.writeFile("\ad\xde\ef\xbe", reinterpret_cast<uint8_t*>(machEdit.machFile->machfile) + 69);
  machEdit.commit(newFileName);
  return !compareFiles(newFileName, testFileName);
}

int test_redefine_entry()
{
  MachEdit machEdit("testVectors/a.out");
  mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machEdit.machFile->machfile);
  entry_point_command* epc = reinterpret_cast<entry_point_command*>(machEdit.machFile->loaderInfo.entryPointPtr);
  size_t og_entrypoint = epc->entryoff;
  uint64_t fileEditOffset = m64->sizeofcmds + sizeof(mach_header_64);
  machEdit.redefineEntry(fileEditOffset);
  bool bOk = 1;
  bOk &= assertEqual(epc->entryoff, fileEditOffset);
  bOk &= assertEqual( *reinterpret_cast<uint8_t*>(machEdit.machFile->machfile + fileEditOffset), 0xe9);
  bOk &= assertEqual( *reinterpret_cast<uint32_t*>(machEdit.machFile->machfile + fileEditOffset + 1), og_entrypoint - fileEditOffset - 5);
  machEdit.commit("test_redefine_entry.test");
  return bOk;
}

int test_redefine_entry_2()
{
  MachEdit machEdit("testVectors/test_heaplib");
  mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machEdit.machFile->machfile);
  entry_point_command* epc = reinterpret_cast<entry_point_command*>(machEdit.machFile->loaderInfo.entryPointPtr);
  size_t og_entrypoint = epc->entryoff;
  uint64_t fileEditOffset = m64->sizeofcmds + sizeof(mach_header_64) - 8;
  machEdit.redefineEntry(0);
  bool bOk = 1;
  bOk &= assertEqual(epc->entryoff, fileEditOffset);
  bOk &= assertEqual( *reinterpret_cast<uint8_t*>(machEdit.machFile->machfile + fileEditOffset), 0xe9); 
  bOk &= assertEqual( *reinterpret_cast<uint32_t*>(machEdit.machFile->machfile + fileEditOffset + 1), og_entrypoint - fileEditOffset - 5);
  machEdit.commit("test_redefine_entry_2.test");
  return bOk; 
}

int test_redefine_entry_hello()
{
  MachEdit machEdit("testVectors/hello");
  entry_point_command* epc = reinterpret_cast<entry_point_command*>(machEdit.machFile->loaderInfo.entryPointPtr);
  size_t og_entrypoint = epc->entryoff;
  mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machEdit.machFile->machfile);
  size_t offset = m64->sizeofcmds + sizeof(mach_header_64) - 8;
  machEdit.redefineEntry();
  bool bOk = 1;
  bOk &= assertEqual(epc->entryoff, offset);
  bOk &= assertEqual( *reinterpret_cast<uint8_t*>(machEdit.machFile->machfile + offset), 0xe9); 
  bOk &= assertEqual( *reinterpret_cast<uintptr_t*>(machEdit.machFile->machfile + offset + 1), og_entrypoint - offset - 5);
  machEdit.commit("test_redefine_entry_hello.test");
  return bOk; 
}

int test_add_section()
{
  const char* testFileName = "testVectors/hello";
  MachEdit machEdit(testFileName);
  linkedit_data_command led;
  led.dataoff = machEdit.machFile->basicInfo.fileSize;
  led.datasize = 0x22;
  led.cmdsize = sizeof(linkedit_data_command);
  led.cmd = LC_DATA_IN_CODE;
  machEdit.addLC(reinterpret_cast<uintptr_t>(&led), led.cmdsize);
  machEdit.commit("test_add_section.test");
  bool bOk = true;
  return bOk;
}

int test_add_seg_cmd()
{
  const char* testFileName = "/bin/ls";
  MachEdit machEdit(testFileName);
  mach_header_64* m64 = reinterpret_cast<mach_header_64*>(machEdit.machFile->machfile);
  // segment_command_64 seg64;
  // memset(&seg64, 0, sizeof(segment_command_64));
  // seg64.cmd = LC_SEGMENT_64;
  // memcpy(seg64.segname, "__TEXT", 6);
  // seg64.cmdsize = sizeof(segment_command_64);
  // seg64.vmaddr = 0x100002260;
  // seg64.vmsize = 0x100;
  // seg64.fileoff = 8800;
  // seg64.filesize = 0x100;
  // seg64.initprot = VM_PROT_EXECUTE | VM_PROT_READ;
  // seg64.maxprot = VM_PROT_EXECUTE | VM_PROT_READ;
  // seg64.nsects = 0;
  // seg64.flags = 0;
  // machEdit.addLC(reinterpret_cast<uintptr_t>(&seg64), seg64.cmdsize);
  machEdit.redefineEntry();
  machEdit.commit("test_add_seg_cmd.test");
  bool bOk = true;
  return bOk;
}

int test_add_dylib()
{
  MachEdit machEdit("/bin/ls");

  return 0;
}

int main()
{
#ifdef IGNORE
  runTest(test_init, "test_init");
  runTest(test_search_head_no_wild, "search for magic no wild cards");
  runTest(test_search_head, "search for magic");
  runTest(test_search_sig, "test_search_sig");
  runTest(test_do_not_write_past_file, "test do not write past end of file");
  runTest(test_edit_file, "test edit file");
  runTest(test_write_unchanged_to_file, "test write unchanged file to new file");
  runTest(test_write_to_file, "write to file");
  runTest(test_redefine_entry, "redefine entrypoint");
  runTest(test_redefine_entry_2, "redefine entry with test_heaplib");
  runTest(test_redefine_entry_hello, "redefine entry with hello world");
  runTest(test_add_section, "add a load command");
#endif
  runTest(test_add_seg_cmd, "add a segment command 64");
  runTest(test_add_dylib, "test add dylib");
  return 0;
}