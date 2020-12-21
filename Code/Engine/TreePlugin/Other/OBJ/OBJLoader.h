// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <stdio.h>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include "Vec3.h"

/*	A light-weight OBJ and MTL Loader.
	
	There are two functions to be used:

	LoadOBJFile:
		Loads an OBJ-file and its MTL-file and stores all data in a MESH-structure.
		You can then either use the MESH-structure directly to render / process all the mesh-data 
		(only advised for getting started, not for final code) or copy and rearrange the data from the MESH into your own structures.

	LoadTextures:
		This is a helper-function, that takes a loader-function, provided by the user, and calls that loader for every material,
		that was found in the MTL-file. There is not much to it, it is only supposed to demonstrate, how to handle the texture data, 
		that is returned by the OBJ-loader.
*/

namespace NS_OBJLOADER
{
	using namespace NS_MATHS;

	struct MESH;

	struct TEXCOORD
	{
		float m_fTexCoordU;
		float m_fTexCoordV;
		// add a third tex-coord here, if you need it
	};

	//! Holds all needed data for one vertex of a face. Through the IDs one can access the arrays in a MESH to get the real data.
	struct FACE_VERTEX
	{
		FACE_VERTEX ()
		{
			m_uiPositionID = 0;
			m_uiNormalID = 0;
			m_uiTexCoordID = 0;
		}

		unsigned int m_uiPositionID;
		unsigned int m_uiNormalID;
		unsigned int m_uiTexCoordID;
	};

	//! Holds the information about one Material. 
	/*! Only the diffuse texture is actually read and stored by this loader, but if needed this can easily be extended.
		The MaterialID is the ID of the Material itself, this is only needed by the loader.
	*/
	struct MATERIAL
	{
		//! The path to the diffuse texture of this material.
    std::string m_sDiffuseTexture;

		//! The ID of this material. Only needed by the loader.
		unsigned int m_uiMaterialID;
	};

	//! Holds all data about one face (ie. polygon, not only triangles).
	struct FACE
	{
		FACE ()
		{
			m_uiMaterialID = 0;
			m_Vertices.reserve (4);
		}

		//! The ID of the material, that this face uses.
		unsigned int m_uiMaterialID;

		//! The face-normal, automatically computed
		VEC3 m_vNormal;

		// These are only calculated on demand (through MESH::ComputeTangentSpaceVectors) and only if texture-coordinates are available.
		// Useful, when doing normal-mapping in tangent-space.
		VEC3 m_vTangent;
		VEC3 m_vBiTangent;

		//! All vertices of the face.
    std::vector<FACE_VERTEX> m_Vertices;

		//! Less-than operator is needed for sorting faces by material.
		bool operator< (const FACE& cc) const
		{
			return (m_uiMaterialID < cc.m_uiMaterialID);
		}

		void ComputeTangentSpaceVectors (const MESH& Mesh);
	};

	//! Intermediate structure to load mesh-data into. Not suitable to directly render from, only to get data out of an OBJ-file.
	/*! This structure stores all data about one OBJ-file. There are three arrays that hold raw data (positions, normals, tex-coords)
		and one array that holds data about faces, which store their data indirectly, by only indexing into the other three arrays.
		This reflects the layout of an OBJ file pretty closely.
	*/
	struct MESH
	{
		std::deque<VEC3> m_Positions;
		std::deque<VEC3> m_Normals;
		std::deque<TEXCOORD> m_TexCoords;
		std::deque<FACE> m_Faces;

		//! Makes sure the mesh is empty.
		void MakeEmpty (void)
		{
			m_Positions.clear ();
			m_Normals.clear ();
			m_TexCoords.clear ();
			m_Faces.clear ();
		}

		//! Call this to calculate a TBN matrix for every face. Useful for tangent-space normal-mapping.
		void ComputeTangentSpaceVectors (void);

		//! Returns whether texture-coordinates are available for this mesh.
		bool HasTextureCoordinates (void) const {return (!m_TexCoords.empty ());}
		//! Returns whether vertex-normals are available for this mesh. Otherwise only face-normals are available.
		bool HasVertexNormals (void) const {return (!m_Normals.empty ());}
	};

	//! Loads an OBJ file and its MTL file (if it has the same name). Returns false, if the OBJ-file cannot be read. 
	/*! Can handle OBJ-files that contain vertices ("v"), normals ("vn") and tex-coords ("vt"). Any of vt and vn can be missing.
		Stores all Materials in the pMaterials-map. Index is the "name" of the Material as stated in the MTL-file. szMaterialBasePath is prepended to 
		all texture-paths. If bIgnoreMaterials is set to true, the MTL file is not loaded and all faces get the MaterialID zero. 
		If bSortByMaterial is true, after loading all faces are sorted by material. Otherwise they are in the exact order, that they were defined in the OBJ-file.
		"Mesh" will automatically be emptied before loading the OBJ-file.
		"Materials" will NOT be emptied, all newly read materials are appended, such that you can load different meshes, but automatically gather all
		material-information in one place.

		Use it like this:

		MESH MyMesh;
		map<string, MATERIAL> MyMaterials;
		if (LoadOBJFile ("MyApplications\\Meshes\\SomeMesh.obj", MyMesh, MyMaterials, "MyApplication\\Textures\\"))
		{
			...
		}
	*/
	bool LoadOBJFile (const char *szFile, MESH& Mesh, std::map<std::string, MATERIAL>& Materials, const char* szMaterialBasePath = "", bool bSortByMaterial = true, bool bIgnoreMaterials = false);

	//! Function-pointer definition for an OpenGL-Texture-Loader function.
	typedef void (*TEXTURE_LOADER) (const MATERIAL& Material, int iIndex, void* pStore);

	//! Allows you to easily load all Textures in "Materials" and store them in an array for efficient lookup during rendering.
	/*! Use it like this:

		// define this structure as you like, depending on the needs of your application
		struct TEXTURE_DATA
		{
			int m_iOpenGLID;
		};

		void MyTextureLoader (const MATERIAL& Mat, int iIndex, void* pPassThrough)
		{
			load texture "Mat.m_sDiffuseTexture" into OpenGL, get a Texture-ID from OpenGL

			vector<TEXTURE_DATA>* pTexData = (vector<TEXTURE_DATA>*) (pPassThrough);
			
			(*pTexData)[iIndex].m_iOpenGLID = ... the ID given by OpenGL ...
		}

		vector<TEXTURE_DATA> TextureData;
		LoadTextures (MyMaterials, &MyTextureLoader, (void*) &TextureData);

		Then, at run-time, you can bind your textures like this:

		glBindTexture (GL_TEXTURE_2D, TextureData[current_polygon.m_uiMaterialID].m_iOpenGLID);
	*/
	void LoadTextures (std::map<std::string, MATERIAL>& Materials, TEXTURE_LOADER Loader, void* pPassThrough);
}

#endif



