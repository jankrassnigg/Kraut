#include "../PathFunctions.h"
#include "../String.h"



namespace AE_NS_FOUNDATION
{
  const char aePathFunctions::s_PossiblePathSeparators[s_uiMaxPossiblePathSeparators] = {'/', '\\'};

  aeFileExtension aePathFunctions::GetFileExtension (const aeBasicString& sPath)
  {
    aeUInt32 uiLen = sPath.length ();

    while (uiLen > 0)
    {
      // if we find a dot, this is the extension separator => return everything behind it
      if (sPath[uiLen - 1] == '.')
        return (aeFileExtension (&(sPath.c_str ()[uiLen])));

      // if we find a path separator before a dot, there is no extension
      if (IsPathSeparator (sPath[uiLen - 1]))
        return ("");

      --uiLen;
    }

    // did not find a dot, ran till the first character => no extension
    return ("");
  }

  /*! Removes all trailing dots to generate a 'clean' file name.
      If a crappy string is put in, a crappy string will come out. There is no guarantee that
      the returned path is a 'valid' file-path.
  */
  aeFilePath aePathFunctions::SetFileExtension (const aeBasicString& sPath, const char* szNewExtension)
  {
    aeFileExtension sExt = GetFileExtension (sPath);

    aeFilePath sResult = sPath;

    // remove all extension characters
    sResult.resize (sResult.length () - sExt.length ());

    // remove all trailing dots
    while (sResult.EndsWith ("."))
      sResult.resize (sResult.length () - 1);

    // skip all dots at the beginning of the extension
    while (aeStringFunctions::StartsWith (szNewExtension, "."))
      szNewExtension = &szNewExtension[1];

    // if one wants to remove any extension
    if (aeStringFunctions::IsNullOrEmpty (szNewExtension))
      return (sResult);

    sResult += ".";
    sResult += szNewExtension;

    return (sResult);
  }

  /*! Removes all trailing dots to generate a 'clean' file name.
      If a crappy string is put in, a crappy string will come out. There is no guarantee that
      the returned path is a 'valid' file-path.
  */
  aeFilePath aePathFunctions::ChangeExistingFileExtension (const aeBasicString& sPath, const char* szNewExtension)
  {
    aeFileExtension sExt = GetFileExtension (sPath);

    if (sExt == "")
      return (sPath);

    return (SetFileExtension (sPath, szNewExtension));
  }

  aeInt32 aePathFunctions::GetUsedExtension (const aeBasicString& sPath, aeUInt32 uiExtensions, const char* szExt1, const char* szExt2, ...)
  {
    aeFileExtension sExt = GetFileExtension (sPath);

    if (IsExtension (sExt, szExt1))
      return (0);
    if (IsExtension (sExt, szExt2))
      return (1);

    va_list ap;
    va_start (ap, szExt2);

    for (aeUInt32 ui = 2; ui < uiExtensions; ++ui)
    {
      const char* szExtN = va_arg (ap, const char*);

      if (IsExtension (sExt, szExtN))
        return (ui);
    }

    va_end (ap);

    return (-1);
  }

  aeFileName aePathFunctions::GetFileNameAndExtension (const aeBasicString& sPath)
  {
    aeUInt32 uiLen = sPath.length ();

    while (uiLen > 0)
    {
      // find the last path seperator
      if (IsPathSeparator (sPath[uiLen - 1]))
        break;

      --uiLen;
    }

    // copy everything after the path-separator into the new file name
    return (&(sPath.c_str ()[uiLen]));
  }

  aeFileName aePathFunctions::GetFileName (const aeBasicString& sPath)
  {
    aeFileName sName = GetFileNameAndExtension (sPath);

    return (SetFileExtension (sName, ""));
  }

  /*! Calling this function repeatedly on its result, will always yield the same string.
  */
  aeFilePath aePathFunctions::GetFileDirectory (const aeBasicString& sPath)
  {
    aeUInt32 uiLen = sPath.length ();

    while (uiLen > 0)
    {
      // find the last path seperator
      if (IsPathSeparator (sPath[uiLen - 1]))
      {
        aeFilePath sDir (sPath);
        sDir.resize (uiLen);
        return (sDir);
      }

      --uiLen;
    }

    return ("");
  }

  static void StripPathSeparators (aeFilePath& sPath)
  {
    bool bFoundAny = true;
    while (bFoundAny)
    {
      bFoundAny = false;

      for (aeUInt32 ui = 0; ui < aePathFunctions::s_uiMaxPossiblePathSeparators; ++ui)
      {
        char c2[2];
        c2[0] = aePathFunctions::s_PossiblePathSeparators[ui];
        c2[1] = '\0';

        if (sPath.EndsWith (c2))
        {
          sPath.resize (sPath.length () - 1);
          bFoundAny = true;
        }
      }
    }
  }

