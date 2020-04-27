#include <iostream>
#include <fstream>
#include "machfnmap.h"


MachFnMap::MachFnMap(const char* fileName)
{
    std::ifstream fs (fileName, std::ios::in | std::ios::binary);
    fs.seekg(0, std::ios_base::end);
    size_t filesize = fs.tellg();
    fs.clear();
    fs.seekg(0, std::ios::beg);

    filedata = new char[filesize];
    fs.read(filedata, filesize);
    if (fs.fail())
        exit(1);
    fs.close();

    pMachHeader = reinterpret_cast<mach_header*>(filedata);
    size_t ncmds = pMachHeader->ncmds;

    load_command* pLoadCommand;
    uintptr_t offset = sizeof(mach_header_64);
    for (size_t i = 0; i < ncmds; ++i)
    {
        pLoadCommand = reinterpret_cast<load_command*>(filedata+offset);
        lcVec.push_back(pLoadCommand);
    }
}

MachFnMap::~MachFnMap()
{

}

// Shift all the load commands down [amt]
void shift_down(load_command* lc, size_t amt)
{
    size_t cmdSize = lc->cmdsize;
    uint8_t* tmp = new uint8_t[cmdSize];
    memcpy(tmp, (void*)lc, cmdSize);
    memcpy(reinterpret_cast<uint8_t*>(lc)+amt, tmp, cmdSize);
    delete [] tmp;
}


void MachFnMap::execute()
{
    // Iterate through each lc command

    // execute the thing in fnMap
}

template <typename T>
Functor<T>::Functor(T* thing) :
me(thing)
{
}
