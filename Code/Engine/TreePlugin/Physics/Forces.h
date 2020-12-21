#ifndef FORCES_H
#define FORCES_H

#include "../Basics/Base.h"
#include <KrautGraphics/glim/glim.h>
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include "../Other/OBJ/OBJLoader.h"
#include <KrautFoundation/DynamicTree/DynamicOctree.h>

using namespace AE_NS_FOUNDATION;
using namespace NS_GLIM;
using namespace AE_NS_EDITORBASICS;

class QListWidgetItem;

class aeForce
{
public:
  aeForce ();
  ~aeForce ();

  void Save (aeStreamOut& stream);
  void Load (aeStreamIn& stream);

  const aeHybridString<32> GetDescription () const;

  const aeVec3 GetForceAt (const aeVec3& vPosition, aeUInt32 uiBranchType) const;

  static void SetSelectedForce (aeForce* pForce);

  static aeForce* GetSelectedForce (void) { return s_pSelectedForce; }

  void SetType (aeForceType::Enum t);
  aeForceType::Enum GetType (void) const { return (m_Type); }
  float GetStrength (void) const { return m_fStrength; }
  aeForceFalloff::Enum GetFalloff (void) const { return m_Falloff; }
  void SetFalloff (aeForceFalloff::Enum ff);
  void SetStrength (float f) { m_fStrength = f; HasChanged (); }
  void SetMinRadius (float f) { m_fMinRadius = f; m_fMaxRadius = aeMath::Max (m_fMaxRadius, f); HasChanged (); }
  void SetMaxRadius (float f) { m_fMaxRadius = f; m_fMinRadius = aeMath::Min (m_fMinRadius, f); HasChanged (); }

  float GetMinRadius (void) const { return m_fMinRadius; }
  float GetMaxRadius (void) const { return m_fMaxRadius; }

  void SetName (const char* szName) { m_sName = szName; HasChanged (); }
  void SetActive (bool b) { if (b != m_bForceIsActive) { m_bForceIsActive = b; HasChanged (); } }

  void Render (bool bForPicking);
  void SetIsDragged (bool b);

  void SetMesh (const char* szMesh);

  void ReactivateAllMeshSamples (void);

  bool EnableMinRadius (void) const { return (m_Falloff != aeForceFalloff::Hard) && (m_Falloff != aeForceFalloff::None); }
  bool EnableMaxRadius (void) const { return (m_Falloff != aeForceFalloff::None); }
  bool EnableMeshSelector (void) const { return (m_Type == aeForceType::Mesh); }

  const char* GetMesh (void) const { return m_sMesh.c_str (); }

  bool IsActive (void) const { return m_bForceIsActive; }

  void SetBranchInfluences (aeUInt32 uiInfluences) { m_InfluencesBranchTypes.SetFlags (uiInfluences); AE_BROADCAST_EVENT (aeTreeEdit_TreeModified); }

private:
  void HasChanged (void);
  void GenerateMeshSamples (void);

  static bool MeshSamplePointCallback (void* pPassThrough, aeDynamicTreeObjectConst Object);

  const aeVec3 GetForceAtMesh (const aeVec3& vPosition) const;

  friend struct aeTreeDescriptor;
  friend class qtTreeEditWidget;

  GLIM_BATCH m_Render;
  GLIM_BATCH m_MeshRender;
  GLIM_BATCH m_MeshSampleRender;
  NS_OBJLOADER::MESH m_ImportedMesh;

  aeHybridString<128> m_sMesh;
  aeUInt32 m_uiPickingID;
  aeHybridString<32> m_sName;
  bool m_bForceIsActive;
  bool m_bIsCurrentlyDragged;

  aeForceFalloff::Enum m_Falloff;
  aeForceType::Enum m_Type;
  aeTransformGizmo m_Gizmo;

  aeFlags32 m_InfluencesBranchTypes;
  float m_fStrength;
  float m_fMinRadius;
  float m_fMaxRadius;

  QListWidgetItem* m_pListItem;

  struct aeMeshSample
  {
    aeVec3 m_vPosition;
    aeVec3 m_vNormal;
    mutable bool m_bActive;
  };

  aeDynamicOctree m_MeshSampleTree;
  aeDeque<aeMeshSample> m_MeshSamples;

  static aeForce* s_pSelectedForce;
};

#endif

