#include "PCH.h"
#include "../../Rendering/AmbientOcclusion.h"
#include "../../Tree/Tree.h"
#include "../../GUI/ProgressBar.h"
#include "../../GUI/qtResourceEditorWidget/MaterialLibrary.h"


void aeTree::ExportToFile (const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4, bool bDDS, aeUInt32 uiImpostorResolution)
{
  aeProgressBar pb ("Exporting Tree", 2);

  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  const bool bExportLod[aeLod::ENUM_COUNT] = { bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4 };

  if ((aePathFunctions::HasExtension (szFile, "obj")) && (!g_Tree.ExportOBJ (szFile, bExportBranches, bExportFronds, bExportLeaves, bTrunkCaps, bBranchCaps,
    bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4)))
  {
    QMessageBox::critical (pMainWindow, "Kraut", "The mesh could not be exported to OBJ.");
  }
  else
  if ((aePathFunctions::HasExtension (szFile, "fbx")) && (!g_Tree.ExportFBX (szFile, bExportBranches, bExportFronds, bExportLeaves, bTrunkCaps, bBranchCaps,
    bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4)))
  {
    QMessageBox::critical (pMainWindow, "Kraut", "The mesh could not be exported to FBX.");
  }
  else
  if ((aePathFunctions::HasExtension (szFile, "kraut")) && (!g_Tree.ExportKraut (szFile, bExportBranches, bExportFronds, bExportLeaves, bTrunkCaps, bBranchCaps,
    bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4)))
  {
    QMessageBox::critical (pMainWindow, "Kraut", "The mesh could not be exported to Kraut-Format.");
  }
  else
  {
    //QMessageBox::information (this, "Kraut", "The mesh was successfully exported.");
  }

  aeProgressBar::Update ("Exporting Impostor Textures"); // 1
  SaveLodImpostors (szFile, bDDS, uiImpostorResolution);

  aeProgressBar::Update (); // 2
}

void aeTree::KrautExport_WriteHeader (aeStreamOut& Stream, bool bLod[aeLod::ENUM_COUNT])
{
  const float fScale = m_Descriptor.m_fExportScale;
  const char* szSignature = "{KRAUT}";

  Stream.Write (szSignature, sizeof (char) * 7);

  aeUInt8 uiVersion = 2;
  Stream << uiVersion;

  // Version 2 added per vertex Ambient Occlusion value

  // Write the Bounding Box
  ComputeBoundingBox ();
  Stream << fScale * m_BBox.m_vMin;
  Stream << fScale * m_BBox.m_vMax;

  // Write the number of LODs
  const aeUInt8 uiLODs = KrautExport_GatherLODs (bLod);
  Stream << uiLODs;
}

void aeTree::KrautExport_GatherMaterial (aeBranchType::Enum bt, aeMeshType::Enum mt, aeMap<aeKrautMaterial, aeSet<aeUInt32> >& Materials)
{
  const aeSpawnNodeDesc& sn = m_Descriptor.m_BranchTypes[bt];

  if (!sn.m_bEnable[mt])
    return;

  const aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (sn.m_sTexture[mt].c_str ());

  aeKrautMaterial mat;
  mat.m_sDiffuseTexture   = pMaterial->m_sTextureDiffuse;
  mat.m_sNormalMapTexture = pMaterial->m_sTextureNormal;
  mat.m_uiVariationColor[0] = sn.m_uiVariationColor[mt][0];
  mat.m_uiVariationColor[1] = sn.m_uiVariationColor[mt][1];
  mat.m_uiVariationColor[2] = sn.m_uiVariationColor[mt][2];
  mat.m_uiVariationColor[3] = sn.m_uiVariationColor[mt][3];

  Materials[mat].insert (bt);
}

void aeTree::KrautExport_GatherMaterials (aeMap<aeKrautMaterial, aeSet<aeUInt32> >* out_Materials)
{
  for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
  {
    const aeSpawnNodeDesc& sn = m_Descriptor.m_BranchTypes[bt];
    if (!sn.m_bUsed)
      continue;

    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      KrautExport_GatherMaterial ((aeBranchType::Enum) bt, (aeMeshType::Enum) mt, out_Materials[mt]);
  }
}

aeUInt8 aeTree::KrautExport_GatherLODs (bool* bLod)
{
  aeUInt8 res = 0;

  for (int lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    if (!bLod[lod])
      continue;
    bLod[lod] = false;

    if (m_Descriptor.m_LodData[lod].m_Mode == aeLodMode::Disabled)
      continue;

    bLod[lod] = true;
    ++res;
  }

  return res;
}

void aeTree::KrautExport_WriteMaterial (aeStreamOut& Stream, const aeKrautMaterial& Mat)
{
  Stream << Mat.m_sDiffuseTexture;
  Stream << Mat.m_sNormalMapTexture;
  Stream << Mat.m_uiVariationColor[0];
  Stream << Mat.m_uiVariationColor[1];
  Stream << Mat.m_uiVariationColor[2];
  Stream << Mat.m_uiVariationColor[3];
}

