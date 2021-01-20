#include "../FileSystem.h"
#include "../FileIn.h"
#include "../FileOut.h"


namespace AE_NS_FOUNDATION
{
  aeArray<aeFileSystem::aeDataDir> aeFileSystem::m_DataDirectories;
  aeMap<aeString, AE_FILE_TYPE_READER_HOOK> aeFileSystem::m_FileTypeHooks;
  aeArray<AE_DATA_DIRECTORY_HANDLER_PROVIDER> aeFileSystem::m_DataDirHandlerProviders;
  aeEvent aeFileSystem::m_FileEvent;
  //aeArray<aeFileSystem::aePathToken> aeFileSystem::s_PathTokens;

  aeFileSystem::aeFileEvent::aeFileEvent ()
  {
    m_EventType = aeFileEventType::None;
    m_szFileOrDirectory = "";
    m_szOther = "";
    m_pDataDirHandler = nullptr;
  }

  void aeDataDirectoryFileReader::Close (void)
  {
    CloseFile ();

    aeFileSystem::aeFileEvent fe;
    fe.m_EventType = aeFileEventType::CloseFile;
    fe.m_szFileOrDirectory = GetFilePath ().c_str ();
    fe.m_pDataDirHandler = m_pDirectoryHandler;
    aeFileSystem::m_FileEvent.RaiseEvent (&fe);

    delete this;
  }

  void aeDataDirectoryFileWriter::Close (void)
  {
    CloseFile ();

    aeFileSystem::aeFileEvent fe;
    fe.m_EventType = aeFileEventType::CloseFile;
    fe.m_szFileOrDirectory = GetFilePath ().c_str ();
    fe.m_pDataDirHandler = m_pDirectoryHandler;
    aeFileSystem::m_FileEvent.RaiseEvent (&fe);

    delete this;
  }

  void aeFileSystem::RemoveDataDirectory (const char* szPath)
  {
    aeFileEvent fe;
    fe.m_EventType = aeFileEventType::RemoveDataDirectory;
    fe.m_szOther = szPath;

    aeString sPath (szPath);

restart:

    for (aeUInt32 ui = 0; ui < m_DataDirectories.size (); ++ui)
    {
      if (m_DataDirectories[ui].m_sDataDirectoryGroup == sPath)
      {
        fe.m_szFileOrDirectory = m_DataDirectories[ui].m_pHandler->GetDataDirectory ().c_str ();
        fe.m_pDataDirHandler = m_DataDirectories[ui].m_pHandler;
        m_FileEvent.RaiseEvent (&fe);

        for (aeInt32 ui2 = ui; ui2 < (aeInt32) (m_DataDirectories.size ()) - 1; ++ui2)
          m_DataDirectories[ui2] = m_DataDirectories[ui2 + 1];

        m_DataDirectories.pop_back ();
        goto restart;
      }
    }
  }

  void aeFileSystem::AddDataDirectory (const char* szPath, const char* szDirCategory, bool bAllowWrite)
  {
    aeFileEvent fe;
    fe.m_szFileOrDirectory = szPath;
    fe.m_szOther = szDirCategory;

    for (aeInt32 i = m_DataDirHandlerProviders.size () - 1; i >= 0; --i)
    {
      if (m_DataDirHandlerProviders[i] (szPath, szDirCategory, bAllowWrite))
      {
        fe.m_EventType = aeFileEventType::AddDataDirectory;
        m_FileEvent.RaiseEvent (&fe);
        return;
      }
    }

    fe.m_EventType = aeFileEventType::AddDataDirectoryFailed;
    m_FileEvent.RaiseEvent (&fe);

    AE_CHECK_DEV (false, "aeFileSystem::AddDataDirectory: No Handler-Provider succeeded to add DataDir \"%s\".", szPath);
  }

  void aeFileSystem::AddDataDirectoryHandlerProvider (AE_DATA_DIRECTORY_HANDLER_PROVIDER Provider)
  {
    m_DataDirHandlerProviders.push_back (Provider);
  }

