#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <string>
using namespace std;

class kvTexture;

class kvShader
{
public:
  kvShader ();
  ~kvShader ();

  unsigned int GetProgramID (void) const { return m_uiProgramID; }

  void LoadShader (const char* szVertexShaderFile, const char* szFragmentShaderFile);

  // Sets this shader as the active one.
  void Activate (void);

  // Will synchronize all known uniforms with the currently bound shader. Call this before a drawcall.
  static void UpdateUniforms (void);

  static void SetUniformMatrix (const char* szName, float* fMatrix);
  static void SetUniformFloat  (const char* szName, float x, float y = 0, float z = 0, float w = 0);
  static void BindTexture (const char* szUniformName, unsigned int uiTextureID, bool bTexCoordClamp);
  static void BindTexture (const char* szUniformName, const kvTexture& Texture, bool bTexCoordClamp);

  static kvShader* GetActiveShader (void) { return s_pCurrentShader; }

  // Returns the index at which the attribute with the given name should be bound. -1 if the shader doesn't use that attribute.
  int GetAttributeLocation (const char* szAttributeName) const;

private:
  static void UpdateUniformMatrices (void);
  static void UpdateUniformVectors (void);
  static void UpdateUniformTextures (void);

  unsigned int m_uiProgramID;

  struct Matrix4x4
  {
    float m_Elements[16];
  };

  struct Vector4
  {
    float m_Elements[4];
  };

  struct BoundTexture
  {
    unsigned int m_TexID;
    bool m_bClamp;
  };

  static kvShader* s_pCurrentShader;
  static map<string, Matrix4x4> s_Matrices;
  static map<string, Vector4> s_Vectors;
  static map<string, BoundTexture> s_Textures;
};


#endif

