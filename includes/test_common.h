
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>
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

    cout << dec << RED "Expected " << expect 
        << " got " << got << RESET << endl;
    return false;
}

// https://stackoverflow.com/questions/6163611/compare-two-files
bool compareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
  std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);

  if (f1.fail() || f2.fail()) {
    return false; //file problem
  }

  if (f1.tellg() != f2.tellg()) {
    return false; //size mismatch
  }

  //seek back to beginning and use std::equal to compare contents
  f1.seekg(0, std::ifstream::beg);
  f2.seekg(0, std::ifstream::beg);
  return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()));
}