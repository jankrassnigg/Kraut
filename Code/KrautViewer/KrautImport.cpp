#include "Main.h"

kvTree g_Tree;
string g_sTextureFolder;

inline unsigned char PackToUI8 (float f)
{
  // packs [-1;1] range into [0;2] then [0;1] then [0;255] range
  f += 1.0f;
  f *= 0.5f;
  f *= 255.0f;
  int i = (int) f;
  i = min (i, 255);
  i = max (i, 0);
  return i & 0xFF;
}

void ReadMesh (FILE* pFile, kvTreeMesh& Mesh)
{
  // index which material to use
  // look it up in g_Materials[mattype][uiMaterialID]
  unsigned char uiMaterialID = 0;
  fread (&uiMaterialID, sizeof (char), 1, pFile);

  Mesh.m_iMaterial = uiMaterialID;

  unsigned int uiVertices = 0;
  fread (&uiVertices, sizeof (int), 1, pFile);

  unsigned int uiTriangles = 0;
  fread (&uiTriangles, sizeof (int), 1, pFile);

  Mesh.m_VertexBuffer.AddVertexStream ("attr_Position", kvVertexBuffer::R32G32B32_FLOAT);
  Mesh.m_VertexBuffer.AddVertexStream ("attr_TexCoord", kvVertexBuffer::R32G32B32_FLOAT);
  Mesh.m_VertexBuffer.AddVertexStream ("attr_PackedNormal",  kvVertexBuffer::R8G8B8A8_UNORM);
  Mesh.m_VertexBuffer.AddVertexStream ("attr_PackedTangent", kvVertexBuffer::R8G8B8A8_UNORM);
  Mesh.m_VertexBuffer.AddVertexStream ("attr_Color", kvVertexBuffer::R8G8B8A8_UNORM);

  Mesh.m_VertexBuffer.SetupBuffer (uiVertices, uiTriangles);

  for (unsigned int v = 0; v < uiVertices; ++v)
  {
    float fPosition[3];
    fread (fPosition, sizeof (float) * 3, 1, pFile);
    Mesh.m_VertexBuffer.SetVertexAttribute3f (0, v, fPosition[0], fPosition[1], fPosition[2]);

    float fTexCoord[3];
    fread (fTexCoord, sizeof (float) * 3, 1, pFile);
    Mesh.m_VertexBuffer.SetVertexAttribute3f (1, v, fTexCoord[0], fTexCoord[1], fTexCoord[2]);

    float fNormal[3];
    fread (fNormal, sizeof (float) * 3, 1, pFile);
    Mesh.m_VertexBuffer.SetVertexAttribute4ub (2, v, PackToUI8(fNormal[0]), PackToUI8(fNormal[1]), PackToUI8(fNormal[2]), 0);

    float fTangent[3];
    fread (fTangent, sizeof (float) * 3, 1, pFile);
    Mesh.m_VertexBuffer.SetVertexAttribute4ub (3, v, PackToUI8(fTangent[0]), PackToUI8(fTangent[1]), PackToUI8(fTangent[2]), 0);

    unsigned char uiColor[4];
    fread (&uiColor[0], sizeof (char), 1, pFile);
    fread (&uiColor[1], sizeof (char), 1, pFile);
    fread (&uiColor[2], sizeof (char), 1, pFile);
    fread (&uiColor[3], sizeof (char), 1, pFile);
    Mesh.m_VertexBuffer.SetVertexAttribute4ub (4, v, uiColor[0], uiColor[1], uiColor[2], uiColor[3]);
  }

  for (unsigned int t = 0; t < uiTriangles; ++t)
  {
    unsigned int uiIndices[3];
    fread (uiIndices, sizeof (int) * 3, 1, pFile);

    Mesh.m_VertexBuffer.SetTriangle (t, uiIndices[0], uiIndices[1], uiIndices[2]);
  }

  Mesh.m_VertexBuffer.UploadToGPU ();
}