  static bool EndsWithFileUpString (const aeFilePath& sPath)
  {
    if (sPath.EndsWith (".."))
    {
      if (sPath.length () == 2)
        return (true);

      int i = sPath.length () - 3;
      char c = sPath[i];
      if (aePathFunctions::IsPathSeparator (c))
        return (true);
    }

    return (false);
  }

  aeFilePath aePathFunctions::GetParentDirectory (const aeBasicString& sPath, aeUInt8 uiLevelsUp)
  {
    aeFilePath sClean = MakeCleanPath (sPath);

    aeFilePath sResult;

    if (!EndsWithFileUpString (sClean))
      sResult = GetFileDirectory (sClean);
    else
      sResult = sPath;

    for (aeUInt32 ui = 0; ui < uiLevelsUp; ++ui)
    {
      StripPathSeparators (sResult);

      if (EndsWithFileUpString (sResult))
        uiLevelsUp += 2;

      if (sResult == "")
      {
        for (; ui < uiLevelsUp; ++ui)
          sResult += "../";

        return (sResult);
      }

      sResult = GetFileDirectory (sResult);
    }

    return (sResult);
  }

  static aeFileName GetNextPathPiece (const aeBasicString& sPath, aeUInt32& uiPos)
  {
    aeFileName sResult;

    while ((uiPos < sPath.length ()) && (!aePathFunctions::IsPathSeparator (sPath[uiPos])))
    {
      sResult += sPath[uiPos];
      ++uiPos;
    }

    // skip ALL path separators
    while ((uiPos < sPath.length ()) && (aePathFunctions::IsPathSeparator (sPath[uiPos])))
      ++uiPos;

    return (sResult);
  }

  aeFilePath aePathFunctions::MakeCleanPath (const aeBasicString& sPath)
  {
    if (sPath.empty ())
      return ("");
    if (sPath == "..")
      return ("../");

    aeFilePath sResult;
    aeUInt32 uiPos = 0;

    aeInt32 iLevelsDown = 0;

    while (uiPos < sPath.length ())
    {
      const aeFileName sPiece = GetNextPathPiece (sPath, uiPos);

      if (sPiece == "..")
      {
        if (iLevelsDown == 0)
          sResult += "../";
        else
        {
          --iLevelsDown;

          // remove the trailing /
          sResult.resize (sResult.length () - 1);

          // remove everything until the previous /
          while ((!sResult.empty ()) && (sResult[sResult.length () - 1] != '/'))
            sResult.resize (sResult.length () - 1);
        }
      }
      else
      {
        if (!sPiece.empty ())
          ++iLevelsDown;

        sResult += sPiece;
        sResult += '/';
      }
    }

    // if the original path ended with "/.." then keep the trailing / (just return immediately)
    if ((sPath.length () > 2) && (sPath.EndsWith ("..")) && (IsPathSeparator (sPath[sPath.length () - 3])))
      return (sResult);

    // if it did not end with a trailing / (ie its a filename), then remove it from the result also
    if ((!IsPathSeparator (sPath[sPath.length () - 1])) && (sResult.EndsWith ("/")))
      sResult.resize (sResult.length () - 1);

    // if it however ended with a path separator, also keep the trailing /
    return (sResult);
  }

  bool aePathFunctions::IsAbsolutePath (const char* szPath)
  {
    if (aeStringFunctions::IsNullOrEmpty (szPath))
      return (false);

    #ifdef AE_PLATFORM_WINDOWS
      return (szPath[1] == ':');
    #endif
    #ifdef AE_PLATFORM_LINUX
      return (szPath[0] == '/');
    #endif
  }

  aeFilePath aePathFunctions::CombinePaths (const aeBasicString& sDirectory1, const char* szFileOrDir2)
  {
    aeFilePath sResult = sDirectory1;

    // remove all path separators from the end of the first path
    while ((!sResult.empty ()) && (IsPathSeparator (sResult[sResult.length () - 1])))
      sResult.resize (sResult.length () - 1);

    // remove all path separators from the beginning of the second path
    while (IsPathSeparator (szFileOrDir2[0]))
      szFileOrDir2 = &szFileOrDir2[1];

    // if the second path is empty, return the first
    if (aeStringFunctions::IsNullOrEmpty (szFileOrDir2))
      return (sResult);

    // if the first path is empty, return the second
    if (sResult.empty ())
      return (szFileOrDir2);

    sResult += '/';
    sResult += szFileOrDir2;

    return (sResult);
  }

