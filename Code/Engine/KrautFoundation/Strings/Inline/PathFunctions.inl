#ifndef AE_FOUNDATION_STRINGS_PATHFUNCTIONS_INL
#define AE_FOUNDATION_STRINGS_PATHFUNCTIONS_INL

namespace AE_NS_FOUNDATION
{
  AE_INLINE bool aePathFunctions::IsPathSeparator (char c)
  {
    for (aeUInt32 ui = 0; ui < s_uiMaxPossiblePathSeparators; ++ui)
    {
      if (c == s_PossiblePathSeparators[ui])
        return (true);
    }

    return (false);
  }

  AE_INLINE bool aePathFunctions::HasAnyExtension (const aeBasicString& sPath)
  {
    return (GetFileExtension (sPath) != "");
  }

  AE_INLINE bool IsExtension (const aeFileExtension& sExt, const char* szExtension)
  {
    while (aeStringFunctions::StartsWith (szExtension, "."))
      szExtension = &szExtension[1];

    return (sExt.CompareEqual_NoCase (szExtension));
  }

  /*! If the path is not well formed or the extension isn't a valid extension (only dots etc.),
      the result might be unexpected.
  */
  AE_INLINE bool aePathFunctions::HasExtension (const aeBasicString& sPath, const char* szExtension)
  {
    AE_CHECK_DEV (!aeStringFunctions::IsNullOrEmpty (szExtension), "aePathFunctions::HasExtension: The queried extension may not be empty.");

    return (IsExtension (GetFileExtension (sPath), szExtension));
  }

  AE_INLINE bool aePathFunctions::IsRelativePath (const char* szPath)
  {
    return (!IsAbsolutePath (szPath));
  }
}

#endif


