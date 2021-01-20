#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../GUI/ProgressBar.h"
#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include "../Tree/Tree.h"
#include "Forces.h"
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include <KrautEditorBasics/Plugin.h>
#include <KrautGraphics/Culling/AABB_Computer.h>
#include <KrautGraphics/FileSystem/FileSystem.h>

using namespace NS_GLIM;
using namespace NS_OBJLOADER;
using namespace AE_NS_FOUNDATION;


aeForce* aeForce::s_pSelectedForce = nullptr;

static void OnForceEvent(void* pPassThrough, const void* pEventData)
{
  aeForce* pObj = (aeForce*)pPassThrough;
  aeTransformGizmo::EventData* pEvent = (aeTransformGizmo::EventData*)pEventData;

  if (pEvent->m_EventType == aeTransformGizmo::Transformed)
  {
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
  }

  if (pEvent->m_EventType == aeTransformGizmo::DragBegin)
    pObj->SetIsDragged(true);

  if (pEvent->m_EventType == aeTransformGizmo::DragEnd)
    pObj->SetIsDragged(false);
}


aeForce::aeForce()
{
  m_Gizmo.m_Events.RegisterEventReceiver(OnForceEvent, this);
  m_Type = aeForceType::Position;
  m_fMinRadius = 0.3f;
  m_fMaxRadius = 2.0f;
  m_fStrength = 0.1f;
  m_InfluencesBranchTypes.SetFlags(0xFFFFFFFF);
  m_bForceIsActive = true;
  m_sName = "Force";
  m_bIsCurrentlyDragged = false;
  m_Falloff = aeForceFalloff::Quadratic;

  m_uiPickingID = aeEditorPlugin::RegisterPickableObject(this, 0, "aeForce");
}

aeForce::~aeForce()
{
  if (m_uiPickingID != 0)
    aeEditorPlugin::UnregisterPickableObject(m_uiPickingID);
}

