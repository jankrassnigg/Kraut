//#include "../Tree.h"
#include <KrautEditorBasics/Plugin.h>
#include <KrautEditorBasics/RenderShapes/RenderShapes.h>
#include <KrautEditorBasics/Picking/Picking.h>
#include "TransformGizmo.h"
#include <KrautGraphics/ShaderManager/ShaderManager.h>
#include <KrautGraphics/RenderAPI/Main.h>



namespace AE_NS_EDITORBASICS
{

  AE_ENUMERABLE_CLASS_CODE (aeTransformGizmo);

  aeTransformGizmo::Mode aeTransformGizmo::s_GizmoMode = aeTransformGizmo::Invisible;
  aeTransformGizmo* aeTransformGizmo::s_pCurrentlyDragged = nullptr;
  aeTransformGizmo::GizmoAxis aeTransformGizmo::s_DragAxis = aeTransformGizmo::GizmoAxisX;
  aeVec3 aeTransformGizmo::s_vDragCameraPos;
  aeShaderResourceHandle aeTransformGizmo::s_hGizmoShader;

  AE_ON_GLOBAL_EVENT_ONCE (aeStartup_ShutdownEngine_Begin)
  {
    aeTransformGizmo::s_hGizmoShader.Invalidate ();
  }

  aeTransformGizmo::aeTransformGizmo ()
  {
    m_bActive = true;
    m_uiPickID = 0;
    m_vPivot.SetZero ();

    m_Transform.SetIdentity ();
    m_Transform.SetTranslationMatrix (aeVec3 (3, 2, 1));

    m_UsageFlags = aeTransformGizmo::TransformAll;
    m_PrevUsageFlags = 0;

    m_pObject = nullptr;
    m_uiObjectID = 0;
  }

  aeTransformGizmo::~aeTransformGizmo ()
  {
    aeEditorPlugin::UnregisterPickableObject (m_uiPickID);
  }

  void aeTransformGizmo::RenderAllGizmos (void)
  {
    if (!s_hGizmoShader.IsValid ())
      s_hGizmoShader = aeShaderResource::LoadResource ("Shaders/Gizmo.shader");

    aeShaderManager::setShader (s_hGizmoShader);
    aeShaderManager::setUniformFloat ("unif_ColorScale", 1, 0.3f);
    aeRenderAPI::setDepthTest (true, AE_COMPARE_GREATER);
    aeRenderAPI::setDepthMask (false);

    aeTransformGizmo* pCur = aeTransformGizmo::GetFirstInstance ();
    while (pCur)
    {
      pCur->Render (false);
      pCur = aeTransformGizmo::GetNextInstance (pCur);
    }

    aeShaderManager::setUniformFloat ("unif_ColorScale", 1, 0.6f);

    pCur = aeTransformGizmo::GetFirstInstance ();
    while (pCur)
    {
      pCur->Render (true);
      pCur = aeTransformGizmo::GetNextInstance (pCur);
    }

    aeShaderManager::setUniformFloat ("unif_ColorScale", 1, 1.0f);

    aeRenderAPI::setDepthTest (true, AE_COMPARE_LEQUAL);
    aeRenderAPI::setDepthMask (true);

    pCur = aeTransformGizmo::GetFirstInstance ();
    while (pCur)
    {
      pCur->Render (false);
      pCur = aeTransformGizmo::GetNextInstance (pCur);
    }
  }

  void aeTransformGizmo::Render (bool bWireframe)
  {
    if (!m_bActive)
      return;

    if (s_GizmoMode == Invisible)
      return;

    if (m_PrevUsageFlags != m_UsageFlags)
    {
      m_PrevUsageFlags = m_UsageFlags;
      m_GlimTranslate.Clear ();
      m_GlimRotate.Clear ();
    }

    if (m_uiPickID == 0)
    {
      m_uiPickID = aeEditorPlugin::RegisterPickableObject (this, 0, "aeTransformGizmo");
    }

    aeRenderAPI::GetMatrix_ObjectToWorld ().PushMatrix ();
    aeRenderAPI::GetMatrix_ObjectToWorld ().LoadMatrix (m_Transform);

    if (s_GizmoMode == Translate)
    {
      if (m_GlimTranslate.IsCleared ())
      {
        m_GlimTranslate.BeginBatch ();
        m_GlimTranslate.Attribute4ub ("attr_PickingID", 0, 0, 0, 0);
        m_GlimTranslate.Attribute4ub ("attr_PickingSubID", 0, 0, 0, 0);

        aeMatrix id;
        id.SetIdentity ();
        aeRenderShapes::CreateTranslationGizmo (m_GlimTranslate, id, 0.75f, 0.04f, m_uiPickID, true, true, true);//m_UsageFlags & TranslateX, m_UsageFlags & TranslateY, m_UsageFlags & TranslateZ);

        m_GlimTranslate.EndBatch ();
      }

      m_GlimTranslate.RenderBatch (bWireframe);
    }
    else
    if (s_GizmoMode == Rotate)
    {
      if (m_GlimRotate.IsCleared ())
      {
        m_GlimRotate.BeginBatch ();
        m_GlimRotate.Attribute4ub ("attr_PickingID", 0, 0, 0, 0);
        m_GlimRotate.Attribute4ub ("attr_PickingSubID", 0, 0, 0, 0);

        aeMatrix id;
        id.SetIdentity ();
        aeRenderShapes::CreateRotationGizmo (m_GlimRotate, id, 0.75f, 0.05f, m_uiPickID, true, true, true);//m_UsageFlags & RotateX, m_UsageFlags & RotateY, m_UsageFlags & RotateZ);

        m_GlimRotate.EndBatch ();
      }

      m_GlimRotate.RenderBatch (bWireframe);
    }

    aeRenderAPI::GetMatrix_ObjectToWorld ().PopMatrix ();
  }

