#ifndef MESHDATA_H
#define MESHDATA_H

#include <KrautFoundation/Math/Vec3.h>

struct aeMeshVertex
{
  aeMeshVertex ()
  {
    m_vPosition.SetZero ();
    m_vTexCoord.SetVector (0, 0, 1);
    m_vNormal.SetZero ();
    m_vTangent.SetZero ();
    m_vBiTangent.SetZero ();
    m_iSharedVertex = -1;

    m_uiColorVariation = 0;
  }

  aeVec3 m_vPosition;
  aeVec3 m_vTexCoord;
  aeVec3 m_vNormal;
  aeVec3 m_vTangent;
  aeVec3 m_vBiTangent;
  aeInt32 m_iSharedVertex;

  aeUInt8 m_uiColorVariation;
};

struct aeMeshTriangle
{
  aeMeshTriangle ()
  {
    m_uiPickingID = 0;
    m_uiPickingSubID = 0;
  }

  void Flip (void)
  {
    aeMath::Swap (m_uiVertexIDs[1], m_uiVertexIDs[2]);
  }

  aeUInt32 m_uiVertexIDs[3];

  aeUInt32 m_uiPickingID;
  aeUInt32 m_uiPickingSubID;
};

struct aeSubMesh
{
  aeUInt32 AddVertex (aeMeshVertex vtx)
  {
    AE_CHECK_DEV (vtx.m_vPosition.IsValid (), "aeSubMesh::AddVertex: Position is degenerate.");
    AE_CHECK_DEV (vtx.m_vNormal.IsValid (), "aeSubMesh::AddVertex: Normal is degenerate.");
    AE_CHECK_DEV (vtx.m_vTangent.IsValid (), "aeSubMesh::AddVertex: Tangent is degenerate: %.8f | %.8f | %.8f", vtx.m_vTangent.x, vtx.m_vTangent.y, vtx.m_vTangent.z);
    AE_CHECK_DEV (vtx.m_vBiTangent.IsValid (), "aeSubMesh::AddVertex: BiTangent is degenerate.");
    AE_CHECK_DEV (vtx.m_vTexCoord.IsValid (), "aeSubMesh::AddVertex: TexCoord is degenerate.");

    if (vtx.m_iSharedVertex == -1)
      vtx.m_iSharedVertex = (aeInt32) m_Vertices.size ();

    m_Vertices.push_back (vtx);
    return ((aeUInt32) m_Vertices.size () - 1);
  }

  aeUInt32 AddVertex (aeMeshVertex vtx, aeInt32& iWriteBack)
  {
    aeUInt32 ui = AddVertex (vtx);

    if (iWriteBack == -1)
      iWriteBack = (aeInt32) ui;

    return ui;
  }

  void clear ()
  {
    m_Vertices.clear ();
    m_Triangles.clear ();
  }

  aeUInt32 GetNumTriangles () const
  {
    return (aeUInt32) m_Triangles.size ();
  }

  void GenerateVertexNormals (void);

  aeDeque<aeMeshVertex> m_Vertices;
  aeDeque<aeMeshTriangle> m_Triangles;
};


struct aeVertexRing
{
  aeHybridArray<aeVec3, 64> m_Vertices;
  aeHybridArray<aeInt32, 64> m_VertexIDs;
  aeHybridArray<aeVec3, 64> m_Normals;
  float m_fDiameter;
};



#endif

