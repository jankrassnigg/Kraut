#ifndef AE_TREEPLUGIN_TREE_H
#define AE_TREEPLUGIN_TREE_H

#include "../Basics/Globals.h"
#include "../GUI/qtResourceEditorWidget/MaterialLibrary.h"
#include "../Physics/Forces.h"
#include "RenderData.h"
#include <KrautFoundation/Containers/HybridArray.h>
#include <KrautFoundation/Containers/Map.h>
#include <KrautFoundation/Containers/Set.h>
#include <KrautGenerator/Description/LodDesc.h>
#include <KrautGenerator/Description/SpawnNodeDesc.h>
#include <KrautGenerator/Description/TreeStructureDesc.h>
#include <KrautGenerator/Lod/BranchStructureLod.h>
#include <KrautGenerator/Lod/TreeStructureLod.h>
#include <KrautGenerator/Mesh/TreeMesh.h>
#include <KrautGenerator/TreeStructure/BranchRandomData.h>
#include <KrautGenerator/TreeStructure/BranchStructure.h>
#include <KrautGenerator/TreeStructure/TreeStructure.h>
#include <KrautGenerator/TreeStructure/TreeStructureGenerator.h>
#include <KrautGraphics/ShaderManager/Declarations.h>
#include <KrautGraphics/TextureManager/Declarations.h>
#include <KrautGraphics/VertexArray/Declarations.h>
#include <KrautGraphics/glim/glim.h>
#include <TreePlugin/Physics/BulletPhysicsImpl.h>

using namespace NS_GLIM;
using namespace AE_NS_GRAPHICS;

struct aeTreeDescriptor
{
  aeTreeDescriptor();
  void Reset(void);

  void Save(aeStreamOut& s);
  void Load(aeStreamIn& s);

  Kraut::TreeStructureDesc m_StructureDesc;

  bool m_bSnapshotFromAbove;
  float m_fBBoxAdjustment;
  float m_fExportScale;
  aeInt8 m_iImpostorResolution; // 0 = 1024, 1 = 512, 2 = 256, ...
  aeLeafCardResolution::Enum m_LeafCardResolution;
  aeLeafCardMipmapResolution::Enum m_LeafCardMipmaps;
  aeUInt8 m_uiLeafCardTexelDilation;

  // *** Lod ***
  float m_fLodCrossFadeTransition;
  Kraut::LodDesc m_LodData[aeLod::ENUM_COUNT];
  void ClampLodValues(aeLod::Enum CurLod);

  // *** Ambient Occlusion ***
  bool m_bUseAO;
  float m_fAOSampleSize;
  float m_fAOContrast;

  // *** Forces ***
  aeDeque<aeForce*> m_Forces;

private:
  void RemoveAllForces(void);
};

struct aeKrautMaterial
{
  aeString m_sDiffuseTexture;
  aeString m_sNormalMapTexture;
  aeUInt8 m_uiVariationColor[4];

  bool operator<(const aeKrautMaterial& rhs) const
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
  aeTree();

public:
  // Tree Export

  void ExportToFile(const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4, bool bDDS, aeUInt32 uiImpostorResolution);

private:
  bool ExportOBJ(const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4);
  bool ExportFBX(const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4);
  bool ExportKraut(const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4);

  void KrautExport_WriteHeader(aeStreamOut& Stream, bool bLod[aeLod::ENUM_COUNT]);
  void KrautExport_GatherMaterials(aeMap<aeKrautMaterial, aeSet<aeUInt32>>* out_Materials);
  void KrautExport_GatherMaterial(Kraut::BranchType::Enum bt, Kraut::BranchGeometryType::Enum mt, aeMap<aeKrautMaterial, aeSet<aeUInt32>>& Materials);
  void KrautExport_WriteMaterials(aeStreamOut& Stream, aeMap<aeKrautMaterial, aeSet<aeUInt32>>* in_Materials);
  void KrautExport_WriteMaterial(aeStreamOut& Stream, const aeKrautMaterial& Mat);
  aeUInt8 KrautExport_GatherLODs(bool* bLod);
  void KrautExport_GatherSubMeshesOfType(aeLod::Enum lod, Kraut::BranchGeometryType::Enum mt, const aeSet<aeUInt32>& Types, aeDeque<const Kraut::Mesh*>& out_SubMeshes);

public:
  void Reset(void);

