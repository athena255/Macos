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
  void writeFile(const char* data, uint8_t* fileOffset);

/**
 * @brief Commit changes and save them to fileName
 * @param newfileName Name of the new file
 */
  void commit(const char* newfileName);
  MachFile* machFile; 

};

/**
 * 
 * Redefine the entrypoint of a mach-o executable.
 * Then jump back to the original entrypoint
 * 
 */


