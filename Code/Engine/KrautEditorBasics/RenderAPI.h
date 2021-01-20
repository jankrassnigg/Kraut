#ifndef AE_EDITORBASICS_RENDERAPI_H
#define AE_EDITORBASICS_RENDERAPI_H

#include "Base.h"

#include <KrautGraphics/Utility/SingletonClass.h>
using namespace AE_NS_FOUNDATION;

namespace AE_NS_EDITORBASICS
{
  AE_SINGLETON_CLASS(AE_EDITORBASICS_DLL, aeEditorRenderAPI);

  //! Abstract Interface for a plugin that initializes some render API.
  class AE_EDITORBASICS_DLL aeEditorRenderAPI : AE_MAKE_SINGLETON(aeEditorRenderAPI)
  {
  public:
    aeEditorRenderAPI() {}
    virtual ~aeEditorRenderAPI() {}
    virtual void CreateContext(aeUInt32 uiWidth, aeUInt32 uiHeight) = 0;
    virtual void DestroyContext() = 0;
    virtual void ResizeFramebuffer(aeUInt32 uiWidth, aeUInt32 uiHeight) = 0;
    virtual void Swap() = 0;
  };
} // namespace AE_NS_EDITORBASICS


#endif
