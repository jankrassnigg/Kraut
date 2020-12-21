#ifndef AE_FOUNDATION_STRINGS_LOCALEFIXER_H
#define AE_FOUNDATION_STRINGS_LOCALEFIXER_H

#include "Declarations.h"
#include "../Strings/StringFunctions.h"
#include <locale.h>

namespace AE_NS_FOUNDATION
{
  //! Smaller Helper class to set and reset the current C locale.
  /*! Some C functions such as sprintf or sscanf use the global locale to
      determine the exact formatting of a string.
      Depending on the OS / C-libs / etc. this locale can be always english,
      but also sometimes german, etc.\n
      Some functions however depend on the locale to always be english.
      So this class sets the locale to english upon creation and resets
      it to whatever it was before, upon destruction.
  */
  struct AE_FOUNDATION_DLL aeLocaleFixer
  {
    aeLocaleFixer (void)
    {
      aeStringFunctions::Copy (m_szPrevLocale, 32, setlocale (LC_ALL, NULL));
      setlocale (LC_ALL, "C");
    }
      
    ~aeLocaleFixer ()
    {
      setlocale (LC_ALL, m_szPrevLocale);
    }

  private:
    char m_szPrevLocale[32];
  };

}

#endif

