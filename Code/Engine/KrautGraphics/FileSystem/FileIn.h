#ifndef AE_FOUNDATION_FILESYSTEM_FILEIN_H
#define AE_FOUNDATION_FILESYSTEM_FILEIN_H

#include <KrautGraphics/FileSystem/FileSystem.h>
#include <KrautFoundation/Streams/Streams.h>
#include <KrautGraphics/Base.h>

namespace AE_NS_FOUNDATION
{
  //! A general class to read from a file. Uses an internal cache to speed up small reads.
  class AE_GRAPHICS_DLL aeFileIn : public aeStreamIn
  {
  public:
    //! Constructor, does nothing.
    aeFileIn(void);
    //! Destructor, closes the file, if it is still open (RAII).
    ~aeFileIn();

    //! Opens the given file for reading. Returns true if successful. A cache is created to speed up small reads.
    bool Open(const char* szFile, aeUInt32 uiCacheSize = 1024 * 64);
    //! Closes the file, if it is open.
    void Close(void);

    const aeString& GetFilePath(void) const { return m_pReader->GetFilePath(); }
    const aeString GetFilePathInDataDir(void) const;

  private:
    //! Attempts to read the given number of bytes into the buffer. Returns the actual number of bytes read.
    virtual aeUInt32 ReadFromStream(void* pData, aeUInt32 uiSize);

    aeDataDirectoryFileReader* m_pReader;

    aeUInt32 m_uiCacheSize;
    aeUInt32 m_uiBytesCached;
    aeUInt32 m_uiCacheReadPosition;

    aeUInt8* m_pCache;
  };

} // namespace AE_NS_FOUNDATION

#endif
