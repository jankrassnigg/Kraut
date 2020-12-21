#ifndef AE_FOUNDATION_FILESYSTEM_FILEOUT_H
#define AE_FOUNDATION_FILESYSTEM_FILEOUT_H

#include "../FileSystem/FileSystem.h"
#include "../Streams/Streams.h"

namespace AE_NS_FOUNDATION
{
  //! A general class to read from a file. Uses an internal cache to speed up small reads.
  class AE_FOUNDATION_DLL aeFileOut : public aeStreamOut
  {
  public:
    //! Constructor, does nothing.
    aeFileOut (void);
    //! Destructor, closes the file, if it is still open (RAII).
    ~aeFileOut ();

    //! Opens the given file for writing. Returns true if successful. A cache is created to speed up small writes.
    bool Open (const char* szFile, aeUInt32 uiCacheSize = 1024 * 64);
    //! Closes the file, if it is open.
    void Close (void);

    //! Will write anything that's currently in the write-cache to disk. Will decrease performance if used excessively.
    void FlushWriteCache (void);

    const aeFilePath& GetFilePath (void) const { return m_pWriter->GetFilePath (); }
    const aeFilePath GetFilePathInDataDir (void) const;

  private:
    //! Writes the given number of bytes to the stream.
    virtual void WriteToStream (const void* pData, aeUInt32 uiSize);

    aeDataDirectoryFileWriter* m_pWriter;

    aeUInt32 m_uiCacheSize;
    aeUInt32 m_uiCacheWritePosition;

    aeUInt8* m_pCache;
  };

}

#endif
