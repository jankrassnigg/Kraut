#include "Main.h"

bool g_bWireframe = false;
int g_iNumTrees = 5;
int g_RenderMode = kvRenderMode::Default;

void RenderFrame (void);
void CallbackKeyDown (unsigned char key, int x, int y);
void CallbackKeyUp (unsigned char key, int x, int y);
void CallbackSpecialDown (int key, int x, int y);
void CallbackSpecialUp (int key, int x, int y);
void CallbackMouseMotion (int x, int y);
void CallbackMouseDown (int, int, int, int);

void ResizeWindow (int w, int h)
{
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
  kvRenderAPI::SetPerspective (70, (float) w / (float) h, 0.01f, 1000.0f);
}

void CallbackIdle (void)
{
  glutPostRedisplay();
}

void Init (void) 
{
  g_AmbientLUT.LoadTexture ("AmbientLUT.tga");

  g_GroundPlaneTexture.LoadTexture ("GroundPlane.dds");

  g_GroundPlaneBuffer.AddVertexStream ("attr_Position", kvVertexBuffer::R32G32B32_FLOAT);
  g_GroundPlaneBuffer.AddVertexStream ("attr_TexCoord", kvVertexBuffer::R32G32_FLOAT);
  g_GroundPlaneBuffer.SetupBuffer (4, 2);

  g_GroundPlaneBuffer.SetVertexAttribute3f (0, 0, -100, 0, 100);
  g_GroundPlaneBuffer.SetVertexAttribute3f (0, 1,  100, 0, 100);
  g_GroundPlaneBuffer.SetVertexAttribute3f (0, 2,  100, 0,-100);
  g_GroundPlaneBuffer.SetVertexAttribute3f (0, 3, -100, 0,-100);

  g_GroundPlaneBuffer.SetVertexAttribute2f (1, 0, -50, 50);
  g_GroundPlaneBuffer.SetVertexAttribute2f (1, 1,  50, 50);
  g_GroundPlaneBuffer.SetVertexAttribute2f (1, 2,  50,-50);
  g_GroundPlaneBuffer.SetVertexAttribute2f (1, 3, -50,-50);

  g_GroundPlaneBuffer.SetTriangle (0, 0, 1, 2);
  g_GroundPlaneBuffer.SetTriangle (1, 0, 2, 3);

  g_GroundPlaneBuffer.UploadToGPU ();

  g_GroundPlaneShader.LoadShader ("Ground.vs", "Ground.fs");
  g_GroundPlaneShader.Activate ();

  kvShader::SetUniformFloat ("unif_RenderMode", (float) g_RenderMode);

  g_ShaderMaterialType[0].LoadShader ("Bark.vs", "Bark.fs");
  g_ShaderMaterialType[1].LoadShader ("Frond.vs", "Frond.fs");
  g_ShaderMaterialType[2].LoadShader ("Leaf.vs", "Leaf.fs");
  g_ShaderStaticImpostor.LoadShader ("Impostor.vs", "Impostor.fs");
  g_ShaderBillboardImpostor.LoadShader ("Billboard.vs", "Billboard.fs");
}

int main(int argc, char** argv)
{
  printf ("Usage:\n");
  printf ("Movement: WSAD, Arrow Keys, Mouse (Left Click + Drag)\n");
  printf ("Toggle Wireframe: SPACE\n");
  printf ("Change Render Mode: Home / End\n");
  printf ("Change Number of Trees: Page Up / Down\n");
  printf ("\n");
  printf ("Command Line Arguments:\n");
  printf ("-tree <path/to/tree.kraut> -> specifies which tree to load\n");
  printf ("-textures <path/to/texture/folder> -> specifies from which folder to load the tree textures\n");
  printf ("\n");

  try
  {
    string sTreeFile = "";

    for (int i=0; i < argc; ++i)
    {
      if ((strcmp (argv[i], "-textures") == 0) && (i+1 < argc))
      {
        g_sTextureFolder = argv[i+1];

        if (!g_sTextureFolder.empty () && g_sTextureFolder[g_sTextureFolder.size()-1] != '\\')
          g_sTextureFolder += '\\';

        printf ("Textures Folder set to \"%s\"\n", g_sTextureFolder.c_str ());
      }

      if ((strcmp (argv[i], "-tree") == 0) && (i+1 < argc))
      {
        sTreeFile = argv[i+1];

        printf ("Loading Tree File \"%s\"\n", sTreeFile.c_str ());
      }
    }

    if (sTreeFile == "")
    {
      printf ("No Tree file was specified.\nPlease use the '-tree' command to define which *.kraut file to load.\n\nAlso please specify the folder from which to load the textures using the '-textures' command.\n\n");
      return 1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize (1024, 768); 
    glutInitWindowPosition (10, 10);
    glutCreateWindow ("Kraut Viewer");

    glewInit ();
    Init();

    ImportKrautFile (sTreeFile.c_str(), g_Tree);

    

    glClearColor (50 / 255.0f, 115 / 255.0f, 255 / 255.0f, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthRange (0.0f, 1.0f);
    glClearDepth (1.0f);

    glutDisplayFunc (RenderFrame); 
    glutReshapeFunc (ResizeWindow);
    glutKeyboardFunc (CallbackKeyDown);
    glutKeyboardUpFunc (CallbackKeyUp);
    glutSpecialFunc (CallbackSpecialDown);
    glutSpecialUpFunc (CallbackSpecialUp);
    glutMouseFunc (CallbackMouseDown);
    glutMotionFunc (CallbackMouseMotion);
    glutIdleFunc(CallbackIdle);
    glutMainLoop();
  }
  catch (std::exception& e)
  {
    glutHideWindow ();
    MessageBoxA (NULL, e.what (), "Kraut Viewer - Exception", MB_ICONERROR);
  }

  return 0;
}