void aeTree::KrautExport_WriteMaterials (aeStreamOut& Stream, aeMap<aeKrautMaterial, aeSet<aeUInt32> >* in_Materials)
{
  const aeUInt8 uiMaterialTypes = aeMeshType::ENUM_COUNT;
  Stream << uiMaterialTypes;

  // go through all mesh types and write their materials
  for (aeUInt8 i = 0; i < uiMaterialTypes; ++i)
  {
    const aeUInt8 uiMaterials = in_Materials[i].size ();
    Stream << uiMaterials;

    aeMap<aeKrautMaterial, aeSet<aeUInt32> >::iterator it = in_Materials[i].begin ();
    aeMap<aeKrautMaterial, aeSet<aeUInt32> >::iterator itend = in_Materials[i].end ();

    // write all materials of this type
    for ( ; it != itend; ++it)
      KrautExport_WriteMaterial (Stream, it.key());
  }
}

void aeTree::KrautExport_GatherSubMeshesOfType (aeLod::Enum lod, aeMeshType::Enum mt, const aeSet<aeUInt32>& Types, aeDeque<aeSubMesh*>& out_SubMeshes)
{
  // go through hall branches, search for branches that use the current material
  const aeUInt32 uiBranches = m_Branches.size ();
  for (aeUInt32 b = 0; b < uiBranches; ++b)
  {
    // it the type of this branch uses the material that 'it' currently points to
    if (Types.find (m_Branches[b].m_Type) != Types.end ())
    {
      // This submesh should be added to the file
      aeSubMesh* pSubMesh = &m_Branches[b].m_LODs[lod].m_SubMesh[mt];

      if (!pSubMesh->m_Triangles.empty ())
        out_SubMeshes.push_back (pSubMesh);
    }
  }
}

static void CombineSubMeshes (const aeDeque<aeSubMesh*> SubMeshes, aeSubMesh& out_Combined)
{
  aeUInt32 uiVertexOffset = 0;

  for (aeUInt32 s = 0; s < SubMeshes.size (); ++s)
  {
    const aeUInt32 uiVertices = SubMeshes[s]->m_Vertices.size ();
    for (aeUInt32 v = 0; v < uiVertices; ++v)
    {
      out_Combined.m_Vertices.push_back (SubMeshes[s]->m_Vertices[v]);
      out_Combined.m_Vertices.back ().m_iSharedVertex += uiVertexOffset;
    }

    const aeUInt32 uiTriangles = SubMeshes[s]->m_Triangles.size ();
    for (aeUInt32 t = 0; t < uiTriangles; ++t)
    {
      out_Combined.m_Triangles.push_back (SubMeshes[s]->m_Triangles[t]);
      out_Combined.m_Triangles.back ().m_uiVertexIDs[0] += uiVertexOffset;
      out_Combined.m_Triangles.back ().m_uiVertexIDs[1] += uiVertexOffset;
      out_Combined.m_Triangles.back ().m_uiVertexIDs[2] += uiVertexOffset;
    }

    uiVertexOffset += SubMeshes[s]->m_Vertices.size ();
  }
}

static void WriteSubMesh (aeStreamOut& Stream, const aeSubMesh& SubMesh, float fScale)
{
  const aeUInt32 uiVertices = SubMesh.m_Vertices.size ();
  const aeUInt32 uiTriangles = SubMesh.m_Triangles.size ();

  Stream << uiVertices;
  Stream << uiTriangles;

  for (aeUInt32 v = 0; v < uiVertices; ++v)
  {
    const aeMeshVertex& mv = SubMesh.m_Vertices[v];

    const aeVec3 vPos = fScale * mv.m_vPosition;

    Stream << vPos;
    Stream << mv.m_vTexCoord;
    Stream << mv.m_vNormal;
    Stream << mv.m_vTangent;

    Stream << mv.m_uiColorVariation;

    float fAO[7];
    fAO[0] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition, mv.m_vNormal);
    fAO[1] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition + aeVec3::GetAxisX() * 0.2f, aeVec3::GetAxisX());
    fAO[2] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition - aeVec3::GetAxisX() * 0.2f,-aeVec3::GetAxisX());
    fAO[3] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition + aeVec3::GetAxisY() * 0.2f, aeVec3::GetAxisY());
    fAO[4] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition - aeVec3::GetAxisY() * 0.2f,-aeVec3::GetAxisY());
    fAO[5] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition + aeVec3::GetAxisZ() * 0.2f, aeVec3::GetAxisZ());
    fAO[6] = aeAOGrid::GetAmbientOcclusionAt (mv.m_vPosition - aeVec3::GetAxisZ() * 0.2f,-aeVec3::GetAxisZ());

    for (aeUInt32 i = 0; i < 7; ++i)
      Stream << fAO[i];
  }

  for (aeUInt32 t = 0; t < uiTriangles; ++t)
  {
    const aeMeshTriangle& mt = SubMesh.m_Triangles[t];
    Stream << mt.m_uiVertexIDs[0];
    Stream << mt.m_uiVertexIDs[1];
    Stream << mt.m_uiVertexIDs[2];
  }
}

