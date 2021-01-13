#include "PCH.h"
#include "../Other/OBJ/OBJLoader.h"
#include <KrautGraphics/glim/glim.h>

using namespace NS_GLIM;
using namespace NS_OBJLOADER;

bool LoadSimpleMesh (const char* szMeshFile, GLIM_BATCH& glim, float fScale)
{
  glim.Clear ();

  aeFilePath sAbsPath;
  if (!aeFileSystem::MakeValidPath (szMeshFile, false, &sAbsPath, nullptr))
    return false;

  MESH obj;
  std::map<std::string, MATERIAL> Materials;
  if (!LoadOBJFile (sAbsPath.c_str (), obj, Materials, "", false, true))
    return false;

  glim.BeginBatch ();
  glim.Attribute4ub ("attr_Color", 255, 255, 0, 255);

  for (aeUInt32 f = 0; f < obj.m_Faces.size (); ++f)
  {
    glim.Begin (GLIM_POLYGON);

    for (aeUInt32 v = 0; v < obj.m_Faces[f].m_Vertices.size (); ++v)
    {
      const VEC3 pos = fScale * obj.m_Positions[obj.m_Faces[f].m_Vertices[v].m_uiPositionID];
      glim.Vertex (pos.x, pos.y, pos.z);
    }

    glim.End ();
  }

  glim.EndBatch ();

  return true;
}


