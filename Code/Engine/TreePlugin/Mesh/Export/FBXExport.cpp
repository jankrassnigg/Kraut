// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "PCH.h"

#include <KrautFoundation/FileSystem/FileSystem.h>

#include "../../Basics/Plugin.h"
#include "../../Tree/Tree.h"
#include "../../GUI/ProgressBar.h"
#include "../../GUI/qtResourceEditorWidget/MaterialLibrary.h"

// #define USE_FBX

#define FBXSDK_SHARED
#define FBXSDK_NEW_API

#ifdef USE_FBX
#include <fbxsdk.h>

FbxSurfaceLambert* CreateFBXMaterial (aeTree* pTree, FbxScene* pScene, aeUInt32 uiBranchType, aeMeshType::Enum MeshType)
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (pTree->m_Descriptor.m_BranchTypes[uiBranchType].m_sTexture[MeshType].c_str ());

  const char* szDiffuse = pMaterial->m_sTextureDiffuse.c_str ();
  const char* szNormal  = pMaterial->m_sTextureNormal.c_str ();

  aeFilePath sDiffuse, sNormal;
  aeFileSystem::MakeValidPath (szDiffuse, false, &sDiffuse, NULL);
  aeFileSystem::MakeValidPath (szNormal,  false, &sNormal,  NULL);

  aeString s;
  s.Format ("%s_%s_Material", aeBranchType::EnumNamesExport[uiBranchType], aeMeshType::EnumNames[MeshType]);

  FbxSurfaceLambert* lMaterial = FbxSurfaceLambert::Create (pScene, s.c_str ());
  lMaterial->ShadingModel.Set ("Lambert");


  {
    s.Format ("%s_%s_Diffuse", aeBranchType::EnumNamesExport[uiBranchType], aeMeshType::EnumNames[MeshType]);

    FbxFileTexture* lTexture = FbxFileTexture::Create (pScene, s.c_str ());

    lTexture->SetFileName (sDiffuse.c_str ());
    lTexture->SetTextureUse (FbxTexture::eStandard);
    lTexture->SetMappingType (FbxTexture::eUV);
    lTexture->SetMaterialUse (FbxFileTexture::eModelMaterial);

    lMaterial->Diffuse.ConnectSrcObject(lTexture);

    if (MeshType == aeMeshType::Frond)
    {
      lMaterial->TransparentColor.ConnectSrcObject(lTexture);
      
    }
  }

  {
    s.Format ("%s_%s_Normals", aeBranchType::EnumNamesExport[uiBranchType], aeMeshType::EnumNames[MeshType]);

    FbxFileTexture* lTexture = FbxFileTexture::Create (pScene, s.c_str ());

    lTexture->SetFileName (sNormal.c_str ());
    lTexture->SetTextureUse (FbxTexture::eBumpNormalMap);
    lTexture->SetMappingType (FbxTexture::eUV);
    lTexture->SetMaterialUse (FbxFileTexture::eModelMaterial);

    lMaterial->NormalMap.ConnectSrcObject(lTexture);
  }

  return lMaterial;
}

#endif

bool aeTree::SearchForBranchWithParent (aeInt32 iParent, aeInt32& iCurNode) const
{
  ++iCurNode;

  for ( ; iCurNode < (aeInt32) m_Branches.size (); ++iCurNode)
  {
    if (m_Branches[iCurNode].IsDeleted ())
      continue;

    if (m_Branches[iCurNode].m_iParentBranchID == iParent)
      return true;
  }

  return false;
}


