GLIM - OpenGL Immediate Mode 
  by Jan Krassnigg (Jan@Krassnigg.de)
  
Version: 0.4
Date: March 27th 2011
Website: www.artifactgames.de
Download: www.artifactgames.de/glim/

This is a full source code release. You need to compile GLIM yourself.
Tested with Visual Studio 2008 using the Microsoft STL.

License:
--------

Released under the zlib-license (see http://www.zlib.net/zlib_license.html)

In short:
You are free to use and modify this software for any purpose, including commercial.


Requirements:
-------------

GLIM uses GLEW for extensions (VBO, GLSL) and the STL (map, vector, string, exception).
You can replace GLEW easily by just changing one #include in GLIM's Header "Config.h".
Furthermore GLIM uses the STL (vector, map, exception).

Description:
------------

GLIM is a very small library that tries to mimic OpenGL's immediate mode rendering.
It does not use any immediate mode functions itself, thus allowing to make applications independent from
all the functions that are deprecated in OpenGL 3.0.

GLIM requires you to use GLSL shaders and generic vertex attributes. GLIM does not do any state management.
All it does is to gather geometry (vertex-positions and generic vertex-attributes) and send them to the GPU.
All state-management needs to be done through regular gl.

It is not GLIM's intention to make immediate mode style rendering faster. It is thought as a replacement for 
when immediate mode is not available. Also using GLIM it becomes much easier to use shaders with generic
vertex attributes, because one sends attribute values by name, instead of by bind-point number.

It is possible that GLIM is actually faster than regular immediate mode, because GLIM exposes only a very small
subset of OpenGL's immediate mode features which allows a very slick implementation. 
I did not do any benchmarks, though.

Note: GLIM sends all geometry as triangles to the GPU. However, as an additional feature, if GLIM detects, that
glPolygonMode is set to GL_LINE (wireframe rendering), it sends the outlines of the primitives as GL_LINES to
the GPU, such that they are NOT rendered like triangles.


Usage:
------

GLIM's source code is commented using Doxygen comments, for a more detailed description, please take a look
directly at the source code or the doxygen documentation.

If GLIM detects a usage error it immediately throws an exception using std::exception and on Windows an 
additional message-box is shown. The exception contains an explanation what exactly the user did wrong. 
There are no error-codes to query or something similar. This is done on purpose to guide the user how to use
GLIM properly.


Example:
-------

#include <glim/glim.h>

using namespace NS_GLIM;

// start with one piece of geometry that uses one set of states
glim.BeginBatch ();
	
	// set all attributes that the current shader uses to a start value
	glim.Attribute2f ("attr_TexCoord", 0.0f, 0.0f);
	glim.Attribute4ub ("attr_Color", 255, 0, 0);
	
	// begin primitive rendering
	glim.Begin (GLIM_QUADS);
		glim.Attribute4ub ("attr_Color", 0, 200, 0);
		
		glim.Attribute2f ("attr_TexCoord", 0.0f, 0.0f);
		glim.Vertex (0.0f, 0.0f);
		
		glim.Attribute2f ("attr_TexCoord", 1.0f, 0.0f);
		glim.Vertex (1.0f, 0.0f);
		
		glim.Attribute2f ("attr_TexCoord", 1.0f, 1.0f);
		glim.Vertex (1.0f, 1.0f);
		
		glim.Attribute2f ("attr_TexCoord", 0.0f, 1.0f);
		glim.Vertex (0.0f, 1.0f);
	
		
		glim.Attribute4ub ("attr_Color", 255, 255, 255);
		
		glim.Attribute2f ("attr_TexCoord", 0.0f, 0.0f);
		glim.Vertex (1.0f, 0.0f);
		
		glim.Attribute2f ("attr_TexCoord", 1.0f, 0.0f);
		glim.Vertex (2.0f, 0.0f);
		
		glim.Attribute2f ("attr_TexCoord", 1.0f, 1.0f);
		glim.Vertex (2.0f, 1.0f);
		
		glim.Attribute2f ("attr_TexCoord", 0.0f, 1.0f);
		glim.Vertex (1.0f, 1.0f);
	
	glim.End ();
glim.EndBatch ();

// set all states, especially GLSL shader
// shader needs to use generic vertex attributes (here: "attr_Color" and "attr_TexCoord")
// position information can still be accessed through gl_Vertex, because position is always sent as attribute 0

// Tell GLIM to do the draw-call.
// This function can be called as often as needed, even with different shaders and states set.
// Each time it will first query the shader for the used attributes and the bind-points and then do the draw-call.
glim.RenderBatch ();


// start another piece of geometry 
glim.BeginBatch ();
	
	// set all attributes that the current shader uses to a start value
	glim.Attribute4ub ("attr_Color", 255, 0, 0);
	
	// begin primitive rendering
	glim.Begin (GLIM_POLYGON);
		glim.Attribute4ub ("attr_Color", 0, 0, 255);

		glim.Vertex (0.0f, 0.0f);
		glim.Vertex (1.0f, 0.0f);
		glim.Vertex (1.0f, 1.0f);
		glim.Vertex (0.0f, 1.0f);
	
	glim.End ();
	glim.Begin (GLIM_POLYGON);
		
		glim.Attribute4ub ("attr_Color", 255, 0, 0);

		glim.Vertex (1.0f, 0.0f);
		glim.Vertex (2.0f, 0.0f);
		glim.Vertex (2.0f, 1.0f);
		glim.Vertex (1.0f, 1.0f);
	
	glim.End ();
glim.EndBatch ();



Change Log - Version 0.2
------------------------

 * Fixed bug regarding VAOs
 * Using unsigned int indices instead of shorts now, thus works with _big_ meshes
 * Fixed bug when mixing point / line primitives
 * Fixed bug when using wireframe mode on points and lines
 * Removed usage of std::exception due to problems compiling it on Linux, for now.
 
Change Log - Version 0.3
------------------------
 
 * entire rewrite of GLIM, less bugs, more performance
 * it should now work flawlessly when mixing point, line and polygon primitives
 * interface has changed slightly
 * now need to call "RenderBatch" after "EndBatch" to render the geometry
 * can repeat rendering the same geometry by calling "RenderBatch" multiple times
 * BeginPart / EndPart have been renamed to BeginBatch / EndBatch
 * the use of GLEW can now more easily be replaced with another extension loader, such as GLEE
 * added RenderInstanced
 
Change Log - Version 0.4
------------------------

 * Bugfix: Not initialized variable could cause a glDeleteBuffer at creation time (on an invalid buffer)
 * During Buffer creation NO gl calls whatsoever are done anymore, allowing to fill a GLIM_BATCH in a thread without a GL context
 * Experimental (but well working) implementation for Direct3D 11, search the code for "AE_RENDERAPI_D3D11" to find out more
 * Bugfix: Integer-Attributes were incorrectly sent to OpenGL
 * Bugfix: Incorrect behavior when mixing Lines, Points and Triangles
 
 
 