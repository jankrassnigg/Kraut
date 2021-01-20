#ifndef AE_FOUNDATION_FILESYSTEM_IMPLEMENTATIONS_FILESYSTEMWINDOWS_H
#define AE_FOUNDATION_FILESYSTEM_IMPLEMENTATIONS_FILESYSTEMWINDOWS_H

#include "../FileSystem.h"

//#ifdef AE_PLATFORM_WINDOWS

namespace AE_NS_FOUNDATION
{
  class AE_GRAPHICS_DLL aeDataDirectoryFileReaderWindows : public aeDataDirectoryFileReader
  {
  public:
    aeDataDirectoryFileReaderWindows(aeDataDirectoryHandler* pHandler, const char* szFile);

    virtual bool Open(void);
    virtual aeUInt32 Read(void* pBuffer, aeUInt32 uiBytes);

  protected:
    virtual void CloseFile(void);

  private:
    FILE* m_pFile;
  };

  class AE_GRAPHICS_DLL aeDataDirectoryFileWriterWindows : public aeDataDirectoryFileWriter
  {
  public:
    aeDataDirectoryFileWriterWindows(aeDataDirectoryHandler* pHandler, const char* szFile);

    virtual bool Open(void);
    virtual void Write(const void* pBuffer, aeUInt32 uiBytes);

  protected:
    virtual void CloseFile(void);

  private:
    FILE* m_pFile;
  };


  class AE_GRAPHICS_DLL aeDataDirectoryHandler_Windows : public aeDataDirectoryHandler
  {
  public:
    aeDataDirectoryHandler_Windows(const char* szDirectory);

    //! Deletes the given file, if possible.
    virtual void Deletefile(const char* szFile);

    //virtual aeUInt32 GetFileModificationTime (const char* szFile) = 0;
    //virtual void EnumFilesInDirectory (const char* szDirectory, bool bRecursive, CALLBACK, void* pPassThrough = nullptr);

    virtual aeDataDirectoryFileReader* OpenFileToRead(const char* szFile);
    virtual aeDataDirectoryFileWriter* OpenFileToWrite(const char* szFile);

  private:
    //! Creates the given directory. Returns false if unsuccesful.
    virtual bool CreateFolder(const char* szDirectory, const char* szNewDirectory);
  };

} // namespace AE_NS_FOUNDATION

//#endif

#endif