bool aeTree::ExportFBX (const char* szFile, bool bExportBranches, bool bExportFronds, bool bExportLeaves, bool bTrunkCaps, bool bBranchCaps, bool bLodNone, bool bLod0, bool bLod1, bool bLod2, bool bLod3, bool bLod4)
{
#ifdef USE_FBX
  const float fScale = m_Descriptor.m_fExportScale;

  FbxManager* lSdkManager = FbxManager::Create();
  FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
  lSdkManager->SetIOSettings(ios);

  bool bLod[aeLod::ENUM_COUNT] = { bLodNone, bLod0, bLod1, bLod2, bLod3, bLod4 };

  aeInt32 iFiles = 0;
  for (aeInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    iFiles += bLod[i] ? 1 : 0;

  aeProgressBar pb ("Exporting to FBX", iFiles + 1);

  const aeFileName sFileName = aePathFunctions::GetFileName (szFile);

  for (int lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    if (!bLod[lod])
      continue;
    if (aeLodMode::IsImpostorMode (m_Descriptor.m_LodData[lod].m_Mode))
      continue;

    aeFilePath sNewPath = szFile;

    if (lod != aeLod::None)
    {
      char szNewFileName[256] = "";
      aeStringFunctions::Format (szNewFileName, 256, "%s_LOD%i", sFileName.c_str (), lod - 1);
      sNewPath = aePathFunctions::ChangeFileName (szFile, szNewFileName);
    }
    
    aeFilePath sText;
    sText.Format ("Exporting to OBJ:\n%s", sNewPath.c_str ());
    aeProgressBar::Update (sText.c_str ());


    FbxScene* lScene = FbxScene::Create(lSdkManager, "Scene");

    FbxSurfaceLambert* pMaterials[aeBranchType::ENUM_COUNT][aeMeshType::ENUM_COUNT];

    for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
    {
      for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      {
        pMaterials[bt][mt] = CreateFBXMaterial (this, lScene, bt, (aeMeshType::Enum) mt);
      }
    }

    EnsureMeshIsGenerated ((aeLod::Enum) lod, bTrunkCaps, bBranchCaps);

    FbxNode* pTreeNode = FbxNode::Create (lSdkManager, "Tree");
    lScene->GetRootNode ()->AddChild (pTreeNode);

    aeStack<aeInt32> ParentID;
    aeStack<FbxNode*> ParentNode;
    ParentID.push (-1);
    ParentNode.push (pTreeNode);

    while (!ParentID.empty ())
    {
      aeInt32 iCurParent = ParentID.top ();
      FbxNode* pParentNode = ParentNode.top ();

      ParentID.pop ();
      ParentNode.pop ();

      aeInt32 iCurNode = -1;

      while (SearchForBranchWithParent (iCurParent, iCurNode))
      {
        aeString s;
        s.Format ("%s__%i", aeBranchType::EnumNames[m_Branches[iCurNode].m_Type], iCurNode);

        FbxNode* pBranchNode = FbxNode::Create (lSdkManager, s.c_str ());
        pParentNode->AddChild (pBranchNode);

        ParentID.push (iCurNode);
        ParentNode.push (pBranchNode);

        for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
        {
          if (m_Branches[iCurNode].m_LODs[lod].m_SubMesh[mt].GetNumTriangles () == 0)
            continue;

          if ((!bExportBranches) && (mt == aeMeshType::Branch))
            continue;
          if ((!bExportFronds) && (mt == aeMeshType::Frond))
            continue;
          if ((!bExportLeaves) && (mt == aeMeshType::Leaf))
            continue;

          const aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (m_Descriptor.m_BranchTypes[m_Branches[iCurNode].m_Type].m_sTexture[mt].c_str ());

          const char* szTexture = pMaterial->m_sTextureDiffuse.c_str ();
          const char* szTextureN= pMaterial->m_sTextureNormal.c_str ();

          aeString s;
          s.Format ("%sMesh__%i", aeMeshType::EnumNames[mt], iCurNode);

          FbxMesh* pMesh = FbxMesh::Create (lSdkManager, s.c_str ());

          aeSubMesh& m = m_Branches[iCurNode].m_LODs[lod].m_SubMesh[mt];

          pMesh->InitControlPoints (m.m_Vertices.size ());
          pMesh->InitNormals (m.m_Vertices.size ());

          FbxGeometryElementUV* lUVDiffuseElement = pMesh->CreateElementUV ("DiffuseUV");

          lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByControlPoint);

          for (aeUInt32 v = 0; v < m.m_Vertices.size (); ++v)
          {
            const aeVec3 pos = fScale * m.m_Vertices[v].m_vPosition;
            const aeVec3 norm= m.m_Vertices[v].m_vNormal;
            aeVec3 tc  = m.m_Vertices[v].m_vTexCoord;
            tc.x /= tc.z;
            tc.y /= tc.z;

            pMesh->SetControlPointAt (FbxVector4 (pos.x, pos.y, pos.z), FbxVector4 (norm.x, norm.y, norm.z), v);
            lUVDiffuseElement->GetDirectArray().Add (FbxVector2 (tc.x, tc.y));
          }

          pMesh->ReservePolygonCount (m.m_Triangles.size ());

          for (aeUInt32 t = 0; t < m.m_Triangles.size (); ++t)
          {
            pMesh->BeginPolygon (-1, -1, -1, false);

            pMesh->AddPolygon (m.m_Triangles[t].m_uiVertexIDs[0]);
            pMesh->AddPolygon (m.m_Triangles[t].m_uiVertexIDs[1]);
            pMesh->AddPolygon (m.m_Triangles[t].m_uiVertexIDs[2]);

            pMesh->EndPolygon ();
          }

          FbxNode* pNode = FbxNode::Create (lSdkManager, s.c_str ());

          pNode->SetNodeAttribute (pMesh);
          pNode->SetShadingMode(FbxNode::eFullShading);

          //CreateTexture (lScene, pMesh, iCurNode, szTexture, szTextureN);
          pNode->AddMaterial (pMaterials[m_Branches[iCurNode].m_Type][mt]);

          pBranchNode->AddChild (pNode);
        }

      }
    }

    {
      const aeFilePath sFileFBX = aePathFunctions::ChangeExistingFileExtension (sNewPath.c_str (), "fbx");

      FbxExporter* lExporterFBX = FbxExporter::Create(lSdkManager, "");
      lExporterFBX->Initialize(sFileFBX.c_str (), -1, ios);
      lExporterFBX->Export(lScene);
      lExporterFBX->Destroy ();
    }

    lScene->Destroy ();
  }

  lSdkManager->Destroy ();

  return true; 
#else
  return false;
#endif
}