#include "PCH.h"

#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include "../Physics/PhysicsObject.h"
#include "../Tree/Tree.h"
#include "../Undo/TreeUndo.h"
#include "Plugin.h"
#include <KrautEditorBasics/Picking/Picking.h>
#include <KrautEditorBasics/Plugin.h>
#include <KrautFoundation/Configuration/VariableRegistry.h>
#include <KrautGraphics/ShaderManager/ShaderManager.h>

#include <KrautFoundation/Configuration/CVar.h>
#include <KrautFoundation/FileSystem/FileIn.h>
#include <KrautFoundation/FileSystem/FileOut.h>

aeTreePlugin g_Plugin;

aeCVarString CVar_LRU_Tree("Tree_LRU", "", aeCVarFlags::Save, "Which tree file was the least recently used.");

aeDeque<aeFilePath> g_LRU_Trees;

AE_ON_GLOBAL_EVENT(aeShutdown_main_begin)
{
  aeFileOut file;
  if (!file.Open("<APP>/LRU.dat"))
    return;

  const aeUInt32 uiNumEntries = g_LRU_Trees.size();
  file << uiNumEntries;

  for (aeUInt32 i = 0; i < g_LRU_Trees.size(); ++i)
    file << g_LRU_Trees[i];
}

AE_ON_GLOBAL_EVENT(aeStartup_main_end)
{
  aeFileIn file;
  if (!file.Open("<APP>/LRU.dat"))
    return;

  aeUInt32 uiNumEntries = 0;
  file >> uiNumEntries;
  g_LRU_Trees.resize(uiNumEntries);

  for (aeUInt32 i = 0; i < g_LRU_Trees.size(); ++i)
  {
    aeString s;
    file >> s;
    g_LRU_Trees[i] = s;
  }
}

void aePlugin_Init(void)
{
  aeLog::Log("Tree Plugin Init");

  aeEditorPlugin::s_EditorPluginEvent.RegisterEventReceiver(aeTreePlugin::aeEditorPlugin_Events, &g_Plugin);
  aeEditorPlugin::s_EditorName = "Kraut (BETA 3)";

  g_Globals.Reset();

  AE_BROADCAST_EVENT(aeTreePlugin_Init);
}

void aePlugin_DeInit(void)
{
  aeLog::Log("Tree Plugin DeInit");

  AE_BROADCAST_EVENT(aeTreePlugin_DeInit);

  aeEditorPlugin::s_EditorPluginEvent.UnregisterEventReceiver(aeTreePlugin::aeEditorPlugin_Events, &g_Plugin);
}

aeTreePlugin::aeTreePlugin()
{
  m_iMouseMoveX = 0;
  m_iMouseMoveY = 0;
  m_iMouseWheel = 0;
  m_bLeftMouseDown = false;
  m_bRightMouseDown = false;
  m_bMiddleMouseDown = false;
  g_Globals.s_vCameraPivot.SetVector(0, 2.0f, 0);
  g_Globals.s_vCameraPosition.SetVector(0, 3.0f, 5);

  g_Globals.m_Camera.setPerspectiveCamera(80.0f, 0.01f, 1000.0f);
  g_Globals.m_Camera.setLookAt(aeVec3(0, 0, 100), aeVec3(0.0f), aeVec3(0, 1, 0));
  g_Globals.m_Camera.setViewport(0, 0, 500, 500);

  g_Globals.m_OrthoCamera.setOrthoCamera(-100, 100, -100, 100, -1000, 1000);
  g_Globals.m_OrthoCamera.setLookAt(aeVec3(0, 0, 100), aeVec3(0.0f), aeVec3(0, 1, 0));
  g_Globals.m_OrthoCamera.setViewport(0, 0, 500, 500);
}

