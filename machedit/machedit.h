#include "../machdump/machdump.h"

/**
 * This class is an abstraction for edits to machfiles
 */
class MachEdit
{
  public:
/**
 * @brief Creates a MachEdit instance
 * Requires that fileName points to a file that exists
 */
  MachEdit(const char* fileName);
  ~MachEdit();

/**
 * @brief Searches for the given byte signature in the file
 * @param signature Byte signature pattern
 * @param offset File offset to start the search (optional)
 * @return address of the first occurence of the signature, 0 if not found
 */
  uint8_t* searchSig(const char* signature, uint32_t offset = 0);

/**
 * @brief Writes data at fileOffset for strlen(data)
 * @param data Null terminated string of bytes i.e. "\xad\xde"
 * @param offset File offset at which to write the edit 
 */
bool writeFile(const char* data, uintptr_t fileOffset, size_t dataLen);


/**
 * @brief Commit changes and save them to fileName
 * @param newfileName Name of the new file
 */
  void commit(const char* newfileName);

/**
 * @brief Adds a jmp main (original entrypoint) opcode at fileOffset and redefines
 * the entrypoint load command to newEntry
 * @param fileOffset offset in the file to write the jmp main command
 * default is the area between end of load commands and start of __text
 * @param newEntry the offset in the file to the new entrypoint
 * default is fileOffset
 * Modifies headers and LC commands to start at this entrypoint
 */
  void redefineEntry(uint64_t fileOffset=0, uint64_t newEntry=0);

/**
 * @brief Adds a load command at the end of the file;
 */
  bool addLC(uintptr_t pLoadCmd, uint32_t cmdSize);

  void replaceOrdinal(uint32_t oldOrdinal, uint32_t newOrdinal, size_t dsymIndex);

  /**
   * @brief Adds a dylibPath to the list of headers
   * (does not check the end of headers)
   * Note: max path length is 255
   */
  void addDylib(const char* dylibPath);

  /**
   * @brief Extend the size of the load command at lcIndex by shifting
   * everyone after it down
   */
  void extendLC(size_t lcIndex, size_t amt);


/**
 * @brief Changes the file to cause it to execute exePathName and then
 * jump to original entrypoint
 */
  void embedBin(char const* exePathName);

  MachFile* machFile; 

};
