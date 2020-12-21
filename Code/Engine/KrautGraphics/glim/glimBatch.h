/*
** GLIM - OpenGL Immediate Mode
** Copyright Jan Krassnigg (Jan@Krassnigg.de)
** For more details, see the included Readme.txt.
*/

#ifndef GLIM_GLIMBATCH_H
#define GLIM_GLIMBATCH_H

#include "glimInterface.h"
#include "glimBatchData.h"

namespace NS_GLIM
{
	//! An Implementation of the GLIM_Interface.
	class GLIM_IMPEXP GLIM_BATCH : public GLIM_Interface
	{
	public:
		GLIM_BATCH ();
		virtual ~GLIM_BATCH ();

		//! Begins defining one piece of geometry that can later be rendered with one set of states.
		virtual void BeginBatch (void);
		//! Ends defining the batch. After this call "RenderBatch" can be called to actually render it.
		virtual void EndBatch (bool bUploadNow = false);

		//! Renders the batch that has been defined previously.
		virtual void RenderBatch (bool bWireframe = false);
		//! Renders n instances of the batch that has been defined previously.
		virtual void RenderBatchInstanced (int iInstances, bool bWireframe = false);

		//! Begins gathering information about the given type of primitives. 
		virtual void Begin (GLIM_ENUM eType);
		//! Ends gathering information about the primitives.
		virtual void End (void);

		//! Specifies a new vertex of a primitive.
		virtual void Vertex (float x, float y, float z = 0.0f);

		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute1f (const char* szAttribute, float a1);
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute2f (const char* szAttribute, float a1, float a2);
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute3f (const char* szAttribute, float a1, float a2, float a3);
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute4f (const char* szAttribute, float a1, float a2, float a3, float a4);

		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute1i (const char* szAttribute, int a1);
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute2i (const char* szAttribute, int a1, int a2);
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute3i (const char* szAttribute, int a1, int a2, int a3);
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute4i (const char* szAttribute, int a1, int a2, int a3, int a4);

		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute4ub (const char* szAttribute, unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4 = 255);


		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute1f (GLIM_ATTRIBUTE Attr, float a1);
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute2f (GLIM_ATTRIBUTE Attr, float a1, float a2);
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute3f (GLIM_ATTRIBUTE Attr, float a1, float a2, float a3);
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute4f (GLIM_ATTRIBUTE Attr, float a1, float a2, float a3, float a4);

		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute1i (GLIM_ATTRIBUTE Attr, int a1);
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute2i (GLIM_ATTRIBUTE Attr, int a1, int a2);
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute3i (GLIM_ATTRIBUTE Attr, int a1, int a2, int a3);
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute4i (GLIM_ATTRIBUTE Attr, int a1, int a2, int a3, int a4);

		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute4ub (GLIM_ATTRIBUTE Attr, unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4 = 255);


		//! Returns the axis-aligned bounding box of the batches geometry, if there is any geometry. Else the results are undefined.
		virtual void getBatchAABB (float& out_fMinX, float& out_fMaxX, float& out_fMinY, float& out_fMaxY, float& out_fMinZ, float& out_fMaxZ);

		//! Deletes all data associated with this object.
		void Clear (void);

		//! Returns true if the GLIM_BATCH contains no batch data.
		bool IsCleared (void) const {return (m_Data->m_State == STATE_EMPTY);}

	private:
		GLIM_BATCH (const GLIM_BATCH& cc);
		const GLIM_BATCH& operator= (const GLIM_BATCH& cc);

		bool BeginRender (void);
		void EndRender (void);

#ifdef AE_RENDERAPI_OPENGL
		bool BeginRenderOGL (void);
		void RenderBatchOGL (bool bWireframe);
		void RenderBatchInstancedOGL (int iInstances, bool bWireframe);
#endif

		// The currently generated primitive type as specified via 'Begin'
		GLIM_ENUM m_PrimitiveType;

		// Counts how many vertices have been added since 'Begin'
		unsigned int m_uiPrimitiveVertex;
		// The index of the first vertex passed after 'Begin'
		unsigned int m_uiPrimitiveFirstIndex;
		// The Index of the Vertex passed previously.
		unsigned int m_uiPrimitiveLastIndex;

		glimBatchData* m_Data;
	};
}

#pragma once


#endif


