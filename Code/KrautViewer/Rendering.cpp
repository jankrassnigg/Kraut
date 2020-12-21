#include "Main.h"

void UpdateCamera (float fTimeDiff);

kvShader g_GroundPlaneShader;
kvTexture g_GroundPlaneTexture;
kvVertexBuffer g_GroundPlaneBuffer;
kvTexture g_AmbientLUT;

kvShader g_ShaderStaticImpostor;
kvShader g_ShaderBillboardImpostor;
kvShader g_ShaderMaterialType[3];

VEC3 g_vLightPosition;

void RenderGroundPlane (void)
{
  kvRenderAPI::SetModelOrientation (0, 0, 0, 0);

  g_GroundPlaneBuffer.Activate ();
  g_GroundPlaneShader.Activate ();
  kvShader::BindTexture ("tex_Diffuse", g_GroundPlaneTexture, false);

  kvRenderAPI::DrawTriangles (2);
}

void RenderTree (const VEC3& vPos, float fRotation)
{
  kvRenderAPI::SetModelOrientation (vPos.x, vPos.y, vPos.z, fRotation);

  const VEC3 vCamDist = vPos - g_vCameraPosition;
  const float fDistance = vCamDist.GetLength ();

  int lod = 0;
  for (int i = 0; i < (int) g_Tree.m_LODs.size (); ++i)
  {
    lod = i;

    if (fDistance <= g_Tree.m_LODs[i].m_fLodDistance)
      break;
  }

  for (int type = 0; type < 3; ++type)
  {
    for (int i = 0; i < (int) g_Tree.m_LODs[lod].m_Meshes[type].size (); ++i)
    {
      unsigned int uiTriangles = g_Tree.m_LODs[lod].m_Meshes[type][i].m_VertexBuffer.GetNumTriangles ();

      if (uiTriangles > 0)
      {
        int iMaterialType = g_Tree.m_LODs[lod].m_Meshes[type][i].m_iMaterialType;
        int iMaterial = g_Tree.m_LODs[lod].m_Meshes[type][i].m_iMaterial;

        if (iMaterialType == 0) // branches
          glEnable (GL_CULL_FACE);
        else // fronds and billboard leaves
          glDisable (GL_CULL_FACE);

        if (g_Tree.m_LODs[lod].m_uiLodType == 0) // full mesh
        {
          kvShader::BindTexture ("tex_Diffuse", g_Tree.m_Materials[iMaterialType][iMaterial].m_Diffuse, iMaterialType != 0);
          kvShader::BindTexture ("tex_NormalMap", g_Tree.m_Materials[iMaterialType][iMaterial].m_NormalMap, iMaterialType != 0);
          kvShader::SetUniformFloat ("unif_VariationColor", g_Tree.m_Materials[iMaterialType][iMaterial].m_uiVariationColor[0] / 255.0f,
                                                            g_Tree.m_Materials[iMaterialType][iMaterial].m_uiVariationColor[1] / 255.0f,
                                                            g_Tree.m_Materials[iMaterialType][iMaterial].m_uiVariationColor[2] / 255.0f,
                                                            g_Tree.m_Materials[iMaterialType][iMaterial].m_uiVariationColor[3] / 255.0f);

          g_ShaderMaterialType[iMaterialType].Activate ();
        }
        else // impostor
        {
          kvShader::BindTexture ("tex_Diffuse", g_Tree.m_ImpostorMaterial.m_Diffuse, true);
          kvShader::BindTexture ("tex_NormalMap", g_Tree.m_ImpostorMaterial.m_NormalMap, true);

          if ((g_Tree.m_LODs[lod].m_uiLodType == 1) ||
              (g_Tree.m_LODs[lod].m_uiLodType == 2)) // static impostor
            g_ShaderStaticImpostor.Activate ();

          if (g_Tree.m_LODs[lod].m_uiLodType == 3) // billboard impostor
            g_ShaderBillboardImpostor.Activate();
        }

        g_Tree.m_LODs[lod].m_Meshes[type][i].m_VertexBuffer.Activate ();
        kvRenderAPI::DrawTriangles (uiTriangles);
      }
    }
  }
}

void SetWindowTitle (void)
{
  switch (g_RenderMode)
  {
  case kvRenderMode::AmbientLight:
    glutSetWindowTitle ("Kraut Viewer (Ambient Light)");
    break;
  case kvRenderMode::Bitangent:
    glutSetWindowTitle ("Kraut Viewer (BiTangents)");
    break;
  case kvRenderMode::Default:
    glutSetWindowTitle ("Kraut Viewer");
    break;
  case kvRenderMode::Diffuse:
    glutSetWindowTitle ("Kraut Viewer (Albedo)");
    break;
  case kvRenderMode::DiffuseLight:
    glutSetWindowTitle ("Kraut Viewer (Diffuse Light)");
    break;
  case kvRenderMode::FaceNormal:
    glutSetWindowTitle ("Kraut Viewer (Face Normals)");
    break;
  case kvRenderMode::MaterialType:
    glutSetWindowTitle ("Kraut Viewer (Material Type)");
    break;
  case kvRenderMode::PixelNormal:
    glutSetWindowTitle ("Kraut Viewer (Pixel Normal)");
    break;
  case kvRenderMode::Tangent:
    glutSetWindowTitle ("Kraut Viewer (Tangents)");
    break;
  case kvRenderMode::TreeCenterDir:
    glutSetWindowTitle ("Kraut Viewer (Tree Center Direction)");
    break;
  }
}

void RenderFrame (void)
{
  SetWindowTitle ();

  kvShader::BindTexture ("tex_Ambient", g_AmbientLUT, false);

  const float fTime = timeGetTime () / 1000.0f;

  static unsigned int uiLastUpdate = timeGetTime ();
  unsigned int uiTimeDiff = timeGetTime () - uiLastUpdate;
  uiLastUpdate = timeGetTime ();

  UpdateCamera (uiTimeDiff / 1000.0f);

  VEC3 vLookAt = g_vCameraPosition + g_vCameraDirection;
  kvRenderAPI::SetLookAt (g_vCameraPosition.x, g_vCameraPosition.y, g_vCameraPosition.z, vLookAt.x, vLookAt.y, vLookAt.z, 0, 1, 0);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  RenderGroundPlane ();

  const float fLightRotation = kvDegToRad * fTime * 90.0f;
  g_vLightPosition.SetVector (cosf (fLightRotation), 0.0f, sinf (fLightRotation));
  g_vLightPosition *= 8.0f;
  g_vLightPosition.y = 4.0f;

  VEC3 vTreeCenter = g_Tree.m_BBoxMin + (g_Tree.m_BBoxMax - g_Tree.m_BBoxMin) * 0.5f;

  kvShader::SetUniformFloat ("unif_LightPosition", g_vLightPosition.x, g_vLightPosition.y, g_vLightPosition.z, 1);
  kvShader::SetUniformFloat ("unif_TreeCenter", vTreeCenter.x, vTreeCenter.y, vTreeCenter.z, 1);

  float fWidth = (g_Tree.m_BBoxMax.x - g_Tree.m_BBoxMin.x) * 1.0f;

  float fRotation = 0;

  glPolygonMode (GL_FRONT_AND_BACK, g_bWireframe ? GL_LINE : GL_FILL);

  for (int x = -g_iNumTrees; x <= g_iNumTrees; ++x)
  {
    for (int y = -g_iNumTrees; y <= g_iNumTrees; ++y)
    {
      RenderTree (VEC3 (x * fWidth, 0, y * fWidth), fRotation);

      fRotation += 17;
    }
  }

  glutSwapBuffers ();
}