  aeVec3 aeTransformGizmo::GetIntersectionPoint (GizmoAxis Axis, aeVec3 vCameraPos, aeInt32 uiScreenPosX, aeInt32 uiScreenPosY) const
  {
    const aeVec3 vDir = aeEditorPicking::GetPickingDirection (uiScreenPosX, uiScreenPosY);
    if (vDir.IsZeroVector (0.01f))
      return aeVec3 (0.0f);

    aePlane p;
    switch (Axis)
    {
    case 0: // X Axis
      {
        const aeVec3 vOrtho1 = m_Transform.TransformDirection (aeVec3 (1, 0, 0)).Cross (vDir).GetNormalized ();
        p.m_vNormal = m_Transform.TransformDirection (aeVec3 (1, 0, 0)).Cross (vOrtho1);
      }
      break;
    case 1:// Y Axis
      {
        const aeVec3 vOrtho1 = m_Transform.TransformDirection (aeVec3 (0, 1, 0)).Cross (vDir).GetNormalized ();
        p.m_vNormal = m_Transform.TransformDirection (aeVec3 (0, 1, 0)).Cross (vOrtho1);
      }
      break;
    case 2: // Z Axis
      {
        const aeVec3 vOrtho1 = m_Transform.TransformDirection (aeVec3 (0, 0, 1)).Cross (vDir).GetNormalized ();
        p.m_vNormal = m_Transform.TransformDirection (aeVec3 (0, 0, 1)).Cross (vOrtho1);
      }
      break;
    case 3: // XY Axis
      p.m_vNormal = m_Transform.TransformDirection (aeVec3 (0, 0, 1));
      break;
    case 4: // XZ Axis
      p.m_vNormal = m_Transform.TransformDirection (aeVec3 (0, 1, 0));
      break;
    case 5: // YZ Axis
      p.m_vNormal = m_Transform.TransformDirection (aeVec3 (1, 0, 0));
      break;
    default:
      AE_CHECK_DEV (false, "Missing Axis?");
      return aeVec3 (0.0f);
    };

    p.m_fDistance = p.m_vNormal.Dot (m_Transform.GetTranslationVector ());

    float fTime;
    aeVec3 vPoint (0.0f);
    p.GetInfiniteRayIntersection (vCameraPos, vDir, fTime, vPoint);

    return vPoint;
  }

  bool aeTransformGizmo::BeginDrag (GizmoAxis Axis, aeVec3 vCameraPos, aeInt32 uiScreenPosX, aeInt32 uiScreenPosY)
  {
    AE_CHECK_DEV (s_pCurrentlyDragged == nullptr, "Cannot start draggin a gizmo, if one is already being dragged.");

    aeVec3 vPoint = GetIntersectionPoint (Axis, vCameraPos, uiScreenPosX, uiScreenPosY);

    if (vPoint.IsZeroVector ())
      return false;

    s_DragAxis = Axis;
    s_vDragCameraPos = vCameraPos;

    s_pCurrentlyDragged = this;
    m_vPivot = vPoint - m_Transform.GetTranslationVector ();

    EventData ed;
    ed.m_EventType = DragBegin;
    ed.m_Gizmo = this;
    ed.m_PrevTransform = m_Transform;
    m_Events.RaiseEvent (&ed);

    return true;
  }

  void aeTransformGizmo::EndDrag (void)
  {
    if (!s_pCurrentlyDragged)
      return;

    EventData ed;
    ed.m_EventType = DragEnd;
    ed.m_Gizmo = s_pCurrentlyDragged;
    ed.m_PrevTransform = s_pCurrentlyDragged->m_Transform;
    s_pCurrentlyDragged->m_Events.RaiseEvent (&ed);

    s_pCurrentlyDragged = nullptr;
  }

