/*
** GLIM - OpenGL Immediate Mode
** Copyright Jan Krassnigg (Jan@Krassnigg.de)
** For more details, see the included Readme.txt.
*/

#ifndef GLIM_GLIMINTERFACE_H
#define GLIM_GLIMINTERFACE_H

#include "Declarations.h"

namespace NS_GLIM
{
	//! Abstract base-class for an immediate mode emulation layer.
	/*! Classes deriving from this interface implement an immediate-mode style way of specifying geometry (no states!).\n
		Internally those classes will use vertex buffers and glDraw* calls instead of deprecated imediate mode functions.\n
		The intention is to make rendering of small batches easy and possible without deprecated functions. It is not meant to
		be efficient.\n
		glim requires you to use shaders. It will send attribute values to the designated attribute bind points, by querying the
		current shader for the attributes names.\n
		glim caches all geometry internally and only issues draw-calls inside of "RenderBatch".
		\n
		In between "BeginBatch" / "EndBatch" you can create as many primitives as you wish, using "Begin" / "End". You can set
		all attribute values also outside of "Begin" / "End", the last value will always be the current one.\n
		All attributes need however be mentioned once before "Begin" is called for the first time (after the previous call to 
		"BeginBatch"). After the first call to "Begin" only attributes are allowed, that have been mentioned before, no new
		attributes my be added. This allows to make sure that ALL attributes have a proper default-value, that the user specifies.
		\n
		Vertex is only allowed in between "Begin" / "End".\n
		\n
		Between "BeginBatch" / "EndBatch" all Attribute* calls for the same attribute need to specify the exact same type.
		That means, you cannot mix for example calls like Attribute3f ("data", x, y, z) and Attribute4f ("data", x, y, z, w).\n
		Different attributes can, of course, use different data types.\n
		\n
		For users that want to handle state-changes manually (for example because they want to cache them and only apply them
		when necessary), glim allows you to specify a callback function, that will be called exactly at the moment that all
		states need to be properly set. Just set GLIM_Interface::s_StateChangeCallback and your application will be informed
		when glim needs to apply the current states.\n
		Not that glim internally tries to changes as few states as possible. The only OpenGL states that glim changes are
		the currently bound VAO, ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER and the vertex-attribute pointers and enable-states.
		glim disables VAOs to prevent accidentally changing them, and after a call to "RenderBatch" all vertex-array states
		are disabled, too.
	*/
	class GLIM_IMPEXP GLIM_Interface
	{
	public:
		virtual ~GLIM_Interface () {}

		//! Begins defining one piece of geometry that can later be rendered with one set of states.
		virtual void BeginBatch (void) = 0;
		//! Ends defining the batch. After this call "RenderBatch" can be called to actually render it.
		virtual void EndBatch (bool bUploadNow = false) = 0;

		//! Renders the batch that has been defined previously.
		virtual void RenderBatch (bool bWireframe = false) = 0;
		//! Renders n instances of the batch that has been defined previously.
		virtual void RenderBatchInstanced (int iInstances, bool bWireframe = false) = 0;

		//! Begins gathering information about the given type of primitives. 
		virtual void Begin (GLIM_ENUM eType) = 0;
		//! Ends gathering information about the primitives.
		virtual void End (void) = 0;

		//! Specifies a new vertex of a primitive.
		virtual void Vertex (float x, float y, float z = 0.0f) = 0;

		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute1f (const char* szAttribute, float a1) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute2f (const char* szAttribute, float a1, float a2) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute3f (const char* szAttribute, float a1, float a2, float a3) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute4f (const char* szAttribute, float a1, float a2, float a3, float a4) = 0;

		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute1i (const char* szAttribute, int a1) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute2i (const char* szAttribute, int a1, int a2) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute3i (const char* szAttribute, int a1, int a2, int a3) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute4i (const char* szAttribute, int a1, int a2, int a3, int a4) = 0;

		//! Specifies a new value for the attribute with the given name.
		virtual GLIM_ATTRIBUTE Attribute4ub (const char* szAttribute, unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4 = 255) = 0;


		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute1f (GLIM_ATTRIBUTE Attr, float a1) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute2f (GLIM_ATTRIBUTE Attr, float a1, float a2) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute3f (GLIM_ATTRIBUTE Attr, float a1, float a2, float a3) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute4f (GLIM_ATTRIBUTE Attr, float a1, float a2, float a3, float a4) = 0;

		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute1i (GLIM_ATTRIBUTE Attr, int a1) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute2i (GLIM_ATTRIBUTE Attr, int a1, int a2) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute3i (GLIM_ATTRIBUTE Attr, int a1, int a2, int a3) = 0;
		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute4i (GLIM_ATTRIBUTE Attr, int a1, int a2, int a3, int a4) = 0;

		//! Specifies a new value for the attribute with the given name.
		virtual void Attribute4ub (GLIM_ATTRIBUTE Attr, unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4 = 255) = 0;


		//! Returns the axis-aligned bounding box of the batches geometry, if there is any geometry. Else the results are undefined.
		virtual void getBatchAABB (float& out_fMinX, float& out_fMaxX, float& out_fMinY, float& out_fMaxY, float& out_fMinZ, float& out_fMaxZ) = 0;

	public:
		//! Set this callback, if you want to hook into GLIM and be informed when it is about to make a drawcall or query GL states. This allows you to apply states exactly when it is needed.
		static GLIM_CALLBACK s_StateChangeCallback;

		//! If set to true, ALL rendered objects will be rendered in wireframe, no matter whether the "bWireframe" parameter is set or not.
		static bool s_bForceWireframe;

	};

}

#pragma once


#endif