void aeTreePlugin::aeEditorPlugin_Events(void* pPassThrough, const void* pEventData)
{
  aeTreePlugin* pPlugin = (aeTreePlugin*)pPassThrough;
  aeEditorPluginEvent* pEvent = (aeEditorPluginEvent*)pEventData;

  static aeInt32 iLastMouseX = 0;
  static aeInt32 iLastMouseY = 0;
  static bool bDisableCameraChange = false;

  switch (pEvent->m_Type)
  {
    //case aeEditorPluginEvent::KeyPress:
    //{
    //  if (pEvent->m_Data.m_KeyEvent->key() == Qt::Key_Shift)
    //  {
    //    g_Globals.s_bManualEditingActive = true;
    //    AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
    //  }
    //}
    //break;
    //case aeEditorPluginEvent::KeyRelease:
    //{
    //  if (pEvent->m_Data.m_KeyEvent->key() == Qt::Key_Shift)
    //  {
    //    g_Globals.s_bManualEditingActive = false;
    //    AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
    //  }
    //}
    //break;
    case aeEditorPluginEvent::MouseMove:
    {
      //if ((pEvent->m_Data.m_MouseEvent->modifiers() & Qt::ShiftModifier) == 0)
      //  g_Globals.s_bManualEditingActive = false;

      pEvent->m_Data.m_MouseEvent->accept();

      //if (g_Globals.s_bPaintingBranch)
      //{
      //  iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
      //  iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();

      //  const aeVec3 vDir = aeEditorPicking::GetPickingDirection(pEvent->m_Data.m_MouseEvent->x(), pEvent->m_Data.m_MouseEvent->y());

      //  float fIntersection;
      //  if (g_Globals.s_PaintingPlane.GetRayIntersection(g_Globals.m_Camera.getPosition(), vDir, fIntersection, pPlugin->m_vPickedPosition))
      //  {
      //    if ((g_Globals.s_PaintedBranch.back() - pPlugin->m_vPickedPosition).GetLength() >= 0.1f)
      //      g_Globals.s_PaintedBranch.push_back(pPlugin->m_vPickedPosition);
      //  }

      //  g_Globals.s_iBranchToBe = g_Tree.AddPaintedBranch(g_Globals.s_iParentBranchID, g_Globals.s_uiParentBranchNodeID, g_Globals.s_PaintedBranch, g_Globals.s_PaintingPlane, true, g_Globals.s_iBranchToBe);

      //  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
      //  break;
      //}

      if (aeTransformGizmo::IsDraggingGizmo() && (pEvent->m_Data.m_MouseEvent->modifiers() == 0))
      {
        pPlugin->m_iMouseMoveX += (pEvent->m_Data.m_MouseEvent->globalX() - iLastMouseX) * 3;
        pPlugin->m_iMouseMoveY += (pEvent->m_Data.m_MouseEvent->globalY() - iLastMouseY) * 3;

        iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
        iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();

        aeUInt32 x = pEvent->m_Data.m_MouseEvent->x();
        aeUInt32 y = pEvent->m_Data.m_MouseEvent->y();

        const aeMatrix mPrev = aeTransformGizmo::GetCurrentlyDraggedGizmo()->GetTransform();

        aeTransformGizmo::DragTo(x, y);

        pPlugin->m_iMouseMoveX = 0;
        pPlugin->m_iMouseMoveY = 0;

        AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
        break;
      }

      if (!bDisableCameraChange)
      {
        if (pPlugin->m_bLeftMouseDown || pPlugin->m_bRightMouseDown || pPlugin->m_bMiddleMouseDown)
        {
          pPlugin->m_iMouseMoveX += (pEvent->m_Data.m_MouseEvent->globalX() - iLastMouseX) * 3;
          pPlugin->m_iMouseMoveY += (pEvent->m_Data.m_MouseEvent->globalY() - iLastMouseY) * 3;

          iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
          iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();

          AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
          break;
        }
      }

      //if (pEvent->m_Data.m_MouseEvent->modifiers () & Qt::ControlModifier)
      {
        aeUInt32 x = pEvent->m_Data.m_MouseEvent->x();
        aeUInt32 y = pEvent->m_Data.m_MouseEvent->y();
        bool bCTRL = false; //(pEvent->m_Data.m_MouseEvent->modifiers() & Qt::ControlModifier);

        static aeUInt32 uiLastID = 0;
        static aeUInt32 uiLastSubID = 0;
        //static bool bLastCTRL = false;

        aeUInt32 SubID;
        aeUInt32 id = pPlugin->PickAt(x, y, &pPlugin->m_vPickedPosition, SubID);

        if ((uiLastID != id) || (uiLastSubID != SubID) /*|| (bCTRL != bLastCTRL)*/)
        {
          uiLastID = id;
          uiLastSubID = SubID;
          //bLastCTRL = bCTRL;

          if ((!bCTRL) && (!aeStringFunctions::CompareEqual(aeEditorPlugin::GetPickableObject(id).m_szType, "aeTransformGizmo")))
          {
            id = 0;
            SubID = 0;
          }

          aeShaderManager::setUniformUB("unif_Selected", (id >> 0) & 0xFF, (id >> 8) & 0xFF, (id >> 16) & 0xFF, (id >> 24) & 0xFF);
          aeShaderManager::setUniformUB("unif_SelectedSubID", (SubID >> 0) & 0xFF, (SubID >> 8) & 0xFF, (SubID >> 16) & 0xFF, (SubID >> 24) & 0xFF);
          AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
        }
      }
    }
    break;
    case aeEditorPluginEvent::MouseRelease:
    {
      bDisableCameraChange = false;

      //if (g_Globals.s_bPaintingBranch)
      //{
      //  g_Globals.s_bPaintingBranch = false;
      //  aeInt32 iBranchID = g_Tree.AddPaintedBranch(g_Globals.s_iParentBranchID, g_Globals.s_uiParentBranchNodeID, g_Globals.s_PaintedBranch, g_Globals.s_PaintingPlane, false, g_Globals.s_iBranchToBe);
      //  g_Globals.s_PaintedBranch.clear();

      //  aeTreePlugin::SetSelectedBranch(iBranchID, 0);
      //}

      aeTransformGizmo::EndDrag();
      aeUndo::EndOperation();
    }
      // break; // THIS IS INTENDED ! (though it sucks)
    case aeEditorPluginEvent::MousePress:
    {
      if (pEvent->m_Type == aeEditorPluginEvent::MousePress)
      {
        if (pEvent->m_Data.m_MouseEvent->modifiers() & Qt::ControlModifier)
          bDisableCameraChange = true;
      }

      pEvent->m_Data.m_MouseEvent->accept();

      //g_Globals.s_bPaintingBranch = false;

      //if ((pEvent->m_Data.m_MouseEvent->button() == Qt::LeftButton) &&
      //    ((pEvent->m_Data.m_MouseEvent->buttons() & Qt::LeftButton) != 0) &&
      //    (pEvent->m_Data.m_MouseEvent->modifiers() & Qt::ShiftModifier))
      //{
      //  // Left-Click + SHIFT -> Manual Branch Painting

      //  iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
      //  iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();

      //  aeShaderManager::setUniformUB("unif_Selected", 0, 0, 0, 0);
      //  aeShaderManager::setUniformUB("unif_SelectedSubID", 0, 0, 0, 0);

      //  aeUInt32 SubID;
      //  aeUInt32 id = pPlugin->PickAt(pEvent->m_Data.m_MouseEvent->x(), pEvent->m_Data.m_MouseEvent->y(), &pPlugin->m_vPickedPosition, SubID);

      //  if (!pPlugin->m_vPickedPosition.IsZeroVector())
      //  {
      //    if (aeStringFunctions::CompareEqual(aeEditorPlugin::GetPickableObject(id).m_szType, "aeTreeBranch"))
      //    {
      //      Kraut::BranchStructure* pBranch = (Kraut::BranchStructure*)aeEditorPlugin::GetPickableObject(id).m_pObject;

      //      //if (!pBranch->m_bManuallyCreated)
      //        return;

      //      //g_Globals.s_iParentBranchID = aeEditorPlugin::GetPickableObject(id).m_uiSubID;
      //      //g_Globals.s_uiParentBranchNodeID = SubID;
      //    }
      //    //else
      //    //  g_Globals.s_iParentBranchID = -1;

      //    //aeUndo::BeginOperation();
      //    //g_Globals.s_bPaintingBranch = true;
      //    //g_Globals.s_PaintedBranch.push_back(pPlugin->m_vPickedPosition);
      //    //g_Globals.s_iBranchToBe = -1;

      //    //aeTreePlugin::CreatePaintingPlane(pPlugin->m_vPickedPosition, g_Globals.m_Camera.getVectorRight(), g_Globals.m_Camera.getVectorUp(), g_Globals.s_iParentBranchID, g_Globals.s_uiParentBranchNodeID);
      //  }
      //}
      //else
      if (pEvent->m_Data.m_MouseEvent->button() == Qt::LeftButton)
      {
        pPlugin->m_bLeftMouseDown = (pEvent->m_Data.m_MouseEvent->buttons() & Qt::LeftButton) != 0;

        iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
        iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();

        if (!pPlugin->m_bLeftMouseDown)
          g_Globals.s_bUpdatePickingBuffer = true;

        // check whether any gizmo was selected
        if (pPlugin->m_bLeftMouseDown)
        {
          aeUInt32 SubID;
          aeUInt32 id = pPlugin->PickAt(pEvent->m_Data.m_MouseEvent->x(), pEvent->m_Data.m_MouseEvent->y(), &pPlugin->m_vPickedPosition, SubID);

          // anything pickable was hit
          if (id != 0)
          {
            const aePickableObject& po = aeEditorPlugin::GetPickableObject(id);

            if (pEvent->m_Data.m_MouseEvent->modifiers() & Qt::ControlModifier)
            {
              // if CTRL is pressed, do not check the gizmo

              //if (aeStringFunctions::CompareEqual(po.m_szType, "aeTreeBranch"))
              //{
              //  // a branch was picked
              //  aeTreePlugin::SetSelectedBranch(po.m_uiSubID, SubID);
              //}
              //else
              if (aeStringFunctions::CompareEqual(po.m_szType, "aePhysicsObject"))
              {
                aePhysicsObject* pObj = (aePhysicsObject*)aeEditorPlugin::GetPickableObject(id).m_pObject;

                aePhysicsObject::SetSelected(pObj);
              }
              else if (aeStringFunctions::CompareEqual(po.m_szType, "aeForce"))
              {
                aeForce* pObj = (aeForce*)aeEditorPlugin::GetPickableObject(id).m_pObject;

                aeForce::SetSelectedForce(pObj);
              }
              else
                aePhysicsObject::SetSelected(nullptr);

              aeShaderManager::setUniformUB("unif_Selected", (id >> 0) & 0xFF, (id >> 8) & 0xFF, (id >> 16) & 0xFF, (id >> 24) & 0xFF);
              aeShaderManager::setUniformUB("unif_SelectedSubID", (SubID >> 0) & 0xFF, (SubID >> 8) & 0xFF, (SubID >> 16) & 0xFF, (SubID >> 24) & 0xFF);
              AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
            }
            else
            {
              if (aeStringFunctions::CompareEqual(po.m_szType, "aeTransformGizmo"))
              {
                ((aeTransformGizmo*)po.m_pObject)->BeginDrag((aeTransformGizmo::GizmoAxis)SubID, g_Globals.s_vCameraPosition, pEvent->m_Data.m_MouseEvent->x(), pEvent->m_Data.m_MouseEvent->y());

                aeShaderManager::setUniformUB("unif_Selected", (id >> 0) & 0xFF, (id >> 8) & 0xFF, (id >> 16) & 0xFF, (id >> 24) & 0xFF);
                aeShaderManager::setUniformUB("unif_SelectedSubID", (SubID >> 0) & 0xFF, (SubID >> 8) & 0xFF, (SubID >> 16) & 0xFF, (SubID >> 24) & 0xFF);
                AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
              }
            }
          }
        }
      }

      if (pEvent->m_Data.m_MouseEvent->button() == Qt::RightButton)
      {
        pPlugin->m_bRightMouseDown = (pEvent->m_Data.m_MouseEvent->buttons() & Qt::RightButton) != 0;

        iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
        iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();
      }

      if (pEvent->m_Data.m_MouseEvent->button() == Qt::MiddleButton)
      {
        pPlugin->m_bMiddleMouseDown = (pEvent->m_Data.m_MouseEvent->buttons() & Qt::MiddleButton) != 0;

        iLastMouseX = pEvent->m_Data.m_MouseEvent->globalX();
        iLastMouseY = pEvent->m_Data.m_MouseEvent->globalY();
      }
    }
    break;
    case aeEditorPluginEvent::MouseWheel:
    {
      pEvent->m_Data.m_WheelEvent->accept();

      pPlugin->m_iMouseWheel += aeMath::Sign(pEvent->m_Data.m_WheelEvent->delta());

      AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
    }
    break;
  }
}

