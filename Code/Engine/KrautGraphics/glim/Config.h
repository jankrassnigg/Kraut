/*
** GLIM - OpenGL Immediate Mode
** Copyright Jan Krassnigg (Jan@Krassnigg.de)
** For more details, see the included Readme.txt.
*/

#ifndef GLIM_CONFIG_H
#define GLIM_CONFIG_H

// Redefine this macro for DLL import export or keep it empty when using static linking
#define GLIM_DYNAMIC_LINKING

// Configure the DLL Import/Export Define
#ifdef GLIM_DYNAMIC_LINKING
  #ifdef BUILDSYSTEM_BUILDING_KRAUTGRAPHICS_LIB
    #define GLIM_IMPEXP __declspec(dllexport)
  #else
    #define GLIM_IMPEXP __declspec(dllimport)
  #endif
#else
  #define GLIM_IMPEXP
#endif


//#define AE_RENDERAPI_OPENGL
//#define AE_RENDERAPI_D3D11

#ifdef AE_RENDERAPI_OPENGL

	// change this line as you need it to use another extension loader
	#include "../glew/glew.h"
	//#include <gl/glew.h>
	//#include <gl/GLee.h>

#endif

#pragma once

#endif

