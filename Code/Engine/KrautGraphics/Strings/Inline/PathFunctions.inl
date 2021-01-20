#ifndef AE_FOUNDATION_STRINGS_PATHFUNCTIONS_INL
#define AE_FOUNDATION_STRINGS_PATHFUNCTIONS_INL

namespace AE_NS_FOUNDATION
{
  inline bool aePathFunctions::IsPathSeparator (char c)
  {
    for (aeUInt32 ui = 0; ui < s_uiMaxPossiblePathSeparators; ++ui)
    {
      if (c == s_PossiblePathSeparators[ui])
        return (true);
    }

    return (false);
  }

  inline bool aePathFunctions::HasAnyExtension (const aeBasicString& sPath)
  {
    return (GetFileExtension (sPath) != "");
  }

  inline bool IsExtension (const aeString& sExt, const char* szExtension)
  {
    while (aeStringFunctions::StartsWith (szExtension, "."))
      szExtension = &szExtension[1];

    return (sExt.CompareEqual_NoCase (szExtension));
  }

  /*! If the path is not well formed or the extension isn't a valid extension (only dots etc.),
      the result might be unexpected.
  */
  inline bool aePathFunctions::HasExtension (const aeBasicString& sPath, const char* szExtension)
  {
    AE_CHECK_DEV (!aeStringFunctions::IsNullOrEmpty (szExtension), "aePathFunctions::HasExtension: The queried extension may not be empty.");

    return (IsExtension (GetFileExtension (sPath), szExtension));
  }

  inline bool aePathFunctions::IsRelativePath (const char* szPath)
  {
    return (!IsAbsolutePath (szPath));
  }
}

#endif


