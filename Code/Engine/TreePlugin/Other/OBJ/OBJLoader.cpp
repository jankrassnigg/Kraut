// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "PCH.h"

#include "OBJLoader.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include "../../Basics/Globals.h"
#include "../../GUI/ProgressBar.h"

using namespace std;

namespace NS_OBJLOADER
{
	/* This is a helper-function, you don't need to use it, it is only there for convenience.
		It iterates over all materials and calls the Loader, provided by the user, for every material (once).
		"pPassThrough" is passed through to the Loader, which can use it to read or store additional information.
	*/
	void LoadTextures (map<string, MATERIAL>& Materials, TEXTURE_LOADER Loader, void* pPassThrough)
	{
		map<string, MATERIAL>::iterator it, itend;

		it = Materials.begin ();
		itend = Materials.end ();

		for (; it != itend; ++it)
			Loader (it->second, it->second.m_uiMaterialID, pPassThrough);
	}

	//! Loads an MTL-file and stores all found Materials in the map provided.
	/*! "sMaterialBasePath" is prepended to every path, which allows to easily make relative paths in the MTL files 
		into absolute paths. Returns without error, and does nothing, if the file cannot be opened.
	*/
	void LoadMTLFile (const char *szFile, map<string, MATERIAL>& Materials, const string& sMaterialBasePath)
	{
		FILE* pFile = fopen (szFile, "rb");

		if (!pFile)
			return;

		// temporary variables for parsing
		char szLine[1024] = "";
		char szID[512] = "";

		string sCurrentMaterialName;

		while (fscanf (pFile, "%s", szID) > 0)
		{
			if (strcmp (szID, "newmtl") == 0)		// declares a new material with a given name
			{
				fscanf (pFile, "%s", szLine);

				sCurrentMaterialName = szLine;									// keep the name, for lookup

				if (Materials.find (sCurrentMaterialName) == Materials.end ())	// checks, whether a material with the same name is known already
				{
					// material is unknown so far, so store it
					unsigned int uiID = (unsigned int) Materials.size ();						// current (!) size, will be incremented on the next line, when "looking" up the non-existing texture name

					Materials[sCurrentMaterialName].m_uiMaterialID = uiID;		// just store the current number of materials as the ID
				}
			}
			else
			if (strcmp (szID, "map_Kd") == 0)		// specifies the diffuse texture's path
			{
				fscanf (pFile, "%s", szLine);

				Materials[sCurrentMaterialName].m_sDiffuseTexture = sMaterialBasePath + szLine;
					
			}
/*
			else
			if (strcmp (szID, "some_other_attrib") == 0)
			{
				// if you need to read other attributes from an MTL-file, add this here !

			}
*/
			else
				fgets (szLine, sizeof (char) * 1024, pFile);	// read anything, that is not understood
		}

		fclose (pFile);
	}

	bool LoadOBJFile (const char *szFile, MESH& Mesh, map<string, MATERIAL>& Materials, const char* szMaterialBasePath, bool bSortByMaterial, bool bIgnoreMaterials)
	{
    aeFilePath sText;
    sText.Format ("Loading Mesh:\n%s", szFile);
    aeProgressBar pb (sText.c_str (), 4);

		// first make sure the mesh is empty
		Mesh.MakeEmpty ();

		// Materials are NOT cleared, in case one wants to use ONE big map to load all material info into (and thus Material IDs need to be consistent in all meshes)

		// load the information from the MTL file
		if (!bIgnoreMaterials)
		{
			char szMaterial[256] = "";
			strcpy (szMaterial, szFile);
			szMaterial[strlen (szMaterial) - 3] = '\0';
			strcat (szMaterial, "mtl");

			LoadMTLFile (szMaterial, Materials, szMaterialBasePath);
		}

    aeProgressBar::Update (); // 1

		// open and read the OBJ file
		FILE* pFile = fopen (szFile, "rb");

		if (!pFile)
			return (false);

		// temporary vairables for parsing
		char szLine[1024] = "";
		char szID[512] = "";

		// which data has been found in the file
		bool bContainsTexCoords = false;
		bool bContainsNormals = false;

		unsigned int uiCurMaterial = 0;

		while (fscanf (pFile, "%s", szID) > 0)
		{
      aeProgressBar::Update (NULL, 0); // just to ensure updates

			if (strcmp (szID, "v") == 0)	// line declares a vertex
			{
				VEC3 v;
				fscanf (pFile, "%f %f %f", &v.x, &v.y, &v.z);

				Mesh.m_Positions.push_back (v);
			}
			else
			if (strcmp (szID, "vt") == 0)	// line declares a texture coordinate
			{
				bContainsTexCoords = true;

				VEC3 v;
				fscanf (pFile, "%f %f %f", &v.x, &v.y, &v.z);	// reads up to three texture-coordinates, but only two are actually stored

				TEXCOORD tc;
				tc.m_fTexCoordU = v.x;
				tc.m_fTexCoordV = v.y;
				// if you need 3D tex-coords, store the third one here, too
					
				Mesh.m_TexCoords.push_back (tc);
			}
			else
			if (strcmp (szID, "vn") == 0)	// line declares a normal
			{
				bContainsNormals = true;

				VEC3 v;
				fscanf (pFile, "%f %f %f", &v.x, &v.y, &v.z);
				v.Normalize ();	// make sure normals are indeed normalized

				Mesh.m_Normals.push_back (v);
			}
			else
			if (strcmp (szID, "f") == 0)	// line declares a face
			{
				FACE Face;
				Face.m_uiMaterialID = uiCurMaterial;

				// loop through all vertices, that are found
				while (true)
				{
					FACE_VERTEX Vertex;

					// read the position index
					if (fscanf (pFile, "%i", &Vertex.m_uiPositionID) <= 0)
						break;	// nothing found, face-declaration is finished

					--Vertex.m_uiPositionID;		// OBJ indices start at 1, so decrement them to start at 0

					// texcoords were declared, so they will be used in the faces
					if (bContainsTexCoords)
					{
						if (fscanf (pFile, "/%i", &Vertex.m_uiTexCoordID) <= 0)
							break;

						--Vertex.m_uiTexCoordID;	// OBJ indices start at 1, so decrement them to start at 0
					}

					// normals were declared, so they will be used in the faces
					if (bContainsNormals)
					{
						if (fscanf (pFile, "/%i", &Vertex.m_uiNormalID) <= 0)
							break;
 
						--Vertex.m_uiNormalID;		// OBJ indices start at 1, so decrement them to start at 0
					}

					// stores the next vertex of the face
					Face.m_Vertices.push_back (Vertex);
				}

				// only allow faces with at least 3 vertices
				if (Face.m_Vertices.size () >= 3)
				{
					VEC3 v1, v2, v3;
					v1 = Mesh.m_Positions[Face.m_Vertices[0].m_uiPositionID];
					v2 = Mesh.m_Positions[Face.m_Vertices[1].m_uiPositionID];
					v3 = Mesh.m_Positions[Face.m_Vertices[2].m_uiPositionID];

					Face.m_vNormal.CalculateNormal (v1, v2, v3);

					// done reading the face, store it
					Mesh.m_Faces.push_back (Face);
				}
			}
			else
			if (strcmp (szID, "usemtl") == 0)		// next material to be used for the following faces
			{
				char szMaterial[256] = "";
				fscanf (pFile, "%s", &szMaterial[0]);

				if (bIgnoreMaterials)
					uiCurMaterial = 0;
				else
				{
					// look-up the ID of this material
					uiCurMaterial = Materials[szMaterial].m_uiMaterialID;
				}
			}
/*
			else
			if (strcmp (szID, "some_other_attrib") == 0)
			{
				// read any other attribute, that you might need, here
			}
*/
			else
				fgets (szLine, sizeof (char) * 1024, pFile);	// read anything else, that is not understood
		}

		fclose (pFile);

    aeProgressBar::Update (); // 2

		if (bSortByMaterial)
		{
			// sort all faces by material-ID
			sort (Mesh.m_Faces.begin (), Mesh.m_Faces.end ());
		}

    aeProgressBar::Update (); // 3

		Mesh.ComputeTangentSpaceVectors ();

    aeProgressBar::Update (); // 4

		return (true);
	}

