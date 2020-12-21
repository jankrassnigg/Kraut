// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef OBJEXPORTER_H
#define OBJEXPORTER_H

#include <stdio.h>
#include <vector>
#include "Vec3.h"

namespace NS_OBJEXPORTER
{
	using namespace NS_MATHS;

  struct TEXCOORD
  {
    float u;
    float v;
  };

	struct FACE_VERTEX
	{
		FACE_VERTEX ()
		{
			m_iPositionID = -1;
			m_iNormalID = -1;
			m_iTexCoordID = -1;
		}

		int m_iPositionID;
		int m_iNormalID;
		int m_iTexCoordID;
	};

  struct FACE
  {
    std::vector<FACE_VERTEX> m_Vertices;
  };

  struct GROUP_INDICES
  {
    GROUP_INDICES ();

    int m_iLastPosition;
    int m_iLastNormal;
    int m_iLastTexCoord;
    int m_iLastFace;
    std::string m_sName;
    int m_iMaterial;
  };

  struct MATERIAL
  {
    MATERIAL ();

    bool m_bUsed;
    std::string m_sMaterialName;
    std::string m_sDiffuseMap;
    std::string m_sNormalMap;
  };

  class EXPORT_MESH
  {
  public:
    EXPORT_MESH ();

    int AddPosition (const VEC3& v);
    int AddNormal (const VEC3& v);
    int AddTexCoord (const TEXCOORD& tc);
    void AddFace (const FACE& face);
    int AddMaterial (const MATERIAL& m);

    bool ExportToFile (const char* szFile);

    void StartNextGroup (const char* szName, int iMaterial);

  private:
    bool ExportMTL (const char* szFile);

    std::vector<GROUP_INDICES> m_Groups;

    std::vector<VEC3> m_Position;
    std::vector<VEC3> m_Normals;
    std::vector<TEXCOORD> m_TexCoords;
    std::vector<FACE> m_Faces;
    std::vector<MATERIAL> m_Materials;
  };
}

#endif



