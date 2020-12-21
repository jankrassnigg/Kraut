#ifndef AE_FOUNDATION_STRINGS_STRINGFUNCTIONS_INL
#define AE_FOUNDATION_STRINGS_STRINGFUNCTIONS_INL

#include "../Defines.h"
#include "../Strings/LocaleFixer.h"
#include "../Math/Math.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

namespace AE_NS_FOUNDATION
{
  // Macro to Handle NULL-pointer strings
  #define AE_STRINGCOMPARE_HANDLE_NULL_PTRS(szString1, szString2, ret_true, ret_false)\
    if (szString1 == szString2)/* Handles the case that both are NULL and that both are actually the same string */ \
      return (ret_true);\
    if (szString1 == NULL)\
      if (szString2[0] == '\0') /* if String1 is NULL, String2 is never NULL, otherwise the previous IF would have returned already */ \
        return (ret_true);\
      else\
        return (ret_false);\
    if (szString2 == NULL) \
      if (szString1[0] == '\0') /* if String2 is NULL, String1 is never NULL, otherwise the previous IF would have returned already */ \
        return (ret_true);\
      else\
        return (ret_false);

  AE_INLINE bool aeStringFunctions::IsNullOrEmpty (const char* szString)
  {
    return ((szString == NULL) || (szString[0] == '\0'));
  }

  AE_INLINE aeUInt32 aeStringFunctions::GetLength (const char* szString)
  {
    // Handle NULL-pointer strings and empty strings (which should be faster than just calling strlen)
    if (IsNullOrEmpty (szString))
      return (0);

    return ((aeUInt32) strlen (szString));
  }