struct aeCombinedMesh
{
  aeUInt32 m_uiMaterial;
  aeSubMesh m_SubMesh;
};

bool aeTree::ExportKraut (const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4)
{
  bool bLod[aeLod::ENUM_COUNT] = { bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4 };

  aeFileOut Stream;
  if (!Stream.Open (szFile))
    return false;

  const float fScale = m_Descriptor.m_fExportScale;

  const aeUInt8 uiLODs = KrautExport_GatherLODs (bLod);

  aeProgressBar pb ("Exporting to Kraut-Format", 1 + uiLODs);
  
  KrautExport_WriteHeader (Stream, bLod);

  aeMap<aeKrautMaterial, aeSet<aeUInt32> > Materials[aeMeshType::ENUM_COUNT];
  KrautExport_GatherMaterials (Materials);
  KrautExport_WriteMaterials (Stream, Materials);

  aeProgressBar::Update (); // 1

  const aeUInt8 uiMeshTypes = aeMeshType::ENUM_COUNT;
  Stream << uiMeshTypes;

  for (int lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    if (!bLod[lod])
      continue;
    //if (aeLodMode::IsImpostorMode (m_Descriptor.m_LodData[lod].m_Mode))
      //continue;

    EnsureMeshIsGenerated ((aeLod::Enum) lod, bTrunkCaps, bBranchCaps);

    const float fLodDistance = m_Descriptor.m_LodData[lod].m_uiLodDistance;
    Stream << fLodDistance;

    const aeUInt8 uiLodMode = m_Descriptor.m_LodData[lod].m_Mode;
    Stream << uiLodMode; // 0 == Mesh, 1 == 4 Quads, 2 == 2 Quads, 3 == Billboard

    aeProgressBar::Update (); // 1n

    if (uiLodMode == 0) // full mesh
    {
      aeUInt8 uiMaterialTypes = aeMeshType::ENUM_COUNT;
      uiMaterialTypes -= bExportBranches ? 0 : 1;
      uiMaterialTypes -= bExportFronds   ? 0 : 1;
      uiMaterialTypes -= bExportLeaves   ? 0 : 1;

      Stream << uiMaterialTypes;

      // go through all mesh types and write their materials
      for (aeUInt8 mt = 0; mt < uiMeshTypes; ++mt)
      {
        if ((!bExportBranches) && (mt == aeMeshType::Branch))
          continue;
        if ((!bExportFronds) && (mt == aeMeshType::Frond))
          continue;
        if ((!bExportLeaves) && (mt == aeMeshType::Leaf))
          continue;

        // write the current material type (allows to skip types later)
        Stream << mt;

        aeMap<aeKrautMaterial, aeSet<aeUInt32> >::iterator it = Materials[mt].begin ();
        aeMap<aeKrautMaterial, aeSet<aeUInt32> >::iterator itend = Materials[mt].end ();

        aeDeque<aeCombinedMesh> Meshes;

        // go through all materials
        int iMaterial = 0;
        for ( ; it != itend; ++it)
        {
          aeDeque<aeSubMesh*> SubMeshes;
          KrautExport_GatherSubMeshesOfType ((aeLod::Enum) lod, (aeMeshType::Enum) mt, it.value (), SubMeshes);

          // now we know ALL the submeshes in this lod, of this mesh-type, that use the same texture/material
          if (!SubMeshes.empty ())
          {
            aeCombinedMesh Combined;
            Combined.m_uiMaterial = iMaterial;
            CombineSubMeshes (SubMeshes, Combined.m_SubMesh);

            Meshes.push_back (Combined);
          }

          ++iMaterial;
        }

        aeUInt8 uiMeshes = Meshes.size ();
        Stream << uiMeshes;

        for (aeUInt32 m = 0; m < uiMeshes; ++m)
        {
          const aeUInt8 uiMaterialID = Meshes[m].m_uiMaterial;
          Stream << uiMaterialID;

          WriteSubMesh (Stream, Meshes[m].m_SubMesh, fScale);
        }
      }
    }
    else
    if ((uiLodMode == 1) || (uiLodMode == 2) || (uiLodMode == 3))
    {
      const aeUInt8 uiMeshTypes = 1;
      Stream << uiMeshTypes;

      aeUInt8 mt = aeMeshType::Frond;
      Stream << mt;

      aeUInt8 uiMeshes = 1;
      Stream << uiMeshes;

      const aeUInt8 uiMaterialID = 0;
      Stream << uiMaterialID;

      WriteSubMesh (Stream, m_LodImpostorMesh[lod], fScale);
    }
  }

  return true;
}