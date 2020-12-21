#include "PCH.h"

#include "Plugin.h"
#include "../Tree/Tree.h"
#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include "../GUI/qtResourceEditorWidget/qtResourceEditorWidget.moc.h"
#include "../GUI/qtCurveEditorWidget/qtCurveEditorWidget.moc.h"
#include "../GUI/qtTreeConfigWidget/qtTreeConfigWidget.moc.h"
#include <KrautFoundation/Configuration/VariableRegistry.h>
#include <KrautFoundation/Configuration/Startup.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/glim/glim.h>
#include <KrautFoundation/FileSystem/FileSystem.h>

aeCVarFloat CVar_Gamma ("Tree_Gamma", 2.2f, aeCVarFlags::Save, "The gamma value to use for gamma correction.");

bool LoadSimpleMesh (const char* szMeshFile, GLIM_BATCH& glim, float fScale);

void SetShaderGammaValue (void)
{
  const float fInverseGamma = 1.0f / CVar_Gamma;

  aeShaderManager::setUniformFloat ("unif_ColorScale", 1, 1.0f);
  aeShaderManager::setUniformFloat ("unif_vGamma", 2, fInverseGamma, CVar_Gamma);
}

using namespace AE_NS_GRAPHICS;
using namespace NS_GLIM;

static aeString GetBinaryDirectory (void)
{
  char szTemp[512] = "";
  GetModuleFileName (NULL, szTemp, 512);

  return (aePathFunctions::GetFileDirectory (szTemp));
}

static void CoreStartup (void)
{
}

static void CoreShutdown (void)
{
}

void GLIM_Callback (void)
{
  aeShaderManager::PrepareForRendering ();
  aeRenderAPI::ApplyStates ();
}

