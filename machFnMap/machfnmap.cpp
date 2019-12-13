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

    mach_header* pMachHeader = reinterpret_cast<mach_header*>(filedata);
    size_t ncmds = pMachHeader->ncmds;
}

void MachFnMap::bind(LC cmd, FN pFn, PARAM pParam)
{

}

void MachFnMap::execute()
{
    // Iterate through each lc command

    // execute the thing in fnMap
}

template <typename T>
Functor<T>::Functor(T* thing, FN fn) :
me(thing),
fn(fn)
{

}
