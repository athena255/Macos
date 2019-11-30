
#include <iostream>
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