static void EngineStartup (void)
{
  aeFileSystem::AddDataDirectory ((aePathFunctions::GetParentDirectory (GetBinaryDirectory ()) + "../../Data/TreePlugin").c_str (), "", false);

  aeShaderManager::setShaderBuilderVariable ("EXPORT_LEAFCARD", false);
  aeShaderManager::setShaderBuilderVariable ("GROUNDPLANE_DISCARD", false);
  aeShaderManager::setShaderBuilderVariable ("OUTPUT_NORMALS_ONLY", false);
  aeShaderManager::setShaderBuilderVariable ("OUTPUT_DIFFUSE_ONLY", false);
  aeShaderManager::setShaderBuilderVariable ("OUTPUT_LIGHT_ONLY", false);
  aeShaderManager::setShaderBuilderVariable ("OUTPUT_AMBIENT_ONLY", false);
  aeShaderManager::setShaderBuilderVariable ("OUTPUT_SKYLIGHT_ONLY", false);
  aeShaderManager::setShaderBuilderVariable ("WIREFRAME", false);
  aeShaderManager::setShaderBuilderVariable ("PICKING", false);

  SetShaderGammaValue ();
  
  g_Globals.s_hBranchShader = aeShaderResource::LoadResource ("Shaders/Branch.shader");
  g_Globals.s_hFrondShader = aeShaderResource::LoadResource ("Shaders/Fronds.shader");
  g_Globals.s_hImpostorShader = aeShaderResource::LoadResource ("Shaders/Impostor.shader");
  g_Globals.s_hLeafShader = aeShaderResource::LoadResource ("Shaders/Leaves.shader");
  g_Globals.s_hSkeletonShader = aeShaderResource::LoadResource ("Shaders/TreeSkeleton.shader");
  g_Globals.s_hGroundPlaneShader = aeShaderResource::LoadResource ("Shaders/GroundPlane.shader");
  g_Globals.s_hPhysicsObjectShader = aeShaderResource::LoadResource ("Shaders/CollisionObject.shader");
  g_Globals.s_hBillboardShader = aeShaderResource::LoadResource ("Shaders/Billboard.shader");
  g_Globals.s_hForceShader = aeShaderResource::LoadResource ("Shaders/Force.shader");
  g_Globals.s_hSimpleGizmoShader = aeShaderResource::LoadResource ("Shaders/SimpleGizmo.shader");

  aeTextureResourceHandle hNoiseTexture = aeTextureResource::LoadResource ("Textures/Noise.tga");
  aeShaderManager::BindTexture ("tex_Noise", hNoiseTexture);

  LoadSimpleMesh ("Gizmos/Magnet.obj", g_Globals.s_GizmoMagnet, 1.0f);
  LoadSimpleMesh ("Gizmos/Direction.obj", g_Globals.s_GizmoDirection, 1.0f);
  

  aeSamplerState ss;
  ss.m_TextureFilter = AE_FILTER_BILINEAR;
  ss.m_TextureWrapU = AE_WRAP_CLAMP;
  ss.m_TextureWrapV = AE_WRAP_CLAMP;
  aeTextureManager::setTextureSampler ("LUT", ss);

  ss.m_TextureFilter = AE_FILTER_ANISOTROPIC_8X;
  ss.m_TextureWrapU = AE_WRAP_REPEAT;
  ss.m_TextureWrapV = AE_WRAP_REPEAT;
  aeTextureManager::setTextureSampler ("BRANCH", ss);
  aeMeshType::TextureSampler[aeMeshType::Branch] = aeTextureManager::getTextureSamplerID ("BRANCH");

  ss.m_TextureFilter = AE_FILTER_ANISOTROPIC_8X;
  ss.m_TextureWrapU = AE_WRAP_REPEAT;
  ss.m_TextureWrapV = AE_WRAP_REPEAT;
  aeTextureManager::setTextureSampler ("FROND", ss);
  aeMeshType::TextureSampler[aeMeshType::Frond] = aeTextureManager::getTextureSamplerID ("FROND");

  ss.m_TextureFilter = AE_FILTER_ANISOTROPIC_8X;
  ss.m_TextureWrapU = AE_WRAP_CLAMP;
  ss.m_TextureWrapV = AE_WRAP_CLAMP;
  aeTextureManager::setTextureSampler ("LEAF", ss);
  aeMeshType::TextureSampler[aeMeshType::Leaf] = aeTextureManager::getTextureSamplerID ("LEAF");

  ss.m_TextureFilter = AE_FILTER_ANISOTROPIC_8X;
  ss.m_TextureWrapU = AE_WRAP_CLAMP;
  ss.m_TextureWrapV = AE_WRAP_CLAMP;
  aeTextureManager::setTextureSampler ("IMPOSTOR", ss);
  aeMeshType::ImpostorTextureSampler = aeTextureManager::getTextureSamplerID ("IMPOSTOR");

  GLIM_BATCH::s_StateChangeCallback = GLIM_Callback;
}

static void EngineShutdown (void)
{
  aeFileSystem::RemoveDataDirectory ("Applications/TreePlugin");
}

AE_ON_GLOBAL_EVENT_ONCE (aeStartup_StartupCore_Begin)
{
  aeStartup::RegisterModule ("aeTreeGenerator", CoreStartup, CoreShutdown, EngineStartup, EngineShutdown, "aeFoundation", "aeRenderAPI", "aeShaderManager");
}

AE_ON_GLOBAL_EVENT_ONCE (aeMainWindow_SetupUI_Begin)
{
  AE_CHECK_ALWAYS (QApplication::instance () != NULL, "The QApplication is not initialized.");

  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  AE_CHECK_ALWAYS (pMainWindow != NULL, "Plugin requires a pointer to the Main Widget.");

  pMainWindow->setWindowIcon (QIcon (":/Icons/Icons/Application.png"));

  // setup the tree edit control widget
  {
    qtTreeEditWidget* pWidget = new qtTreeEditWidget (pMainWindow);
    AE_CHECK_DEV (pWidget != NULL, "");
  }

  // setup the material edit control widget
  {
    qtResourceEditorWidget* pWidget = new qtResourceEditorWidget (pMainWindow);
    AE_CHECK_DEV (pWidget != NULL, "");
  }

  // setup the curve editor widget
  {
    qtCurveEditorWidget* pWidget = new qtCurveEditorWidget (pMainWindow);
    AE_CHECK_DEV (pWidget != NULL, "");
  }

  // setup the tree config widget
  {
    qtTreeConfigWidget* pWidget = new qtTreeConfigWidget (pMainWindow);
    AE_CHECK_DEV (pWidget != NULL, "");
  }
}

