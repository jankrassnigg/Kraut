#ifndef AE_GRAPHICS_BASE_H
#define AE_GRAPHICS_BASE_H

#include <KrautFoundation/Defines.h>

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_GRAPHICS;

// Configure the DLL Import/Export Define
#ifdef AE_COMPILE_ENGINE_AS_DLL
#  ifdef BUILDSYSTEM_BUILDING_KRAUTGRAPHICS_LIB
#    define AE_GRAPHICS_DLL __declspec(dllexport)
#    define AE_GRAPHICS_TEMPLATE
#  else
#    define AE_GRAPHICS_DLL __declspec(dllimport)
#    define AE_GRAPHICS_TEMPLATE extern
#  endif
#else
#  define AE_GRAPHICS_DLL
#  define AE_GRAPHICS_TEMPLATE
#endif
} // namespace AE_NS_GRAPHICS

#endif