  aeFilePath aePathFunctions::ChangeFileName (const aeBasicString& sPath, const char* szNewFileName)
  {
    aeUInt32 uiLen = sPath.length ();
    aeInt32 iExtensionPos = uiLen;
    aeInt32 iNameStartPos = -1;

    while (uiLen > 0)
    {

      if ((iExtensionPos == sPath.length ()) && (sPath[uiLen - 1] == '.'))
        iExtensionPos = uiLen - 1;

      if (IsPathSeparator (sPath[uiLen - 1]))
        break;

      --uiLen;
    }

    aeFilePath sResult = sPath;
    sResult.ReplaceSubString (uiLen, iExtensionPos - uiLen, szNewFileName);

    return (sResult);	
  }

  aeFilePath aePathFunctions::ChangeFileNameAndExtension (const aeBasicString& sPath, const char* szNewFileNameWithExtension)
  {
    aeUInt32 uiLen = sPath.length ();
    aeInt32 iNameStartPos = -1;

    while (uiLen > 0)
    {
      if (IsPathSeparator (sPath[uiLen - 1]))
        break;

      --uiLen;
    }

    aeFilePath sResult = sPath;
    sResult.ReplaceSubString (uiLen, sPath.length () - uiLen, szNewFileNameWithExtension);

    return (sResult);
  }

  /*! The function just concatenates the two paths and cleans them up. That does not guarantee that the result
      is an absolute path, because if the combination of the two paths does not yield a valid absolute path,
      a relative path is returned (for example when the path goes more levels up, than there are folders).
      There is no error code for that, because it is a farely uncommon problem when modifying paths (crap in -> crap out).
      One could check, whether the output really is an absolute path, though.
  */
  aeFilePath aePathFunctions::MakeAbsolute (const aeBasicString& sAbsoluteBasePath, const char* szRelativePath)
  {
    aeFilePath sCombined = CombinePaths (sAbsoluteBasePath, szRelativePath);

    return (MakeCleanPath (sCombined));
  }

  aeFilePath aePathFunctions::MakeRelative (const aeBasicString& sAbsoluteBasePath, const aeBasicString& sAbsolutePathToFile)
  {
    aeFilePath sBasePath = MakeCleanPath (sAbsoluteBasePath);
    aeFilePath sFilePath = MakeCleanPath (sAbsolutePathToFile);

    if (!sBasePath.EndsWith ("/"))
      sBasePath += '/';

    const aeUInt32 uiMinLen = aeMath::Min (sBasePath.length (), sFilePath.length ());

    aeInt32 iSame = uiMinLen - 1;
    for (; iSame > 0; --iSame)
    {
      if (sBasePath[iSame] != '/')
        continue;

      if (sBasePath.CompareEqual_NoCase (sFilePath.c_str (), iSame))
        break;
    }

    aeFilePath sResult;
    for (aeUInt32 ui = iSame + 1; ui < sBasePath.length (); ++ui)
    {
      if (sBasePath[ui] == '/')
        sResult += "../";
    }

    if (sFilePath[iSame] == '/')
      ++iSame;

    sResult += &(sFilePath.c_str ()[iSame]);
    return (sResult);
  }

  /*! An empty folder (zero length) does not contain ANY files.\n
      A non-existing file-name (zero length) is never in any folder.\n
      Example:\n
      IsFileBelowFolder ("", "XYZ") -> always false\n
      IsFileBelowFolder ("XYZ", "") -> always false\n
      IsFileBelowFolder ("", "") -> always false\n
  */
  bool aePathFunctions::IsFileBelowFolder (const aeBasicString& sAbsoluteFolderPath, const aeBasicString& sAbsolutePathToFile)
  {
    // nothing is under the empty folder
    if (sAbsoluteFolderPath.empty ())
      return (false);
    // a non-existing file is never in any folder
    if (sAbsolutePathToFile.empty ())
      return (false);

    aeFilePath sBasePath = MakeCleanPath (sAbsoluteFolderPath);
    aeFilePath sFilePath = MakeCleanPath (sAbsolutePathToFile);

    if (!sBasePath.EndsWith ("/"))
      sBasePath += '/';

    return (sFilePath.CompareEqual_NoCase (sBasePath.c_str (), sBasePath.length ()));
  }

  aeFilePath aePathFunctions::MakePathValidForOS (const aeBasicString& sSomePath)
  {
    aeFilePath sResult = MakeCleanPath (sSomePath);

    #ifdef AE_PLATFORM_WINDOWS
      sResult.ReplaceAll ("/", "\\");
    #endif

    return (sResult);
  }
}



