#ifndef MAIN_H
#define MAIN_H

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "GL/glew.h"
#include "GL/freeglut.h"

#include <vector>
#include <string>
#include <deque>
#include <exception>
#include <map>
#include <math.h>
#include "Vec3.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"

using namespace std;

#define kvPI 3.1415926535897932384626433832795f
#define kvDegToRad (kvPI / 180.0f)

#include "Shader.h"
#include "VertexBuffer.h"
#include "RenderAPI.h"
#include "Texture.h"
#include "KrautImport.h"

extern bool g_bStrafeLeft;
extern bool g_bStrafeRight;
extern bool g_bMoveForwards;
extern bool g_bMoveBackwards;
extern bool g_bMoveUp;
extern bool g_bMoveDown;
extern bool g_bTurnLeft;
extern bool g_bTurnRight;
extern bool g_bTurnUp;
extern bool g_bTurnDown;

extern VEC3 g_vCameraPosition;
extern VEC3 g_vCameraDirection;
extern VEC3 g_vCameraUp;
extern VEC3 g_vCameraRight;
extern float g_fCameraRotationY;
extern float g_fCameraRotationX;
extern string g_sTextureFolder;

extern kvShader g_GroundPlaneShader;
extern kvTexture g_GroundPlaneTexture;
extern kvVertexBuffer g_GroundPlaneBuffer;
extern kvTexture g_AmbientLUT;

extern kvShader g_ShaderStaticImpostor;
extern kvShader g_ShaderBillboardImpostor;
extern kvShader g_ShaderMaterialType[3];

extern bool g_bWireframe;
extern int g_iNumTrees;

struct kvRenderMode
{
  enum Enum
  {
    Default,
    Diffuse,
    PixelNormal,
    FaceNormal,
    Tangent,
    Bitangent,
    DiffuseLight,
    AmbientLight,
    MaterialType,
    TreeCenterDir,

    ENUM_COUNT
  };
};

extern int g_RenderMode;

#endif

