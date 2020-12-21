#ifndef AE_TREEPLUGIN_TREE_H
#define AE_TREEPLUGIN_TREE_H

#include <KrautGraphics/VertexArray/Declarations.h>
#include <KrautGraphics/glim/glim.h>
#include <KrautGraphics/ShaderManager/Declarations.h>
#include <KrautFoundation/Containers/HybridArray.h>
#include <KrautFoundation/Containers/Map.h>
#include <KrautFoundation/Containers/Set.h>
#include "../Mesh/MeshData.h"
#include "../Other/BoundingBox.h"
#include "BranchLOD.h"
#include "Branch.h"
#include <KrautGraphics/TextureManager/Declarations.h>
#include "../Other/Curve.h"
#include "RandomData.h"
#include "SpawnNode.h"
#include "../Basics/Globals.h"
#include "../Physics/Forces.h"
#include "../GUI/qtResourceEditorWidget/MaterialLibrary.h"

using namespace NS_GLIM;
using namespace AE_NS_GRAPHICS;

struct aeTreeDescriptor
{
  aeTreeDescriptor ();
  void Reset (void);

  void Save (aeStreamOut& s);
  void Load (aeStreamIn& s);

  aeUInt32 m_uiRandomSeed;
  bool m_bLeafCardMode;
  bool m_bGrowProceduralTrunks;
  bool m_bSnapshotFromAbove;
  float m_fBBoxAdjustment;
  float m_fExportScale;
  aeInt8 m_iImpostorResolution; // 0 = 1024, 1 = 512, 2 = 256, ...
  aeLeafCardResolution::Enum m_LeafCardResolution;
  aeLeafCardMipmapResolution::Enum m_LeafCardMipmaps;
  aeUInt8 m_uiLeafCardTexelDilation;

  aeSpawnNodeDesc m_BranchTypes[aeBranchType::ENUM_COUNT];

  // *** Lod ***
  float m_fLodCrossFadeTransition;
  aeLodData m_LodData[aeLod::ENUM_COUNT];
  void ClampLodValues (aeLod::Enum CurLod);

  // *** Ambient Occlusion ***
  bool m_bUseAO;
  float m_fAOSampleSize;
  float m_fAOContrast;

  // *** Forces ***
  aeDeque<aeForce*> m_Forces;

private:
  void RemoveAllForces (void);
};

struct aeKrautMaterial
{
  aeString m_sDiffuseTexture;
  aeString m_sNormalMapTexture;
  aeUInt8 m_uiVariationColor[4];

  bool operator< (const aeKrautMaterial& rhs) const
  {
    const aeUInt32 uiColor1 = m_uiVariationColor[0] | (m_uiVariationColor[1] << 8) | (m_uiVariationColor[2] << 16) | (m_uiVariationColor[3] << 24);
    const aeUInt32 uiColor2 = rhs.m_uiVariationColor[0] | (rhs.m_uiVariationColor[1] << 8) | (rhs.m_uiVariationColor[2] << 16) | (rhs.m_uiVariationColor[3] << 24);

    if (uiColor1 < uiColor2)
      return true;
    if (uiColor1 > uiColor2)
      return false;

    if (m_sDiffuseTexture < rhs.m_sDiffuseTexture)
      return true;
    if (m_sDiffuseTexture > rhs.m_sDiffuseTexture)
      return false;

    return m_sNormalMapTexture < rhs.m_sNormalMapTexture;
  }
};

class aeTree
{
public:
  aeTree ();

public:
  // Tree Export
  
  void ExportToFile (const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4, bool bDDS, aeUInt32 uiImpostorResolution);

private:

  bool ExportOBJ (const char* szFile,   bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4);
  bool ExportFBX (const char* szFile,   bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4);
  bool ExportKraut (const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4);

  void KrautExport_WriteHeader (aeStreamOut& Stream, bool bLod[aeLod::ENUM_COUNT]);
  void KrautExport_GatherMaterials (aeMap<aeKrautMaterial, aeSet<aeUInt32> >* out_Materials);
  void KrautExport_GatherMaterial (aeBranchType::Enum bt, aeMeshType::Enum mt, aeMap<aeKrautMaterial, aeSet<aeUInt32> >& Materials);
  void KrautExport_WriteMaterials (aeStreamOut& Stream, aeMap<aeKrautMaterial, aeSet<aeUInt32> >* in_Materials);
  void KrautExport_WriteMaterial (aeStreamOut& Stream, const aeKrautMaterial& Mat);
  aeUInt8 KrautExport_GatherLODs (bool* bLod);
  void KrautExport_GatherSubMeshesOfType (aeLod::Enum lod, aeMeshType::Enum mt, const aeSet<aeUInt32>& Types, aeDeque<aeSubMesh*>& out_SubMeshes);

public:
  void Reset (void);

