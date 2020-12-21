#include "Main.h"

void kvRenderAPI::DrawTriangles (unsigned int uiNumTriangles, unsigned int uiFirstTriangle)
{
  kvShader* pShader = kvShader::GetActiveShader ();

  if (!pShader)
    throw exception ("kvRenderAPI::DrawTriangles: No shader is active."); 

  pShader->UpdateUniforms ();

  kvVertexBuffer* pBuffer = kvVertexBuffer::GetActiveBuffer ();

  if (!pBuffer)
    throw exception ("kvRenderAPI::DrawTriangles: No vertex buffer is active.");

  pBuffer->ConnectBufferWithShader (pShader);

  const unsigned int uiMaxTriangles = pBuffer->GetNumTriangles ();

  if (uiFirstTriangle + uiNumTriangles > uiMaxTriangles)
    throw exception ("kvRenderAPI::DrawTriangles: The uiFirstTriangle + uiNumTriangles is larger than the amount of triangles that is in the active buffer.");

  unsigned int uiOffset = (uiFirstTriangle * 3) * sizeof (int);
  glDrawElements (GL_TRIANGLES, uiNumTriangles * 3, GL_UNSIGNED_INT, (void*) uiOffset);
}

void kvRenderAPI::SetFrustum (float left, float right, float bottom, float top, float fnear, float ffar)
{
  float fMatrix[4][4];
  memset (&fMatrix[0][0], 0, sizeof (float) * 16);

  fMatrix[0][0] = 2.0f * fnear / (right -left);
  fMatrix[1][1] = 2.0f * fnear / (top - bottom);
  fMatrix[2][2] = - (ffar + fnear) / (ffar - fnear);
            
  fMatrix[2][0] = (right + left) / (right - left);
  fMatrix[2][1] = (top + bottom) / (top - bottom);
  fMatrix[3][2] = -2.0f * ffar * fnear / (ffar - fnear);
  fMatrix[2][3] = -1.0f;

  kvShader::SetUniformMatrix ("kvCameraToProjection", &fMatrix[0][0]);
}

void kvRenderAPI::SetPerspective (float fovy, float aspect, float zNear, float zFar)
{
  const float radians = kvDegToRad * (fovy / 2.0f);

  float xmin, xmax, ymin, ymax;

  ymax = zNear * tanf (radians);
  ymin = -ymax;

  xmin = ymin * aspect;
  xmax = ymax * aspect;

  SetFrustum (xmin, xmax, ymin, ymax, zNear, zFar);
}

void kvRenderAPI::SetOrtho (float left, float right, float bottom, float top, float fnear, float ffar)
{
  float fMatrix[4][4];
  memset (&fMatrix[0][0], 0, sizeof (float) * 16);

  fMatrix[0][0] = 2.0f / (right - left);
  fMatrix[3][0] = -(right + left) / (right - left);
            
  fMatrix[1][1] = 2.0f / (top - bottom);
  fMatrix[3][1] = -(top + bottom) / (top - bottom);
            
  fMatrix[2][2] = -2.0f / (ffar - fnear);
  fMatrix[3][2] = -(ffar + fnear) / (ffar - fnear);
            
  fMatrix[3][3] = 1.0f;

  kvShader::SetUniformMatrix ("kvCameraToProjection", &fMatrix[0][0]);
}

void kvRenderAPI::SetLookAt (float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz)
{
  float fMatrix[4][4];
  memset (&fMatrix[0][0], 0, sizeof (float) * 16);

  const VEC3 center (centerx, centery, centerz);
  const VEC3 eye (eyex, eyey, eyez);
  VEC3 up (upx, upy, upz);

  const VEC3 forward = (center - eye).GetNormalized ();

  const VEC3 side = Cross (forward, up).GetNormalized ();

  /* Recompute up as: up = side x forward */
  up = Cross (side, forward);

  fMatrix[0][0] = side.x;
  fMatrix[1][0] = side.y;
  fMatrix[2][0] = side.z;
            
  fMatrix[0][1] = up.x;
  fMatrix[1][1] = up.y;
  fMatrix[2][1] = up.z;
            
  fMatrix[0][2] = -forward.x;
  fMatrix[1][2] = -forward.y;
  fMatrix[2][2] = -forward.z;

  fMatrix[3][3] = 1.0f;

  fMatrix[3][0] = fMatrix[0][0] * -eyex + fMatrix[1][0] * -eyey + fMatrix[2][0] * -eyez;
  fMatrix[3][1] = fMatrix[0][1] * -eyex + fMatrix[1][1] * -eyey + fMatrix[2][1] * -eyez;
  fMatrix[3][2] = fMatrix[0][2] * -eyex + fMatrix[1][2] * -eyey + fMatrix[2][2] * -eyez;

  kvShader::SetUniformMatrix ("kvWorldToCamera", &fMatrix[0][0]);
}

void kvRenderAPI::SetModelOrientation (float posx, float posy, float posz, float fRotationY)
{
  float fMatrix[4][4];
  memset (&fMatrix[0][0], 0, sizeof (float) * 16);

  fMatrix[3][0] = posx;
  fMatrix[3][1] = posy;
  fMatrix[3][2] = posz;
  fMatrix[3][3] = 1;

  const float fSin = sinf (fRotationY);
  const float fCos = cosf (fRotationY);

  fMatrix[0][0] = fCos;
  fMatrix[2][0] = fSin;
  fMatrix[1][1] = 1.0f;
  fMatrix[0][2] = -fSin;
  fMatrix[2][2] = fCos;

  kvShader::SetUniformMatrix ("kvObjectToWorld", &fMatrix[0][0]);
}
