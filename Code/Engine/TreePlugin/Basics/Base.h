#ifndef AE_TREEPLUGIN_BASE_H
#define AE_TREEPLUGIN_BASE_H

#include <KrautFoundation/Defines.h>
#include <KrautFoundation/Math/Vec3.h>
#include <KrautFoundation/Math/Matrix.h>
#include "Enums.h"

using namespace AE_NS_FOUNDATION;

  // Configure the DLL Import/Export Define
  #ifdef AE_COMPILE_ENGINE_AS_DLL
    #ifdef BUILDSYSTEM_BUILDING_TREEPLUGIN_LIB
      #define AE_TREEGEN_DLL __declspec(dllexport)
      #define AE_TREEGEN_TEMPLATE
    #else
      #define AE_TREEGEN_DLL __declspec(dllimport)
      #define AE_TREEGEN_TEMPLATE extern
    #endif
  #else
    #define AE_TREEGEN_DLL
    #define AE_TREEGEN_TEMPLATE
  #endif

  struct aeRecursionGuard
  {
    aeRecursionGuard (bool* bRecursion)
    {
      m_bRecursion = bRecursion;
      *m_bRecursion = true;
    }

    ~aeRecursionGuard ()
    {
      *m_bRecursion = false;
    }

    bool* m_bRecursion;
  };

  #define AE_PREVENT_RECURSION static bool s_bRecursion = false; if (s_bRecursion) return; aeRecursionGuard rc (&s_bRecursion);
  #define AE_PREVENT_RECURSION_RET(ret) static bool s_bRecursion = false; if (s_bRecursion) return ret; aeRecursionGuard rc (&s_bRecursion);
  #define AE_EXECUTE_ONLY_ONCE static bool s_bDone1234 = false; if (s_bDone1234) return; s_bDone1234 = true;
  #define AE_EXECUTE_ONLY_ONCE_RET(ret) static bool s_bDone1234 = false; if (s_bDone1234) return ret; s_bDone1234 = true;

#endif

