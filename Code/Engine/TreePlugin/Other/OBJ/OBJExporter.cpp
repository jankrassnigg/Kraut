// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "PCH.h"

#include <KrautFoundation/Strings/StringFunctions.h>
#include "OBJExporter.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

namespace NS_OBJEXPORTER
{
  using namespace AE_NS_FOUNDATION;

  EXPORT_MESH::EXPORT_MESH ()
  {
    m_Position.reserve (1024 * 10);
    m_Normals.reserve (1024 * 10);
    m_TexCoords.reserve (1024 * 10);
    m_Faces.reserve (1024 * 10);
  }

  MATERIAL::MATERIAL ()
  {
    m_bUsed = false;
  }

  int EXPORT_MESH::AddMaterial (const MATERIAL& m)
  {
    m_Materials.push_back (m);
    return (int)m_Materials.size () - 1;
  }

  GROUP_INDICES::GROUP_INDICES ()
  {
    m_iLastPosition = 0;
    m_iLastNormal = 0;
    m_iLastTexCoord = 0;
    m_iLastFace = 0;
    m_sName = "Main Group";
    m_iMaterial = -1;
  }

  void EXPORT_MESH::StartNextGroup (const char* szName, int iMaterial)
  {
    m_Groups.push_back (GROUP_INDICES ());

    if (m_Groups.size () > 1)
      m_Groups.back () = m_Groups[m_Groups.size () - 2];

    m_Groups.back ().m_sName = szName;
    m_Groups.back ().m_iMaterial = iMaterial;
  }

  int EXPORT_MESH::AddPosition (const VEC3& v)
  {
    if ((!aeMath::IsFinite (v.x)) ||
        (!aeMath::IsFinite (v.y)) ||
        (!aeMath::IsFinite (v.z)))
    {
      AE_CHECK_DEV (false, "EXPORT_MESH::AddPosition: Vertex is degenerate.");

      m_Position.push_back (VEC3 (0, 0, 0));
    }
    else
      m_Position.push_back (v);

    m_Groups.back ().m_iLastPosition = (int)m_Position.size ();

    return ((int) m_Position.size () - 1);
  }

  int EXPORT_MESH::AddNormal (const VEC3& v)
  {
    m_Normals.push_back (v);

    m_Groups.back ().m_iLastNormal = (int)m_Normals.size ();

    return ((int) m_Normals.size () - 1);
  }

  int EXPORT_MESH::AddTexCoord (const TEXCOORD& tc)
  {
    m_TexCoords.push_back (tc);

    m_Groups.back ().m_iLastTexCoord = (int)m_TexCoords.size ();

    return ((int) m_TexCoords.size () - 1);
  }

  void EXPORT_MESH::AddFace (const FACE& face)
  {
    m_Faces.push_back (face);

    m_Groups.back ().m_iLastFace = (int)m_Faces.size ();
  }


