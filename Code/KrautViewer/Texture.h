#ifndef TEXTURE_H
#define TEXTURE_H

#include <map>
using namespace std;

class kvTexture
{
public:
  kvTexture ();
  ~kvTexture ();

  void LoadTexture (const char* szPath);

  unsigned int GetTextureID (void) const { return m_uiTextureID; }

private:
  unsigned int m_uiTextureID;

  static map<string, unsigned int> s_LoadedFiles;
};




#endif