  AE_INLINE bool aeStringFunctions::CompareEqual (const char* szString1, const char* szString2)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, true, false);

    return (strcmp (szString1, szString2) == 0);
  }

  AE_INLINE bool aeStringFunctions::CompareEqual (const char* szString1, const char* szString2, aeUInt32 uiCharsToCompare)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, true, false);

    return (strncmp (szString1, szString2, uiCharsToCompare) == 0);
  }

  AE_INLINE bool aeStringFunctions::CompareEqual_NoCase (const char* szString1, const char* szString2)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, true, false);

    return (_stricmp (szString1, szString2) == 0);
  }

  AE_INLINE bool aeStringFunctions::CompareEqual_NoCase (const char* szString1, const char* szString2, aeUInt32 uiCharsToCompare)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, true, false);

    return (_strnicmp (szString1, szString2, uiCharsToCompare) == 0);
  }


  AE_INLINE aeInt32 aeStringFunctions::CompareAlphabetically (const char* szString1, const char* szString2)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, 0, 1);

    return (strcmp (szString1, szString2));
  }

  AE_INLINE aeInt32 aeStringFunctions::CompareAlphabetically (const char* szString1, const char* szString2, aeUInt32 uiCharsToCompare)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, 0, 1);

    return (strncmp (szString1, szString2, uiCharsToCompare));
  }

  AE_INLINE aeInt32 aeStringFunctions::CompareAlphabetically_NoCase (const char* szString1, const char* szString2)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, 0, 1);

    return (_stricmp (szString1, szString2));
  }

  AE_INLINE aeInt32 aeStringFunctions::CompareAlphabetically_NoCase (const char* szString1, const char* szString2, aeUInt32 uiCharsToCompare)
  {
    AE_STRINGCOMPARE_HANDLE_NULL_PTRS (szString1, szString2, 0, 1);

    return (_strnicmp (szString1, szString2, uiCharsToCompare));
  }


  AE_INLINE void aeStringFunctions::Copy (char* szDest, aeUInt32 uiDstSize, const char* szSource)
  {
    // Handle NULL-pointer strings
    if (IsNullOrEmpty (szSource))
    {
      szDest[0] = '\0';
      return;
    }

    AE_CHECK_DEV (uiDstSize > 0, "aeStringFunctions::Copy: Cannot copy into a Zero-Length-String.");
    AE_CHECK_DEV (szDest != NULL, "aeStringFunctions::Copy: Cannot copy into a NULL-String.");

    strncpy (szDest, szSource, uiDstSize - 1);
    szDest[uiDstSize - 1] = '\0';
  }

  AE_INLINE void aeStringFunctions::Copy (char* szDest, aeUInt32 uiDstSize, const char* szSource, aeUInt32 uiCharsToCopy)
  {
    // Handle NULL-pointer strings
    if (IsNullOrEmpty (szSource))
    {
      szDest[0] = '\0';
      return;
    }

    AE_CHECK_DEV (uiDstSize > 0, "aeStringFunctions::Copy: Cannot copy into a Zero-Length-String.");
    AE_CHECK_DEV (szDest != NULL, "aeStringFunctions::Copy: Cannot copy into a NULL-String.");

    const aeUInt32 uiSize = aeMath::Min (uiCharsToCopy, uiDstSize - 1);
    strncpy (szDest, szSource, uiSize);
    szDest[uiSize] = '\0';
  }

  AE_INLINE aeUInt32 aeStringFunctions::Concatenate (char* szDest, aeUInt32 uiDstSize, aeUInt32 uiDstStrLen, const char* szSource)
  {
    // Handle NULL-pointer strings
    if (IsNullOrEmpty (szSource))
      return (uiDstStrLen);

    AE_CHECK_DEV (szDest != NULL, "aeStringFunctions::Concatenate: Cannot concatenate to a NULL-String.");

    aeUInt32 uiWritePos = uiDstStrLen;
    aeUInt32 uiReadPos = 0;
    while ((uiWritePos < uiDstSize-1) && (szSource[uiReadPos] != '\0'))
    {
      szDest[uiWritePos] = szSource[uiReadPos];

      ++uiReadPos;		
      ++uiWritePos;
    }

    szDest[uiWritePos] = '\0';
    return (uiWritePos);
  }

  AE_INLINE aeUInt32 aeStringFunctions::Concatenate (char* szDest, aeUInt32 uiDstSize, aeUInt32 uiDstStrLen, const char* szSource, aeUInt32 uiCharsToCopy)
  {
    // Handle NULL-pointer strings
    if (IsNullOrEmpty (szSource))
      return (uiDstStrLen);

    AE_CHECK_DEV (szDest != NULL, "aeStringFunctions::Concatenate: Cannot concatenate to a NULL-String.");

    aeUInt32 uiWritePos = uiDstStrLen;
    aeUInt32 uiReadPos = 0;
    while ((uiWritePos < uiDstSize-1) && (szSource[uiReadPos] != '\0') && (uiReadPos < uiCharsToCopy))
    {
      szDest[uiWritePos] = szSource[uiReadPos];

      ++uiReadPos;		
      ++uiWritePos;
    }

    szDest[uiWritePos] = '\0';
    return (uiWritePos);
  }

  AE_INLINE aeUInt32 aeStringFunctions::Concatenate (char* szDest, aeUInt32 uiDstSize, const char* szSource)
  {
    // Handle NULL-pointer strings
    if (IsNullOrEmpty (szSource))
      return (GetLength (szDest));

    return (Concatenate (szDest, uiDstSize, GetLength (szDest), szSource));
  }

  AE_INLINE aeUInt32 aeStringFunctions::Concatenate (char* szDest, aeUInt32 uiDstSize, const char* szSource, aeUInt32 uiCharsToCopy)
  {
    // Handle NULL-pointer strings
    if (IsNullOrEmpty (szSource))
      return (GetLength (szDest));

    return (Concatenate (szDest, uiDstSize, GetLength (szDest), szSource, uiCharsToCopy));
  }

  AE_INLINE char aeStringFunctions::ToUpperCase (char cChar)
  {
    return (toupper (cChar));
  }

  AE_INLINE char aeStringFunctions::ToLowerCase (char cChar)
  {
    return (tolower (cChar));
  }
}

#endif