//AE_ON_GLOBAL_EVENT(aeTreeEdit_BranchDeleted)
//{
//  if (aeTreePlugin::GetSelectedBranchID() < 0)
//    return;
//
//  if (aeTreePlugin::GetSelectedBranchID() >= (aeInt32)g_Tree.m_TreeStructure.m_Branches.size())
//  {
//    aeTreePlugin::SetSelectedBranch(-1, 0);
//    return;
//  }
//}

AE_ON_GLOBAL_EVENT(aeTreePlugin_AfterUndo)
{
  //aeTreePlugin::SetSelectedBranch(-1, 0);
  //g_Globals.s_bPaintingBranch = false;
  aeTransformGizmo::EndDrag();
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_AfterRedo)
{
  //aeTreePlugin::SetSelectedBranch(-1, 0);
  //g_Globals.s_bPaintingBranch = false;
  aeTransformGizmo::EndDrag();
}

extern bool s_bTreeNeedsUpdate;
extern bool s_bTreeInfluencesChanged;
extern bool s_bTreeHasSimpleChanges;

AE_ON_GLOBAL_EVENT(aeTreeEdit_TreeModified)
{
  s_bTreeNeedsUpdate = true;

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

AE_ON_GLOBAL_EVENT(aeTreeEdit_TreeModifiedSimple)
{
  s_bTreeHasSimpleChanges = true;

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

AE_ON_GLOBAL_EVENT(aeTreeEdit_TreeInfluencesChanged)
{
  s_bTreeInfluencesChanged = true;

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

//void aeTreePlugin::SetSelectedBranch(aeInt32 iSelectedBranch, aeUInt32 uiBranchNode)
//{
//  if ((iSelectedBranch < 0) && (g_Globals.s_iSelectedBranch < 0))
//    return;
//
//  if (iSelectedBranch < 0)
//  {
//    iSelectedBranch = -1;
//    uiBranchNode = 0;
//  }
//
//  if ((iSelectedBranch == g_Globals.s_iSelectedBranch) && (uiBranchNode == g_Globals.s_uiSelectedBranchNode))
//    return;
//
//  AE_BROADCAST_EVENT(aeTreePlugin_SelectedBranchChanged, iSelectedBranch, g_Globals.s_iSelectedBranch);
//
//  g_Globals.s_iSelectedBranch = iSelectedBranch;
//  g_Globals.s_uiSelectedBranchNode = uiBranchNode;
//
//  if (g_Globals.s_iSelectedBranch != -1)
//  {
//    Kraut::BranchStructure* pSelected = g_Tree.m_TreeStructure.m_Branches[iSelectedBranch].get();
//    g_Globals.s_BranchTransform.Reset(pSelected, g_Globals.s_uiSelectedBranchNode, pSelected->GetNodeTransformation(uiBranchNode), pSelected->m_bManuallyCreated);
//  }
//  else
//    g_Globals.s_BranchTransform.SetActive(false);
//}

static bool IsInList(aeDeque<aeFilePath> List, const aeFilePath& sPath)
{
  for (aeUInt32 i2 = 0; i2 < List.size(); ++i2)
  {
    if (List[i2] == sPath)
      return true;
  }

  return false;
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeLoaded)
{
  CVar_LRU_Tree = param0.Text;

  aeDeque<aeFilePath> NewLRU;
  NewLRU.push_back(param0.Text);

  for (aeUInt32 i = 0; i < aeMath::Min<aeUInt32>(10, g_LRU_Trees.size()); ++i)
  {
    if (IsInList(NewLRU, g_LRU_Trees[i]))
      continue;

    NewLRU.push_back(g_LRU_Trees[i]);
  }

  g_LRU_Trees = NewLRU;

  qtTreeEditWidget::s_pWidget->UpdateRecentFileMenu();
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeSaved)
{
  CVar_LRU_Tree = param0.Text;
  s_bTreeHasSimpleChanges = false;
}

extern aeFilePath sPrevFile;

bool aeTreePlugin::LoadTree(const char* szFile)
{
  aeFileIn f;
  if (!f.Open(szFile))
    return false;

  sPrevFile = szFile;

  g_Tree.m_sTreeFile = szFile;
  g_Tree.Load(f);

  AE_BROADCAST_EVENT(aeEditor_SetWindowTitle, szFile);

  AE_BROADCAST_EVENT(aeEditor_WorkspaceUnmodified);

  AE_BROADCAST_EVENT(aeTreePlugin_TreeLoaded, szFile);

  qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType(Kraut::BranchType::Trunk1, true);

  s_bTreeNeedsUpdate = true;
  s_bTreeInfluencesChanged = true;

  // select the first active branch type
  for (aeInt32 i = 0; i < Kraut::BranchType::ENUM_COUNT; ++i)
  {
    if (g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[i].m_bUsed)
    {
      qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType((Kraut::BranchType::Enum)i, false);
      goto found;
    }
  }

  qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType(Kraut::BranchType::Trunk1, false);

found:

  qtTreeEditWidget::s_pWidget->combo_LoD->setCurrentIndex(aeLod::Lod0);

  return true;
}

AE_ON_GLOBAL_EVENT(aeEditor_BeforeFirstFrame)
{
  const char* szValue = CVar_LRU_Tree.GetValue();
  if ((aeStringFunctions::CompareEqual(szValue, "")) || (!g_Plugin.LoadTree(szValue)))
    qtTreeEditWidget::s_pWidget->on_action_New_triggered();
}

bool aeTreePlugin::CloseTree(void)
{
  if (!aeUndo::HasBeenModified())
  {
    AE_BROADCAST_EVENT(aeTreePlugin_TreeUnloaded);
    return true;
  }

  QMainWindow* pMainWindow = nullptr;
  aeVariableRegistry::RetrieveRaw("system/qt/mainwidget", &pMainWindow, sizeof(QMainWindow*));

  int res = QMessageBox::question(pMainWindow, "Kraut", "The tree has been modified. Save before closing?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

  if (res == QMessageBox::Cancel)
    return false;

  if (res == QMessageBox::Yes)
  {
    if (!qtTreeEditWidget::s_pWidget->SaveTree())
      return false;
  }

  AE_BROADCAST_EVENT(aeTreePlugin_TreeUnloaded);
  return true;
}

AE_ON_GLOBAL_EVENT(aeEditor_Close)
{
  bool* bClose = (bool*)param0.Ptr;

  if (*bClose == false)
    return;

  if (!g_Plugin.CloseTree())
    *bClose = false;
}
