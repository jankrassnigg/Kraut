#include "Main.h"

kvShader* kvShader::s_pCurrentShader = NULL;
map<string, kvShader::Matrix4x4> kvShader::s_Matrices;
map<string, kvShader::Vector4> kvShader::s_Vectors;
map<string, kvShader::BoundTexture> kvShader::s_Textures;

// Reads one entire file and returns it as a string.
static string LoadTextFile (const char* szFile)
{
  FILE* pFile = fopen (szFile, "rb");

  if (!pFile)
  {
    char szTemp[256];
    sprintf (szTemp, "Could not open file \"%s\".", szFile);
    throw exception (szTemp);
  }

  string sResult;

  char szTemp[1024];
  while (!feof (pFile))
  {
    int iRead = fread (szTemp, 1, 1023, pFile);
    szTemp[iRead] = '\0';

    sResult += szTemp;
  }

  fclose (pFile);

  return sResult;
}

static unsigned int CompileShader (GLenum ShaderType, const char* szSource)
{
  unsigned int uiShader = glCreateShader (ShaderType);
  glShaderSource (uiShader, 1, &szSource, NULL);

  glCompileShader (uiShader);

  int iStatus = 0;
  glGetShaderiv (uiShader, GL_COMPILE_STATUS, &iStatus);

  if (iStatus == GL_FALSE) 
  {
    int iInfoLogLength;
    vector<char> sInfoLog;

    glGetShaderiv (uiShader, GL_INFO_LOG_LENGTH, &iInfoLogLength);
    sInfoLog.resize (iInfoLogLength + 1);

    glGetShaderInfoLog (uiShader, iInfoLogLength, NULL, &sInfoLog[0]);
    sInfoLog[iInfoLogLength] = '\0';

    const char* szError = &sInfoLog[0];

    throw exception (szError);
  }

  return uiShader;
}

static unsigned int CreateProgram (unsigned int uiVertexShader, unsigned int uiFragmentShader)
{
  unsigned int uiProgram = glCreateProgram ();

  glAttachShader (uiProgram, uiVertexShader);
  glAttachShader (uiProgram, uiFragmentShader);

  
  glLinkProgram (uiProgram);

  int iStatus = 0;
  glGetProgramiv (uiProgram, GL_LINK_STATUS, &iStatus);

  if (iStatus == GL_FALSE) 
  {
    int iInfoLogLength;
    vector<char> sInfoLog;

    glGetProgramiv (uiProgram, GL_INFO_LOG_LENGTH, &iInfoLogLength);
    sInfoLog.resize (iInfoLogLength + 1);

    glGetProgramInfoLog (uiProgram, iInfoLogLength, NULL, &sInfoLog[0]);
    sInfoLog[iInfoLogLength] = '\0';

    const char* szError = &sInfoLog[0];

    throw exception (szError);
  }

  return uiProgram;
}

kvShader::kvShader ()
{
  m_uiProgramID = 0xFFFFFFFF;
}

kvShader::~kvShader ()
{
  // someone should clean up here...
}

void kvShader::LoadShader (const char* szVertexShaderFile, const char* szFragmentShaderFile)
{
  printf ("Loading Shader \"%s\" | \"%s\"\n", szVertexShaderFile, szFragmentShaderFile);

  const string sVertexShader   = LoadTextFile (szVertexShaderFile);
  const string sFragmentShader = LoadTextFile (szFragmentShaderFile);

  unsigned int uiVertexShader   = CompileShader (GL_VERTEX_SHADER,   sVertexShader.c_str ());
  unsigned int uiFragmentShader = CompileShader (GL_FRAGMENT_SHADER, sFragmentShader.c_str ());

  m_uiProgramID = CreateProgram (uiVertexShader, uiFragmentShader);
}

void kvShader::Activate (void)
{
  s_pCurrentShader = this;

  glUseProgram (m_uiProgramID);
}

void kvShader::SetUniformMatrix (const char* szName, float* fMatrix)
{
  Matrix4x4 m;
  memcpy (&m.m_Elements[0], fMatrix, sizeof (float) * 16);

  s_Matrices[szName] = m;
}

void kvShader::SetUniformFloat  (const char* szName, float x, float y, float z, float w)
{
  Vector4 v;
  v.m_Elements[0] = x;
  v.m_Elements[1] = y;
  v.m_Elements[2] = z;
  v.m_Elements[3] = w;

  s_Vectors[szName] = v;
}

void kvShader::BindTexture (const char* szUniformName, unsigned int uiTextureID, bool bTexCoordClamp)
{
  s_Textures[szUniformName].m_TexID = uiTextureID;
  s_Textures[szUniformName].m_bClamp = bTexCoordClamp;
}

void kvShader::BindTexture (const char* szUniformName, const kvTexture& Texture, bool bTexCoordClamp)
{
  BindTexture (szUniformName, Texture.GetTextureID (), bTexCoordClamp);
}

void kvShader::UpdateUniformMatrices (void)
{
  const map<string, Matrix4x4>::iterator itend = s_Matrices.end ();
  for (map<string, Matrix4x4>::iterator it = s_Matrices.begin (); it != itend; ++it)
  {
    int iUniformLocation = glGetUniformLocation (kvShader::s_pCurrentShader->m_uiProgramID, it->first.c_str ());

    if (iUniformLocation >= 0)
    {
      glUniformMatrix4fv (iUniformLocation, 1, GL_FALSE, &it->second.m_Elements[0]);
    }
  }
}

void kvShader::UpdateUniformVectors (void)
{
  const map<string, Vector4>::iterator itend = s_Vectors.end ();
  for (map<string, Vector4>::iterator it = s_Vectors.begin (); it != itend; ++it)
  {
    int iUniformLocation = glGetUniformLocation (kvShader::s_pCurrentShader->m_uiProgramID, it->first.c_str ());

    if (iUniformLocation >= 0)
    {
      glUniform4f (iUniformLocation, it->second.m_Elements[0], it->second.m_Elements[1], it->second.m_Elements[2], it->second.m_Elements[3]);
    }
  }
}

void kvShader::UpdateUniformTextures (void)
{
  int iTextureUnit = 0;

  const map<string, kvShader::BoundTexture>::iterator itend = s_Textures.end ();
  for (map<string, kvShader::BoundTexture>::iterator it = s_Textures.begin (); it != itend; ++it)
  {
    int iUniformLocation = glGetUniformLocation (kvShader::s_pCurrentShader->m_uiProgramID, it->first.c_str ());

    if (iUniformLocation >= 0)
    {
      if (iTextureUnit >= 16)
        throw exception ("The shader uses too many input textures.");

      BoundTexture bt = it->second;

      glActiveTexture (GL_TEXTURE0 + iTextureUnit);
      glEnable (GL_TEXTURE_2D);
      glBindTexture (GL_TEXTURE_2D, bt.m_TexID);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bt.m_bClamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bt.m_bClamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);

      glUniform1i (iUniformLocation, iTextureUnit);
      ++iTextureUnit;
    }
  }
}

void kvShader::UpdateUniforms (void)
{
  if (!kvShader::s_pCurrentShader)
    return;

  UpdateUniformMatrices ();
  UpdateUniformVectors ();
  UpdateUniformTextures ();
}

int kvShader::GetAttributeLocation (const char* szAttributeName) const
{
  return glGetAttribLocation(m_uiProgramID, szAttributeName);
}