  void aeTransformGizmo::DragTo (aeInt32 uiScreenPosX, aeInt32 uiScreenPosY)
  {
    AE_CHECK_DEV (s_pCurrentlyDragged != nullptr, "No gizmo is active for dragging (possibly BeginDrag failed).");

    if (!s_pCurrentlyDragged->m_bActive)
      return;

    aeVec3 vPoint = s_pCurrentlyDragged->GetIntersectionPoint (s_DragAxis, s_vDragCameraPos, uiScreenPosX, uiScreenPosY);

    if (vPoint.IsZeroVector ())
      return;

    const aeMatrix mPrevTransform = s_pCurrentlyDragged->m_Transform;

    if (s_GizmoMode == Translate)
    {
      aeVec3 vDist = vPoint - (s_pCurrentlyDragged->m_Transform.GetTranslationVector () + s_pCurrentlyDragged->m_vPivot);

      switch (s_DragAxis)
      {
      case 0:
        vDist = vDist.Dot (mPrevTransform.TransformDirection (aeVec3 (1, 0, 0))) * mPrevTransform.TransformDirection (aeVec3 (1, 0, 0));
        break;
      case 1:
        vDist = vDist.Dot (mPrevTransform.TransformDirection (aeVec3 (0, 1, 0))) * mPrevTransform.TransformDirection (aeVec3 (0, 1, 0));
        break;
      case 2:
        vDist = vDist.Dot (mPrevTransform.TransformDirection (aeVec3 (0, 0, 1))) * mPrevTransform.TransformDirection (aeVec3 (0, 0, 1));
        break;
      };

      s_pCurrentlyDragged->m_Transform.SetTranslationVector (mPrevTransform.GetTranslationVector () + vDist);
    }
    else
    if (s_GizmoMode == Rotate)
    {
      const aeVec3 vBaseNormal = s_pCurrentlyDragged->m_vPivot.GetNormalized ();
      const aeVec3 vNormal = (vPoint - mPrevTransform.GetTranslationVector ()).GetNormalized ();

      float fAngle = vBaseNormal.GetAngleBetween (vNormal);

      aePlane SignPlane;

      switch (s_DragAxis)
      {
      case 3:
        SignPlane.CreatePlane (mPrevTransform.TransformDirection (aeVec3 (0, 0, 1)).Cross (vBaseNormal).GetNormalized (), mPrevTransform.GetTranslationVector ());
        break;
      case 4:
        SignPlane.CreatePlane (mPrevTransform.TransformDirection (aeVec3 (0, 1, 0)).Cross (vBaseNormal).GetNormalized (), mPrevTransform.GetTranslationVector ());
        break;
      case 5:
        SignPlane.CreatePlane (mPrevTransform.TransformDirection (aeVec3 (1, 0, 0)).Cross (vBaseNormal).GetNormalized (), mPrevTransform.GetTranslationVector ());
        break;
      };

      fAngle *= aeMath::Sign (SignPlane.GetDistanceToPoint (vPoint));

      aeMatrix mRotate;

      switch (s_DragAxis)
      {
      case 3:
        mRotate.SetRotationMatrixZ (fAngle);
        break;
      case 4:
        mRotate.SetRotationMatrixY (fAngle);
        break;
      case 5:
        mRotate.SetRotationMatrixX (fAngle);
        break;
      };

      s_pCurrentlyDragged->m_Transform = mPrevTransform * mRotate;
      s_pCurrentlyDragged->m_vPivot = vNormal;
    }

    if (!s_pCurrentlyDragged->m_Transform.IsValid ())
      s_pCurrentlyDragged->m_Transform = mPrevTransform;
    else
    if (s_pCurrentlyDragged->m_Transform != mPrevTransform)
    {
      EventData ed;
      ed.m_Gizmo = s_pCurrentlyDragged;
      ed.m_EventType = Transformed;
      ed.m_PrevTransform = mPrevTransform;
      s_pCurrentlyDragged->m_Events.RaiseEvent (&ed);
    }
  }

  void aeTransformGizmo::Reset (void* pObject, aeUInt32 uiObjectID, const aeMatrix& mCurTransform, bool bActiveState)
  {
    EventData ed;
    ed.m_Gizmo = this;
    ed.m_PrevTransform = m_Transform;
    ed.m_EventType = aeTransformGizmo::BeforeReset;
    m_Events.RaiseEvent (&ed);

    m_bActive = bActiveState;
    m_pObject = pObject;
    m_uiObjectID = uiObjectID;
    m_Transform = mCurTransform;

    ed.m_PrevTransform = m_Transform;
    ed.m_EventType = aeTransformGizmo::AfterReset;
    m_Events.RaiseEvent (&ed);
  }

  void aeTransformGizmo::SetActive (bool bActive)
  {
    if (bActive == m_bActive)
      return;

    m_bActive = bActive;

    EventData ed;
    ed.m_Gizmo = this;
    ed.m_PrevTransform = m_Transform;
    ed.m_EventType = StatusChanged;
    
    m_Events.RaiseEvent (&ed);
  }

  void aeTransformGizmo::SetTransform (const aeMatrix& m)
  {
    if (m_Transform == m)
      return;

    EventData ed;
    ed.m_Gizmo = this;
    ed.m_PrevTransform = m_Transform;
    ed.m_EventType = Transformed;

    m_Transform = m;
    
    m_Events.RaiseEvent (&ed);
  }
}