  bool EXPORT_MESH::ExportMTL (const char* szFile)
  {
    char szFileName[256];
    strcpy (szFileName, szFile);
    szFileName[strlen (szFileName) - 3] = 'm';
    szFileName[strlen (szFileName) - 2] = 't';
    szFileName[strlen (szFileName) - 1] = 'l';

    FILE* pFile = fopen (szFileName, "wb");

    if (!pFile)
      return false;

    char szOutput[4096] = "";

    for (int m = 0; m < (int) m_Materials.size (); ++m)
    {
      if (!m_Materials[m].m_bUsed)
        continue;

      {
        aeStringFunctions::Format (szOutput, 4096, "\nnewmtl %s\n", m_Materials[m].m_sMaterialName.c_str ());
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      {
        aeStringFunctions::Format (szOutput, 4096, "map_Kd %s\n", m_Materials[m].m_sDiffuseMap.c_str ());
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      if (!m_Materials[m].m_sNormalMap.empty ())
      {
        {
          aeStringFunctions::Format (szOutput, 4096, "map_bump %s\n", m_Materials[m].m_sNormalMap.c_str ());
          int len = (int) strlen (szOutput);

          fwrite (szOutput, sizeof (char), len, pFile);
        }

        {
          aeStringFunctions::Format (szOutput, 4096, "bump %s\n", m_Materials[m].m_sNormalMap.c_str ());
          int len = (int) strlen (szOutput);

          fwrite (szOutput, sizeof (char), len, pFile);
        }
      }
    }

    fclose (pFile);

    return true;
  }

  bool EXPORT_MESH::ExportToFile (const char* szFile)
  {
    FILE* pFile = fopen (szFile, "wb");

    if (!pFile)
      return false;

    char szTemp[256];
    char* sMTLzFileName = szTemp;
    strcpy (sMTLzFileName, szFile);
    sMTLzFileName[strlen (sMTLzFileName) - 3] = 'm';
    sMTLzFileName[strlen (sMTLzFileName) - 2] = 't';
    sMTLzFileName[strlen (sMTLzFileName) - 1] = 'l';

    int iLastSeperator = (int)strlen (sMTLzFileName) - 1;
    for (; iLastSeperator >= 0; --iLastSeperator)
    {
      if ((sMTLzFileName[iLastSeperator] == '\\') || (sMTLzFileName[iLastSeperator] == '/'))
      {
        sMTLzFileName = &sMTLzFileName[iLastSeperator + 1];
        break;
      }
    }

    char szOutput[4096] = "";

    GROUP_INDICES PrevIdx;

    {
      aeStringFunctions::Format (szOutput, 4096, "\nmtllib %s\n", sMTLzFileName);
      int len = (int) strlen (szOutput);

      fwrite (szOutput, sizeof (char), len, pFile);
    }

    for (int g = 0; g < (int) m_Groups.size (); ++g)
    {
      const GROUP_INDICES& NextIdx = m_Groups[g];

      if (m_Groups.size () > 1)
      {
        aeStringFunctions::Format (szOutput, 4096, "\ng %s\n", NextIdx.m_sName.c_str ());
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      if (NextIdx.m_iMaterial != -1)
      {
        m_Materials[NextIdx.m_iMaterial].m_bUsed = true;
        aeStringFunctions::Format (szOutput, 4096, "usemtl %s\n", m_Materials[NextIdx.m_iMaterial].m_sMaterialName.c_str ());
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      for (int vert = PrevIdx.m_iLastPosition; vert < NextIdx.m_iLastPosition; ++vert)
      {
        const VEC3& v = m_Position[vert];

        aeStringFunctions::Format (szOutput, 4096, "v %1.8f %1.8f %1.8f\n", v.x, v.y, v.z);
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      for (int vert = PrevIdx.m_iLastTexCoord; vert < NextIdx.m_iLastTexCoord; ++vert)
      {
        const TEXCOORD& v = m_TexCoords[vert];

        aeStringFunctions::Format (szOutput, 4096, "vt %1.8f %1.8f\n", v.u, v.v);
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      for (int vert = PrevIdx.m_iLastNormal; vert < NextIdx.m_iLastNormal; ++vert)
      {
        const VEC3& v = m_Normals[vert];

        aeStringFunctions::Format (szOutput, 4096, "vn %1.8f %1.8f %1.8f\n", v.x, v.y, v.z);
        int len = (int) strlen (szOutput);

        fwrite (szOutput, sizeof (char), len, pFile);
      }

      for (int face = PrevIdx.m_iLastFace; face < NextIdx.m_iLastFace; ++face)
      {
        const FACE& Face = m_Faces[face];

        strcpy (szOutput, "f ");
        int len = (int) strlen (szOutput);
        fwrite (szOutput, sizeof (char), len, pFile);

        for (unsigned int vert = 0; vert < Face.m_Vertices.size (); ++vert)
        {
          const FACE_VERTEX& Vertex = Face.m_Vertices[vert];

          aeStringFunctions::Format (szOutput, 4096, "%i", Vertex.m_iPositionID + 1);
          len = (int) strlen (szOutput);
          fwrite (szOutput, sizeof (char), len, pFile);

          if (!m_TexCoords.empty ())
          {
            aeStringFunctions::Format (szOutput, 4096, "/%i", Vertex.m_iTexCoordID + 1);
            len = (int) strlen (szOutput);
            fwrite (szOutput, sizeof (char), len, pFile);
          }

          if (!m_Normals.empty ())
          {
            aeStringFunctions::Format (szOutput, 4096, "/%i", Vertex.m_iNormalID + 1);
            len = (int) strlen (szOutput);
            fwrite (szOutput, sizeof (char), len, pFile);
          }

          strcpy (szOutput, " ");
          len = (int) strlen (szOutput);
          fwrite (szOutput, sizeof (char), len, pFile);
        }

        strcpy (szOutput, "\n");
        len = (int) strlen (szOutput);
        fwrite (szOutput, sizeof (char), len, pFile);
      }

      PrevIdx = m_Groups[g];
    }

    fclose (pFile);

    return ExportMTL (szFile);
  }
}