  aeFilePath m_sTreeFile;

  void Shutdown (void);

  void DeleteBranch (aeInt32 iBranch, bool bIsRecursion);

  void GenerateTree (bool bUndoableAction);

  bool SearchForBranchWithParent (aeInt32 iParent, aeInt32& iCurNode) const;

  void Save (aeStreamOut& stream);
  void Load (aeStreamIn& stream);

  aePlane GetAdjustedPaintingPlane (aeInt32 iParentBranch, aeUInt32 uiParentBranchNode, const aeVec3& vStartPos, aePlane PaintingPlane);

  aeInt32 AddPaintedBranch (aeInt32 iParentBranch, aeUInt32 uiParentBranchNode, const aeDeque<aeVec3>& vPoints, const aePlane& PaintingPlane, bool bForPreview, aeInt32 iReuseBranch);

  //! The Skeleton was changed due to manual editing -> regenerate the mesh.
  void SkeletonHasChanged (void);

  void SkeletonHasChanged (aeUInt32 uiBranch, bool bInsertUndoStep);

  aeDeque<aeBranch> m_Branches;

  void ChangeBoundingAdjustment (float fNewAdjustment);

  aeBBox m_BBox;
  bool m_bModifiedViaUndo;

  aeTreeDescriptor m_Descriptor;

  aeUInt32 GetNumBones (aeLod::Enum lod) const;
  aeUInt32 GetNumAllTriangles (aeLod::Enum lod) const;
  aeUInt32 GetNumTriangles (aeLod::Enum lod, aeMeshType::Enum mt) const;

  aeUInt32 GetFreeBranch ();

private:
  void GrowSkeleton ();
  aeInt32 GrowBranch (const aeBranchDesc& desc);
  void SpawnSubBranches_Reverse (aeUInt32 uiParentBranch, const aeSpawnNodeDesc& desc1, const aeSpawnNodeDesc& desc2, const aeSpawnNodeDesc& desc3);
  aeInt32 InsertOneBranch (aeUInt32 uiParentBranch, aeUInt32 uiStartNode, const aeBranchDesc& desc, float fBranchDistance, float fRotation, const aeSpawnNodeDesc& NodeDesc, aeBranchRandomData& BranchRD, bool bReverse);
  void InsertBranchNode (aeUInt32 uiParentBranch, aeUInt32 uiStartNode, const aeSpawnNodeDesc& desc, aeBranchNodeRandomData& NodeRD, float fDistAtStartNode, float fBranchlessPartEnd, bool bReverse);
  aeUInt32 DuplicateBranch (aeUInt32 uiBranchID, float fRotation, aeInt32 iBranchBuddy);

  aeVec3 ComputeLeafUpDirection (const aeBranch& Branch, const aeSpawnNodeDesc& bnd, const aeVec3& vGrowDirection);

  void GenerateAllTreeMeshes (aeLod::Enum lod, bool bTrunkCap, bool bBranchCap);
  aeInt32 GenerateVertexRing (aeBranch* pBranch, aeUInt32 uiNode, aeVertexRing* pVertexRing, aeLod::Enum lod, bool bTip, const aeVec3& vNormalAnchor) const;

  void EnsureSkeletonIsGenerated (aeLod::Enum lod);
  void EnsureMeshIsGenerated (aeLod::Enum lod, bool bTrunkCap = false, bool bBranchCap = false);

  void ComputeBoundingBox (void);
  

  // Forces
public:
  const aeVec3 GetAverageForceAt (const aeVec3& vPosition, aeUInt32 uiBranchType);

  void AddForce ();
  void RemoveSelectedForce ();

private:
  // Branch Functions