  void aeFileSystem::AddDataDirectoryHandler (aeDataDirectoryHandler* pDirectoryHandler, const char* szDirCategory, const char* szDataDirectoryGroup, bool bAllowWrite)
  {
    aeDataDir dd;
    dd.m_pHandler = pDirectoryHandler;
    dd.m_sCategory = szDirCategory;
    dd.m_sDataDirectoryGroup = szDataDirectoryGroup;
    dd.m_bAllowWrite = bAllowWrite;

    m_DataDirectories.push_back (dd);


    aeFileEvent fe;
    fe.m_EventType = aeFileEventType::AddDataDirectoryHandler;
    fe.m_szFileOrDirectory = pDirectoryHandler->GetDataDirectory ().c_str ();
    fe.m_szOther = szDirCategory;
    fe.m_pDataDirHandler = pDirectoryHandler;
    m_FileEvent.RaiseEvent (&fe);
  }


  //static bool Action_OpenFileToWrite (const char* szFile, void* pPassThrough)
  //{
  //	aeDataDirectoryFileWriter** ppResult = (aeDataDirectoryFileWriter**) pPassThrough;

  //	*ppResult = aeFileSystem::OpenFileToWrite (szFile, true);

  //	return (*ppResult != nullptr);
  //}

  aeDataDirectoryFileWriter* aeFileSystem::OpenFileToWrite (const char* szFile/*, bool bReplacingTokens*/)
  {
    const aeString sFile = aePathFunctions::MakeCleanPath (szFile);
    szFile = sFile.c_str ();

    aeFileEvent fe;
    fe.m_szFileOrDirectory = szFile;

    //aeDataDirectoryFileWriter* pResult = nullptr;
    //aeDataDirectoryFileWriter* pResult = aeFileSystem::OpenFileToWrite (szFile, true);
    //if (ExecuteActionOnPath (szFile, Action_OpenFileToWrite, &pResult))
    //{
    //	if (pResult == nullptr)
    //	{
    //		fe.m_EventType = aeFileEventType::CreateFileFailed;
    //		m_FileEvent.RaiseEvent (&fe);
    //	}

    //	return (pResult);
    //}


    aeArray<aeHybridString<16> > SearchDirs;
    aeUInt32 uiStartPos = ExtractDataDirsToSearch (szFile, SearchDirs);
    szFile = &szFile[uiStartPos];

    aeDataDirectoryFileWriter* pWriter;

    for (aeUInt32 uiSearchDir = 0; uiSearchDir < SearchDirs.size (); ++uiSearchDir)
    {
      for (aeInt32 i = m_DataDirectories.size () - 1; i >= 0; --i)
      {
        if (!m_DataDirectories[i].m_bAllowWrite)
          continue;

        if (m_DataDirectories[i].m_sCategory != SearchDirs[uiSearchDir])
          continue;

        aeString sDataDir = m_DataDirectories[i].m_pHandler->GetDataDirectory ();

        if (sDataDir.CompareEqual_NoCase (szFile, sDataDir.length ()))
          pWriter = m_DataDirectories[i].m_pHandler->OpenFileToWrite (&szFile[sDataDir.length ()]);
        else
          pWriter = m_DataDirectories[i].m_pHandler->OpenFileToWrite (szFile);

        if (pWriter != nullptr)
        {
          fe.m_EventType = aeFileEventType::CreateFile;
          fe.m_pDataDirHandler = m_DataDirectories[i].m_pHandler;
          fe.m_szOther = SearchDirs[uiSearchDir].c_str ();
          fe.m_szFileOrDirectory = pWriter->GetFilePath ().c_str ();
          m_FileEvent.RaiseEvent (&fe);
          return (pWriter);
        }
      }
    }

    //if (!bReplacingTokens)
    {
      fe.m_EventType = aeFileEventType::CreateFileFailed;
      m_FileEvent.RaiseEvent (&fe);
    }

    return (nullptr);
  }


  //static bool Action_OpenFileToRead (const char* szFile, void* pPassThrough)
  //{
  //	aeDataDirectoryFileReader** ppResult = (aeDataDirectoryFileReader**) pPassThrough;

