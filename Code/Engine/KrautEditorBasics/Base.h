#ifndef AE_EDITORBASICS_BASE_H
#define AE_EDITORBASICS_BASE_H

#include <KrautFoundation/Defines.h>

  // Configure the DLL Import/Export Define
  #ifdef AE_COMPILE_ENGINE_AS_DLL
    #ifdef BUILDSYSTEM_BUILDING_KRAUTEDITORBASICS_LIB
      #define AE_EDITORBASICS_DLL __declspec(dllexport)
      #define AE_EDITORBASICS_TEMPLATE
    #else
      #define AE_EDITORBASICS_DLL __declspec(dllimport)
      #define AE_EDITORBASICS_TEMPLATE extern
    #endif
  #else
    #define AE_EDITORBASICS_DLL
    #define AE_EDITORBASICS_TEMPLATE
  #endif

#endif

