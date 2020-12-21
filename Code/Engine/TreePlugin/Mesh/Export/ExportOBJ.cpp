#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../../Other/OBJ/OBJExporter.h"
#include "../../Tree/Tree.h"
#include "../../GUI/ProgressBar.h"
#include "../../GUI/qtResourceEditorWidget/MaterialLibrary.h"


using namespace NS_OBJEXPORTER;

bool aeTree::ExportOBJ (const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4)
{
  const float fScale = m_Descriptor.m_fExportScale;

  bool bLod[aeLod::ENUM_COUNT] = { bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4 };

  aeInt32 iFiles = 0;
  for (aeInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    iFiles += bLod[i] ? 1 : 0;

  aeProgressBar pb ("Exporting to OBJ", iFiles + 1);

  const aeFileName sFileName = aePathFunctions::GetFileName (szFile);

  for (int lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    if (!bLod[lod])
      continue;
    if (aeLodMode::IsImpostorMode (m_Descriptor.m_LodData[lod].m_Mode))
      continue;

    EnsureMeshIsGenerated ((aeLod::Enum) lod, bTrunkCaps, bBranchCaps);

    aeFilePath sNewPath;

    if (lod != aeLod::None)
    {
      char szNewFileName[256] = "";
      aeStringFunctions::Format (szNewFileName, 256, "%s_LOD%i", sFileName.c_str (), lod - 1);
      sNewPath = aePathFunctions::ChangeFileName (szFile, szNewFileName);
    }
    else
      sNewPath = szFile;

    sNewPath = aePathFunctions::ChangeExistingFileExtension (sNewPath.c_str (), "obj");

    aeFilePath sText;
    sText.Format ("Exporting to OBJ:\n%s", sNewPath.c_str ());
    aeProgressBar::Update (sText.c_str ());

    EXPORT_MESH obj;

    int Material[aeBranchType::ENUM_COUNT][aeMeshType::ENUM_COUNT];

    for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
    {
      for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      {
        const aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (m_Descriptor.m_BranchTypes[bt].m_sTexture[mt].c_str ());

        const char* szDiffuse = pMaterial->m_sTextureDiffuse.c_str ();
        const char* szNormal  = pMaterial->m_sTextureNormal.c_str ();

        aeString sName;
        sName.Format ("%s_%s", aeBranchType::EnumNamesExport[bt], aeMeshType::EnumNames[mt]);

        NS_OBJEXPORTER::MATERIAL mat;
        mat.m_sDiffuseMap = szDiffuse;
        mat.m_sNormalMap  = szNormal;
        mat.m_sMaterialName = sName.c_str ();

        Material[bt][mt] = obj.AddMaterial (mat);
      }
    }

    for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
    {
      if (m_Branches[b].IsDeleted ())
        continue;

      aeString sName;
      sName.Format ("Branch%i", b);

      aeUInt32 n = m_Branches[b].m_Type;

      for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      {
        if ((!bExportBranches) && (mt == aeMeshType::Branch))
          continue;
        if ((!bExportFronds) && (mt == aeMeshType::Frond))
          continue;
        if ((!bExportLeaves) && (mt == aeMeshType::Leaf))
          continue;

        aeString sMeshName;
        sMeshName.Format ("%s_%s_%i", aeBranchType::EnumNamesExport[n], aeMeshType::EnumNames[mt], b);

        obj.StartNextGroup (sMeshName.c_str (), Material[n][mt]);

        for (aeUInt32 tri = 0; tri < m_Branches[b].m_LODs[lod].m_SubMesh[mt].GetNumTriangles (); ++tri)
        {
          const aeMeshTriangle& t = m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Triangles[tri];

          FACE f;

          for (int vert = 0; vert < 3; ++vert)
          {
            const aeMeshVertex& vtx = m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Vertices[t.m_uiVertexIDs[vert]];

            FACE_VERTEX v;

            TEXCOORD tc;
            tc.u = vtx.m_vTexCoord.x / vtx.m_vTexCoord.z;
            tc.v = vtx.m_vTexCoord.y / vtx.m_vTexCoord.z;

            v.m_iPositionID = obj.AddPosition (fScale * VEC3 (vtx.m_vPosition.x, vtx.m_vPosition.y, vtx.m_vPosition.z));
            v.m_iTexCoordID = obj.AddTexCoord (tc);
            v.m_iNormalID   = obj.AddNormal (VEC3 (vtx.m_vNormal.x, vtx.m_vNormal.y, vtx.m_vNormal.z));

            f.m_Vertices.push_back (v);
          }

          obj.AddFace (f);
        }
      }
    }

    if (!obj.ExportToFile (sNewPath.c_str ()))
      return false;
  }

  return true;
}