	void MESH::ComputeTangentSpaceVectors (void)
	{
		for (int i = 0; i < (int) m_Faces.size (); ++i)
			m_Faces[i].ComputeTangentSpaceVectors (*this);
	}

	void FACE::ComputeTangentSpaceVectors (const MESH& Mesh)
	{
		// cannot compute tangents without texture-coordinates
		if (Mesh.m_TexCoords.empty ())
			return;

		const VEC3 p1 = Mesh.m_Positions[m_Vertices[0].m_uiPositionID];
		const VEC3 p2 = Mesh.m_Positions[m_Vertices[1].m_uiPositionID];
		const VEC3 p3 = Mesh.m_Positions[m_Vertices[2].m_uiPositionID];

		const TEXCOORD tc1 = Mesh.m_TexCoords[m_Vertices[0].m_uiTexCoordID];
		const TEXCOORD tc2 = Mesh.m_TexCoords[m_Vertices[1].m_uiTexCoordID];
		const TEXCOORD tc3 = Mesh.m_TexCoords[m_Vertices[2].m_uiTexCoordID];




		// Calculate the vectors from the current vertex to the two other vertices in the triangle
		VEC3 v2v1 = p2 - p1;
		VEC3 v3v1 = p3 - p1;

		// The equation presented in the article states that:

		// Calculate c2c1_T and c2c1_B
		float c2c1_T = tc2.m_fTexCoordU - tc1.m_fTexCoordU;
		float c2c1_B = tc2.m_fTexCoordV - tc1.m_fTexCoordV;

		// Calculate c3c1_T and c3c1_B
		float c3c1_T = tc3.m_fTexCoordU - tc1.m_fTexCoordU;
		float c3c1_B = tc3.m_fTexCoordV - tc1.m_fTexCoordV;

		float fDenominator = c2c1_T * c3c1_B - c3c1_T * c2c1_B;

		// Calculate the reciprocal value once and for all (to achieve speed)
		float fScale1 = 1.0f / fDenominator;

		// T and B are calculated just as the equation in the article states
		VEC3 T, B;
		T = VEC3 ((c3c1_B * v2v1.x - c2c1_B * v3v1.x) * fScale1,
				(c3c1_B * v2v1.y - c2c1_B * v3v1.y) * fScale1,
				(c3c1_B * v2v1.z - c2c1_B * v3v1.z) * fScale1);

		B = VEC3 ((-c3c1_T * v2v1.x + c2c1_T * v3v1.x) * fScale1,
				(-c3c1_T * v2v1.y + c2c1_T * v3v1.y) * fScale1,
				(-c3c1_T * v2v1.z + c2c1_T * v3v1.z) * fScale1);


		T.Normalize ();
		B.Normalize ();


		m_vTangent = T;
		m_vBiTangent = cross (m_vNormal, m_vTangent).getNormalized ();
	}
}

