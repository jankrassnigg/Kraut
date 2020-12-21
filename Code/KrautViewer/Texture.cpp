#include "Main.h"

map<string, unsigned int> kvTexture::s_LoadedFiles;

kvTexture::kvTexture ()
{
  m_uiTextureID = 0;
}

kvTexture::~kvTexture ()
{
}

void kvTexture::LoadTexture (const char* szPath)
{
  printf ("Loading Texture \"%s\"\n", szPath);

  static bool s_bInitializedIL = false;

  if (!s_bInitializedIL)
  {
    s_bInitializedIL = true;

    ilInit();
    iluInit();
    ilutRenderer(ILUT_OPENGL);
  }

  if (s_LoadedFiles.find (szPath) != s_LoadedFiles.end ())
  {
    m_uiTextureID = s_LoadedFiles[szPath];
    return;
  }

  m_uiTextureID = ilutGLLoadImage ((char*) szPath);

  if (m_uiTextureID == 0)
    throw exception ("Could not load texture.");

  s_LoadedFiles[szPath] = m_uiTextureID;

  glActiveTexture (GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, m_uiTextureID);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

  glGenerateMipmap (GL_TEXTURE_2D);
}
