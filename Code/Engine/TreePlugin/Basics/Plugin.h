#ifndef AE_TREEPLUGIN_PLUGIN_H
#define AE_TREEPLUGIN_PLUGIN_H

#include "../Basics/Base.h"
#include "Globals.h"

extern "C"
{
  AE_TREEGEN_DLL void aePlugin_Init(void);
  AE_TREEGEN_DLL void aePlugin_DeInit(void);
}

class aeTreePlugin
{
public:
  aeTreePlugin();

  void Render();
  aeUInt32 PickAt(aeUInt32 x, aeUInt32 y, aeVec3* out_pPosition, aeUInt32& out_uiSubID);

  void RenderLeafCard(aeUInt32 uiRenderSize, bool bExportNow);
  bool RenderBranchOfType(aeUInt32 uiRenderSize, aeUInt32 uiBranchType);
  bool ExportLeafCard(aeUInt32 uiRenderSize, const char* szFilePath, bool bDDS);
  bool ExportLeafCards(const char* szFilePath, bool bDDS);

  bool CreateBranchTypeThumbnail(aeUInt32 uiBranch, aeArray<aeUInt8>& out_Thumbnail);

  bool LoadTree(const char* szFile);

  bool CloseTree(void);

  aeInt32 m_iMouseMoveX;
  aeInt32 m_iMouseMoveY;
  aeInt32 m_iMouseWheel;
  bool m_bLeftMouseDown;
  bool m_bRightMouseDown;
  bool m_bMiddleMouseDown;
  aeVec3 m_vPickedPosition;

  static void aeEditorPlugin_Events(void* pPassThrough, const void* pEventData);


private:
  void UpdateCamera(void);
  void UpdatePickingBuffer(void);
};

extern aeTreePlugin g_Plugin;

#endif