  //	*ppResult = aeFileSystem::OpenFileToRead (szFile, false, true);

  //	return (*ppResult != nullptr);
  //}

  aeDataDirectoryFileReader* aeFileSystem::OpenFileToRead (const char* szFile, bool bAllowReaderHooks/*, bool bReplacingTokens*/)
  {
    const aeString sFile = aePathFunctions::MakeCleanPath (szFile);
    szFile = sFile.c_str ();

    aeFileEvent fe;
    fe.m_szFileOrDirectory = szFile;


    // if hooks are allowed, check if there is one for this file type
    if (bAllowReaderHooks)
    {
      // check file extension
      aeString sExt = aePathFunctions::GetFileExtension (szFile);
      sExt.ToUpperCase ();

      // check if there is a hook for it
      aeMap<aeString, AE_FILE_TYPE_READER_HOOK>::iterator it = m_FileTypeHooks.find (sExt);

      // if so, return its result, ignore anything else
      if (it != m_FileTypeHooks.end ())
      {
        fe.m_EventType = aeFileEventType::OpenFileHook;
        fe.m_szOther = it.key ().c_str ();
        m_FileEvent.RaiseEvent (&fe);

        return (it.value () (szFile));
      }
    }

    //aeDataDirectoryFileReader* pResult = nullptr;
    //if (ExecuteActionOnPath (szFile, Action_OpenFileToRead, &pResult))
    //{
    //	if (pResult == nullptr)
    //	{
    //		fe.m_EventType = aeFileEventType::OpenFileFailed;
    //		m_FileEvent.RaiseEvent (&fe);
    //	}

    //	return (pResult);
    //}


    aeArray<aeHybridString<16> > SearchDirs;
    aeUInt32 uiStartPos = ExtractDataDirsToSearch (szFile, SearchDirs);
    szFile = &szFile[uiStartPos];
    

    aeDataDirectoryFileReader* pReader;

    for (aeUInt32 uiSearchDir = 0; uiSearchDir < SearchDirs.size (); ++uiSearchDir)
    {
      for (aeInt32 i = m_DataDirectories.size () - 1; i >= 0; --i)
      {
        if (m_DataDirectories[i].m_sCategory != SearchDirs[uiSearchDir])
          continue;

        aeString sDataDir = m_DataDirectories[i].m_pHandler->GetDataDirectory ();

        if (sDataDir.CompareEqual_NoCase (szFile, sDataDir.length ()))
          pReader = m_DataDirectories[i].m_pHandler->OpenFileToRead (&szFile[sDataDir.length ()]);
        else
          pReader = m_DataDirectories[i].m_pHandler->OpenFileToRead (szFile);

        if (pReader != nullptr)
        {
          const aeString sPath = pReader->GetFilePath ();

          fe.m_EventType = aeFileEventType::OpenFile;
          fe.m_szFileOrDirectory = sPath.c_str ();
          fe.m_szOther = SearchDirs[uiSearchDir].c_str ();
          fe.m_pDataDirHandler = m_DataDirectories[i].m_pHandler;
          m_FileEvent.RaiseEvent (&fe);

          return (pReader);
        }
      }
    }

    //if (!bReplacingTokens)
    {
      fe.m_EventType = aeFileEventType::OpenFileFailed;
      m_FileEvent.RaiseEvent (&fe);
    }

    return (nullptr);
  }



  aeUInt32 aeFileSystem::ExtractDataDirsToSearch (const aeBasicString& sPath, aeArray<aeHybridString<16> >& SearchDirs)
  {
    SearchDirs.clear ();

    if (!sPath.StartsWith ("<"))
    {
      SearchDirs.push_back ("");
      return (0);
    }

    aeHybridString<16> sCur;

    aeUInt32 uiPos = 1;
    while ((uiPos < sPath.length ()) && (sPath[uiPos] != '>'))
    {
      if (sPath[uiPos] == '|')
      {
        SearchDirs.push_back (sCur);
        sCur.clear ();
      }
      else
        sCur += sPath[uiPos];

      ++uiPos;
    }

    AE_CHECK_DEV (uiPos < sPath.length (), "aeFileSystem::ExtractDataDirsToSearch: Cannot parse the path \"%s\". The data-dir declaration does start with a { but does not end with an }.", sPath.c_str ());

    SearchDirs.push_back (sCur);

    return (uiPos + 1);
  }

