#include "FileSystemWindows.h"
#include "../../Communication/GlobalEvent.h"
#include "../../Strings/StringFunctions.h"


#ifdef AE_PLATFORM_WINDOWS

#include <windows.h>

namespace AE_NS_FOUNDATION
{
  aeDataDirectoryFileReaderWindows::aeDataDirectoryFileReaderWindows (aeDataDirectoryHandler* pHandler, const char* szFile) : aeDataDirectoryFileReader (pHandler, szFile) 
  {
    m_pFile = NULL;
  }

  bool aeDataDirectoryFileReaderWindows::Open (void)
  {
    AE_CHECK_DEV (m_pFile == NULL, "aeDataDirectoryFileReaderWindows::Open: File is already open.");

    m_pFile = fopen (GetFilePath ().c_str (), "rb");

    return (m_pFile != NULL);
  }

  aeUInt32 aeDataDirectoryFileReaderWindows::Read (void* pBuffer, aeUInt32 uiBytes)
  {
    AE_CHECK_DEV (m_pFile != NULL, "aeDataDirectoryFileReaderWindows::Read: File is not open.");

    return ((aeUInt32) fread (pBuffer, 1, uiBytes, m_pFile));
  }

  void aeDataDirectoryFileReaderWindows::CloseFile (void)
  {
    AE_CHECK_DEV (m_pFile != NULL, "aeDataDirectoryFileReaderWindows::CloseFile: File is not open.");

    fclose (m_pFile);
    m_pFile = NULL;
  }


  aeDataDirectoryFileWriterWindows::aeDataDirectoryFileWriterWindows (aeDataDirectoryHandler* pHandler, const char* szFile) : aeDataDirectoryFileWriter (pHandler, szFile) 
  {
    m_pFile = NULL;
  }

  bool aeDataDirectoryFileWriterWindows::Open (void)
  {
    AE_CHECK_DEV (m_pFile == NULL, "aeDataDirectoryFileWriterWindows::Open: File is already open.");

    m_pFile = fopen (GetFilePath ().c_str (), "wb");

    return (m_pFile != NULL);
  }

  void aeDataDirectoryFileWriterWindows::Write (const void* pBuffer, aeUInt32 uiBytes)
  {
    AE_CHECK_DEV (m_pFile != NULL, "aeDataDirectoryFileWriterWindows::Write: File is not open.");

    fwrite (pBuffer, 1, uiBytes, m_pFile);
  }

  void aeDataDirectoryFileWriterWindows::CloseFile (void)
  {
    AE_CHECK_DEV (m_pFile != NULL, "aeDataDirectoryFileWriterWindows::CloseFile: File is not open.");

    fclose (m_pFile);
    m_pFile = NULL;
  }

  aeDataDirectoryHandler_Windows::aeDataDirectoryHandler_Windows (const char* szDirectory) : aeDataDirectoryHandler (szDirectory) 
  {
  }

  aeDataDirectoryFileReader* aeDataDirectoryHandler_Windows::OpenFileToRead (const char* szFile)
  {
    const aeFilePath sPath = aePathFunctions::CombinePaths (GetDataDirectory (), szFile);

    aeDataDirectoryFileReaderWindows* pAccessor = new aeDataDirectoryFileReaderWindows (this, sPath.c_str ());
    if (!pAccessor->Open ())
    {
      delete pAccessor;
      return (NULL);
    }
    
    return (pAccessor);
  }

  aeDataDirectoryFileWriter* aeDataDirectoryHandler_Windows::OpenFileToWrite (const char* szFile)
  {
    const aeFilePath sPath = aePathFunctions::CombinePaths (GetDataDirectory (), szFile);

    if (!aeFileSystem::CreateFolders (sPath.c_str ()))
      return (NULL);

    aeDataDirectoryFileWriterWindows* pAccessor = new aeDataDirectoryFileWriterWindows (this, sPath.c_str ());
    if (!pAccessor->Open ())
    {
      delete pAccessor;
      return (NULL);
    }
    
    return (pAccessor);
  }

  void aeDataDirectoryHandler_Windows::Deletefile (const char* szFile)
  {
    const aeFilePath sPath = aePathFunctions::CombinePaths (GetDataDirectory (), szFile);
    const aeFilePath sOSPath = aePathFunctions::MakePathValidForOS (sPath);

    DeleteFile (sOSPath.c_str ());
  }

  bool aeDataDirectoryHandler_Windows::CreateFolder (const char* szDirectory, const char* szNewDirectory)
  {
    // if the NEW directory is something like "C:", return success
    if (szNewDirectory[1] == ':')
      return (true);
    // if the NEW directory is something like "%appdata%, return success
    if (szNewDirectory[0] == '%')
      return (true);

    aeFilePath sPath = aePathFunctions::CombinePaths (GetDataDirectory (), szDirectory);
    sPath = aePathFunctions::CombinePaths (sPath, szNewDirectory);

    if (CreateDirectory (sPath.c_str (), NULL) == FALSE)
      return (GetLastError () == ERROR_ALREADY_EXISTS);

    return (true);
  }



  bool AddDataDir_Default (const char* szPath, const char* szCategory, bool bAllowWrite)
  {
    aeFilePath sPath = szPath;

    //if ((!sPath.EndsWith ("/")) && (!sPath.EndsWith ("\\")))
//      sPath += "/";

    aeFileSystem::CreateFolders (sPath.c_str ());

    aeFileSystem::AddDataDirectoryHandler (new aeDataDirectoryHandler_Windows (szPath), szCategory, szPath, bAllowWrite);
    return (true);
  }

  //bool AddDataDir_Zip (const char* szPath, const char* szCategory)
  //{
  //  if (!aePathFunctions::HasExtension (szPath, "ZIP"))
  //    return (false);

  //  aeFileSystem::AddDataDirectoryHandler (new aeDataDirectoryHandler_ZLib (szPath), szCategory, szPath);

  //  aeFilePath sPath = aePathFunctions::SetFileExtension (szPath, "");

  //  aeFileSystem::AddDataDirectoryHandler (new aeDataDirectoryHandler_Windows (sPath.c_str ()), szCategory, szPath);
  //  return (true);
  //}

  AE_ON_GLOBAL_EVENT_ONCE (aeStartup_main_begin)
  {
    aeFileSystem::AddDataDirectoryHandlerProvider (AddDataDir_Default);
    //aeFileSystem::AddDataDirectoryHandlerProvider (AddDataDir_Zip);

    // handles all global paths
    aeFileSystem::AddDataDirectory ("");
  }
}

#endif



