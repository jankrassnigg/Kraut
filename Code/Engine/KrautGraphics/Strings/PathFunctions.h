#ifndef AE_FOUNDATION_STRINGS_PATHFUNCTIONS_H
#define AE_FOUNDATION_STRINGS_PATHFUNCTIONS_H

#include <KrautFoundation/Strings/String.h>
#include <KrautGraphics/Strings/HybridString.h>

namespace AE_NS_FOUNDATION
{
  //! Static class that implements functions for handling file paths.
  struct AE_GRAPHICS_DLL aePathFunctions
  {
    //! The path separator of choice.
    static const char s_PathSeparator = '/';

    //! The number of known characters that act as path separators on different OSes.
    static const aeUInt8 s_uiMaxPossiblePathSeparators = 2;
    //! The different characters that act as path separators on different OSes (/ and \)
    static const char s_PossiblePathSeparators[s_uiMaxPossiblePathSeparators]; // = {'\\', '/'};

    //! Returns whether c is any known path separator.
    static bool IsPathSeparator(char c);

    //! Returns whether the given path has any extension.
    static bool HasAnyExtension(const aeBasicString& sPath);
    //! Returns whether the given path has exactly the given extension. Comparison is case-insensitive.
    static bool HasExtension(const aeBasicString& sPath, const char* szExtension);
    //! Returns the file extension of a path.
    static aeString GetFileExtension(const aeBasicString& sPath);
    //! Returns the index which given extension is used in the file-name. -1 if none of them. The empty extension ("") is allowed to query.
    static aeInt32 GetUsedExtension(const aeBasicString& sPath, aeUInt32 uiExtensions, const char* szExt1, const char* szExt2, ...);
    //! Appends or changes an extension. Makes sure there is only one dot.
    static aeString SetFileExtension(const aeBasicString& sPath, const char* szNewExtension);
    //! Changes the file extension, IF IT EXISTS. Otherwise does not change the path at all.
    static aeString ChangeExistingFileExtension(const aeBasicString& sPath, const char* szNewExtension);

    //! Returns the file-name of a path. The extension is stripped away. The path may not end with a separator, then the file-name will be empty.
    static aeString GetFileName(const aeBasicString& sPath);
    //! Returns the file-name of a path including the extension. The path may not end with a separator, then the file-name will be empty.
    static aeString GetFileNameAndExtension(const aeBasicString& sPath);

    //! Returns the directory of a file/path. If the given path already ends with a path separator, it returns the same string.
    static aeString GetFileDirectory(const aeBasicString& sPath);
    //! Will return the parent directory of a path.
    static aeString GetParentDirectory(const aeBasicString& sPath, aeUInt8 uiLevelsUp = 1);

    //! Combines to paths / filenames by making sure there is exactly ONE path-separator in between. Does not clean up the rest of the path.
    static aeString CombinePaths(const aeBasicString& sDirectory1, const char* szFileOrDir2);
    //! Removes double path-separators (ie. folder//folder -> folder/folder), removes "../" where possible, replaces all path separators with /
    static aeString MakeCleanPath(const aeBasicString& sPath);

    //! Returns whether a path is an absolute path. OS dependent.
    static bool IsAbsolutePath(const char* szPath);
    //! Returns whether a path is a relative path. Opposite of IsAbsolutePath.
    static bool IsRelativePath(const char* szPath);

    //! Changes the name of the file in the path. Does not change the file's extension.
    static aeString ChangeFileName(const aeBasicString& sPath, const char* szNewFileName);
    //! Changes the name of the file in the path, including its extension.
    static aeString ChangeFileNameAndExtension(const aeBasicString& sPath, const char* szNewFileNameWithExtension);

    //! Returns an absolute path that is the combination of the absolute path and the relative path.
    static aeString MakeAbsolute(const aeBasicString& sAbsoluteBasePath, const char* szRelativePath);
    //! Returns the relative path from sAbsoluteBasePath to sAbsolutePathToFile. Does NOT handle the case that those files are on different drives.
    static aeString MakeRelative(const aeBasicString& sAbsoluteBasePath, const aeBasicString& sAbsolutePathToFile);
    //! Returns whether the file is located somewhere inside the folder.
    static bool IsFileBelowFolder(const aeBasicString& sAbsoluteFolderPath, const aeBasicString& sAbsolutePathToFile);

    //! Cleans up the path and replaces all path-separators with the OS specific ones. Does NOT handle driver letters (ie. "C:\folder" vs "/folder")
    static aeString MakePathValidForOS(const aeBasicString& sSomePath);
  };
} // namespace AE_NS_FOUNDATION

#include "Inline/PathFunctions.inl"

#endif
