#include "FileSystemWindows.h"
#include <KrautFoundation/Strings/StringFunctions.h>
#include <KrautGraphics/Communication/GlobalEvent.h>

//#ifdef AE_PLATFORM_WINDOWS

#include <windows.h>

namespace AE_NS_FOUNDATION
{
  aeDataDirectoryFileReaderWindows::aeDataDirectoryFileReaderWindows(aeDataDirectoryHandler* pHandler, const char* szFile)
    : aeDataDirectoryFileReader(pHandler, szFile)
  {
    m_pFile = nullptr;
  }

  bool aeDataDirectoryFileReaderWindows::Open(void)
  {
    AE_CHECK_DEV(m_pFile == nullptr, "aeDataDirectoryFileReaderWindows::Open: File is already open.");

    m_pFile = fopen(GetFilePath().c_str(), "rb");

    return (m_pFile != nullptr);
  }

  aeUInt32 aeDataDirectoryFileReaderWindows::Read(void* pBuffer, aeUInt32 uiBytes)
  {
    AE_CHECK_DEV(m_pFile != nullptr, "aeDataDirectoryFileReaderWindows::Read: File is not open.");

    return ((aeUInt32)fread(pBuffer, 1, uiBytes, m_pFile));
  }

  void aeDataDirectoryFileReaderWindows::CloseFile(void)
  {
    AE_CHECK_DEV(m_pFile != nullptr, "aeDataDirectoryFileReaderWindows::CloseFile: File is not open.");

    fclose(m_pFile);
    m_pFile = nullptr;
  }


  aeDataDirectoryFileWriterWindows::aeDataDirectoryFileWriterWindows(aeDataDirectoryHandler* pHandler, const char* szFile)
    : aeDataDirectoryFileWriter(pHandler, szFile)
  {
    m_pFile = nullptr;
  }

  bool aeDataDirectoryFileWriterWindows::Open(void)
  {
    AE_CHECK_DEV(m_pFile == nullptr, "aeDataDirectoryFileWriterWindows::Open: File is already open.");

    m_pFile = fopen(GetFilePath().c_str(), "wb");

    return (m_pFile != nullptr);
  }

  void aeDataDirectoryFileWriterWindows::Write(const void* pBuffer, aeUInt32 uiBytes)
  {
    AE_CHECK_DEV(m_pFile != nullptr, "aeDataDirectoryFileWriterWindows::Write: File is not open.");

    fwrite(pBuffer, 1, uiBytes, m_pFile);
  }

  void aeDataDirectoryFileWriterWindows::CloseFile(void)
  {
    AE_CHECK_DEV(m_pFile != nullptr, "aeDataDirectoryFileWriterWindows::CloseFile: File is not open.");

    fclose(m_pFile);
    m_pFile = nullptr;
  }

  aeDataDirectoryHandler_Windows::aeDataDirectoryHandler_Windows(const char* szDirectory)
    : aeDataDirectoryHandler(szDirectory)
  {
  }

  aeDataDirectoryFileReader* aeDataDirectoryHandler_Windows::OpenFileToRead(const char* szFile)
  {
    const aeString sPath = aePathFunctions::CombinePaths(GetDataDirectory(), szFile);

    aeDataDirectoryFileReaderWindows* pAccessor = new aeDataDirectoryFileReaderWindows(this, sPath.c_str());
    if (!pAccessor->Open())
    {
      delete pAccessor;
      return (nullptr);
    }

    return (pAccessor);
  }

  aeDataDirectoryFileWriter* aeDataDirectoryHandler_Windows::OpenFileToWrite(const char* szFile)
  {
    const aeString sPath = aePathFunctions::CombinePaths(GetDataDirectory(), szFile);

    if (!aeFileSystem::CreateFolders(sPath.c_str()))
      return (nullptr);

    aeDataDirectoryFileWriterWindows* pAccessor = new aeDataDirectoryFileWriterWindows(this, sPath.c_str());
    if (!pAccessor->Open())
    {
      delete pAccessor;
      return (nullptr);
    }

    return (pAccessor);
  }

  void aeDataDirectoryHandler_Windows::Deletefile(const char* szFile)
  {
    const aeString sPath = aePathFunctions::CombinePaths(GetDataDirectory(), szFile);
    const aeString sOSPath = aePathFunctions::MakePathValidForOS(sPath);

    DeleteFile(sOSPath.c_str());
  }

  bool aeDataDirectoryHandler_Windows::CreateFolder(const char* szDirectory, const char* szNewDirectory)
  {
    // if the NEW directory is something like "C:", return success
    if (szNewDirectory[1] == ':')
      return (true);
    // if the NEW directory is something like "%appdata%, return success
    if (szNewDirectory[0] == '%')
      return (true);

    aeString sPath = aePathFunctions::CombinePaths(GetDataDirectory(), szDirectory);
    sPath = aePathFunctions::CombinePaths(sPath, szNewDirectory);

    if (CreateDirectory(sPath.c_str(), nullptr) == FALSE)
      return (GetLastError() == ERROR_ALREADY_EXISTS);

    return (true);
  }



  bool AddDataDir_Default(const char* szPath, const char* szCategory, bool bAllowWrite)
  {
    aeString sPath = szPath;

    //if ((!sPath.EndsWith ("/")) && (!sPath.EndsWith ("\\")))
    //      sPath += "/";

    aeFileSystem::CreateFolders(sPath.c_str());

    aeFileSystem::AddDataDirectoryHandler(new aeDataDirectoryHandler_Windows(szPath), szCategory, szPath, bAllowWrite);
    return (true);
  }

  //bool AddDataDir_Zip (const char* szPath, const char* szCategory)
  //{
  //  if (!aePathFunctions::HasExtension (szPath, "ZIP"))
  //    return (false);

  //  aeFileSystem::AddDataDirectoryHandler (new aeDataDirectoryHandler_ZLib (szPath), szCategory, szPath);

  //  aeString sPath = aePathFunctions::SetFileExtension (szPath, "");

  //  aeFileSystem::AddDataDirectoryHandler (new aeDataDirectoryHandler_Windows (sPath.c_str ()), szCategory, szPath);
  //  return (true);
  //}

  AE_ON_GLOBAL_EVENT_ONCE(aeStartup_main_begin)
  {
    aeFileSystem::AddDataDirectoryHandlerProvider(AddDataDir_Default);
    //aeFileSystem::AddDataDirectoryHandlerProvider (AddDataDir_Zip);

    // handles all global paths
    aeFileSystem::AddDataDirectory("");
  }
} // namespace AE_NS_FOUNDATION

//#endif
