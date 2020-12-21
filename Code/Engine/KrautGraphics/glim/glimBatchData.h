/*
** GLIM - OpenGL Immediate Mode
** Copyright Jan Krassnigg (Jan@Krassnigg.de)
** For more details, see the included Readme.txt.
*/

#ifndef GLIM_GLIMBATCHDATA_H
#define GLIM_GLIMBATCHDATA_H

#include "Declarations.h"
#include <map>
#include <string>

namespace NS_GLIM
{
	// holds one element of attribute data
	union GLIM_IMPEXP Glim4ByteData
	{
		Glim4ByteData () : Int (0) {}

		int Int;
		float Float;
		unsigned char Bytes[4];
	};

	// one attribute array
	struct GLIM_IMPEXP GlimArrayData
	{
		GlimArrayData ();

		void Reset (void);
		void PushElement (void);

		// what kind of data this array holds (1 float, 2 floats, 4 unsigned byte, etc.)
		GLIM_ENUM m_DataType;
		// the current value that shall be used for all new elements
		Glim4ByteData m_CurrentValue[4];
		// the actual array of accumulated elements
		std::vector<Glim4ByteData> m_ArrayData;

		// the offset into the GL buffer, needed for binding it
		unsigned int m_uiBufferOffset;
		unsigned int m_uiBufferStride;
	};

	// used for tracking erroneous use of the interface
	enum GLIM_BATCH_STATE
	{
		STATE_EMPTY,
		STATE_BEGINNING_BATCH,
		STATE_FINISHED_BATCH,
		STATE_BEGIN_PRIMITIVE,
		STATE_END_PRIMITIVE,

	};

	// the data of one entire batch
	struct GLIM_IMPEXP glimBatchData
	{
		glimBatchData ();
		~glimBatchData ();

		// Deletes all allocated data and resets default states
		void Reset (void);

		// Uploads the data onto the GPU
		void Upload (void);
		// Binds the vertex arrays for rendering.
		void Bind (unsigned int uiCurrentProgram);
		// Unbinds all data after rendering.
		void Unbind (void);

#ifdef AE_RENDERAPI_OPENGL
		// Uploads the data onto the GPU
		void UploadOGL (void);
		// Binds the vertex arrays for rendering.
		void BindOGL (unsigned int uiCurrentProgram);
		// Unbinds all data after rendering.
		void UnbindOGL (void);
#endif

		// Returns the size in bytes of ONE vertex (all attributes together)
		unsigned int getVertexDataSize (void) const;

		// Returns the index of the just added vertex.
		unsigned int AddVertex (float x, float y, float z);

		// Used for error detection.
		GLIM_BATCH_STATE m_State;

		// All attributes accessible by name.
		std::map<std::string, GlimArrayData> m_Attributes;

		// Position data is stored separately, not as an attribute.
		std::vector<float> m_PositionData;

		// Index Buffer for points.
		std::vector<unsigned int> m_IndexBuffer_Points;
		// Index Buffer for Lines.
		std::vector<unsigned int> m_IndexBuffer_Lines;
		// Index Buffer for Triangles.
		std::vector<unsigned int> m_IndexBuffer_Triangles;
		// Index Buffer for wireframe rendering of polygons.
		std::vector<unsigned int> m_IndexBuffer_Wireframe;

		// Number of Points to render. Used after m_IndexBuffer_Points has been cleared. 
		unsigned int m_uiPointElements;
		// Number of Lines to render. Used after m_IndexBuffer_Lines has been cleared. 
		unsigned int m_uiLineElements;
		// Number of Triangles to render. Used after m_IndexBuffer_Triangles has been cleared. 
		unsigned int m_uiTriangleElements;
		// Number of Lines to render. Used after m_IndexBuffer_Wireframe has been cleared. 
		unsigned int m_uiWireframeElements;

		// Whether the data has already been uploaded to the GPU.
		bool m_bUploadedToGPU;
		// Whether VBOs where ever created.
		bool m_bCreatedVBOs;

#ifdef AE_RENDERAPI_OPENGL
		// Queries the GL for the bind-point of a vertex-attribute. Returns -1 if the attribute is not used by the shader.
		int getVertexAttribBindPoint (unsigned int uiCurrentProgram, const char* szAttribName) const;

		// GL ID of the vertex array.
		unsigned int m_uiVertexBufferID;
		// GL ID of the index buffer for points.
		unsigned int m_uiElementBufferID_Points;
		// GL ID of the index buffer for lines.
		unsigned int m_uiElementBufferID_Lines;
		// GL ID of the index buffer for triangles.
		unsigned int m_uiElementBufferID_Triangles;
		// GL ID of the index buffer for wireframe rendering.
		unsigned int m_uiElementBufferID_Wireframe;
#endif

		// AABB 
		float m_fMinX;
		float m_fMaxX;
		float m_fMinY;
		float m_fMaxY;
		float m_fMinZ;
		float m_fMaxZ;

	};
}


#pragma once

#endif