  void aeFileSystem::SetFileTypeReaderHook (const char* szFileExtension, AE_FILE_TYPE_READER_HOOK Hook)
  {
    aeString sExt = szFileExtension;
    sExt.ToUpperCase ();
    if (sExt.StartsWith ("."))
      sExt = &(sExt.c_str ()[1]);

    if (Hook == nullptr)
      m_FileTypeHooks.erase (sExt);
    else
      m_FileTypeHooks[sExt] = Hook;
  }

  //void aeFileSystem::AddPathToken (const char* szToken, const char* szReplacement)
  //{
  //	aePathToken pt;
  //	pt.m_sPathToken = szToken;
  //	pt.m_sReplacement = szReplacement;

  //	pt.m_sPathToken.ToUpperCase ();

  //	AE_CHECK_DEV (pt.m_sPathToken == szToken, "aeFileSystem::AddPathToken: Path Tokens must be all upper-case (\"%s\" instead of \"%s\").", pt.m_sPathToken.c_str (), szToken);

  //	s_PathTokens.push_back (pt);
  //}

  static bool Action_DeleteFile (const char* szPath, void* pPassThrough)
  {
    aeFileSystem::Deletefile (szPath);
    return (false);
  }

  void aeFileSystem::Deletefile (const char* szFile)
  {
    //if (ExecuteActionOnPath (szFile, Action_DeleteFile, nullptr))
      //return;


    aeArray<aeHybridString<16> > SearchDirs;
    aeUInt32 uiStartPos = ExtractDataDirsToSearch (szFile, SearchDirs);
    szFile = &szFile[uiStartPos];

    for (aeUInt32 uiSearchDir = 0; uiSearchDir < SearchDirs.size (); ++uiSearchDir)
    {
      for (aeInt32 i = m_DataDirectories.size () - 1; i >= 0; --i)
      {
        if (m_DataDirectories[i].m_sCategory != SearchDirs[uiSearchDir])
          continue;

        m_DataDirectories[i].m_pHandler->Deletefile (szFile);
      }
    }
  }

  bool aeFileSystem::ExistsFile (const char* szFile)
  {
    aeDataDirectoryFileReader* pReader = OpenFileToRead (szFile, true);

    if (pReader != nullptr)
    {
      pReader->Close ();
      return (true);
    }

    return (false);
  }

  //bool aeFileSystem::ExecuteActionOnPath (const char* szPathWithTokens, AE_PATH_ACTION Action, void* pPassThrough)
  //{
  //	aeString sFilePath = szPathWithTokens;
  //	const aeInt32 iTokenPos = sFilePath.FindFirstStringPos ("$");
  //	if (iTokenPos == -1)
  //		return (false);

  //	const aeInt32 iTokenPosEnd = sFilePath.FindFirstStringPos ("$", iTokenPos + 1);

  //	AE_CHECK_DEV (iTokenPosEnd != -1, "Invalid Path \"%s\".", sFilePath.c_str ());
  //	if (iTokenPosEnd == -1)
  //		return (false);

  //	aeString sToken = &(sFilePath.c_str ()[iTokenPos + 1]);
  //	sToken.resize (iTokenPosEnd - iTokenPos - 1);

  //	aeString sNewPath;

  //	for (aeInt32 ui = s_PathTokens.size () - 1; ui >= 0; --ui)
  //	{
  //		if (s_PathTokens[ui].m_sPathToken != sToken)
  //			continue;

  //		sNewPath = sFilePath;
  //		sNewPath.ReplaceSubString (iTokenPos, sToken.length () + 2, s_PathTokens[ui].m_sReplacement.c_str ());

