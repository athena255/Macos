#pragma once
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <map>

/**
 * An interface that maps function pointers to load_commands, segments, or sections.
 * When execute is called, go through the mach file and execute the commands on each
 * element
 */

using PARAM = void*;
using FN = void (*)(void*, void*);
using LC = uint32_t;

template <typename T>
class Functor
{
  private:
    T* me;
    FN fn;
  public:
    Functor(T* thing, FN fn)
    T* operator()(T* thing) const {
      fn(thing);
    }

};

class MachFnMap
{
  public:
  MachFnMap(const char* fileName);
  ~MachFnMap();

/**
 * @brief registers the specified function and parameter with the load command
 * specified by cmd
 * @param fn function
 * @param param optional paramter to the function
 * @param cmd LC_COMMAND identifier
 */
  void bind(LC, FN, PARAM);

  /**
   * @brief iterates through the given file and executes the registered functions
   */ 
  void execute();

  void commit(const char* fileName);


  private:

  // map a LC_COMMAND : <function, param>
  std::map<LC, FN> fnMap;

  // working copy of machfile
  char* filedata;
};
