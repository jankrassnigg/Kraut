#include <stdio.h>
#include <ctype.h>

#include "../Checks.h"

// for the nice debug-report window
#ifdef AE_PLATFORM_WINDOWS
  #include <windows.h>
  #include <crtdbg.h>
#endif



namespace AE_NS_FOUNDATION
{
  void aeFailedCheck (const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szErrorMsg, ...)
  {
    va_list ap;
    va_start (ap, szErrorMsg);

    char szMsg[1024 * 10] = "";
    vsprintf (szMsg, szErrorMsg, ap);
    szMsg[1024 * 10 - 1] = '\0';

    va_end (ap);

    printf ("%s\n", szMsg);
    printf ("%s(%d) : error 13: Expression '%s' failed: %s\n", szSourceFile, uiLine, szExpression, szMsg);

    #ifdef AE_PLATFORM_WINDOWS

      #ifdef AE_COMPILE_FOR_DEBUG

        aeInt32 iRes = _CrtDbgReport(_CRT_ASSERT, szSourceFile, uiLine, NULL, "'%s'\nFunction: %s\nMessage: %s", szExpression, szFunction, szMsg);

        // currently we will ALWAYS trigger the breakpoint / crash (except for when the user presses 'cancel'
        if (iRes != 0)
          AE_DEBUG_BREAK

      #else
        char szTemp[1024 * 10] = "";
        sprintf (szTemp, " *** Assertion ***\n\nExpression: \"%s\"\nFunction: \"%s\"\nFile: \"%s\"\nLine: %d\nMessage: \"%s\"", szExpression, szFunction, szSourceFile, uiLine, szMsg);
        MessageBox (NULL, szTemp, "Assertion", MB_ICONERROR);
      #endif

    #else
      printf ("\n *** Assertion *** \n");
      printf ("Expression: \"%s\"\n", szExpression);
      printf ("Function: \"%s\"\n", szFunction);
      printf ("File: \"%s\"\n", szSourceFile);
      printf ("Line: %d\n", uiLine);
      printf ("Message: \"%s\"\n", szMsg);
      printf (" +++ Assertion +++ \n\n");
    #endif

    // always do a debug-break
    // in release-builds this will just crash the app
    AE_DEBUG_BREAK
  }
}


