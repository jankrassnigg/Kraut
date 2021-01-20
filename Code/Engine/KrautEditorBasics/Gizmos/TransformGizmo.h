#ifndef AE_TREEPLUGIN_GIZMO_H
#define AE_TREEPLUGIN_GIZMO_H

#include "../Base.h"
#include <KrautGraphics/ShaderManager/Declarations.h>
#include <KrautGraphics/Utility/EnumerableClass.h>
#include <KrautGraphics/glim/glim.h>

namespace AE_NS_EDITORBASICS
{
  using namespace NS_GLIM;
  using namespace AE_NS_GRAPHICS;

  AE_ENUMERABLE_CLASS(AE_EDITORBASICS_DLL, aeTransformGizmo);

  class AE_EDITORBASICS_DLL aeTransformGizmo : AE_MAKE_ENUMERABLE(aeTransformGizmo)
  {
  public:
    enum Mode
    {
      Invisible,
      Translate,
      Rotate,
    };

    enum UsageFlags
    {
      TranslateX = 1 << 0,
      TranslateY = 1 << 1,
      TranslateZ = 1 << 2,
      TranslateAll = TranslateX | TranslateY | TranslateZ,
      RotateX = 1 << 3,
      RotateY = 1 << 4,
      RotateZ = 1 << 5,
      RotateAll = RotateX | RotateY | RotateZ,
      TransformAll = TranslateAll | RotateAll,
    };

    enum EventType
    {
      BeforeReset,
      AfterReset,
      DragBegin,
      DragEnd,
      Transformed,
      StatusChanged,
    };

    struct EventData
    {
      EventType m_EventType;
      aeTransformGizmo* m_Gizmo;
      aeMatrix m_PrevTransform;
    };

    enum GizmoAxis
    {
      GizmoAxisX,
      GizmoAxisY,
      GizmoAxisZ,
      GizmoAxisXY,
      GizmoAxisXZ,
      GizmoAxisYZ,
    };

  public:
    aeTransformGizmo();
    ~aeTransformGizmo();

    //! The current usage mode for all gizmos.
    static Mode s_GizmoMode;

    //! Renders all active gizmos.
    static void RenderAllGizmos(void);

    //! Sets to which object this gizmo belongs.
    void Reset(void* pObject, aeUInt32 uiObjectID, const aeMatrix& mCurTransform, bool bActiveState);

    //! Renders the gizmo, if it is active.
    void Render(bool bWireframe);

    //! Begins dragging the gizmo to the position on screen. Will trigger an event and return true, if it was possible to begin dragging.
    bool BeginDrag(GizmoAxis Axis, aeVec3 vCameraPos, aeInt32 uiScreenPosX, aeInt32 uiScreenPosY);
    //! Stops dragging any gizmo. Can be called even if no gizmo is currently being dragged. Will trigger an event, if there was an active gizmo.
    static void EndDrag(void);
    //! Drags the current gizmo to the specified position (on screen). Will trigger an event, if the transform has changed.
    static void DragTo(aeInt32 uiScreenPosX, aeInt32 uiScreenPosY);

    //! Returns the current transformation.
    const aeMatrix& GetTransform() const { return m_Transform; }
    //! Sets the current transformation directly. Will trigger an event, if the transform has changed.
    void SetTransform(const aeMatrix& m);

    //! Returns the object pointer associated with this gizmo.
    void* GetObjectPointer() const { return m_pObject; }
    //! Returns the object id associated with this gizmo.
    aeUInt32 GetObjectID() const { return m_uiObjectID; }

    //! Sets the active status. Will trigger an event, if it has changed.
    void SetActive(bool bActive);

    //! Event dispatcher.
    aeEvent m_Events;

    //! Which axis are active on this gizmo. Can be changed at any time.
    aeUInt16 m_UsageFlags;

    //! Will return the currently dragged gizmo (or nullptr).
    static aeTransformGizmo* GetCurrentlyDraggedGizmo() { return s_pCurrentlyDragged; }

    //! Returns whether currently a gizmo is being dragged.
    static bool IsDraggingGizmo() { return s_pCurrentlyDragged != nullptr; }

    static aeShaderResourceHandle s_hGizmoShader;

  private:
    static aeTransformGizmo* s_pCurrentlyDragged;
    static GizmoAxis s_DragAxis;
    static aeVec3 s_vDragCameraPos;

    void* m_pObject;
    aeUInt32 m_uiObjectID;

    bool m_bActive;

    aeMatrix m_Transform;
    aeUInt16 m_PrevUsageFlags;

    aeVec3 GetIntersectionPoint(GizmoAxis Axis, aeVec3 vCameraPos, aeInt32 uiScreenPosX, aeInt32 uiScreenPosY) const;
    aeVec3 m_vPivot;

    GLIM_BATCH m_GlimTranslate;
    GLIM_BATCH m_GlimRotate;

    aeUInt32 m_uiPickID;
  };
} // namespace AE_NS_EDITORBASICS

#endif
