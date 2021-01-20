#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../../GUI/ProgressBar.h"
#include "../../GUI/qtResourceEditorWidget/MaterialLibrary.h"
#include "../../Other/OBJ/OBJExporter.h"
#include "../../Tree/Tree.h"


using namespace NS_OBJEXPORTER;

const char* BranchTypeEnumNamesExport[Kraut::BranchType::ENUM_COUNT] =
  {
    "Trunk_1",
    "Trunk_2",
    "Trunk_3",
    "Main_Branches_1",
    "Main_Branches_2",
    "Main_Branches_3",
    "Sub_Branches_1",
    "Sub_Branches_2",
    "Sub_Branches_3",
    "Twigs_1",
    "Twigs_2",
    "Twigs_3",
};

const char* BranchGeometryTypeEnumNames[Kraut::BranchGeometryType::ENUM_COUNT] =
  {
    "Branch",
    "Frond",
    "Leaf",
};

bool aeTree::ExportOBJ(const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4)
{
  const float fScale = m_Descriptor.m_fExportScale;

  bool bLod[aeLod::ENUM_COUNT] = {bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4};

  aeInt32 iFiles = 0;
  for (aeInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    iFiles += bLod[i] ? 1 : 0;

  aeProgressBar pb("Exporting to OBJ", iFiles + 1);

  const aeString sFileName = aePathFunctions::GetFileName(szFile);

  for (int lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    if (!bLod[lod])
      continue;
    if (Kraut::LodMode::IsImpostorMode(m_Descriptor.m_LodData[lod].m_Mode))
      continue;

    EnsureMeshIsGenerated((aeLod::Enum)lod);

    aeString sNewPath;

    if (lod != aeLod::None)
    {
      char szNewFileName[256] = "";
      aeStringFunctions::Format(szNewFileName, 256, "%s_LOD%i", sFileName.c_str(), lod - 1);
      sNewPath = aePathFunctions::ChangeFileName(szFile, szNewFileName);
    }
    else
      sNewPath = szFile;

    sNewPath = aePathFunctions::ChangeExistingFileExtension(sNewPath.c_str(), "obj");

    aeString sText;
    sText.Format("Exporting to OBJ:\n%s", sNewPath.c_str());
    aeProgressBar::Update(sText.c_str());

    EXPORT_MESH obj;

    int Material[Kraut::BranchType::ENUM_COUNT][Kraut::BranchGeometryType::ENUM_COUNT];

    for (aeUInt32 bt = 0; bt < Kraut::BranchType::ENUM_COUNT; ++bt)
    {
      for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
      {
        const aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_sTexture[mt].c_str());

        const char* szDiffuse = pMaterial->m_sTextureDiffuse.c_str();
        const char* szNormal = pMaterial->m_sTextureNormal.c_str();

        aeString sName;
        sName.Format("%s_%s", BranchTypeEnumNamesExport[bt], BranchGeometryTypeEnumNames[mt]);

        NS_OBJEXPORTER::MATERIAL mat;
        mat.m_sDiffuseMap = szDiffuse;
        mat.m_sNormalMap = szNormal;
        mat.m_sMaterialName = sName.c_str();

        Material[bt][mt] = obj.AddMaterial(mat);
      }
    }

    for (aeUInt32 b = 0; b < m_TreeStructure.m_BranchStructures.size(); ++b)
    {
      const Kraut::BranchMesh& branchMesh = m_TreeMesh[lod].m_BranchMeshes[b];

      aeString sName;
      sName.Format("Branch%i", b);

      const aeUInt32 typeIdx = m_TreeStructure.m_BranchStructures[b].m_Type;

      for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
      {
        if ((!bExportBranches) && (mt == Kraut::BranchGeometryType::Branch))
          continue;
        if ((!bExportFronds) && (mt == Kraut::BranchGeometryType::Frond))
          continue;
        if ((!bExportLeaves) && (mt == Kraut::BranchGeometryType::Leaf))
          continue;

        aeString sMeshName;
        sMeshName.Format("%s_%s_%i", BranchTypeEnumNamesExport[typeIdx], BranchGeometryTypeEnumNames[mt], b);

        obj.StartNextGroup(sMeshName.c_str(), Material[typeIdx][mt]);

        for (aeUInt32 tri = 0; tri < branchMesh.m_Mesh[mt].GetNumTriangles(); ++tri)
        {
          const Kraut::Triangle& t = branchMesh.m_Mesh[mt].m_Triangles[tri];

          FACE f;

          for (int vert = 0; vert < 3; ++vert)
          {
            const Kraut::Vertex& vtx = branchMesh.m_Mesh[mt].m_Vertices[t.m_uiVertexIDs[vert]];

            FACE_VERTEX v;

            TEXCOORD tc;
            tc.u = vtx.m_vTexCoord.x / vtx.m_vTexCoord.z;
            tc.v = vtx.m_vTexCoord.y / vtx.m_vTexCoord.z;

            v.m_iPositionID = obj.AddPosition(fScale * VEC3(vtx.m_vPosition.x, vtx.m_vPosition.y, vtx.m_vPosition.z));
            v.m_iTexCoordID = obj.AddTexCoord(tc);
            v.m_iNormalID = obj.AddNormal(VEC3(vtx.m_vNormal.x, vtx.m_vNormal.y, vtx.m_vNormal.z));

            f.m_Vertices.push_back(v);
          }

          obj.AddFace(f);
        }
      }
    }

    if (!obj.ExportToFile(sNewPath.c_str()))
      return false;
  }

  return true;
}