  aeFilePath m_sTreeFile;

  void Shutdown(void);

  void GenerateTree(bool bUndoableAction);

  bool SearchForBranchWithParent(aeInt32 iParent, aeInt32& iCurNode) const;

  void Save(aeStreamOut& stream);
  void Load(aeStreamIn& stream);

  BulletPhysicsImpl m_BulletPhysicsImpl;
  Kraut::Physics_EmptyImpl m_NoPhysicsImpl;

  Kraut::TreeStructureGenerator m_TreeGenerator;
  Kraut::TreeStructure m_TreeStructure;
  Kraut::TreeStructureLod m_TreeStructureLod[aeLod::ENUM_COUNT];
  Kraut::TreeMesh m_TreeMesh[aeLod::ENUM_COUNT];
  aeTreeRenderData m_TreeRenderData[aeLod::ENUM_COUNT];

  aeArray<aeBranchRenderInfo> m_BranchRenderInfo;

  void ChangeBoundingAdjustment(float fNewAdjustment);

  Kraut::BoundingBox m_BBox;
  bool m_bModifiedViaUndo = false;

  aeTreeDescriptor m_Descriptor;

  aeUInt32 GetNumBones(aeLod::Enum lod) const;
  aeUInt32 GetNumAllTriangles(aeLod::Enum lod) const;
  aeUInt32 GetNumTriangles(aeLod::Enum lod, Kraut::BranchGeometryType::Enum mt) const;

private:
  void EnsureSkeletonIsGenerated(aeLod::Enum lod);
  void EnsureMeshIsGenerated(aeLod::Enum lod);

  void ComputeBoundingBox(void);

  void ConfigureInfluences();

  // Forces
public:
  void AddForce();
  void RemoveSelectedForce();

public:
  void RenderSkeleton(aeLod::Enum lod);
  void Render(const aeVec3& vCameraPosition, bool bForExport, bool bForExportPreview);
  void RenderSingleBranch(aeUInt32 uiBranch);
  void RenderBranchMesh(aeLod::Enum lod, aeUInt32 uiBranch);
  void RenderTree(aeLod::Enum lod);
  void RenderTreeMesh(aeLod::Enum lod);
  void RenderTreeImpostor(aeLod::Enum lod);
  void RenderCollisionMesh(void);
  void RenderGroundPlane(void);
  void RenderForces(bool bForPicking);

private:
  aeLod::Enum GetLodToRender(const aeVec3& vCameraPosition);
  void ClearSkeletonRenderData(void);
  void CreateSkeletonRenderData(aeLod::Enum lod);
  bool CreateLodImposters(aeUInt32 uiImpostorResolution = 0);
  void CreateTreeImpostorMesh(aeLod::Enum lod);
  bool SaveLodImpostors(const char* szFile, bool bDDS, aeUInt32 uiImpostorResolution);
  void UpdateImpostorSize(void);

public:
  void CreateTreeRenderData(aeLod::Enum lod);
  void ClearTreeRenderData(void);

  bool m_bLodStructureGenerated[aeLod::ENUM_COUNT] = {};
  bool m_bLodMeshGenerated[aeLod::ENUM_COUNT] = {};
  bool m_bLodRenderDataGenerated[aeLod::ENUM_COUNT] = {};

  bool m_bLodImpostorsAreUpToDate = false;
  aeTextureResourceHandle m_hLodTextures;
  aeTextureResourceHandle m_hLodTexturesN;

private:
  float m_fImposterWidth;
  float m_fImposterHeight;
  GLIM_BATCH m_SkeletonMesh[aeLod::ENUM_COUNT];
  GLIM_BATCH m_LodImpostors[aeLod::ENUM_COUNT];
  Kraut::Mesh m_LodImpostorMesh[aeLod::ENUM_COUNT];
};

extern aeTree g_Tree;

#endif