void aeForce::SetIsDragged(bool b)
{
  m_bIsCurrentlyDragged = b;
  m_Render.Clear();
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void aeForce::SetSelectedForce(aeForce* pNewSelection)
{
  if (s_pSelectedForce == pNewSelection)
    return;

  aeForce* pOldSelection = s_pSelectedForce;
  s_pSelectedForce = pNewSelection;

  if (pOldSelection)
    pOldSelection->m_Gizmo.SetActive(false);

  if (pNewSelection)
    pNewSelection->m_Gizmo.SetActive(true);

  AE_BROADCAST_EVENT(aeTreeEdit_SelectedForceChanged);
}

const aeHybridString<32> aeForce::GetDescription() const
{
  return m_sName;
}

void aeTree::RemoveSelectedForce(void)
{
  if (!aeForce::GetSelectedForce())
    return;

  const aeForce* pSelected = aeForce::GetSelectedForce();
  aeForce::SetSelectedForce(nullptr);

  for (aeUInt32 i = 0; i < m_Descriptor.m_Forces.size(); ++i)
  {
    if (m_Descriptor.m_Forces[i] == pSelected)
    {
      delete pSelected;

      for (aeUInt32 i2 = i + 1; i2 < m_Descriptor.m_Forces.size(); ++i2)
        m_Descriptor.m_Forces[i2 - 1] = m_Descriptor.m_Forces[i2];

      m_Descriptor.m_Forces.pop_back();
    }
  }

  AE_BROADCAST_EVENT(aeTreeEdit_ForceListChanged);
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void aeTree::AddForce()
{
  m_Descriptor.m_Forces.push_back(new aeForce());

  aeForce::SetSelectedForce(m_Descriptor.m_Forces.back());

  AE_BROADCAST_EVENT(aeTreeEdit_ForceListChanged);
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void aeForce::SetType(aeForceType::Enum t)
{
  m_Type = t;

  HasChanged();
}

void aeForce::SetFalloff(aeForceFalloff::Enum ff)
{
  m_Falloff = ff;

  HasChanged();
}

void aeForce::ReactivateAllMeshSamples(void)
{
  for (aeUInt32 s = 0; s < m_MeshSamples.size(); ++s)
    m_MeshSamples[s].m_bActive = true;
}

bool aeForce::MeshSamplePointCallback(void* pPassThrough, aeDynamicTreeObjectConst Object)
{
  aeDeque<aeUInt32>* SamplesInRange = (aeDeque<aeUInt32>*)pPassThrough;
  SamplesInRange->push_back(Object->second.m_iObjectInstance);

  return true;
}

const aeVec3 aeForce::GetForceAtMesh(const aeVec3& vPosition0) const
{
  if (m_MeshSamples.empty())
    return aeVec3(0.0f);

  const aeVec3 vPosition = m_Gizmo.GetTransform().GetInverse() * vPosition0;

  aeVec3 vForce(0.0f);

  aeDeque<aeUInt32> SamplesInRange;

  m_MeshSampleTree.FindObjectsInRange(vPosition, m_fMaxRadius, MeshSamplePointCallback, &SamplesInRange);

  float fClosestDistance = aeMath::Infinity();

  for (aeUInt32 s0 = 0; s0 < SamplesInRange.size(); ++s0)
  {
    const aeUInt32 s = SamplesInRange[s0];

    if (!m_MeshSamples[s].m_bActive)
      continue;

    const aeVec3 vSamplePos = m_MeshSamples[s].m_vPosition + m_MeshSamples[s].m_vNormal * 0.0f;

    const aePlane Plane(m_MeshSamples[s].m_vNormal, vSamplePos - m_MeshSamples[s].m_vNormal * 0.1f);

    if (Plane.GetPointPosition(vPosition) != aePositionOnPlane::Front)
      continue;

    const aeVec3 vDirection = vSamplePos - vPosition;
    const float fDistance = vDirection.GetLength();

    if (fDistance > fClosestDistance)
      continue;

    fClosestDistance = fDistance;

    vForce.SetZero();

    const aeVec3 vNormDir = vDirection.GetNormalized();

    const aeVec3 vMaxForce = vNormDir * m_fStrength;

    if (m_Falloff == aeForceFalloff::None)
    {
      vForce += vMaxForce;
      continue;
    }

    if (fDistance >= m_fMaxRadius)
      continue;

    if (m_Falloff == aeForceFalloff::Hard)
    {
      if (fDistance <= m_fMaxRadius)
      {
        m_MeshSamples[s].m_bActive = false;
        vForce += vMaxForce;
      }

      continue;
    }

    const float fLinearFactor = 1.0f - ((fDistance - m_fMinRadius) / (m_fMaxRadius - m_fMinRadius));

    if (m_Falloff == aeForceFalloff::Linear)
    {
      if (fDistance <= m_fMinRadius)
      {
        m_MeshSamples[s].m_bActive = false;
        vForce += vMaxForce;
      }
      else
        vForce += vNormDir * m_fStrength * fLinearFactor;

      continue;
    }

    if (m_Falloff == aeForceFalloff::Quadratic)
    {
      if (fDistance <= m_fMinRadius)
      {
        m_MeshSamples[s].m_bActive = false;
        vForce += vMaxForce;
      }
      else
        vForce += vNormDir * m_fStrength * aeMath::Square(fLinearFactor);

      continue;
    }
  }

  return m_Gizmo.GetTransform().TransformDirection(vForce);
}

void aeTreeDescriptor::RemoveAllForces(void)
{
  aeForce::SetSelectedForce(nullptr);

  for (aeUInt32 i = 0; i < m_Forces.size(); ++i)
    delete m_Forces[i];

  m_Forces.clear();
}


void aeForce::Save(aeStreamOut& stream)
{
  const aeUInt8 uiVersion = 3;

  stream << uiVersion;

  stream << m_sName;
  stream << m_bForceIsActive;

  const aeInt8 iType = m_Type;
  stream << iType;

  const aeUInt32 uiInfluences = m_InfluencesBranchTypes.GetData();
  stream << uiInfluences;

  stream << m_fStrength;
  stream << m_fMinRadius;
  stream << m_fMaxRadius;
  stream << m_Gizmo.GetTransform();

  // Version 2
  const aeInt8 iFalloff = m_Falloff;
  stream << iFalloff;

  // Version 3
  stream << m_sMesh;
}

void aeForce::Load(aeStreamIn& stream)
{
  aeUInt8 uiVersion = 0;

  stream >> uiVersion;

  aeString s;
  stream >> s;
  m_sName = s;

  stream >> m_bForceIsActive;

  aeInt8 iType = 0;
  stream >> iType;
  m_Type = (aeForceType::Enum)iType;

  aeUInt32 uiInfluences = 0;
  stream >> uiInfluences;
  m_InfluencesBranchTypes.SetFlags(uiInfluences);

  stream >> m_fStrength;
  stream >> m_fMinRadius;
  stream >> m_fMaxRadius;

  aeMatrix mTransform;
  stream >> mTransform;
  m_Gizmo.SetTransform(mTransform);

  if (uiVersion >= 2)
  {
    aeInt8 iFalloff = 0;
    stream >> iFalloff;
    m_Falloff = (aeForceFalloff::Enum)iFalloff;
  }

  if (uiVersion >= 3)
  {
    aeString sMesh;
    stream >> sMesh;

    SetMesh(sMesh.c_str());
  }
}

void aeForce::HasChanged(void)
{
  m_Render.Clear();
  AE_BROADCAST_EVENT(aeTreeEdit_SelectedForceModified);
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void aeForce::Render(bool bForPicking)
{
  aeRenderAPI::GetMatrix_ObjectToWorld().PushMatrix();

  if (!bForPicking)
  {
    aeShaderManager::setShader(g_Globals.s_hForceShader);

    aeRenderAPI::PushRenderStates(false);
    aeRenderAPI::setBlending(true, AE_FACTOR_SRC_ALPHA, AE_FACTOR_INV_SRC_ALPHA);

    if (m_Render.IsCleared())
    {
      m_Render.BeginBatch();

      if (m_bIsCurrentlyDragged)
        m_Render.Attribute4ub("attr_Color", 50, 50, 200, 200);
      else
        m_Render.Attribute4ub("attr_Color", 50, 50, 200, 50);

      if ((m_fMinRadius > 0.0f) && ((m_Falloff == aeForceFalloff::Linear) || (m_Falloff == aeForceFalloff::Quadratic)))
      {
        if (m_bIsCurrentlyDragged)
          m_Render.Attribute4ub("attr_Color", 200, 50, 50, 200);
        else
          m_Render.Attribute4ub("attr_Color", 200, 50, 50, 50);

        const aeInt32 iSteps = aeMath::Max<aeInt32>(12, (m_fMinRadius * 2 * aeMath::PI()) / 0.25f);

        if (m_bIsCurrentlyDragged)
          m_Render.Begin(GLIM_POLYGON);
        else
          m_Render.Begin(GLIM_LINE_LOOP);

        const float fAngleStep = 360.0f / iSteps;
        for (aeInt32 i = 0; i < iSteps; ++i)
        {
          const float fAngle = i * fAngleStep;

          const aeVec3 v = aeVec3(aeMath::CosDeg(fAngle), aeMath::SinDeg(fAngle), 0) * m_fMinRadius;

          m_Render.Vertex(v.x, v.y, v.z);
        }

        m_Render.End();
      }

      if ((m_fMaxRadius > 0.0f) && (m_Falloff != aeForceFalloff::None))
      {
        if (m_bIsCurrentlyDragged)
          m_Render.Attribute4ub("attr_Color", 50, 50, 200, 200);
        else
          m_Render.Attribute4ub("attr_Color", 50, 50, 200, 50);

        const aeInt32 iSteps = aeMath::Max<aeInt32>(12, (m_fMaxRadius * 2 * aeMath::PI()) / 0.25f);

        //if (m_bIsCurrentlyDragged)
        //m_Render.Begin (GLIM_POLYGON);
        //else
        m_Render.Begin(GLIM_LINE_LOOP);

        const float fAngleStep = 360.0f / iSteps;
        for (aeInt32 i = 0; i < iSteps; ++i)
        {
          const float fAngle = i * fAngleStep;

          const aeVec3 v = aeVec3(aeMath::CosDeg(fAngle), aeMath::SinDeg(fAngle), 0) * m_fMaxRadius;

          m_Render.Vertex(v.x, v.y, v.z);
        }

        m_Render.End();
      }

      m_Render.EndBatch();
    }

    aeMatrix mTransform;
    mTransform.SetIdentity();
    mTransform.SetTranslationVector(m_Gizmo.GetTransform().GetTranslationVector());

    aeRenderAPI::GetMatrix_ObjectToWorld().LoadMatrix(mTransform);

    m_Render.RenderBatch();

    aeRenderAPI::PopRenderStates();
  }

  aeShaderManager::setShader(g_Globals.s_hSimpleGizmoShader);

  aeRenderAPI::GetMatrix_ObjectToWorld().LoadMatrix(m_Gizmo.GetTransform());

  aeShaderManager::setUniformUB("unif_PickingID", (m_uiPickingID >> 0) & 0xFF, (m_uiPickingID >> 8) & 0xFF, (m_uiPickingID >> 16) & 0xFF, (m_uiPickingID >> 24) & 0xFF);
  aeShaderManager::setUniformUB("unif_PickingSubID", 0, 0, 0, 0);

  if (!m_MeshSamples.empty())
  {
    if (m_MeshSampleRender.IsCleared())
    {
      m_MeshSampleRender.BeginBatch();
      m_MeshSampleRender.Attribute4ub("attr_Color", 255, 255, 0, 255);
      m_MeshSampleRender.Begin(GLIM_POINTS);

      for (aeUInt32 i = 0; i < m_MeshSamples.size(); ++i)
      {
        if (m_MeshSamples[i].m_bActive)
          m_MeshSampleRender.Attribute4ub("attr_Color", 255, 255, 0, 255);
        else
          m_MeshSampleRender.Attribute4ub("attr_Color", 255, 0, 0, 255);

        const aeVec3 v = m_MeshSamples[i].m_vPosition + m_MeshSamples[i].m_vNormal * 0.05f;
        m_MeshSampleRender.Vertex(v.x, v.y, v.z);
      }

      m_MeshSampleRender.End();
      m_MeshSampleRender.EndBatch();
    }

    m_MeshSampleRender.RenderBatch();
  }

  switch (m_Type)
  {
    case aeForceType::Position:
      g_Globals.s_GizmoMagnet.RenderBatch();
      break;
    case aeForceType::Direction:
      g_Globals.s_GizmoDirection.RenderBatch();
      break;
  }

  if ((!m_MeshRender.IsCleared()) && (m_Type == aeForceType::Mesh))
  {
    aeShaderManager::setShader(g_Globals.s_hPhysicsObjectShader);

    //aeMatrix mScale;
    //mScale.SetScalingMatrix (g_PhysicsObjects[o]->GetScaling ().x, g_PhysicsObjects[o]->GetScaling ().y, g_PhysicsObjects[o]->GetScaling ().z);
    //aeRenderAPI::GetMatrix_ObjectToWorld ().MultMatrix (mScale);
    aeShaderManager::setShaderBuilderVariable("SELECTED", aeForce::GetSelectedForce() == this);
    m_MeshRender.RenderBatch();
  }

  aeRenderAPI::GetMatrix_ObjectToWorld().PopMatrix();
}

void aeForce::SetMesh(const char* szMesh)
{
  m_sMesh = szMesh;
  m_MeshSamples.clear();
  m_MeshSampleTree.RemoveAllObjects();
  m_MeshSampleRender.Clear();

  aeString sAbsolutePath;
  if (!aeFileSystem::MakeValidPath(szMesh, false, &sAbsolutePath, nullptr))
    return;

  aeProgressBar pb("Importing Mesh", 3);

  MESH obj;
  std::map<std::string, MATERIAL> Materials;
  if (!LoadOBJFile(sAbsolutePath.c_str(), obj, Materials, "", false, true))
    return;

  aeProgressBar::Update(); // 1

  m_MeshRender.BeginBatch();
  m_MeshRender.Attribute4ub("attr_Color", 128, 128, 128, 255);
  m_MeshRender.Attribute4ub("attr_PickingID", (m_uiPickingID >> 0) & 0xFF, (m_uiPickingID >> 8) & 0xFF, (m_uiPickingID >> 16) & 0xFF, (m_uiPickingID >> 24) & 0xFF);

  for (aeUInt32 f = 0; f < obj.m_Faces.size(); ++f)
  {
    m_MeshRender.Begin(GLIM_POLYGON);

    for (aeUInt32 v = 0; v < obj.m_Faces[f].m_Vertices.size(); ++v)
    {
      const VEC3 pos = /*fScale * */ obj.m_Positions[obj.m_Faces[f].m_Vertices[v].m_uiPositionID];
      m_MeshRender.Vertex(pos.x, pos.y, pos.z);
    }

    m_MeshRender.End();
  }

  m_MeshRender.EndBatch();

  aeProgressBar::Update(); // 2

  m_ImportedMesh = obj;
  GenerateMeshSamples();

  aeProgressBar::Update(); // 3

  HasChanged();
}

inline static aeVec3 ToVec3(const VEC3& v)
{
  return aeVec3(v.x, v.y, v.z);
}

static void ComputeFaceFrame(const NS_OBJLOADER::MESH& Mesh, const FACE& ThisFace, float& fMinX, float& fMaxX, float& fMinY, float& fMaxY, aeVec3& vTangent1, aeVec3& vTangent2)
{
  const aeVec3 vNormal(ToVec3(ThisFace.m_vNormal));

  const aeVec3 vPos1 = ToVec3(Mesh.m_Positions[ThisFace.m_Vertices[0].m_uiPositionID]);
  const aeVec3 vPos2 = ToVec3(Mesh.m_Positions[ThisFace.m_Vertices[1].m_uiPositionID]);

  vTangent1 = (vPos2 - vPos1).GetNormalized();
  vTangent2 = vNormal.Cross(vTangent1).GetNormalized();

  const aePlane Plane1(vTangent1, vPos1);
  const aePlane Plane2(vTangent2, vPos1);

  fMinX = aeMath::Infinity();
  fMaxX = -aeMath::Infinity();
  fMinY = aeMath::Infinity();
  fMaxY = -aeMath::Infinity();

  for (aeUInt32 vert = 0; vert < ThisFace.m_Vertices.size(); ++vert)
  {
    const aeVec3 vPos = ToVec3(Mesh.m_Positions[ThisFace.m_Vertices[vert].m_uiPositionID]);

    const float fDistX = Plane1.GetDistanceToPoint(vPos);
    const float fDistY = Plane2.GetDistanceToPoint(vPos);

    fMinX = aeMath::Min(fMinX, fDistX);
    fMaxX = aeMath::Max(fMaxX, fDistX);
    fMinY = aeMath::Min(fMinY, fDistY);
    fMaxY = aeMath::Max(fMaxY, fDistY);
  }
}

static inline void CopyFaceVertices(const NS_OBJLOADER::MESH& Mesh, const FACE& ThisFace, aeHybridArray<aeVec3, 16>& FaceVertices)
{
  FaceVertices.clear();

  for (aeUInt32 i = 0; i < ThisFace.m_Vertices.size(); ++i)
  {
    FaceVertices.push_back(ToVec3(Mesh.m_Positions[ThisFace.m_Vertices[i].m_uiPositionID]));
  }
}

void aeForce::GenerateMeshSamples(void)
{
  m_MeshSampleRender.Clear();
  m_MeshSamples.clear();
  m_MeshSampleTree.RemoveAllObjects();

  if (m_ImportedMesh.m_Faces.size() == 0)
    return;

  float fMinX, fMaxX, fMinY, fMaxY;
  aeHybridArray<aeVec3, 16> FaceVertices;

  const float fSpacing = 0.2f;

  aeProgressBar pb("Generating Mesh Samples", (aeUInt32)m_ImportedMesh.m_Faces.size(), 100);

  aeAABBComputer bbox;

  for (aeUInt32 face = 0; face < m_ImportedMesh.m_Faces.size(); ++face)
  {
    const FACE& ThisFace = m_ImportedMesh.m_Faces[face];
    const aeVec3 vNormal(ToVec3(ThisFace.m_vNormal));
    aeVec3 vTangent1;
    aeVec3 vTangent2;

    ComputeFaceFrame(m_ImportedMesh, ThisFace, fMinX, fMaxX, fMinY, fMaxY, vTangent1, vTangent2);
    CopyFaceVertices(m_ImportedMesh, ThisFace, FaceVertices);

    const aeVec3 vStart = ToVec3(m_ImportedMesh.m_Positions[ThisFace.m_Vertices[0].m_uiPositionID]);

    for (float x = aeMath::Floor(fMinX, fSpacing) + fSpacing * 0.5f; x < aeMath::Ceil(fMaxX, fSpacing); x += fSpacing)
    {
      for (float y = aeMath::Floor(fMinY, fSpacing) + fSpacing * 0.5f; y < aeMath::Ceil(fMaxY, fSpacing); y += fSpacing)
      {
        const aeVec3 vCurPos = vStart + vTangent1 * x + vTangent2 * y;

        if (aeMath::IsPointInPolygon(FaceVertices.data(), FaceVertices.size(), vCurPos))
        {
          aeMeshSample s;
          s.m_vPosition = vCurPos + vNormal * 0.1f;
          s.m_vNormal = vNormal;

          bbox.AddPoint(s.m_vPosition);

          m_MeshSamples.push_back(s);
        }
      }
    }

    aeProgressBar::Update();
  }

  aeVec3 vBBoxCenter, vBBoxExtents;
  bbox.GetAABBCenterAndExtents(vBBoxCenter, vBBoxExtents);

  m_MeshSampleTree.CreateTree(vBBoxCenter, vBBoxExtents + aeVec3(0.25f), 1.0f);

  for (aeUInt32 i = 0; i < m_MeshSamples.size(); ++i)
  {
    m_MeshSampleTree.InsertObject(m_MeshSamples[i].m_vPosition, aeVec3(0.0f), 0, i, false);
  }
}