  void GrowTip (aeUInt32 uiBranch, aeUInt32 iSegments, float fSegmentLength, aeLod::Enum lod);
  bool ComputeBranchThickness (aeUInt32 uiBranch);
  void GrowBranchTip (aeUInt32 uiBranch, aeLod::Enum lod);
  void GenerateFullDetailSkeleton (void);
  void GenerateReducedSkeleton (aeLod::Enum lod);
  void CreateBranchCollisionCapsules (aeUInt32 uiBranch);

private:
  void GenerateReducedNodes (aeUInt32 uiBranch, aeLod::Enum lod);
  void GenerateBranchMesh (aeUInt32 uiBranch, aeLod::Enum lod, bool bGenerateCap);
  void GenerateFrondMesh (aeUInt32 uiBranch, aeLod::Enum lod);
  void ComputeNodeVTextureCoordinates (aeUInt32 uiBranch, aeLod::Enum lod);
  void GenerateFrondMesh (aeUInt32 uiBranch, aeLod::Enum lod, const aeVec3& vUpVector, aeInt32 iCurFrondIndex);

  // Umbrella Mesh
  void GenerateUmbrellaMesh (aeUInt32 uiBranch, aeLod::Enum lod);
  void AddUmbrellaMeshSegment (aeBranch& Branch, aeLod::Enum lod, const aeArray<aeVec3>& Positions1, const aeArray<aeVec3>& Positions2, const aeArray<aeVec3>& Normals1, const aeArray<aeVec3>& Normals2, aeUInt32 uiFirstVertex, aeUInt32 uiSlice, aeUInt32 uiMaxSlices, float fTexCoordBase, float fTexCoordFraction, const aeVec3& vTexCoordDir0, const aeVec3& vTexCoordDir1);

  void GenerateCapTriangles (aeUInt32 uiBranch, aeLod::Enum lod, aeVertexRing& VertexRing);
  void GenerateSegmentTriangles (aeLod::Enum lod, aeVertexRing& VertexRing0, aeVertexRing& VertexRing1, aeBranch* pBranch, aeBranchNode* pNode1, aeBranchNode* pNode2, aeUInt32 uiPickingSubID, bool bIsLastSegment, float fTexCoordUOffset1, float fTexCoordUOffset2);
  static void AlignVertexRing (aeVertexRing& VertexRing, const aeVec3& vNodePosition, aeVec3& vRotationalDir);

  // *** Billboard Leaves ***
  void GenerateLeafMesh (aeUInt32 uiBranch, aeLod::Enum lod);
  void AddBillbardLeaf (aeUInt32 uiBranch, aeLod::Enum lod, const aeVec3& vPos, float fSize, const aeTreeMaterial* pMaterial);
  void AddStaticLeaf (aeUInt32 uiBranch, aeLod::Enum lod, aeUInt32 uiNodeID, float fSize, const aeTreeMaterial* pMaterial);


  // *** Forces ***

public:
  void RenderSkeleton (aeLod::Enum lod);
  void Render (const aeVec3& vCameraPosition, bool bForExport, bool bForExportPreview);
  void RenderSingleBranch (aeUInt32 uiBranch);
  void RenderBranchMesh (aeLod::Enum lod, aeUInt32 uiBranch);
  void RenderTree (aeLod::Enum lod);
  void RenderTreeMesh (aeLod::Enum lod);
  void RenderTreeImpostor (aeLod::Enum lod);
  void RenderCollisionMesh (void);
  void RenderGroundPlane (void);
  void RenderForces (bool bForPicking);

private:
  aeLod::Enum GetLodToRender (const aeVec3& vCameraPosition);
  void ClearSkeletonRenderData (void);
  void CreateSkeletonRenderData (aeLod::Enum lod);
  bool CreateLodImposters (aeUInt32 uiImpostorResolution = 0);
  void CreateTreeImpostorMesh (aeLod::Enum lod);
  bool SaveLodImpostors (const char* szFile, bool bDDS, aeUInt32 uiImpostorResolution);
  void UpdateImpostorSize (void);

public:
  void CreateTreeRenderData (aeLod::Enum lod);
  void ClearTreeRenderData (void);
  void ClearBranchRenderData (aeUInt32 uiBranch);

  bool m_bLodImpostorsAreUpToDate;
  aeTextureResourceHandle m_hLodTextures;
  aeTextureResourceHandle m_hLodTexturesN;

private:
  float m_fImposterWidth;
  float m_fImposterHeight;
  GLIM_BATCH m_SkeletonMesh[aeLod::ENUM_COUNT];
  GLIM_BATCH m_LodImpostors[aeLod::ENUM_COUNT];
  aeSubMesh m_LodImpostorMesh[aeLod::ENUM_COUNT];
};

extern aeTree g_Tree;

#endif

