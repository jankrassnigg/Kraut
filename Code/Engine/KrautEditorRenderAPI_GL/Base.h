#ifndef AE_EDITORRENDERAPIGL_BASE_H
#define AE_EDITORRENDERAPIGL_BASE_H

#include <KrautFoundation/Defines.h>

  // Configure the DLL Import/Export Define
  #ifdef AE_COMPILE_ENGINE_AS_DLL
    #ifdef BUILDSYSTEM_BUILDING_KRAUTEDITORRENDERAPI_GL_LIB
      #define AE_EDITORRENDERAPIGL_DLL __declspec(dllexport)
      #define AE_EDITORRENDERAPIGL_TEMPLATE
    #else
      #define AE_EDITORRENDERAPIGL_DLL __declspec(dllimport)
      #define AE_EDITORRENDERAPIGL_TEMPLATE extern
    #endif
  #else
    #define AE_EDITORRENDERAPIGL_DLL
    #define AE_EDITORRENDERAPIGL_TEMPLATE
  #endif

#endif

