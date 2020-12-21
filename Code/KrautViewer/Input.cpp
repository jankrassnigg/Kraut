#include "Main.h"

bool g_bStrafeLeft = false;
bool g_bStrafeRight = false;
bool g_bMoveForwards = false;
bool g_bMoveBackwards = false;
bool g_bMoveUp = false;
bool g_bMoveDown = false;
bool g_bTurnLeft = false;
bool g_bTurnRight = false;
bool g_bTurnUp = false;
bool g_bTurnDown = false;

VEC3 g_vCameraPosition (0, 3, 8);
VEC3 g_vCameraDirection (0, 0, -1);
VEC3 g_vCameraUp (0, 1, 0);
VEC3 g_vCameraRight (1, 0, 0);
float g_fCameraRotationY = 0;
float g_fCameraRotationX = 0;

void CallbackSpecialDown (int key, int x, int y)
{
  switch (key)
  {
  case GLUT_KEY_LEFT:
    g_bTurnLeft = true;
    break;
  case GLUT_KEY_RIGHT:
    g_bTurnRight = true;
    break;
  case GLUT_KEY_UP:
    g_bTurnUp = true;
    break;
  case GLUT_KEY_DOWN:
    g_bTurnDown = true;
    break;
  case GLUT_KEY_PAGE_UP:
    g_iNumTrees++;
    break;
  case GLUT_KEY_PAGE_DOWN:
    g_iNumTrees = max(g_iNumTrees - 1, 0);
    break;
  case GLUT_KEY_HOME:
    g_RenderMode = (g_RenderMode + 1) % kvRenderMode::ENUM_COUNT;
    break;
  case GLUT_KEY_END:
    g_RenderMode = g_RenderMode - 1;
    if (g_RenderMode < 0)
      g_RenderMode = kvRenderMode::ENUM_COUNT - 1;
    break;
  }

  kvShader::SetUniformFloat ("unif_RenderMode", (float) g_RenderMode);
}

void CallbackSpecialUp (int key, int x, int y)
{
  switch (key)
  {
  case GLUT_KEY_LEFT:
    g_bTurnLeft = false;
    break;
  case GLUT_KEY_RIGHT:
    g_bTurnRight = false;
    break;
  case GLUT_KEY_UP:
    g_bTurnUp = false;
    break;
  case GLUT_KEY_DOWN:
    g_bTurnDown = false;
    break;
  }
}

void CallbackKeyDown (unsigned char key, int x, int y)
{
  switch (key) 
  {
  case 27:
    exit(0);
    break;
  case 'w':
  case 'W':
    g_bMoveForwards = true;
    break;
  case 's':
  case 'S':
    g_bMoveBackwards = true;
    break;
  case 'a':
  case 'A':
    g_bStrafeLeft = true;
    break;
  case 'd':
  case 'D':
    g_bStrafeRight = true;
    break;
  case 'q':
  case 'Q':
    g_bMoveUp = true;
    break;
  case 'e':
  case 'E':
    g_bMoveDown = true;
    break;
  case ' ':
    g_bWireframe = !g_bWireframe;
    break;
  }
}

void CallbackKeyUp (unsigned char key, int x, int y)
{
  switch (key) 
  {
  case 27:
    exit(0);
    break;
  case 'w':
  case 'W':
    g_bMoveForwards = false;
    break;
  case 's':
  case 'S':
    g_bMoveBackwards = false;
    break;
  case 'a':
  case 'A':
    g_bStrafeLeft = false;
    break;
  case 'd':
  case 'D':
    g_bStrafeRight = false;
    break;
  case 'q':
  case 'Q':
    g_bMoveUp = false;
    break;
  case 'e':
  case 'E':
    g_bMoveDown = false;
    break;
  }
}

static int iLastX = 0;
static int iLastY = 0;

void CallbackMouseDown (int, int, int x, int y)
{
  iLastX = x;
  iLastY = y;
}

void CallbackMouseMotion (int x, int y)
{
  const int iDiffX = x - iLastX;
  const int iDiffY = y - iLastY;

  iLastX = x;
  iLastY = y;

  g_fCameraRotationX -= iDiffY * 0.5f;
  g_fCameraRotationY -= iDiffX * 0.5f;
}

void UpdateCamera (float fTimeDiff)
{
  const float fMoveSpeed = 10.0f * fTimeDiff;
  const float fTurnSpeed = 100.0f * fTimeDiff;

  if (g_bTurnRight)
    g_fCameraRotationY -= fTurnSpeed;
  if (g_bTurnLeft)
    g_fCameraRotationY += fTurnSpeed;
  if (g_bTurnUp)
    g_fCameraRotationX += fTurnSpeed;
  if (g_bTurnDown)
    g_fCameraRotationX -= fTurnSpeed;

  if (g_fCameraRotationY < 0.0f)
    g_fCameraRotationY += 360.0f;
  if (g_fCameraRotationY > 360.0f)
    g_fCameraRotationY -= 360.0f;
  if (g_fCameraRotationX < -80)
    g_fCameraRotationX = -80;
  if (g_fCameraRotationX > 80)
    g_fCameraRotationX = 80;

  g_vCameraRight.SetVector (cosf (kvDegToRad * g_fCameraRotationY), 0, -sinf (kvDegToRad * g_fCameraRotationY));
  g_vCameraDirection.SetVector (-sinf (kvDegToRad * g_fCameraRotationY), 0, -cosf (kvDegToRad * g_fCameraRotationY));

  g_vCameraDirection.y = sinf (kvDegToRad * g_fCameraRotationX);
  g_vCameraDirection.x *= cosf (kvDegToRad * g_fCameraRotationX);
  g_vCameraDirection.z *= cosf (kvDegToRad * g_fCameraRotationX);

  g_vCameraUp = Cross (g_vCameraRight, g_vCameraDirection);

  g_vCameraRight.Normalize ();
  g_vCameraDirection.Normalize ();
  g_vCameraUp.Normalize ();

  if (g_bMoveForwards)
    g_vCameraPosition += g_vCameraDirection * fMoveSpeed;
  if (g_bMoveBackwards)
    g_vCameraPosition -= g_vCameraDirection * fMoveSpeed;
  if (g_bStrafeLeft)
    g_vCameraPosition -= g_vCameraRight * fMoveSpeed;
  if (g_bStrafeRight)
    g_vCameraPosition += g_vCameraRight * fMoveSpeed;
  if (g_bMoveUp)
    g_vCameraPosition += VEC3 (0, 1, 0) * fMoveSpeed;
  if (g_bMoveDown)
    g_vCameraPosition -= VEC3 (0, 1, 0) * fMoveSpeed;

  kvShader::SetUniformFloat ("unif_CameraPos", g_vCameraPosition.x, g_vCameraPosition.y, g_vCameraPosition.z, 1);
  kvShader::SetUniformFloat ("unif_CameraRight", g_vCameraRight.x, g_vCameraRight.y, g_vCameraRight.z, 0);
  kvShader::SetUniformFloat ("unif_CameraUp", g_vCameraUp.x, g_vCameraUp.y, g_vCameraUp.z, 0);
  kvShader::SetUniformFloat ("unif_CameraDir", g_vCameraDirection.x, g_vCameraDirection.y, g_vCameraDirection.z, 0);
}

