#pragma once
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <vector>

/**
 * An interface that maps function pointers to load_commands, segments, or sections.
 * When execute is called, go through the mach file and execute the commands on each
 * element
 */

template <typename T>
class Functor
{
  private:
    T* me;
  public:
    explicit Functor(T*);
    
    T* operator()(size_t idx) const {
      
    }

  friend class MachFnMap;
};

class MachFnMap
{
  public:
  MachFnMap(const char* fileName);
  ~MachFnMap();

  /**
   * @brief iterates through the given file and executes the registered functions
   */ 
  void execute();

  void commit(const char* fileName);


  private:

  mach_header* pMachHeader;
  std::vector<load_command*> lcVec;

  // working copy of machfile
  char* filedata;
};
