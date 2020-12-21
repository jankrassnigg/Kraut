#ifndef TREEPLUGIN_BRANCHLOD_H
#define TREEPLUGIN_BRANCHLOD_H

#include <KrautGraphics/VertexArray/Declarations.h>
#include <KrautGraphics/ShaderManager/Declarations.h>
#include <KrautFoundation/Containers/HybridArray.h>
#include "../Mesh/MeshData.h"
#include "../Other/BoundingBox.h"
#include "BranchNode.h"

using namespace AE_NS_GRAPHICS;

struct aeBranchLod
{
  aeBranchLod ();
  void Reset (void);

  aeDeque<aeInt32> m_NodeIDs;
  aeDeque<aeBranchNode> m_TipNodes;
  aeVertexArrayResourceHandle m_hMeshes[aeMeshType::ENUM_COUNT];
  aeBBox m_MeshBBox[aeMeshType::ENUM_COUNT];
  aeSubMesh m_SubMesh[aeMeshType::ENUM_COUNT];
  bool m_bMeshGenerated[aeMeshType::ENUM_COUNT];
  bool m_bMeshGeneratedTrunkCaps;
  bool m_bMeshGeneratedBranchCaps;
  bool m_bSkeletonGenerated;
};

struct aeLodData
{
  aeLodData ();
  void Reset (aeLod::Enum lod);

  void Load (aeStreamIn& s, aeLod::Enum lod);
  void Save (aeStreamOut& s);

  aeLodMode::Enum m_Mode;         //!< How to represent the tree in this lod.
  float m_fTipDetail;             //!< in 'meters per ring', e.g. 0.04 for one ring every 4 centimeters
  float m_fCurvatureThreshold;    //!< in degree deviation, 0 -> full detail, 5 -> merge branches with less than 5 degree difference
  float m_fThicknessThreshold;    //!< in percent deviation, between 0.0 and 1.0
  float m_fVertexRingDetail;      //!< in 'distance between vertices on the vertex ring'
  aeFlags32 m_AllowTypes[aeMeshType::ENUM_COUNT]; //!< Bitfield that says which branch types are allowed in this LOD 

  aeInt8 m_iMaxFrondDetail;       //!< The value to clamp the "frond detail" value to
  aeInt8 m_iFrondDetailReduction; //!<By how much to reduce the frond detail (before clamping)

  aeUInt32 m_uiLodDistance;       //!< At which distance (in meters) this LOD should be used
};

#endif