  //		if (Action (sNewPath.c_str (), pPassThrough))
  //			return (true);
  //	}

  //	return (true);
  //}

  /*! If the path ends with a / (or \) the path is considered to be a folder-path and will be created entirely.
      If it does not end with a path separator, if is considered to be a file-path and the last piece of the path
      will not be created as a directory.
  */
  bool aeDataDirectoryHandler::CreateFolderStructure (const char* szPathToFileOrFolder)
  {
    const aeString sFolderStructure = aePathFunctions::MakeCleanPath (szPathToFileOrFolder);
    
    aeString sPath;
    aeString sSubFolder;

    aeUInt32 iPos = 0;

    while (iPos < sFolderStructure.length ())
    {
      if (sFolderStructure[iPos] == '/')
      {
        if (!CreateFolder (sPath.c_str (), sSubFolder.c_str ()))
          return (false);

        sPath = aePathFunctions::CombinePaths (sPath, sSubFolder.c_str ());
        sSubFolder.clear ();
      }
      else
        sSubFolder += sFolderStructure[iPos];

      ++iPos;
    }

    return (true);
  }

  //static bool Action_CreateFolderStructure (const char* szPath, void* pPassThrough)
  //{
  //	return (aeFileSystem::CreateFolders (szPath));
  //}

  bool aeFileSystem::CreateFolders (const char* szFile)
  {
    //if (ExecuteActionOnPath (szFile, Action_CreateFolderStructure, nullptr))
      //return (true);

    aeArray<aeHybridString<16> > SearchDirs;
    aeUInt32 uiStartPos = ExtractDataDirsToSearch (szFile, SearchDirs);
    szFile = &szFile[uiStartPos];

    for (aeUInt32 uiSearchDir = 0; uiSearchDir < SearchDirs.size (); ++uiSearchDir)
    {
      for (aeInt32 i = m_DataDirectories.size () - 1; i >= 0; --i)
      {
        if (m_DataDirectories[i].m_sCategory != SearchDirs[uiSearchDir])
          continue;

        if (m_DataDirectories[i].m_pHandler->CreateFolderStructure (szFile))
          return (true);
      }
    }

    return (false);
  }

  bool aeFileSystem::MakeValidPath (const char* szPath, bool bForWriting, aeString* out_sAbsolutePath, aeString* out_sDataDirRelativePath)
  {
    aeString sPath = aePathFunctions::MakeCleanPath (szPath);
    bool bRet = false;
    bool bWrite = bForWriting;

    if (!bWrite)
    {
      aeFileIn FileIn;
      if (FileIn.Open (sPath.c_str ()))
      {
        if (out_sAbsolutePath)
          *out_sAbsolutePath = FileIn.GetFilePath ();
        if (out_sDataDirRelativePath)
          *out_sDataDirRelativePath = FileIn.GetFilePathInDataDir ();

        bRet = true;
      }
      else
        bWrite = true;
    }

    if (bWrite)
    {
      const aeString sFileNameAndExt = aePathFunctions::GetFileNameAndExtension (sPath);

      aeFileOut FileOut;
      sPath = aePathFunctions::ChangeFileNameAndExtension (sPath, "__dummy__.tmp");

      if (FileOut.Open (sPath.c_str ()))
      {
        if (out_sAbsolutePath)
          *out_sAbsolutePath = aePathFunctions::ChangeFileNameAndExtension (FileOut.GetFilePath (), sFileNameAndExt.c_str ());
        if (out_sDataDirRelativePath)
          *out_sDataDirRelativePath = aePathFunctions::ChangeFileNameAndExtension (FileOut.GetFilePathInDataDir (), sFileNameAndExt.c_str ());

        bRet = true;
      }

      FileOut.Close ();

      aeFileSystem::Deletefile (sPath.c_str ());
    }

    if (!bRet && !aeStringFunctions::IsNullOrEmpty (szPath))
      MakeValidPath ("", bForWriting, out_sAbsolutePath, out_sDataDirRelativePath);

    return bRet;
  }
}




