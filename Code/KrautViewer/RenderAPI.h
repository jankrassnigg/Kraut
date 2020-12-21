#ifndef RENDERAPI_H
#define RENDERAPI_H

#include "Shader.h"
#include "VertexBuffer.h"

class kvRenderAPI
{
public:

  static void DrawTriangles (unsigned int uiNumTriangles, unsigned int uiFirstTriangle = 0);

  static void SetFrustum (float left, float right, float bottom, float top, float fnear, float ffar);
  static void SetPerspective (float fovy, float aspect, float zNear, float zFar);
  static void SetOrtho (float left, float right, float bottom, float top, float fnear, float ffar);

  static void SetLookAt (float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz);

  static void SetModelOrientation (float posx, float posy, float posz, float fRotationY);



};


#endif