void ReadMaterials (FILE* pFile, kvTree& out_Tree, const string& sFile)
{
  const string sBaseName = sFile.substr (0, sFile.length () - 6);
  out_Tree.m_ImpostorMaterial.m_sDiffuseTexture = sBaseName + "_D.tga";
  out_Tree.m_ImpostorMaterial.m_sNormalMap      = sBaseName + "_N.tga";

  // load the impostor textures
  out_Tree.m_ImpostorMaterial.m_Diffuse.LoadTexture   (out_Tree.m_ImpostorMaterial.m_sDiffuseTexture.c_str ());
  out_Tree.m_ImpostorMaterial.m_NormalMap.LoadTexture (out_Tree.m_ImpostorMaterial.m_sNormalMap.c_str ());


  // how many different material types are stored in this file (typically 3)
  // type 0: branches
  // type 1: fronds
  // type 2: billboard leaves
  unsigned char uiNumMaterialTypes = 0;
  fread (&uiNumMaterialTypes, sizeof (char), 1, pFile);

  // read all materials of each type
  for (int mattype = 0; mattype < uiNumMaterialTypes; ++mattype)
  {
    unsigned char uiNumMaterialsOfType = 0;
    fread (&uiNumMaterialsOfType, sizeof (char), 1, pFile);

    out_Tree.m_Materials[mattype].resize (uiNumMaterialsOfType);

    char szTemp[1024];

    for (int mat = 0; mat < uiNumMaterialsOfType; ++mat)
    {
      // read the diffuse texture
      unsigned int uiStringLength = 0;
      fread (&uiStringLength, sizeof (int), 1, pFile);

      fread (szTemp, sizeof (char) * uiStringLength, 1, pFile);
      szTemp[uiStringLength] = '\0';

      out_Tree.m_Materials[mattype][mat].m_sDiffuseTexture = szTemp;

      // read the normalmap
      uiStringLength = 0;
      fread (&uiStringLength, sizeof (int), 1, pFile);

      fread (szTemp, sizeof (char) * uiStringLength, 1, pFile);
      szTemp[uiStringLength] = '\0';

      out_Tree.m_Materials[mattype][mat].m_sNormalMap = szTemp;

      // load the textures
      out_Tree.m_Materials[mattype][mat].m_Diffuse.LoadTexture   ((g_sTextureFolder + out_Tree.m_Materials[mattype][mat].m_sDiffuseTexture).c_str ());
      out_Tree.m_Materials[mattype][mat].m_NormalMap.LoadTexture ((g_sTextureFolder + out_Tree.m_Materials[mattype][mat].m_sNormalMap).c_str ());

      // Read the variation color
      fread (&out_Tree.m_Materials[mattype][mat].m_uiVariationColor[0], sizeof (char), 1, pFile);
      fread (&out_Tree.m_Materials[mattype][mat].m_uiVariationColor[1], sizeof (char), 1, pFile);
      fread (&out_Tree.m_Materials[mattype][mat].m_uiVariationColor[2], sizeof (char), 1, pFile);
      fread (&out_Tree.m_Materials[mattype][mat].m_uiVariationColor[3], sizeof (char), 1, pFile);
    }
  }
}

void ImportKrautFile (const char* szFile, kvTree& out_Tree)
{
  // See the file format documentation under "Documentation/Kraut File Format v1.txt" for more information.

  FILE* pFile = fopen(szFile, "rb");
  if (!pFile)
    throw exception ("Could not open Kraut-File.");

  char szSignature[8];
  fread (szSignature, sizeof (char) * 7, 1, pFile);
  szSignature[7] = '\0';

  if (strcmp (szSignature, "{KRAUT}") != 0)
    throw exception ("The given file is not a valid Kraut-File.");

  unsigned char uiVersion = 0;
  fread (&uiVersion, sizeof (char), 1, pFile);

  if (uiVersion != 1)
    throw exception ("The version of the given Kraut-File is unknown.");

  // the bounding box of the tree (min x, min y, min z, max x, max y, max z)
  fread (&out_Tree.m_BBoxMin.x, sizeof (float) * 3, 1, pFile);
  fread (&out_Tree.m_BBoxMax.x, sizeof (float) * 3, 1, pFile);

  // how many lod level meshes are stored in this file
  unsigned char uiLodLevels = 0;
  fread (&uiLodLevels, sizeof (char), 1, pFile);

  out_Tree.m_LODs.resize (uiLodLevels);

  ReadMaterials (pFile, out_Tree, szFile);

  unsigned char uiMeshTypes = 0;
  fread (&uiMeshTypes, sizeof (char), 1, pFile);

  // read each lod level
  for (int lod = 0; lod < uiLodLevels; ++lod)
  {
    float fLodDistance = 0.0f;
    fread (&fLodDistance, sizeof (float), 1, pFile);

    out_Tree.m_LODs[lod].m_fLodDistance = fLodDistance;

    unsigned char uiLodType = 0;
    fread (&uiLodType, sizeof (char), 1, pFile);
    out_Tree.m_LODs[lod].m_uiLodType = uiLodType; // 0 == full mesh, 1 == 4 quads impostor, 2 == 2 quads impostor, 3 == 1 quad billboard impostor


    unsigned char uiMaterialTypes = 0;
    fread (&uiMaterialTypes, sizeof (char), 1, pFile); // typically 3, but could change or skip some in the future

    // the number of different material types that this lod uses
    // usually this is 3, but can be 1 for impostors
    // impostors should/can use the same rendering as fronds, so they use the material type 'frond' as well
    // however you can distinguish them via the 'uiLodType'
    for (int mattype = 0; mattype < uiMaterialTypes; ++mattype)
    {
      unsigned char uiThisMaterialType = 0; // 0 == branch, 1 == frond, 2 == leaf
      fread (&uiThisMaterialType, sizeof (char), 1, pFile);

      unsigned char uiMeshes = 0;
      fread (&uiMeshes, sizeof (char), 1, pFile);

      if (uiMeshes > 0)
        out_Tree.m_LODs[lod].m_Meshes[mattype].resize (uiMeshes);

      for (int mesh = 0; mesh < uiMeshes; ++mesh)
      {
        out_Tree.m_LODs[lod].m_Meshes[mattype][mesh].m_iMaterialType = uiThisMaterialType;
        ReadMesh (pFile, out_Tree.m_LODs[lod].m_Meshes[mattype][mesh]);
      }
    }
  }

  fclose (pFile);
}

