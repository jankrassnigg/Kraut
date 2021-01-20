/*
** The OpenGL Extension Wrangler Library
** Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>
** Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>
** Copyright (C) 2002, Lev Povalahev
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without 
** modification, are permitted provided that the following conditions are met:
** 
** * Redistributions of source code must retain the above copyright notice, 
**   this list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice, 
**   this list of conditions and the following disclaimer in the documentation 
**   and/or other materials provided with the distribution.
** * The name of the author may be used to endorse or promote products 
**   derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "glew.h"
#if defined(_WIN32)
#  include "wglew.h"
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)
#  include "glxew.h"
#endif

#pragma optimize("", off)

/*
 * Define glewGetContext and related helper macros.
 */
#ifdef GLEW_MX
#  define glewGetContext() ctx
#  ifdef _WIN32
#    define GLEW_CONTEXT_ARG_DEF_INIT GLEWContext* ctx
#    define GLEW_CONTEXT_ARG_VAR_INIT ctx
#    define wglewGetContext() ctx
#    define WGLEW_CONTEXT_ARG_DEF_INIT WGLEWContext* ctx
#    define WGLEW_CONTEXT_ARG_DEF_LIST WGLEWContext* ctx
#  else /* _WIN32 */
#    define GLEW_CONTEXT_ARG_DEF_INIT void
#    define GLEW_CONTEXT_ARG_VAR_INIT
#    define glxewGetContext() ctx
#    define GLXEW_CONTEXT_ARG_DEF_INIT void
#    define GLXEW_CONTEXT_ARG_DEF_LIST GLXEWContext* ctx
#  endif /* _WIN32 */
#  define GLEW_CONTEXT_ARG_DEF_LIST GLEWContext* ctx
#else /* GLEW_MX */
#  define GLEW_CONTEXT_ARG_DEF_INIT void
#  define GLEW_CONTEXT_ARG_VAR_INIT
#  define GLEW_CONTEXT_ARG_DEF_LIST void
#  define WGLEW_CONTEXT_ARG_DEF_INIT void
#  define WGLEW_CONTEXT_ARG_DEF_LIST void
#  define GLXEW_CONTEXT_ARG_DEF_INIT void
#  define GLXEW_CONTEXT_ARG_DEF_LIST void
#endif /* GLEW_MX */

#if defined(__sgi) || defined (__sun) || defined(GLEW_APPLE_GLX)
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void* dlGetProcAddress (const GLubyte* name)
{
  static void* h = nullptr;
  static void* gpa;

  if (h == nullptr)
  {
    if ((h = dlopen(nullptr, RTLD_LAZY | RTLD_LOCAL)) == nullptr) return nullptr;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != nullptr)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun || GLEW_APPLE_GLX */

#if defined(__APPLE__)
#include <stdlib.h>
#include <string.h>
#include <AvailabilityMacros.h>

#ifdef MAC_OS_X_VERSION_10_3

#include <dlfcn.h>

void* NSGLGetProcAddress (const GLubyte *name)
{
  static void* image = nullptr;
  if (nullptr == image) 
  {
    image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
  }
  if( !image ) return nullptr;
  void* addr = dlsym(image, (const char*)name);
  if( addr ) return addr;
#ifdef GLEW_APPLE_GLX
  return dlGetProcAddress( name ); // try next for glx symbols
#else
  return nullptr;
#endif
}
#else

#include <mach-o/dyld.h>

void* NSGLGetProcAddress (const GLubyte *name)
{
  static const struct mach_header* image = nullptr;
  NSSymbol symbol;
  char* symbolName;
  if (nullptr == image)
  {
    image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
  }
  /* prepend a '_' for the Unix C symbol mangling convention */
  symbolName = malloc(strlen((const char*)name) + 2);
  strcpy(symbolName+1, (const char*)name);
  symbolName[0] = '_';
  symbol = nullptr;
  /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
  symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : nullptr;
  free(symbolName);
  if( symbol ) return NSAddressOfSymbol(symbol);
#ifdef GLEW_APPLE_GLX
  return dlGetProcAddress( name ); // try next for glx symbols
#else
  return nullptr;
#endif
}
#endif /* MAC_OS_X_VERSION_10_3 */
#endif /* __APPLE__ */

/*
 * Define glewGetProcAddress.
 */
#if defined(_WIN32)
#  define glewGetProcAddress(name) wglGetProcAddress((LPCSTR)name)
#else
#  if defined(__APPLE__)
#    define glewGetProcAddress(name) NSGLGetProcAddress(name)
#  else
#    if defined(__sgi) || defined(__sun)
#      define glewGetProcAddress(name) dlGetProcAddress(name)
#    else /* __linux */
#      define glewGetProcAddress(name) (*glXGetProcAddressARB)(name)
#    endif
#  endif
#endif

/*
 * Define GLboolean const cast.
 */
#define CONST_CAST(x) (*(GLboolean*)&x)

/*
 * GLEW, just like OpenGL or GLU, does not rely on the standard C library.
 * These functions implement the functionality required in this file.
 */
static GLuint _glewStrLen (const GLubyte* s)
{
  GLuint i=0;
  if (s == nullptr) return 0;
  while (s[i] != '\0') i++;
  return i;
}

static GLuint _glewStrCLen (const GLubyte* s, GLubyte c)
{
  GLuint i=0;
  if (s == nullptr) return 0;
  while (s[i] != '\0' && s[i] != c) i++;
  return (s[i] == '\0' || s[i] == c) ? i : 0;
}

static GLboolean _glewStrSame (const GLubyte* a, const GLubyte* b, GLuint n)
{
  GLuint i=0;
  if(a == nullptr || b == nullptr)
    return (a == nullptr && b == nullptr && n == 0) ? GL_TRUE : GL_FALSE;
  while (i < n && a[i] != '\0' && b[i] != '\0' && a[i] == b[i]) i++;
  return i == n ? GL_TRUE : GL_FALSE;
}

static GLboolean _glewStrSame1 (GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  while (*na > 0 && (**a == ' ' || **a == '\n' || **a == '\r' || **a == '\t'))
  {
    (*a)++;
    (*na)--;
  }
  if(*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)+i != nullptr && b+i != nullptr && (*a)[i] == b[i]) i++;
    if(i == nb)
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

static GLboolean _glewStrSame2 (GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  if(*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)+i != nullptr && b+i != nullptr && (*a)[i] == b[i]) i++;
    if(i == nb)
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

static GLboolean _glewStrSame3 (GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  if(*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)+i != nullptr && b+i != nullptr && (*a)[i] == b[i]) i++;
    if (i == nb && (*na == nb || (*a)[i] == ' ' || (*a)[i] == '\n' || (*a)[i] == '\r' || (*a)[i] == '\t'))
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

/*
 * Search for name in the extensions string. Use of strstr()
 * is not sufficient because extension names can be prefixes of
 * other extension names. Could use strtok() but the constant
 * string returned by glGetString might be in read-only memory.
 */
static GLboolean _glewSearchExtension (const char* name, const GLubyte *start, const GLubyte *end)
{
  const GLubyte* p;
  GLuint len = _glewStrLen((const GLubyte*)name);
  p = start;
  while (p < end)
  {
    GLuint n = _glewStrCLen(p, ' ');
    if (len == n && _glewStrSame((const GLubyte*)name, p, n)) return GL_TRUE;
    p += n+1;
  }
  return GL_FALSE;
}

#if !defined(_WIN32) || !defined(GLEW_MX)

PFNGLCOPYTEXSUBIMAGE3DPROC __glewCopyTexSubImage3D = nullptr;
PFNGLDRAWRANGEELEMENTSPROC __glewDrawRangeElements = nullptr;
PFNGLTEXIMAGE3DPROC __glewTexImage3D = nullptr;
PFNGLTEXSUBIMAGE3DPROC __glewTexSubImage3D = nullptr;

PFNGLACTIVETEXTUREPROC __glewActiveTexture = nullptr;
PFNGLCLIENTACTIVETEXTUREPROC __glewClientActiveTexture = nullptr;
PFNGLCOMPRESSEDTEXIMAGE1DPROC __glewCompressedTexImage1D = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC __glewCompressedTexImage2D = nullptr;
PFNGLCOMPRESSEDTEXIMAGE3DPROC __glewCompressedTexImage3D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC __glewCompressedTexSubImage1D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC __glewCompressedTexSubImage2D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC __glewCompressedTexSubImage3D = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEPROC __glewGetCompressedTexImage = nullptr;
PFNGLLOADTRANSPOSEMATRIXDPROC __glewLoadTransposeMatrixd = nullptr;
PFNGLLOADTRANSPOSEMATRIXFPROC __glewLoadTransposeMatrixf = nullptr;
PFNGLMULTTRANSPOSEMATRIXDPROC __glewMultTransposeMatrixd = nullptr;
PFNGLMULTTRANSPOSEMATRIXFPROC __glewMultTransposeMatrixf = nullptr;
PFNGLMULTITEXCOORD1DPROC __glewMultiTexCoord1d = nullptr;
PFNGLMULTITEXCOORD1DVPROC __glewMultiTexCoord1dv = nullptr;
PFNGLMULTITEXCOORD1FPROC __glewMultiTexCoord1f = nullptr;
PFNGLMULTITEXCOORD1FVPROC __glewMultiTexCoord1fv = nullptr;
PFNGLMULTITEXCOORD1IPROC __glewMultiTexCoord1i = nullptr;
PFNGLMULTITEXCOORD1IVPROC __glewMultiTexCoord1iv = nullptr;
PFNGLMULTITEXCOORD1SPROC __glewMultiTexCoord1s = nullptr;
PFNGLMULTITEXCOORD1SVPROC __glewMultiTexCoord1sv = nullptr;
PFNGLMULTITEXCOORD2DPROC __glewMultiTexCoord2d = nullptr;
PFNGLMULTITEXCOORD2DVPROC __glewMultiTexCoord2dv = nullptr;
PFNGLMULTITEXCOORD2FPROC __glewMultiTexCoord2f = nullptr;
PFNGLMULTITEXCOORD2FVPROC __glewMultiTexCoord2fv = nullptr;
PFNGLMULTITEXCOORD2IPROC __glewMultiTexCoord2i = nullptr;
PFNGLMULTITEXCOORD2IVPROC __glewMultiTexCoord2iv = nullptr;
PFNGLMULTITEXCOORD2SPROC __glewMultiTexCoord2s = nullptr;
PFNGLMULTITEXCOORD2SVPROC __glewMultiTexCoord2sv = nullptr;
PFNGLMULTITEXCOORD3DPROC __glewMultiTexCoord3d = nullptr;
PFNGLMULTITEXCOORD3DVPROC __glewMultiTexCoord3dv = nullptr;
PFNGLMULTITEXCOORD3FPROC __glewMultiTexCoord3f = nullptr;
PFNGLMULTITEXCOORD3FVPROC __glewMultiTexCoord3fv = nullptr;
PFNGLMULTITEXCOORD3IPROC __glewMultiTexCoord3i = nullptr;
PFNGLMULTITEXCOORD3IVPROC __glewMultiTexCoord3iv = nullptr;
PFNGLMULTITEXCOORD3SPROC __glewMultiTexCoord3s = nullptr;
PFNGLMULTITEXCOORD3SVPROC __glewMultiTexCoord3sv = nullptr;
PFNGLMULTITEXCOORD4DPROC __glewMultiTexCoord4d = nullptr;
PFNGLMULTITEXCOORD4DVPROC __glewMultiTexCoord4dv = nullptr;
PFNGLMULTITEXCOORD4FPROC __glewMultiTexCoord4f = nullptr;
PFNGLMULTITEXCOORD4FVPROC __glewMultiTexCoord4fv = nullptr;
PFNGLMULTITEXCOORD4IPROC __glewMultiTexCoord4i = nullptr;
PFNGLMULTITEXCOORD4IVPROC __glewMultiTexCoord4iv = nullptr;
PFNGLMULTITEXCOORD4SPROC __glewMultiTexCoord4s = nullptr;
PFNGLMULTITEXCOORD4SVPROC __glewMultiTexCoord4sv = nullptr;
PFNGLSAMPLECOVERAGEPROC __glewSampleCoverage = nullptr;

PFNGLBLENDCOLORPROC __glewBlendColor = nullptr;
PFNGLBLENDEQUATIONPROC __glewBlendEquation = nullptr;
PFNGLBLENDFUNCSEPARATEPROC __glewBlendFuncSeparate = nullptr;
PFNGLFOGCOORDPOINTERPROC __glewFogCoordPointer = nullptr;
PFNGLFOGCOORDDPROC __glewFogCoordd = nullptr;
PFNGLFOGCOORDDVPROC __glewFogCoorddv = nullptr;
PFNGLFOGCOORDFPROC __glewFogCoordf = nullptr;
PFNGLFOGCOORDFVPROC __glewFogCoordfv = nullptr;
PFNGLMULTIDRAWARRAYSPROC __glewMultiDrawArrays = nullptr;
PFNGLMULTIDRAWELEMENTSPROC __glewMultiDrawElements = nullptr;
PFNGLPOINTPARAMETERFPROC __glewPointParameterf = nullptr;
PFNGLPOINTPARAMETERFVPROC __glewPointParameterfv = nullptr;
PFNGLPOINTPARAMETERIPROC __glewPointParameteri = nullptr;
PFNGLPOINTPARAMETERIVPROC __glewPointParameteriv = nullptr;
PFNGLSECONDARYCOLOR3BPROC __glewSecondaryColor3b = nullptr;
PFNGLSECONDARYCOLOR3BVPROC __glewSecondaryColor3bv = nullptr;
PFNGLSECONDARYCOLOR3DPROC __glewSecondaryColor3d = nullptr;
PFNGLSECONDARYCOLOR3DVPROC __glewSecondaryColor3dv = nullptr;
PFNGLSECONDARYCOLOR3FPROC __glewSecondaryColor3f = nullptr;
PFNGLSECONDARYCOLOR3FVPROC __glewSecondaryColor3fv = nullptr;
PFNGLSECONDARYCOLOR3IPROC __glewSecondaryColor3i = nullptr;
PFNGLSECONDARYCOLOR3IVPROC __glewSecondaryColor3iv = nullptr;
PFNGLSECONDARYCOLOR3SPROC __glewSecondaryColor3s = nullptr;
PFNGLSECONDARYCOLOR3SVPROC __glewSecondaryColor3sv = nullptr;
PFNGLSECONDARYCOLOR3UBPROC __glewSecondaryColor3ub = nullptr;
PFNGLSECONDARYCOLOR3UBVPROC __glewSecondaryColor3ubv = nullptr;
PFNGLSECONDARYCOLOR3UIPROC __glewSecondaryColor3ui = nullptr;
PFNGLSECONDARYCOLOR3UIVPROC __glewSecondaryColor3uiv = nullptr;
PFNGLSECONDARYCOLOR3USPROC __glewSecondaryColor3us = nullptr;
PFNGLSECONDARYCOLOR3USVPROC __glewSecondaryColor3usv = nullptr;
PFNGLSECONDARYCOLORPOINTERPROC __glewSecondaryColorPointer = nullptr;
PFNGLWINDOWPOS2DPROC __glewWindowPos2d = nullptr;
PFNGLWINDOWPOS2DVPROC __glewWindowPos2dv = nullptr;
PFNGLWINDOWPOS2FPROC __glewWindowPos2f = nullptr;
PFNGLWINDOWPOS2FVPROC __glewWindowPos2fv = nullptr;
PFNGLWINDOWPOS2IPROC __glewWindowPos2i = nullptr;
PFNGLWINDOWPOS2IVPROC __glewWindowPos2iv = nullptr;
PFNGLWINDOWPOS2SPROC __glewWindowPos2s = nullptr;
PFNGLWINDOWPOS2SVPROC __glewWindowPos2sv = nullptr;
PFNGLWINDOWPOS3DPROC __glewWindowPos3d = nullptr;
PFNGLWINDOWPOS3DVPROC __glewWindowPos3dv = nullptr;
PFNGLWINDOWPOS3FPROC __glewWindowPos3f = nullptr;
PFNGLWINDOWPOS3FVPROC __glewWindowPos3fv = nullptr;
PFNGLWINDOWPOS3IPROC __glewWindowPos3i = nullptr;
PFNGLWINDOWPOS3IVPROC __glewWindowPos3iv = nullptr;
PFNGLWINDOWPOS3SPROC __glewWindowPos3s = nullptr;
PFNGLWINDOWPOS3SVPROC __glewWindowPos3sv = nullptr;

PFNGLBEGINQUERYPROC __glewBeginQuery = nullptr;
PFNGLBINDBUFFERPROC __glewBindBuffer = nullptr;
PFNGLBUFFERDATAPROC __glewBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC __glewBufferSubData = nullptr;
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = nullptr;
PFNGLDELETEQUERIESPROC __glewDeleteQueries = nullptr;
PFNGLENDQUERYPROC __glewEndQuery = nullptr;
PFNGLGENBUFFERSPROC __glewGenBuffers = nullptr;
PFNGLGENQUERIESPROC __glewGenQueries = nullptr;
PFNGLGETBUFFERPARAMETERIVPROC __glewGetBufferParameteriv = nullptr;
PFNGLGETBUFFERPOINTERVPROC __glewGetBufferPointerv = nullptr;
PFNGLGETBUFFERSUBDATAPROC __glewGetBufferSubData = nullptr;
PFNGLGETQUERYOBJECTIVPROC __glewGetQueryObjectiv = nullptr;
PFNGLGETQUERYOBJECTUIVPROC __glewGetQueryObjectuiv = nullptr;
PFNGLGETQUERYIVPROC __glewGetQueryiv = nullptr;
PFNGLISBUFFERPROC __glewIsBuffer = nullptr;
PFNGLISQUERYPROC __glewIsQuery = nullptr;
PFNGLMAPBUFFERPROC __glewMapBuffer = nullptr;
PFNGLUNMAPBUFFERPROC __glewUnmapBuffer = nullptr;

PFNGLATTACHSHADERPROC __glewAttachShader = nullptr;
PFNGLBINDATTRIBLOCATIONPROC __glewBindAttribLocation = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC __glewBlendEquationSeparate = nullptr;
PFNGLCOMPILESHADERPROC __glewCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC __glewCreateProgram = nullptr;
PFNGLCREATESHADERPROC __glewCreateShader = nullptr;
PFNGLDELETEPROGRAMPROC __glewDeleteProgram = nullptr;
PFNGLDELETESHADERPROC __glewDeleteShader = nullptr;
PFNGLDETACHSHADERPROC __glewDetachShader = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC __glewDisableVertexAttribArray = nullptr;
PFNGLDRAWBUFFERSPROC __glewDrawBuffers = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray = nullptr;
PFNGLGETACTIVEATTRIBPROC __glewGetActiveAttrib = nullptr;
PFNGLGETACTIVEUNIFORMPROC __glewGetActiveUniform = nullptr;
PFNGLGETATTACHEDSHADERSPROC __glewGetAttachedShaders = nullptr;
PFNGLGETATTRIBLOCATIONPROC __glewGetAttribLocation = nullptr;
PFNGLGETPROGRAMINFOLOGPROC __glewGetProgramInfoLog = nullptr;
PFNGLGETPROGRAMIVPROC __glewGetProgramiv = nullptr;
PFNGLGETSHADERINFOLOGPROC __glewGetShaderInfoLog = nullptr;
PFNGLGETSHADERSOURCEPROC __glewGetShaderSource = nullptr;
PFNGLGETSHADERIVPROC __glewGetShaderiv = nullptr;
PFNGLGETUNIFORMLOCATIONPROC __glewGetUniformLocation = nullptr;
PFNGLGETUNIFORMFVPROC __glewGetUniformfv = nullptr;
PFNGLGETUNIFORMIVPROC __glewGetUniformiv = nullptr;
PFNGLGETVERTEXATTRIBPOINTERVPROC __glewGetVertexAttribPointerv = nullptr;
PFNGLGETVERTEXATTRIBDVPROC __glewGetVertexAttribdv = nullptr;
PFNGLGETVERTEXATTRIBFVPROC __glewGetVertexAttribfv = nullptr;
PFNGLGETVERTEXATTRIBIVPROC __glewGetVertexAttribiv = nullptr;
PFNGLISPROGRAMPROC __glewIsProgram = nullptr;
PFNGLISSHADERPROC __glewIsShader = nullptr;
PFNGLLINKPROGRAMPROC __glewLinkProgram = nullptr;
PFNGLSHADERSOURCEPROC __glewShaderSource = nullptr;
PFNGLSTENCILFUNCSEPARATEPROC __glewStencilFuncSeparate = nullptr;
PFNGLSTENCILMASKSEPARATEPROC __glewStencilMaskSeparate = nullptr;
PFNGLSTENCILOPSEPARATEPROC __glewStencilOpSeparate = nullptr;
PFNGLUNIFORM1FPROC __glewUniform1f = nullptr;
PFNGLUNIFORM1FVPROC __glewUniform1fv = nullptr;
PFNGLUNIFORM1IPROC __glewUniform1i = nullptr;
PFNGLUNIFORM1IVPROC __glewUniform1iv = nullptr;
PFNGLUNIFORM2FPROC __glewUniform2f = nullptr;
PFNGLUNIFORM2FVPROC __glewUniform2fv = nullptr;
PFNGLUNIFORM2IPROC __glewUniform2i = nullptr;
PFNGLUNIFORM2IVPROC __glewUniform2iv = nullptr;
PFNGLUNIFORM3FPROC __glewUniform3f = nullptr;
PFNGLUNIFORM3FVPROC __glewUniform3fv = nullptr;
PFNGLUNIFORM3IPROC __glewUniform3i = nullptr;
PFNGLUNIFORM3IVPROC __glewUniform3iv = nullptr;
PFNGLUNIFORM4FPROC __glewUniform4f = nullptr;
PFNGLUNIFORM4FVPROC __glewUniform4fv = nullptr;
PFNGLUNIFORM4IPROC __glewUniform4i = nullptr;
PFNGLUNIFORM4IVPROC __glewUniform4iv = nullptr;
PFNGLUNIFORMMATRIX2FVPROC __glewUniformMatrix2fv = nullptr;
PFNGLUNIFORMMATRIX3FVPROC __glewUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC __glewUniformMatrix4fv = nullptr;
PFNGLUSEPROGRAMPROC __glewUseProgram = nullptr;
PFNGLVALIDATEPROGRAMPROC __glewValidateProgram = nullptr;
PFNGLVERTEXATTRIB1DPROC __glewVertexAttrib1d = nullptr;
PFNGLVERTEXATTRIB1DVPROC __glewVertexAttrib1dv = nullptr;
PFNGLVERTEXATTRIB1FPROC __glewVertexAttrib1f = nullptr;
PFNGLVERTEXATTRIB1FVPROC __glewVertexAttrib1fv = nullptr;
PFNGLVERTEXATTRIB1SPROC __glewVertexAttrib1s = nullptr;
PFNGLVERTEXATTRIB1SVPROC __glewVertexAttrib1sv = nullptr;
PFNGLVERTEXATTRIB2DPROC __glewVertexAttrib2d = nullptr;
PFNGLVERTEXATTRIB2DVPROC __glewVertexAttrib2dv = nullptr;
PFNGLVERTEXATTRIB2FPROC __glewVertexAttrib2f = nullptr;
PFNGLVERTEXATTRIB2FVPROC __glewVertexAttrib2fv = nullptr;
PFNGLVERTEXATTRIB2SPROC __glewVertexAttrib2s = nullptr;
PFNGLVERTEXATTRIB2SVPROC __glewVertexAttrib2sv = nullptr;
PFNGLVERTEXATTRIB3DPROC __glewVertexAttrib3d = nullptr;
PFNGLVERTEXATTRIB3DVPROC __glewVertexAttrib3dv = nullptr;
PFNGLVERTEXATTRIB3FPROC __glewVertexAttrib3f = nullptr;
PFNGLVERTEXATTRIB3FVPROC __glewVertexAttrib3fv = nullptr;
PFNGLVERTEXATTRIB3SPROC __glewVertexAttrib3s = nullptr;
PFNGLVERTEXATTRIB3SVPROC __glewVertexAttrib3sv = nullptr;
PFNGLVERTEXATTRIB4NBVPROC __glewVertexAttrib4Nbv = nullptr;
PFNGLVERTEXATTRIB4NIVPROC __glewVertexAttrib4Niv = nullptr;
PFNGLVERTEXATTRIB4NSVPROC __glewVertexAttrib4Nsv = nullptr;
PFNGLVERTEXATTRIB4NUBPROC __glewVertexAttrib4Nub = nullptr;
PFNGLVERTEXATTRIB4NUBVPROC __glewVertexAttrib4Nubv = nullptr;
PFNGLVERTEXATTRIB4NUIVPROC __glewVertexAttrib4Nuiv = nullptr;
PFNGLVERTEXATTRIB4NUSVPROC __glewVertexAttrib4Nusv = nullptr;
PFNGLVERTEXATTRIB4BVPROC __glewVertexAttrib4bv = nullptr;
PFNGLVERTEXATTRIB4DPROC __glewVertexAttrib4d = nullptr;
PFNGLVERTEXATTRIB4DVPROC __glewVertexAttrib4dv = nullptr;
PFNGLVERTEXATTRIB4FPROC __glewVertexAttrib4f = nullptr;
PFNGLVERTEXATTRIB4FVPROC __glewVertexAttrib4fv = nullptr;
PFNGLVERTEXATTRIB4IVPROC __glewVertexAttrib4iv = nullptr;
PFNGLVERTEXATTRIB4SPROC __glewVertexAttrib4s = nullptr;
PFNGLVERTEXATTRIB4SVPROC __glewVertexAttrib4sv = nullptr;
PFNGLVERTEXATTRIB4UBVPROC __glewVertexAttrib4ubv = nullptr;
PFNGLVERTEXATTRIB4UIVPROC __glewVertexAttrib4uiv = nullptr;
PFNGLVERTEXATTRIB4USVPROC __glewVertexAttrib4usv = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer = nullptr;

PFNGLUNIFORMMATRIX2X3FVPROC __glewUniformMatrix2x3fv = nullptr;
PFNGLUNIFORMMATRIX2X4FVPROC __glewUniformMatrix2x4fv = nullptr;
PFNGLUNIFORMMATRIX3X2FVPROC __glewUniformMatrix3x2fv = nullptr;
PFNGLUNIFORMMATRIX3X4FVPROC __glewUniformMatrix3x4fv = nullptr;
PFNGLUNIFORMMATRIX4X2FVPROC __glewUniformMatrix4x2fv = nullptr;
PFNGLUNIFORMMATRIX4X3FVPROC __glewUniformMatrix4x3fv = nullptr;

PFNGLBEGINCONDITIONALRENDERPROC __glewBeginConditionalRender = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKPROC __glewBeginTransformFeedback = nullptr;
PFNGLBINDFRAGDATALOCATIONPROC __glewBindFragDataLocation = nullptr;
PFNGLCLAMPCOLORPROC __glewClampColor = nullptr;
PFNGLCLEARBUFFERFIPROC __glewClearBufferfi = nullptr;
PFNGLCLEARBUFFERFVPROC __glewClearBufferfv = nullptr;
PFNGLCLEARBUFFERIVPROC __glewClearBufferiv = nullptr;
PFNGLCLEARBUFFERUIVPROC __glewClearBufferuiv = nullptr;
PFNGLCOLORMASKIPROC __glewColorMaski = nullptr;
PFNGLDISABLEIPROC __glewDisablei = nullptr;
PFNGLENABLEIPROC __glewEnablei = nullptr;
PFNGLENDCONDITIONALRENDERPROC __glewEndConditionalRender = nullptr;
PFNGLENDTRANSFORMFEEDBACKPROC __glewEndTransformFeedback = nullptr;
PFNGLGETBOOLEANI_VPROC __glewGetBooleani_v = nullptr;
PFNGLGETFRAGDATALOCATIONPROC __glewGetFragDataLocation = nullptr;
PFNGLGETSTRINGIPROC __glewGetStringi = nullptr;
PFNGLGETTEXPARAMETERIIVPROC __glewGetTexParameterIiv = nullptr;
PFNGLGETTEXPARAMETERIUIVPROC __glewGetTexParameterIuiv = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC __glewGetTransformFeedbackVarying = nullptr;
PFNGLGETUNIFORMUIVPROC __glewGetUniformuiv = nullptr;
PFNGLGETVERTEXATTRIBIIVPROC __glewGetVertexAttribIiv = nullptr;
PFNGLGETVERTEXATTRIBIUIVPROC __glewGetVertexAttribIuiv = nullptr;
PFNGLISENABLEDIPROC __glewIsEnabledi = nullptr;
PFNGLTEXPARAMETERIIVPROC __glewTexParameterIiv = nullptr;
PFNGLTEXPARAMETERIUIVPROC __glewTexParameterIuiv = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC __glewTransformFeedbackVaryings = nullptr;
PFNGLUNIFORM1UIPROC __glewUniform1ui = nullptr;
PFNGLUNIFORM1UIVPROC __glewUniform1uiv = nullptr;
PFNGLUNIFORM2UIPROC __glewUniform2ui = nullptr;
PFNGLUNIFORM2UIVPROC __glewUniform2uiv = nullptr;
PFNGLUNIFORM3UIPROC __glewUniform3ui = nullptr;
PFNGLUNIFORM3UIVPROC __glewUniform3uiv = nullptr;
PFNGLUNIFORM4UIPROC __glewUniform4ui = nullptr;
PFNGLUNIFORM4UIVPROC __glewUniform4uiv = nullptr;
PFNGLVERTEXATTRIBI1IPROC __glewVertexAttribI1i = nullptr;
PFNGLVERTEXATTRIBI1IVPROC __glewVertexAttribI1iv = nullptr;
PFNGLVERTEXATTRIBI1UIPROC __glewVertexAttribI1ui = nullptr;
PFNGLVERTEXATTRIBI1UIVPROC __glewVertexAttribI1uiv = nullptr;
PFNGLVERTEXATTRIBI2IPROC __glewVertexAttribI2i = nullptr;
PFNGLVERTEXATTRIBI2IVPROC __glewVertexAttribI2iv = nullptr;
PFNGLVERTEXATTRIBI2UIPROC __glewVertexAttribI2ui = nullptr;
PFNGLVERTEXATTRIBI2UIVPROC __glewVertexAttribI2uiv = nullptr;
PFNGLVERTEXATTRIBI3IPROC __glewVertexAttribI3i = nullptr;
PFNGLVERTEXATTRIBI3IVPROC __glewVertexAttribI3iv = nullptr;
PFNGLVERTEXATTRIBI3UIPROC __glewVertexAttribI3ui = nullptr;
PFNGLVERTEXATTRIBI3UIVPROC __glewVertexAttribI3uiv = nullptr;
PFNGLVERTEXATTRIBI4BVPROC __glewVertexAttribI4bv = nullptr;
PFNGLVERTEXATTRIBI4IPROC __glewVertexAttribI4i = nullptr;
PFNGLVERTEXATTRIBI4IVPROC __glewVertexAttribI4iv = nullptr;
PFNGLVERTEXATTRIBI4SVPROC __glewVertexAttribI4sv = nullptr;
PFNGLVERTEXATTRIBI4UBVPROC __glewVertexAttribI4ubv = nullptr;
PFNGLVERTEXATTRIBI4UIPROC __glewVertexAttribI4ui = nullptr;
PFNGLVERTEXATTRIBI4UIVPROC __glewVertexAttribI4uiv = nullptr;
PFNGLVERTEXATTRIBI4USVPROC __glewVertexAttribI4usv = nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC __glewVertexAttribIPointer = nullptr;

PFNGLDRAWARRAYSINSTANCEDPROC __glewDrawArraysInstanced = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC __glewDrawElementsInstanced = nullptr;
PFNGLPRIMITIVERESTARTINDEXPROC __glewPrimitiveRestartIndex = nullptr;
PFNGLTEXBUFFERPROC __glewTexBuffer = nullptr;

PFNGLFRAMEBUFFERTEXTUREPROC __glewFramebufferTexture = nullptr;
PFNGLGETBUFFERPARAMETERI64VPROC __glewGetBufferParameteri64v = nullptr;
PFNGLGETINTEGER64I_VPROC __glewGetInteger64i_v = nullptr;

PFNGLVERTEXATTRIBDIVISORPROC __glewVertexAttribDivisor = nullptr;

PFNGLBLENDEQUATIONSEPARATEIPROC __glewBlendEquationSeparatei = nullptr;
PFNGLBLENDEQUATIONIPROC __glewBlendEquationi = nullptr;
PFNGLBLENDFUNCSEPARATEIPROC __glewBlendFuncSeparatei = nullptr;
PFNGLBLENDFUNCIPROC __glewBlendFunci = nullptr;
PFNGLMINSAMPLESHADINGPROC __glewMinSampleShading = nullptr;

PFNGLTBUFFERMASK3DFXPROC __glewTbufferMask3DFX = nullptr;

PFNGLDEBUGMESSAGECALLBACKAMDPROC __glewDebugMessageCallbackAMD = nullptr;
PFNGLDEBUGMESSAGEENABLEAMDPROC __glewDebugMessageEnableAMD = nullptr;
PFNGLDEBUGMESSAGEINSERTAMDPROC __glewDebugMessageInsertAMD = nullptr;
PFNGLGETDEBUGMESSAGELOGAMDPROC __glewGetDebugMessageLogAMD = nullptr;

PFNGLBLENDEQUATIONINDEXEDAMDPROC __glewBlendEquationIndexedAMD = nullptr;
PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC __glewBlendEquationSeparateIndexedAMD = nullptr;
PFNGLBLENDFUNCINDEXEDAMDPROC __glewBlendFuncIndexedAMD = nullptr;
PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC __glewBlendFuncSeparateIndexedAMD = nullptr;

PFNGLMULTIDRAWARRAYSINDIRECTAMDPROC __glewMultiDrawArraysIndirectAMD = nullptr;
PFNGLMULTIDRAWELEMENTSINDIRECTAMDPROC __glewMultiDrawElementsIndirectAMD = nullptr;

PFNGLDELETENAMESAMDPROC __glewDeleteNamesAMD = nullptr;
PFNGLGENNAMESAMDPROC __glewGenNamesAMD = nullptr;
PFNGLISNAMEAMDPROC __glewIsNameAMD = nullptr;

PFNGLBEGINPERFMONITORAMDPROC __glewBeginPerfMonitorAMD = nullptr;
PFNGLDELETEPERFMONITORSAMDPROC __glewDeletePerfMonitorsAMD = nullptr;
PFNGLENDPERFMONITORAMDPROC __glewEndPerfMonitorAMD = nullptr;
PFNGLGENPERFMONITORSAMDPROC __glewGenPerfMonitorsAMD = nullptr;
PFNGLGETPERFMONITORCOUNTERDATAAMDPROC __glewGetPerfMonitorCounterDataAMD = nullptr;
PFNGLGETPERFMONITORCOUNTERINFOAMDPROC __glewGetPerfMonitorCounterInfoAMD = nullptr;
PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC __glewGetPerfMonitorCounterStringAMD = nullptr;
PFNGLGETPERFMONITORCOUNTERSAMDPROC __glewGetPerfMonitorCountersAMD = nullptr;
PFNGLGETPERFMONITORGROUPSTRINGAMDPROC __glewGetPerfMonitorGroupStringAMD = nullptr;
PFNGLGETPERFMONITORGROUPSAMDPROC __glewGetPerfMonitorGroupsAMD = nullptr;
PFNGLSELECTPERFMONITORCOUNTERSAMDPROC __glewSelectPerfMonitorCountersAMD = nullptr;

PFNGLSETMULTISAMPLEFVAMDPROC __glewSetMultisamplefvAMD = nullptr;

PFNGLTESSELLATIONFACTORAMDPROC __glewTessellationFactorAMD = nullptr;
PFNGLTESSELLATIONMODEAMDPROC __glewTessellationModeAMD = nullptr;

PFNGLDRAWELEMENTARRAYAPPLEPROC __glewDrawElementArrayAPPLE = nullptr;
PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC __glewDrawRangeElementArrayAPPLE = nullptr;
PFNGLELEMENTPOINTERAPPLEPROC __glewElementPointerAPPLE = nullptr;
PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC __glewMultiDrawElementArrayAPPLE = nullptr;
PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC __glewMultiDrawRangeElementArrayAPPLE = nullptr;

PFNGLDELETEFENCESAPPLEPROC __glewDeleteFencesAPPLE = nullptr;
PFNGLFINISHFENCEAPPLEPROC __glewFinishFenceAPPLE = nullptr;
PFNGLFINISHOBJECTAPPLEPROC __glewFinishObjectAPPLE = nullptr;
PFNGLGENFENCESAPPLEPROC __glewGenFencesAPPLE = nullptr;
PFNGLISFENCEAPPLEPROC __glewIsFenceAPPLE = nullptr;
PFNGLSETFENCEAPPLEPROC __glewSetFenceAPPLE = nullptr;
PFNGLTESTFENCEAPPLEPROC __glewTestFenceAPPLE = nullptr;
PFNGLTESTOBJECTAPPLEPROC __glewTestObjectAPPLE = nullptr;

PFNGLBUFFERPARAMETERIAPPLEPROC __glewBufferParameteriAPPLE = nullptr;
PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC __glewFlushMappedBufferRangeAPPLE = nullptr;

PFNGLGETOBJECTPARAMETERIVAPPLEPROC __glewGetObjectParameterivAPPLE = nullptr;
PFNGLOBJECTPURGEABLEAPPLEPROC __glewObjectPurgeableAPPLE = nullptr;
PFNGLOBJECTUNPURGEABLEAPPLEPROC __glewObjectUnpurgeableAPPLE = nullptr;

PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC __glewGetTexParameterPointervAPPLE = nullptr;
PFNGLTEXTURERANGEAPPLEPROC __glewTextureRangeAPPLE = nullptr;

PFNGLBINDVERTEXARRAYAPPLEPROC __glewBindVertexArrayAPPLE = nullptr;
PFNGLDELETEVERTEXARRAYSAPPLEPROC __glewDeleteVertexArraysAPPLE = nullptr;
PFNGLGENVERTEXARRAYSAPPLEPROC __glewGenVertexArraysAPPLE = nullptr;
PFNGLISVERTEXARRAYAPPLEPROC __glewIsVertexArrayAPPLE = nullptr;

PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC __glewFlushVertexArrayRangeAPPLE = nullptr;
PFNGLVERTEXARRAYPARAMETERIAPPLEPROC __glewVertexArrayParameteriAPPLE = nullptr;
PFNGLVERTEXARRAYRANGEAPPLEPROC __glewVertexArrayRangeAPPLE = nullptr;

PFNGLDISABLEVERTEXATTRIBAPPLEPROC __glewDisableVertexAttribAPPLE = nullptr;
PFNGLENABLEVERTEXATTRIBAPPLEPROC __glewEnableVertexAttribAPPLE = nullptr;
PFNGLISVERTEXATTRIBENABLEDAPPLEPROC __glewIsVertexAttribEnabledAPPLE = nullptr;
PFNGLMAPVERTEXATTRIB1DAPPLEPROC __glewMapVertexAttrib1dAPPLE = nullptr;
PFNGLMAPVERTEXATTRIB1FAPPLEPROC __glewMapVertexAttrib1fAPPLE = nullptr;
PFNGLMAPVERTEXATTRIB2DAPPLEPROC __glewMapVertexAttrib2dAPPLE = nullptr;
PFNGLMAPVERTEXATTRIB2FAPPLEPROC __glewMapVertexAttrib2fAPPLE = nullptr;

PFNGLCLEARDEPTHFPROC __glewClearDepthf = nullptr;
PFNGLDEPTHRANGEFPROC __glewDepthRangef = nullptr;
PFNGLGETSHADERPRECISIONFORMATPROC __glewGetShaderPrecisionFormat = nullptr;
PFNGLRELEASESHADERCOMPILERPROC __glewReleaseShaderCompiler = nullptr;
PFNGLSHADERBINARYPROC __glewShaderBinary = nullptr;

PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC __glewDrawArraysInstancedBaseInstance = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC __glewDrawElementsInstancedBaseInstance = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC __glewDrawElementsInstancedBaseVertexBaseInstance = nullptr;

PFNGLBINDFRAGDATALOCATIONINDEXEDPROC __glewBindFragDataLocationIndexed = nullptr;
PFNGLGETFRAGDATAINDEXPROC __glewGetFragDataIndex = nullptr;

PFNGLCREATESYNCFROMCLEVENTARBPROC __glewCreateSyncFromCLeventARB = nullptr;

PFNGLCLAMPCOLORARBPROC __glewClampColorARB = nullptr;

PFNGLCOPYBUFFERSUBDATAPROC __glewCopyBufferSubData = nullptr;

PFNGLDEBUGMESSAGECALLBACKARBPROC __glewDebugMessageCallbackARB = nullptr;
PFNGLDEBUGMESSAGECONTROLARBPROC __glewDebugMessageControlARB = nullptr;
PFNGLDEBUGMESSAGEINSERTARBPROC __glewDebugMessageInsertARB = nullptr;
PFNGLGETDEBUGMESSAGELOGARBPROC __glewGetDebugMessageLogARB = nullptr;

PFNGLDRAWBUFFERSARBPROC __glewDrawBuffersARB = nullptr;

PFNGLBLENDEQUATIONSEPARATEIARBPROC __glewBlendEquationSeparateiARB = nullptr;
PFNGLBLENDEQUATIONIARBPROC __glewBlendEquationiARB = nullptr;
PFNGLBLENDFUNCSEPARATEIARBPROC __glewBlendFuncSeparateiARB = nullptr;
PFNGLBLENDFUNCIARBPROC __glewBlendFunciARB = nullptr;

PFNGLDRAWELEMENTSBASEVERTEXPROC __glewDrawElementsBaseVertex = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC __glewDrawElementsInstancedBaseVertex = nullptr;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC __glewDrawRangeElementsBaseVertex = nullptr;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC __glewMultiDrawElementsBaseVertex = nullptr;

PFNGLDRAWARRAYSINDIRECTPROC __glewDrawArraysIndirect = nullptr;
PFNGLDRAWELEMENTSINDIRECTPROC __glewDrawElementsIndirect = nullptr;

PFNGLBINDFRAMEBUFFERPROC __glewBindFramebuffer = nullptr;
PFNGLBINDRENDERBUFFERPROC __glewBindRenderbuffer = nullptr;
PFNGLBLITFRAMEBUFFERPROC __glewBlitFramebuffer = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC __glewCheckFramebufferStatus = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC __glewDeleteFramebuffers = nullptr;
PFNGLDELETERENDERBUFFERSPROC __glewDeleteRenderbuffers = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC __glewFramebufferRenderbuffer = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DPROC __glewFramebufferTexture1D = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC __glewFramebufferTexture2D = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DPROC __glewFramebufferTexture3D = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC __glewFramebufferTextureLayer = nullptr;
PFNGLGENFRAMEBUFFERSPROC __glewGenFramebuffers = nullptr;
PFNGLGENRENDERBUFFERSPROC __glewGenRenderbuffers = nullptr;
PFNGLGENERATEMIPMAPPROC __glewGenerateMipmap = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC __glewGetFramebufferAttachmentParameteriv = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVPROC __glewGetRenderbufferParameteriv = nullptr;
PFNGLISFRAMEBUFFERPROC __glewIsFramebuffer = nullptr;
PFNGLISRENDERBUFFERPROC __glewIsRenderbuffer = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC __glewRenderbufferStorage = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC __glewRenderbufferStorageMultisample = nullptr;

PFNGLFRAMEBUFFERTEXTUREARBPROC __glewFramebufferTextureARB = nullptr;
PFNGLFRAMEBUFFERTEXTUREFACEARBPROC __glewFramebufferTextureFaceARB = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERARBPROC __glewFramebufferTextureLayerARB = nullptr;
PFNGLPROGRAMPARAMETERIARBPROC __glewProgramParameteriARB = nullptr;

PFNGLGETPROGRAMBINARYPROC __glewGetProgramBinary = nullptr;
PFNGLPROGRAMBINARYPROC __glewProgramBinary = nullptr;
PFNGLPROGRAMPARAMETERIPROC __glewProgramParameteri = nullptr;

PFNGLGETUNIFORMDVPROC __glewGetUniformdv = nullptr;
PFNGLPROGRAMUNIFORM1DEXTPROC __glewProgramUniform1dEXT = nullptr;
PFNGLPROGRAMUNIFORM1DVEXTPROC __glewProgramUniform1dvEXT = nullptr;
PFNGLPROGRAMUNIFORM2DEXTPROC __glewProgramUniform2dEXT = nullptr;
PFNGLPROGRAMUNIFORM2DVEXTPROC __glewProgramUniform2dvEXT = nullptr;
PFNGLPROGRAMUNIFORM3DEXTPROC __glewProgramUniform3dEXT = nullptr;
PFNGLPROGRAMUNIFORM3DVEXTPROC __glewProgramUniform3dvEXT = nullptr;
PFNGLPROGRAMUNIFORM4DEXTPROC __glewProgramUniform4dEXT = nullptr;
PFNGLPROGRAMUNIFORM4DVEXTPROC __glewProgramUniform4dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC __glewProgramUniformMatrix2dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC __glewProgramUniformMatrix2x3dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC __glewProgramUniformMatrix2x4dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC __glewProgramUniformMatrix3dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC __glewProgramUniformMatrix3x2dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC __glewProgramUniformMatrix3x4dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC __glewProgramUniformMatrix4dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC __glewProgramUniformMatrix4x2dvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC __glewProgramUniformMatrix4x3dvEXT = nullptr;
PFNGLUNIFORM1DPROC __glewUniform1d = nullptr;
PFNGLUNIFORM1DVPROC __glewUniform1dv = nullptr;
PFNGLUNIFORM2DPROC __glewUniform2d = nullptr;
PFNGLUNIFORM2DVPROC __glewUniform2dv = nullptr;
PFNGLUNIFORM3DPROC __glewUniform3d = nullptr;
PFNGLUNIFORM3DVPROC __glewUniform3dv = nullptr;
PFNGLUNIFORM4DPROC __glewUniform4d = nullptr;
PFNGLUNIFORM4DVPROC __glewUniform4dv = nullptr;
PFNGLUNIFORMMATRIX2DVPROC __glewUniformMatrix2dv = nullptr;
PFNGLUNIFORMMATRIX2X3DVPROC __glewUniformMatrix2x3dv = nullptr;
PFNGLUNIFORMMATRIX2X4DVPROC __glewUniformMatrix2x4dv = nullptr;
PFNGLUNIFORMMATRIX3DVPROC __glewUniformMatrix3dv = nullptr;
PFNGLUNIFORMMATRIX3X2DVPROC __glewUniformMatrix3x2dv = nullptr;
PFNGLUNIFORMMATRIX3X4DVPROC __glewUniformMatrix3x4dv = nullptr;
PFNGLUNIFORMMATRIX4DVPROC __glewUniformMatrix4dv = nullptr;
PFNGLUNIFORMMATRIX4X2DVPROC __glewUniformMatrix4x2dv = nullptr;
PFNGLUNIFORMMATRIX4X3DVPROC __glewUniformMatrix4x3dv = nullptr;

PFNGLCOLORSUBTABLEPROC __glewColorSubTable = nullptr;
PFNGLCOLORTABLEPROC __glewColorTable = nullptr;
PFNGLCOLORTABLEPARAMETERFVPROC __glewColorTableParameterfv = nullptr;
PFNGLCOLORTABLEPARAMETERIVPROC __glewColorTableParameteriv = nullptr;
PFNGLCONVOLUTIONFILTER1DPROC __glewConvolutionFilter1D = nullptr;
PFNGLCONVOLUTIONFILTER2DPROC __glewConvolutionFilter2D = nullptr;
PFNGLCONVOLUTIONPARAMETERFPROC __glewConvolutionParameterf = nullptr;
PFNGLCONVOLUTIONPARAMETERFVPROC __glewConvolutionParameterfv = nullptr;
PFNGLCONVOLUTIONPARAMETERIPROC __glewConvolutionParameteri = nullptr;
PFNGLCONVOLUTIONPARAMETERIVPROC __glewConvolutionParameteriv = nullptr;
PFNGLCOPYCOLORSUBTABLEPROC __glewCopyColorSubTable = nullptr;
PFNGLCOPYCOLORTABLEPROC __glewCopyColorTable = nullptr;
PFNGLCOPYCONVOLUTIONFILTER1DPROC __glewCopyConvolutionFilter1D = nullptr;
PFNGLCOPYCONVOLUTIONFILTER2DPROC __glewCopyConvolutionFilter2D = nullptr;
PFNGLGETCOLORTABLEPROC __glewGetColorTable = nullptr;
PFNGLGETCOLORTABLEPARAMETERFVPROC __glewGetColorTableParameterfv = nullptr;
PFNGLGETCOLORTABLEPARAMETERIVPROC __glewGetColorTableParameteriv = nullptr;
PFNGLGETCONVOLUTIONFILTERPROC __glewGetConvolutionFilter = nullptr;
PFNGLGETCONVOLUTIONPARAMETERFVPROC __glewGetConvolutionParameterfv = nullptr;
PFNGLGETCONVOLUTIONPARAMETERIVPROC __glewGetConvolutionParameteriv = nullptr;
PFNGLGETHISTOGRAMPROC __glewGetHistogram = nullptr;
PFNGLGETHISTOGRAMPARAMETERFVPROC __glewGetHistogramParameterfv = nullptr;
PFNGLGETHISTOGRAMPARAMETERIVPROC __glewGetHistogramParameteriv = nullptr;
PFNGLGETMINMAXPROC __glewGetMinmax = nullptr;
PFNGLGETMINMAXPARAMETERFVPROC __glewGetMinmaxParameterfv = nullptr;
PFNGLGETMINMAXPARAMETERIVPROC __glewGetMinmaxParameteriv = nullptr;
PFNGLGETSEPARABLEFILTERPROC __glewGetSeparableFilter = nullptr;
PFNGLHISTOGRAMPROC __glewHistogram = nullptr;
PFNGLMINMAXPROC __glewMinmax = nullptr;
PFNGLRESETHISTOGRAMPROC __glewResetHistogram = nullptr;
PFNGLRESETMINMAXPROC __glewResetMinmax = nullptr;
PFNGLSEPARABLEFILTER2DPROC __glewSeparableFilter2D = nullptr;

PFNGLDRAWARRAYSINSTANCEDARBPROC __glewDrawArraysInstancedARB = nullptr;
PFNGLDRAWELEMENTSINSTANCEDARBPROC __glewDrawElementsInstancedARB = nullptr;
PFNGLVERTEXATTRIBDIVISORARBPROC __glewVertexAttribDivisorARB = nullptr;

PFNGLGETINTERNALFORMATIVPROC __glewGetInternalformativ = nullptr;

PFNGLFLUSHMAPPEDBUFFERRANGEPROC __glewFlushMappedBufferRange = nullptr;
PFNGLMAPBUFFERRANGEPROC __glewMapBufferRange = nullptr;

PFNGLCURRENTPALETTEMATRIXARBPROC __glewCurrentPaletteMatrixARB = nullptr;
PFNGLMATRIXINDEXPOINTERARBPROC __glewMatrixIndexPointerARB = nullptr;
PFNGLMATRIXINDEXUBVARBPROC __glewMatrixIndexubvARB = nullptr;
PFNGLMATRIXINDEXUIVARBPROC __glewMatrixIndexuivARB = nullptr;
PFNGLMATRIXINDEXUSVARBPROC __glewMatrixIndexusvARB = nullptr;

PFNGLSAMPLECOVERAGEARBPROC __glewSampleCoverageARB = nullptr;

PFNGLACTIVETEXTUREARBPROC __glewActiveTextureARB = nullptr;
PFNGLCLIENTACTIVETEXTUREARBPROC __glewClientActiveTextureARB = nullptr;
PFNGLMULTITEXCOORD1DARBPROC __glewMultiTexCoord1dARB = nullptr;
PFNGLMULTITEXCOORD1DVARBPROC __glewMultiTexCoord1dvARB = nullptr;
PFNGLMULTITEXCOORD1FARBPROC __glewMultiTexCoord1fARB = nullptr;
PFNGLMULTITEXCOORD1FVARBPROC __glewMultiTexCoord1fvARB = nullptr;
PFNGLMULTITEXCOORD1IARBPROC __glewMultiTexCoord1iARB = nullptr;
PFNGLMULTITEXCOORD1IVARBPROC __glewMultiTexCoord1ivARB = nullptr;
PFNGLMULTITEXCOORD1SARBPROC __glewMultiTexCoord1sARB = nullptr;
PFNGLMULTITEXCOORD1SVARBPROC __glewMultiTexCoord1svARB = nullptr;
PFNGLMULTITEXCOORD2DARBPROC __glewMultiTexCoord2dARB = nullptr;
PFNGLMULTITEXCOORD2DVARBPROC __glewMultiTexCoord2dvARB = nullptr;
PFNGLMULTITEXCOORD2FARBPROC __glewMultiTexCoord2fARB = nullptr;
PFNGLMULTITEXCOORD2FVARBPROC __glewMultiTexCoord2fvARB = nullptr;
PFNGLMULTITEXCOORD2IARBPROC __glewMultiTexCoord2iARB = nullptr;
PFNGLMULTITEXCOORD2IVARBPROC __glewMultiTexCoord2ivARB = nullptr;
PFNGLMULTITEXCOORD2SARBPROC __glewMultiTexCoord2sARB = nullptr;
PFNGLMULTITEXCOORD2SVARBPROC __glewMultiTexCoord2svARB = nullptr;
PFNGLMULTITEXCOORD3DARBPROC __glewMultiTexCoord3dARB = nullptr;
PFNGLMULTITEXCOORD3DVARBPROC __glewMultiTexCoord3dvARB = nullptr;
PFNGLMULTITEXCOORD3FARBPROC __glewMultiTexCoord3fARB = nullptr;
PFNGLMULTITEXCOORD3FVARBPROC __glewMultiTexCoord3fvARB = nullptr;
PFNGLMULTITEXCOORD3IARBPROC __glewMultiTexCoord3iARB = nullptr;
PFNGLMULTITEXCOORD3IVARBPROC __glewMultiTexCoord3ivARB = nullptr;
PFNGLMULTITEXCOORD3SARBPROC __glewMultiTexCoord3sARB = nullptr;
PFNGLMULTITEXCOORD3SVARBPROC __glewMultiTexCoord3svARB = nullptr;
PFNGLMULTITEXCOORD4DARBPROC __glewMultiTexCoord4dARB = nullptr;
PFNGLMULTITEXCOORD4DVARBPROC __glewMultiTexCoord4dvARB = nullptr;
PFNGLMULTITEXCOORD4FARBPROC __glewMultiTexCoord4fARB = nullptr;
PFNGLMULTITEXCOORD4FVARBPROC __glewMultiTexCoord4fvARB = nullptr;
PFNGLMULTITEXCOORD4IARBPROC __glewMultiTexCoord4iARB = nullptr;
PFNGLMULTITEXCOORD4IVARBPROC __glewMultiTexCoord4ivARB = nullptr;
PFNGLMULTITEXCOORD4SARBPROC __glewMultiTexCoord4sARB = nullptr;
PFNGLMULTITEXCOORD4SVARBPROC __glewMultiTexCoord4svARB = nullptr;

PFNGLBEGINQUERYARBPROC __glewBeginQueryARB = nullptr;
PFNGLDELETEQUERIESARBPROC __glewDeleteQueriesARB = nullptr;
PFNGLENDQUERYARBPROC __glewEndQueryARB = nullptr;
PFNGLGENQUERIESARBPROC __glewGenQueriesARB = nullptr;
PFNGLGETQUERYOBJECTIVARBPROC __glewGetQueryObjectivARB = nullptr;
PFNGLGETQUERYOBJECTUIVARBPROC __glewGetQueryObjectuivARB = nullptr;
PFNGLGETQUERYIVARBPROC __glewGetQueryivARB = nullptr;
PFNGLISQUERYARBPROC __glewIsQueryARB = nullptr;

PFNGLPOINTPARAMETERFARBPROC __glewPointParameterfARB = nullptr;
PFNGLPOINTPARAMETERFVARBPROC __glewPointParameterfvARB = nullptr;

PFNGLPROVOKINGVERTEXPROC __glewProvokingVertex = nullptr;

PFNGLGETGRAPHICSRESETSTATUSARBPROC __glewGetGraphicsResetStatusARB = nullptr;
PFNGLGETNCOLORTABLEARBPROC __glewGetnColorTableARB = nullptr;
PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC __glewGetnCompressedTexImageARB = nullptr;
PFNGLGETNCONVOLUTIONFILTERARBPROC __glewGetnConvolutionFilterARB = nullptr;
PFNGLGETNHISTOGRAMARBPROC __glewGetnHistogramARB = nullptr;
PFNGLGETNMAPDVARBPROC __glewGetnMapdvARB = nullptr;
PFNGLGETNMAPFVARBPROC __glewGetnMapfvARB = nullptr;
PFNGLGETNMAPIVARBPROC __glewGetnMapivARB = nullptr;
PFNGLGETNMINMAXARBPROC __glewGetnMinmaxARB = nullptr;
PFNGLGETNPIXELMAPFVARBPROC __glewGetnPixelMapfvARB = nullptr;
PFNGLGETNPIXELMAPUIVARBPROC __glewGetnPixelMapuivARB = nullptr;
PFNGLGETNPIXELMAPUSVARBPROC __glewGetnPixelMapusvARB = nullptr;
PFNGLGETNPOLYGONSTIPPLEARBPROC __glewGetnPolygonStippleARB = nullptr;
PFNGLGETNSEPARABLEFILTERARBPROC __glewGetnSeparableFilterARB = nullptr;
PFNGLGETNTEXIMAGEARBPROC __glewGetnTexImageARB = nullptr;
PFNGLGETNUNIFORMDVARBPROC __glewGetnUniformdvARB = nullptr;
PFNGLGETNUNIFORMFVARBPROC __glewGetnUniformfvARB = nullptr;
PFNGLGETNUNIFORMIVARBPROC __glewGetnUniformivARB = nullptr;
PFNGLGETNUNIFORMUIVARBPROC __glewGetnUniformuivARB = nullptr;
PFNGLREADNPIXELSARBPROC __glewReadnPixelsARB = nullptr;

PFNGLMINSAMPLESHADINGARBPROC __glewMinSampleShadingARB = nullptr;

PFNGLBINDSAMPLERPROC __glewBindSampler = nullptr;
PFNGLDELETESAMPLERSPROC __glewDeleteSamplers = nullptr;
PFNGLGENSAMPLERSPROC __glewGenSamplers = nullptr;
PFNGLGETSAMPLERPARAMETERIIVPROC __glewGetSamplerParameterIiv = nullptr;
PFNGLGETSAMPLERPARAMETERIUIVPROC __glewGetSamplerParameterIuiv = nullptr;
PFNGLGETSAMPLERPARAMETERFVPROC __glewGetSamplerParameterfv = nullptr;
PFNGLGETSAMPLERPARAMETERIVPROC __glewGetSamplerParameteriv = nullptr;
PFNGLISSAMPLERPROC __glewIsSampler = nullptr;
PFNGLSAMPLERPARAMETERIIVPROC __glewSamplerParameterIiv = nullptr;
PFNGLSAMPLERPARAMETERIUIVPROC __glewSamplerParameterIuiv = nullptr;
PFNGLSAMPLERPARAMETERFPROC __glewSamplerParameterf = nullptr;
PFNGLSAMPLERPARAMETERFVPROC __glewSamplerParameterfv = nullptr;
PFNGLSAMPLERPARAMETERIPROC __glewSamplerParameteri = nullptr;
PFNGLSAMPLERPARAMETERIVPROC __glewSamplerParameteriv = nullptr;

PFNGLACTIVESHADERPROGRAMPROC __glewActiveShaderProgram = nullptr;
PFNGLBINDPROGRAMPIPELINEPROC __glewBindProgramPipeline = nullptr;
PFNGLCREATESHADERPROGRAMVPROC __glewCreateShaderProgramv = nullptr;
PFNGLDELETEPROGRAMPIPELINESPROC __glewDeleteProgramPipelines = nullptr;
PFNGLGENPROGRAMPIPELINESPROC __glewGenProgramPipelines = nullptr;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC __glewGetProgramPipelineInfoLog = nullptr;
PFNGLGETPROGRAMPIPELINEIVPROC __glewGetProgramPipelineiv = nullptr;
PFNGLISPROGRAMPIPELINEPROC __glewIsProgramPipeline = nullptr;
PFNGLPROGRAMUNIFORM1DPROC __glewProgramUniform1d = nullptr;
PFNGLPROGRAMUNIFORM1DVPROC __glewProgramUniform1dv = nullptr;
PFNGLPROGRAMUNIFORM1FPROC __glewProgramUniform1f = nullptr;
PFNGLPROGRAMUNIFORM1FVPROC __glewProgramUniform1fv = nullptr;
PFNGLPROGRAMUNIFORM1IPROC __glewProgramUniform1i = nullptr;
PFNGLPROGRAMUNIFORM1IVPROC __glewProgramUniform1iv = nullptr;
PFNGLPROGRAMUNIFORM1UIPROC __glewProgramUniform1ui = nullptr;
PFNGLPROGRAMUNIFORM1UIVPROC __glewProgramUniform1uiv = nullptr;
PFNGLPROGRAMUNIFORM2DPROC __glewProgramUniform2d = nullptr;
PFNGLPROGRAMUNIFORM2DVPROC __glewProgramUniform2dv = nullptr;
PFNGLPROGRAMUNIFORM2FPROC __glewProgramUniform2f = nullptr;
PFNGLPROGRAMUNIFORM2FVPROC __glewProgramUniform2fv = nullptr;
PFNGLPROGRAMUNIFORM2IPROC __glewProgramUniform2i = nullptr;
PFNGLPROGRAMUNIFORM2IVPROC __glewProgramUniform2iv = nullptr;
PFNGLPROGRAMUNIFORM2UIPROC __glewProgramUniform2ui = nullptr;
PFNGLPROGRAMUNIFORM2UIVPROC __glewProgramUniform2uiv = nullptr;
PFNGLPROGRAMUNIFORM3DPROC __glewProgramUniform3d = nullptr;
PFNGLPROGRAMUNIFORM3DVPROC __glewProgramUniform3dv = nullptr;
PFNGLPROGRAMUNIFORM3FPROC __glewProgramUniform3f = nullptr;
PFNGLPROGRAMUNIFORM3FVPROC __glewProgramUniform3fv = nullptr;
PFNGLPROGRAMUNIFORM3IPROC __glewProgramUniform3i = nullptr;
PFNGLPROGRAMUNIFORM3IVPROC __glewProgramUniform3iv = nullptr;
PFNGLPROGRAMUNIFORM3UIPROC __glewProgramUniform3ui = nullptr;
PFNGLPROGRAMUNIFORM3UIVPROC __glewProgramUniform3uiv = nullptr;
PFNGLPROGRAMUNIFORM4DPROC __glewProgramUniform4d = nullptr;
PFNGLPROGRAMUNIFORM4DVPROC __glewProgramUniform4dv = nullptr;
PFNGLPROGRAMUNIFORM4FPROC __glewProgramUniform4f = nullptr;
PFNGLPROGRAMUNIFORM4FVPROC __glewProgramUniform4fv = nullptr;
PFNGLPROGRAMUNIFORM4IPROC __glewProgramUniform4i = nullptr;
PFNGLPROGRAMUNIFORM4IVPROC __glewProgramUniform4iv = nullptr;
PFNGLPROGRAMUNIFORM4UIPROC __glewProgramUniform4ui = nullptr;
PFNGLPROGRAMUNIFORM4UIVPROC __glewProgramUniform4uiv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC __glewProgramUniformMatrix2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC __glewProgramUniformMatrix2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC __glewProgramUniformMatrix2x3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC __glewProgramUniformMatrix2x3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC __glewProgramUniformMatrix2x4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC __glewProgramUniformMatrix2x4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC __glewProgramUniformMatrix3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC __glewProgramUniformMatrix3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC __glewProgramUniformMatrix3x2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC __glewProgramUniformMatrix3x2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC __glewProgramUniformMatrix3x4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC __glewProgramUniformMatrix3x4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC __glewProgramUniformMatrix4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC __glewProgramUniformMatrix4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC __glewProgramUniformMatrix4x2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC __glewProgramUniformMatrix4x2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC __glewProgramUniformMatrix4x3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC __glewProgramUniformMatrix4x3fv = nullptr;
PFNGLUSEPROGRAMSTAGESPROC __glewUseProgramStages = nullptr;
PFNGLVALIDATEPROGRAMPIPELINEPROC __glewValidateProgramPipeline = nullptr;

PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC __glewGetActiveAtomicCounterBufferiv = nullptr;

PFNGLBINDIMAGETEXTUREPROC __glewBindImageTexture = nullptr;
PFNGLMEMORYBARRIERPROC __glewMemoryBarrier = nullptr;

PFNGLATTACHOBJECTARBPROC __glewAttachObjectARB = nullptr;
PFNGLCOMPILESHADERARBPROC __glewCompileShaderARB = nullptr;
PFNGLCREATEPROGRAMOBJECTARBPROC __glewCreateProgramObjectARB = nullptr;
PFNGLCREATESHADEROBJECTARBPROC __glewCreateShaderObjectARB = nullptr;
PFNGLDELETEOBJECTARBPROC __glewDeleteObjectARB = nullptr;
PFNGLDETACHOBJECTARBPROC __glewDetachObjectARB = nullptr;
PFNGLGETACTIVEUNIFORMARBPROC __glewGetActiveUniformARB = nullptr;
PFNGLGETATTACHEDOBJECTSARBPROC __glewGetAttachedObjectsARB = nullptr;
PFNGLGETHANDLEARBPROC __glewGetHandleARB = nullptr;
PFNGLGETINFOLOGARBPROC __glewGetInfoLogARB = nullptr;
PFNGLGETOBJECTPARAMETERFVARBPROC __glewGetObjectParameterfvARB = nullptr;
PFNGLGETOBJECTPARAMETERIVARBPROC __glewGetObjectParameterivARB = nullptr;
PFNGLGETSHADERSOURCEARBPROC __glewGetShaderSourceARB = nullptr;
PFNGLGETUNIFORMLOCATIONARBPROC __glewGetUniformLocationARB = nullptr;
PFNGLGETUNIFORMFVARBPROC __glewGetUniformfvARB = nullptr;
PFNGLGETUNIFORMIVARBPROC __glewGetUniformivARB = nullptr;
PFNGLLINKPROGRAMARBPROC __glewLinkProgramARB = nullptr;
PFNGLSHADERSOURCEARBPROC __glewShaderSourceARB = nullptr;
PFNGLUNIFORM1FARBPROC __glewUniform1fARB = nullptr;
PFNGLUNIFORM1FVARBPROC __glewUniform1fvARB = nullptr;
PFNGLUNIFORM1IARBPROC __glewUniform1iARB = nullptr;
PFNGLUNIFORM1IVARBPROC __glewUniform1ivARB = nullptr;
PFNGLUNIFORM2FARBPROC __glewUniform2fARB = nullptr;
PFNGLUNIFORM2FVARBPROC __glewUniform2fvARB = nullptr;
PFNGLUNIFORM2IARBPROC __glewUniform2iARB = nullptr;
PFNGLUNIFORM2IVARBPROC __glewUniform2ivARB = nullptr;
PFNGLUNIFORM3FARBPROC __glewUniform3fARB = nullptr;
PFNGLUNIFORM3FVARBPROC __glewUniform3fvARB = nullptr;
PFNGLUNIFORM3IARBPROC __glewUniform3iARB = nullptr;
PFNGLUNIFORM3IVARBPROC __glewUniform3ivARB = nullptr;
PFNGLUNIFORM4FARBPROC __glewUniform4fARB = nullptr;
PFNGLUNIFORM4FVARBPROC __glewUniform4fvARB = nullptr;
PFNGLUNIFORM4IARBPROC __glewUniform4iARB = nullptr;
PFNGLUNIFORM4IVARBPROC __glewUniform4ivARB = nullptr;
PFNGLUNIFORMMATRIX2FVARBPROC __glewUniformMatrix2fvARB = nullptr;
PFNGLUNIFORMMATRIX3FVARBPROC __glewUniformMatrix3fvARB = nullptr;
PFNGLUNIFORMMATRIX4FVARBPROC __glewUniformMatrix4fvARB = nullptr;
PFNGLUSEPROGRAMOBJECTARBPROC __glewUseProgramObjectARB = nullptr;
PFNGLVALIDATEPROGRAMARBPROC __glewValidateProgramARB = nullptr;

PFNGLGETACTIVESUBROUTINENAMEPROC __glewGetActiveSubroutineName = nullptr;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC __glewGetActiveSubroutineUniformName = nullptr;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC __glewGetActiveSubroutineUniformiv = nullptr;
PFNGLGETPROGRAMSTAGEIVPROC __glewGetProgramStageiv = nullptr;
PFNGLGETSUBROUTINEINDEXPROC __glewGetSubroutineIndex = nullptr;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC __glewGetSubroutineUniformLocation = nullptr;
PFNGLGETUNIFORMSUBROUTINEUIVPROC __glewGetUniformSubroutineuiv = nullptr;
PFNGLUNIFORMSUBROUTINESUIVPROC __glewUniformSubroutinesuiv = nullptr;

PFNGLCOMPILESHADERINCLUDEARBPROC __glewCompileShaderIncludeARB = nullptr;
PFNGLDELETENAMEDSTRINGARBPROC __glewDeleteNamedStringARB = nullptr;
PFNGLGETNAMEDSTRINGARBPROC __glewGetNamedStringARB = nullptr;
PFNGLGETNAMEDSTRINGIVARBPROC __glewGetNamedStringivARB = nullptr;
PFNGLISNAMEDSTRINGARBPROC __glewIsNamedStringARB = nullptr;
PFNGLNAMEDSTRINGARBPROC __glewNamedStringARB = nullptr;

PFNGLCLIENTWAITSYNCPROC __glewClientWaitSync = nullptr;
PFNGLDELETESYNCPROC __glewDeleteSync = nullptr;
PFNGLFENCESYNCPROC __glewFenceSync = nullptr;
PFNGLGETINTEGER64VPROC __glewGetInteger64v = nullptr;
PFNGLGETSYNCIVPROC __glewGetSynciv = nullptr;
PFNGLISSYNCPROC __glewIsSync = nullptr;
PFNGLWAITSYNCPROC __glewWaitSync = nullptr;

PFNGLPATCHPARAMETERFVPROC __glewPatchParameterfv = nullptr;
PFNGLPATCHPARAMETERIPROC __glewPatchParameteri = nullptr;

PFNGLTEXBUFFERARBPROC __glewTexBufferARB = nullptr;

PFNGLCOMPRESSEDTEXIMAGE1DARBPROC __glewCompressedTexImage1DARB = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC __glewCompressedTexImage2DARB = nullptr;
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC __glewCompressedTexImage3DARB = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC __glewCompressedTexSubImage1DARB = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC __glewCompressedTexSubImage2DARB = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC __glewCompressedTexSubImage3DARB = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC __glewGetCompressedTexImageARB = nullptr;

PFNGLGETMULTISAMPLEFVPROC __glewGetMultisamplefv = nullptr;
PFNGLSAMPLEMASKIPROC __glewSampleMaski = nullptr;
PFNGLTEXIMAGE2DMULTISAMPLEPROC __glewTexImage2DMultisample = nullptr;
PFNGLTEXIMAGE3DMULTISAMPLEPROC __glewTexImage3DMultisample = nullptr;

PFNGLTEXSTORAGE1DPROC __glewTexStorage1D = nullptr;
PFNGLTEXSTORAGE2DPROC __glewTexStorage2D = nullptr;
PFNGLTEXSTORAGE3DPROC __glewTexStorage3D = nullptr;
PFNGLTEXTURESTORAGE1DEXTPROC __glewTextureStorage1DEXT = nullptr;
PFNGLTEXTURESTORAGE2DEXTPROC __glewTextureStorage2DEXT = nullptr;
PFNGLTEXTURESTORAGE3DEXTPROC __glewTextureStorage3DEXT = nullptr;

PFNGLGETQUERYOBJECTI64VPROC __glewGetQueryObjecti64v = nullptr;
PFNGLGETQUERYOBJECTUI64VPROC __glewGetQueryObjectui64v = nullptr;
PFNGLQUERYCOUNTERPROC __glewQueryCounter = nullptr;

PFNGLBINDTRANSFORMFEEDBACKPROC __glewBindTransformFeedback = nullptr;
PFNGLDELETETRANSFORMFEEDBACKSPROC __glewDeleteTransformFeedbacks = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKPROC __glewDrawTransformFeedback = nullptr;
PFNGLGENTRANSFORMFEEDBACKSPROC __glewGenTransformFeedbacks = nullptr;
PFNGLISTRANSFORMFEEDBACKPROC __glewIsTransformFeedback = nullptr;
PFNGLPAUSETRANSFORMFEEDBACKPROC __glewPauseTransformFeedback = nullptr;
PFNGLRESUMETRANSFORMFEEDBACKPROC __glewResumeTransformFeedback = nullptr;

PFNGLBEGINQUERYINDEXEDPROC __glewBeginQueryIndexed = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC __glewDrawTransformFeedbackStream = nullptr;
PFNGLENDQUERYINDEXEDPROC __glewEndQueryIndexed = nullptr;
PFNGLGETQUERYINDEXEDIVPROC __glewGetQueryIndexediv = nullptr;

PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC __glewDrawTransformFeedbackInstanced = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC __glewDrawTransformFeedbackStreamInstanced = nullptr;

PFNGLLOADTRANSPOSEMATRIXDARBPROC __glewLoadTransposeMatrixdARB = nullptr;
PFNGLLOADTRANSPOSEMATRIXFARBPROC __glewLoadTransposeMatrixfARB = nullptr;
PFNGLMULTTRANSPOSEMATRIXDARBPROC __glewMultTransposeMatrixdARB = nullptr;
PFNGLMULTTRANSPOSEMATRIXFARBPROC __glewMultTransposeMatrixfARB = nullptr;

PFNGLBINDBUFFERBASEPROC __glewBindBufferBase = nullptr;
PFNGLBINDBUFFERRANGEPROC __glewBindBufferRange = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC __glewGetActiveUniformBlockName = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC __glewGetActiveUniformBlockiv = nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC __glewGetActiveUniformName = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC __glewGetActiveUniformsiv = nullptr;
PFNGLGETINTEGERI_VPROC __glewGetIntegeri_v = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC __glewGetUniformBlockIndex = nullptr;
PFNGLGETUNIFORMINDICESPROC __glewGetUniformIndices = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC __glewUniformBlockBinding = nullptr;

PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays = nullptr;
PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays = nullptr;
PFNGLISVERTEXARRAYPROC __glewIsVertexArray = nullptr;

PFNGLGETVERTEXATTRIBLDVPROC __glewGetVertexAttribLdv = nullptr;
PFNGLVERTEXATTRIBL1DPROC __glewVertexAttribL1d = nullptr;
PFNGLVERTEXATTRIBL1DVPROC __glewVertexAttribL1dv = nullptr;
PFNGLVERTEXATTRIBL2DPROC __glewVertexAttribL2d = nullptr;
PFNGLVERTEXATTRIBL2DVPROC __glewVertexAttribL2dv = nullptr;
PFNGLVERTEXATTRIBL3DPROC __glewVertexAttribL3d = nullptr;
PFNGLVERTEXATTRIBL3DVPROC __glewVertexAttribL3dv = nullptr;
PFNGLVERTEXATTRIBL4DPROC __glewVertexAttribL4d = nullptr;
PFNGLVERTEXATTRIBL4DVPROC __glewVertexAttribL4dv = nullptr;
PFNGLVERTEXATTRIBLPOINTERPROC __glewVertexAttribLPointer = nullptr;

PFNGLVERTEXBLENDARBPROC __glewVertexBlendARB = nullptr;
PFNGLWEIGHTPOINTERARBPROC __glewWeightPointerARB = nullptr;
PFNGLWEIGHTBVARBPROC __glewWeightbvARB = nullptr;
PFNGLWEIGHTDVARBPROC __glewWeightdvARB = nullptr;
PFNGLWEIGHTFVARBPROC __glewWeightfvARB = nullptr;
PFNGLWEIGHTIVARBPROC __glewWeightivARB = nullptr;
PFNGLWEIGHTSVARBPROC __glewWeightsvARB = nullptr;
PFNGLWEIGHTUBVARBPROC __glewWeightubvARB = nullptr;
PFNGLWEIGHTUIVARBPROC __glewWeightuivARB = nullptr;
PFNGLWEIGHTUSVARBPROC __glewWeightusvARB = nullptr;

PFNGLBINDBUFFERARBPROC __glewBindBufferARB = nullptr;
PFNGLBUFFERDATAARBPROC __glewBufferDataARB = nullptr;
PFNGLBUFFERSUBDATAARBPROC __glewBufferSubDataARB = nullptr;
PFNGLDELETEBUFFERSARBPROC __glewDeleteBuffersARB = nullptr;
PFNGLGENBUFFERSARBPROC __glewGenBuffersARB = nullptr;
PFNGLGETBUFFERPARAMETERIVARBPROC __glewGetBufferParameterivARB = nullptr;
PFNGLGETBUFFERPOINTERVARBPROC __glewGetBufferPointervARB = nullptr;
PFNGLGETBUFFERSUBDATAARBPROC __glewGetBufferSubDataARB = nullptr;
PFNGLISBUFFERARBPROC __glewIsBufferARB = nullptr;
PFNGLMAPBUFFERARBPROC __glewMapBufferARB = nullptr;
PFNGLUNMAPBUFFERARBPROC __glewUnmapBufferARB = nullptr;

PFNGLBINDPROGRAMARBPROC __glewBindProgramARB = nullptr;
PFNGLDELETEPROGRAMSARBPROC __glewDeleteProgramsARB = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC __glewDisableVertexAttribArrayARB = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC __glewEnableVertexAttribArrayARB = nullptr;
PFNGLGENPROGRAMSARBPROC __glewGenProgramsARB = nullptr;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC __glewGetProgramEnvParameterdvARB = nullptr;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC __glewGetProgramEnvParameterfvARB = nullptr;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC __glewGetProgramLocalParameterdvARB = nullptr;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC __glewGetProgramLocalParameterfvARB = nullptr;
PFNGLGETPROGRAMSTRINGARBPROC __glewGetProgramStringARB = nullptr;
PFNGLGETPROGRAMIVARBPROC __glewGetProgramivARB = nullptr;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC __glewGetVertexAttribPointervARB = nullptr;
PFNGLGETVERTEXATTRIBDVARBPROC __glewGetVertexAttribdvARB = nullptr;
PFNGLGETVERTEXATTRIBFVARBPROC __glewGetVertexAttribfvARB = nullptr;
PFNGLGETVERTEXATTRIBIVARBPROC __glewGetVertexAttribivARB = nullptr;
PFNGLISPROGRAMARBPROC __glewIsProgramARB = nullptr;
PFNGLPROGRAMENVPARAMETER4DARBPROC __glewProgramEnvParameter4dARB = nullptr;
PFNGLPROGRAMENVPARAMETER4DVARBPROC __glewProgramEnvParameter4dvARB = nullptr;
PFNGLPROGRAMENVPARAMETER4FARBPROC __glewProgramEnvParameter4fARB = nullptr;
PFNGLPROGRAMENVPARAMETER4FVARBPROC __glewProgramEnvParameter4fvARB = nullptr;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC __glewProgramLocalParameter4dARB = nullptr;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC __glewProgramLocalParameter4dvARB = nullptr;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC __glewProgramLocalParameter4fARB = nullptr;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC __glewProgramLocalParameter4fvARB = nullptr;
PFNGLPROGRAMSTRINGARBPROC __glewProgramStringARB = nullptr;
PFNGLVERTEXATTRIB1DARBPROC __glewVertexAttrib1dARB = nullptr;
PFNGLVERTEXATTRIB1DVARBPROC __glewVertexAttrib1dvARB = nullptr;
PFNGLVERTEXATTRIB1FARBPROC __glewVertexAttrib1fARB = nullptr;
PFNGLVERTEXATTRIB1FVARBPROC __glewVertexAttrib1fvARB = nullptr;
PFNGLVERTEXATTRIB1SARBPROC __glewVertexAttrib1sARB = nullptr;
PFNGLVERTEXATTRIB1SVARBPROC __glewVertexAttrib1svARB = nullptr;
PFNGLVERTEXATTRIB2DARBPROC __glewVertexAttrib2dARB = nullptr;
PFNGLVERTEXATTRIB2DVARBPROC __glewVertexAttrib2dvARB = nullptr;
PFNGLVERTEXATTRIB2FARBPROC __glewVertexAttrib2fARB = nullptr;
PFNGLVERTEXATTRIB2FVARBPROC __glewVertexAttrib2fvARB = nullptr;
PFNGLVERTEXATTRIB2SARBPROC __glewVertexAttrib2sARB = nullptr;
PFNGLVERTEXATTRIB2SVARBPROC __glewVertexAttrib2svARB = nullptr;
PFNGLVERTEXATTRIB3DARBPROC __glewVertexAttrib3dARB = nullptr;
PFNGLVERTEXATTRIB3DVARBPROC __glewVertexAttrib3dvARB = nullptr;
PFNGLVERTEXATTRIB3FARBPROC __glewVertexAttrib3fARB = nullptr;
PFNGLVERTEXATTRIB3FVARBPROC __glewVertexAttrib3fvARB = nullptr;
PFNGLVERTEXATTRIB3SARBPROC __glewVertexAttrib3sARB = nullptr;
PFNGLVERTEXATTRIB3SVARBPROC __glewVertexAttrib3svARB = nullptr;
PFNGLVERTEXATTRIB4NBVARBPROC __glewVertexAttrib4NbvARB = nullptr;
PFNGLVERTEXATTRIB4NIVARBPROC __glewVertexAttrib4NivARB = nullptr;
PFNGLVERTEXATTRIB4NSVARBPROC __glewVertexAttrib4NsvARB = nullptr;
PFNGLVERTEXATTRIB4NUBARBPROC __glewVertexAttrib4NubARB = nullptr;
PFNGLVERTEXATTRIB4NUBVARBPROC __glewVertexAttrib4NubvARB = nullptr;
PFNGLVERTEXATTRIB4NUIVARBPROC __glewVertexAttrib4NuivARB = nullptr;
PFNGLVERTEXATTRIB4NUSVARBPROC __glewVertexAttrib4NusvARB = nullptr;
PFNGLVERTEXATTRIB4BVARBPROC __glewVertexAttrib4bvARB = nullptr;
PFNGLVERTEXATTRIB4DARBPROC __glewVertexAttrib4dARB = nullptr;
PFNGLVERTEXATTRIB4DVARBPROC __glewVertexAttrib4dvARB = nullptr;
PFNGLVERTEXATTRIB4FARBPROC __glewVertexAttrib4fARB = nullptr;
PFNGLVERTEXATTRIB4FVARBPROC __glewVertexAttrib4fvARB = nullptr;
PFNGLVERTEXATTRIB4IVARBPROC __glewVertexAttrib4ivARB = nullptr;
PFNGLVERTEXATTRIB4SARBPROC __glewVertexAttrib4sARB = nullptr;
PFNGLVERTEXATTRIB4SVARBPROC __glewVertexAttrib4svARB = nullptr;
PFNGLVERTEXATTRIB4UBVARBPROC __glewVertexAttrib4ubvARB = nullptr;
PFNGLVERTEXATTRIB4UIVARBPROC __glewVertexAttrib4uivARB = nullptr;
PFNGLVERTEXATTRIB4USVARBPROC __glewVertexAttrib4usvARB = nullptr;
PFNGLVERTEXATTRIBPOINTERARBPROC __glewVertexAttribPointerARB = nullptr;

PFNGLBINDATTRIBLOCATIONARBPROC __glewBindAttribLocationARB = nullptr;
PFNGLGETACTIVEATTRIBARBPROC __glewGetActiveAttribARB = nullptr;
PFNGLGETATTRIBLOCATIONARBPROC __glewGetAttribLocationARB = nullptr;

PFNGLCOLORP3UIPROC __glewColorP3ui = nullptr;
PFNGLCOLORP3UIVPROC __glewColorP3uiv = nullptr;
PFNGLCOLORP4UIPROC __glewColorP4ui = nullptr;
PFNGLCOLORP4UIVPROC __glewColorP4uiv = nullptr;
PFNGLMULTITEXCOORDP1UIPROC __glewMultiTexCoordP1ui = nullptr;
PFNGLMULTITEXCOORDP1UIVPROC __glewMultiTexCoordP1uiv = nullptr;
PFNGLMULTITEXCOORDP2UIPROC __glewMultiTexCoordP2ui = nullptr;
PFNGLMULTITEXCOORDP2UIVPROC __glewMultiTexCoordP2uiv = nullptr;
PFNGLMULTITEXCOORDP3UIPROC __glewMultiTexCoordP3ui = nullptr;
PFNGLMULTITEXCOORDP3UIVPROC __glewMultiTexCoordP3uiv = nullptr;
PFNGLMULTITEXCOORDP4UIPROC __glewMultiTexCoordP4ui = nullptr;
PFNGLMULTITEXCOORDP4UIVPROC __glewMultiTexCoordP4uiv = nullptr;
PFNGLNORMALP3UIPROC __glewNormalP3ui = nullptr;
PFNGLNORMALP3UIVPROC __glewNormalP3uiv = nullptr;
PFNGLSECONDARYCOLORP3UIPROC __glewSecondaryColorP3ui = nullptr;
PFNGLSECONDARYCOLORP3UIVPROC __glewSecondaryColorP3uiv = nullptr;
PFNGLTEXCOORDP1UIPROC __glewTexCoordP1ui = nullptr;
PFNGLTEXCOORDP1UIVPROC __glewTexCoordP1uiv = nullptr;
PFNGLTEXCOORDP2UIPROC __glewTexCoordP2ui = nullptr;
PFNGLTEXCOORDP2UIVPROC __glewTexCoordP2uiv = nullptr;
PFNGLTEXCOORDP3UIPROC __glewTexCoordP3ui = nullptr;
PFNGLTEXCOORDP3UIVPROC __glewTexCoordP3uiv = nullptr;
PFNGLTEXCOORDP4UIPROC __glewTexCoordP4ui = nullptr;
PFNGLTEXCOORDP4UIVPROC __glewTexCoordP4uiv = nullptr;
PFNGLVERTEXATTRIBP1UIPROC __glewVertexAttribP1ui = nullptr;
PFNGLVERTEXATTRIBP1UIVPROC __glewVertexAttribP1uiv = nullptr;
PFNGLVERTEXATTRIBP2UIPROC __glewVertexAttribP2ui = nullptr;
PFNGLVERTEXATTRIBP2UIVPROC __glewVertexAttribP2uiv = nullptr;
PFNGLVERTEXATTRIBP3UIPROC __glewVertexAttribP3ui = nullptr;
PFNGLVERTEXATTRIBP3UIVPROC __glewVertexAttribP3uiv = nullptr;
PFNGLVERTEXATTRIBP4UIPROC __glewVertexAttribP4ui = nullptr;
PFNGLVERTEXATTRIBP4UIVPROC __glewVertexAttribP4uiv = nullptr;
PFNGLVERTEXP2UIPROC __glewVertexP2ui = nullptr;
PFNGLVERTEXP2UIVPROC __glewVertexP2uiv = nullptr;
PFNGLVERTEXP3UIPROC __glewVertexP3ui = nullptr;
PFNGLVERTEXP3UIVPROC __glewVertexP3uiv = nullptr;
PFNGLVERTEXP4UIPROC __glewVertexP4ui = nullptr;
PFNGLVERTEXP4UIVPROC __glewVertexP4uiv = nullptr;

PFNGLDEPTHRANGEARRAYVPROC __glewDepthRangeArrayv = nullptr;
PFNGLDEPTHRANGEINDEXEDPROC __glewDepthRangeIndexed = nullptr;
PFNGLGETDOUBLEI_VPROC __glewGetDoublei_v = nullptr;
PFNGLGETFLOATI_VPROC __glewGetFloati_v = nullptr;
PFNGLSCISSORARRAYVPROC __glewScissorArrayv = nullptr;
PFNGLSCISSORINDEXEDPROC __glewScissorIndexed = nullptr;
PFNGLSCISSORINDEXEDVPROC __glewScissorIndexedv = nullptr;
PFNGLVIEWPORTARRAYVPROC __glewViewportArrayv = nullptr;
PFNGLVIEWPORTINDEXEDFPROC __glewViewportIndexedf = nullptr;
PFNGLVIEWPORTINDEXEDFVPROC __glewViewportIndexedfv = nullptr;

PFNGLWINDOWPOS2DARBPROC __glewWindowPos2dARB = nullptr;
PFNGLWINDOWPOS2DVARBPROC __glewWindowPos2dvARB = nullptr;
PFNGLWINDOWPOS2FARBPROC __glewWindowPos2fARB = nullptr;
PFNGLWINDOWPOS2FVARBPROC __glewWindowPos2fvARB = nullptr;
PFNGLWINDOWPOS2IARBPROC __glewWindowPos2iARB = nullptr;
PFNGLWINDOWPOS2IVARBPROC __glewWindowPos2ivARB = nullptr;
PFNGLWINDOWPOS2SARBPROC __glewWindowPos2sARB = nullptr;
PFNGLWINDOWPOS2SVARBPROC __glewWindowPos2svARB = nullptr;
PFNGLWINDOWPOS3DARBPROC __glewWindowPos3dARB = nullptr;
PFNGLWINDOWPOS3DVARBPROC __glewWindowPos3dvARB = nullptr;
PFNGLWINDOWPOS3FARBPROC __glewWindowPos3fARB = nullptr;
PFNGLWINDOWPOS3FVARBPROC __glewWindowPos3fvARB = nullptr;
PFNGLWINDOWPOS3IARBPROC __glewWindowPos3iARB = nullptr;
PFNGLWINDOWPOS3IVARBPROC __glewWindowPos3ivARB = nullptr;
PFNGLWINDOWPOS3SARBPROC __glewWindowPos3sARB = nullptr;
PFNGLWINDOWPOS3SVARBPROC __glewWindowPos3svARB = nullptr;

PFNGLDRAWBUFFERSATIPROC __glewDrawBuffersATI = nullptr;

PFNGLDRAWELEMENTARRAYATIPROC __glewDrawElementArrayATI = nullptr;
PFNGLDRAWRANGEELEMENTARRAYATIPROC __glewDrawRangeElementArrayATI = nullptr;
PFNGLELEMENTPOINTERATIPROC __glewElementPointerATI = nullptr;

PFNGLGETTEXBUMPPARAMETERFVATIPROC __glewGetTexBumpParameterfvATI = nullptr;
PFNGLGETTEXBUMPPARAMETERIVATIPROC __glewGetTexBumpParameterivATI = nullptr;
PFNGLTEXBUMPPARAMETERFVATIPROC __glewTexBumpParameterfvATI = nullptr;
PFNGLTEXBUMPPARAMETERIVATIPROC __glewTexBumpParameterivATI = nullptr;

PFNGLALPHAFRAGMENTOP1ATIPROC __glewAlphaFragmentOp1ATI = nullptr;
PFNGLALPHAFRAGMENTOP2ATIPROC __glewAlphaFragmentOp2ATI = nullptr;
PFNGLALPHAFRAGMENTOP3ATIPROC __glewAlphaFragmentOp3ATI = nullptr;
PFNGLBEGINFRAGMENTSHADERATIPROC __glewBeginFragmentShaderATI = nullptr;
PFNGLBINDFRAGMENTSHADERATIPROC __glewBindFragmentShaderATI = nullptr;
PFNGLCOLORFRAGMENTOP1ATIPROC __glewColorFragmentOp1ATI = nullptr;
PFNGLCOLORFRAGMENTOP2ATIPROC __glewColorFragmentOp2ATI = nullptr;
PFNGLCOLORFRAGMENTOP3ATIPROC __glewColorFragmentOp3ATI = nullptr;
PFNGLDELETEFRAGMENTSHADERATIPROC __glewDeleteFragmentShaderATI = nullptr;
PFNGLENDFRAGMENTSHADERATIPROC __glewEndFragmentShaderATI = nullptr;
PFNGLGENFRAGMENTSHADERSATIPROC __glewGenFragmentShadersATI = nullptr;
PFNGLPASSTEXCOORDATIPROC __glewPassTexCoordATI = nullptr;
PFNGLSAMPLEMAPATIPROC __glewSampleMapATI = nullptr;
PFNGLSETFRAGMENTSHADERCONSTANTATIPROC __glewSetFragmentShaderConstantATI = nullptr;

PFNGLMAPOBJECTBUFFERATIPROC __glewMapObjectBufferATI = nullptr;
PFNGLUNMAPOBJECTBUFFERATIPROC __glewUnmapObjectBufferATI = nullptr;

PFNGLPNTRIANGLESFATIPROC __glewPNTrianglesfATI = nullptr;
PFNGLPNTRIANGLESIATIPROC __glewPNTrianglesiATI = nullptr;

PFNGLSTENCILFUNCSEPARATEATIPROC __glewStencilFuncSeparateATI = nullptr;
PFNGLSTENCILOPSEPARATEATIPROC __glewStencilOpSeparateATI = nullptr;

PFNGLARRAYOBJECTATIPROC __glewArrayObjectATI = nullptr;
PFNGLFREEOBJECTBUFFERATIPROC __glewFreeObjectBufferATI = nullptr;
PFNGLGETARRAYOBJECTFVATIPROC __glewGetArrayObjectfvATI = nullptr;
PFNGLGETARRAYOBJECTIVATIPROC __glewGetArrayObjectivATI = nullptr;
PFNGLGETOBJECTBUFFERFVATIPROC __glewGetObjectBufferfvATI = nullptr;
PFNGLGETOBJECTBUFFERIVATIPROC __glewGetObjectBufferivATI = nullptr;
PFNGLGETVARIANTARRAYOBJECTFVATIPROC __glewGetVariantArrayObjectfvATI = nullptr;
PFNGLGETVARIANTARRAYOBJECTIVATIPROC __glewGetVariantArrayObjectivATI = nullptr;
PFNGLISOBJECTBUFFERATIPROC __glewIsObjectBufferATI = nullptr;
PFNGLNEWOBJECTBUFFERATIPROC __glewNewObjectBufferATI = nullptr;
PFNGLUPDATEOBJECTBUFFERATIPROC __glewUpdateObjectBufferATI = nullptr;
PFNGLVARIANTARRAYOBJECTATIPROC __glewVariantArrayObjectATI = nullptr;

PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC __glewGetVertexAttribArrayObjectfvATI = nullptr;
PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC __glewGetVertexAttribArrayObjectivATI = nullptr;
PFNGLVERTEXATTRIBARRAYOBJECTATIPROC __glewVertexAttribArrayObjectATI = nullptr;

PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC __glewClientActiveVertexStreamATI = nullptr;
PFNGLNORMALSTREAM3BATIPROC __glewNormalStream3bATI = nullptr;
PFNGLNORMALSTREAM3BVATIPROC __glewNormalStream3bvATI = nullptr;
PFNGLNORMALSTREAM3DATIPROC __glewNormalStream3dATI = nullptr;
PFNGLNORMALSTREAM3DVATIPROC __glewNormalStream3dvATI = nullptr;
PFNGLNORMALSTREAM3FATIPROC __glewNormalStream3fATI = nullptr;
PFNGLNORMALSTREAM3FVATIPROC __glewNormalStream3fvATI = nullptr;
PFNGLNORMALSTREAM3IATIPROC __glewNormalStream3iATI = nullptr;
PFNGLNORMALSTREAM3IVATIPROC __glewNormalStream3ivATI = nullptr;
PFNGLNORMALSTREAM3SATIPROC __glewNormalStream3sATI = nullptr;
PFNGLNORMALSTREAM3SVATIPROC __glewNormalStream3svATI = nullptr;
PFNGLVERTEXBLENDENVFATIPROC __glewVertexBlendEnvfATI = nullptr;
PFNGLVERTEXBLENDENVIATIPROC __glewVertexBlendEnviATI = nullptr;
PFNGLVERTEXSTREAM2DATIPROC __glewVertexStream2dATI = nullptr;
PFNGLVERTEXSTREAM2DVATIPROC __glewVertexStream2dvATI = nullptr;
PFNGLVERTEXSTREAM2FATIPROC __glewVertexStream2fATI = nullptr;
PFNGLVERTEXSTREAM2FVATIPROC __glewVertexStream2fvATI = nullptr;
PFNGLVERTEXSTREAM2IATIPROC __glewVertexStream2iATI = nullptr;
PFNGLVERTEXSTREAM2IVATIPROC __glewVertexStream2ivATI = nullptr;
PFNGLVERTEXSTREAM2SATIPROC __glewVertexStream2sATI = nullptr;
PFNGLVERTEXSTREAM2SVATIPROC __glewVertexStream2svATI = nullptr;
PFNGLVERTEXSTREAM3DATIPROC __glewVertexStream3dATI = nullptr;
PFNGLVERTEXSTREAM3DVATIPROC __glewVertexStream3dvATI = nullptr;
PFNGLVERTEXSTREAM3FATIPROC __glewVertexStream3fATI = nullptr;
PFNGLVERTEXSTREAM3FVATIPROC __glewVertexStream3fvATI = nullptr;
PFNGLVERTEXSTREAM3IATIPROC __glewVertexStream3iATI = nullptr;
PFNGLVERTEXSTREAM3IVATIPROC __glewVertexStream3ivATI = nullptr;
PFNGLVERTEXSTREAM3SATIPROC __glewVertexStream3sATI = nullptr;
PFNGLVERTEXSTREAM3SVATIPROC __glewVertexStream3svATI = nullptr;
PFNGLVERTEXSTREAM4DATIPROC __glewVertexStream4dATI = nullptr;
PFNGLVERTEXSTREAM4DVATIPROC __glewVertexStream4dvATI = nullptr;
PFNGLVERTEXSTREAM4FATIPROC __glewVertexStream4fATI = nullptr;
PFNGLVERTEXSTREAM4FVATIPROC __glewVertexStream4fvATI = nullptr;
PFNGLVERTEXSTREAM4IATIPROC __glewVertexStream4iATI = nullptr;
PFNGLVERTEXSTREAM4IVATIPROC __glewVertexStream4ivATI = nullptr;
PFNGLVERTEXSTREAM4SATIPROC __glewVertexStream4sATI = nullptr;
PFNGLVERTEXSTREAM4SVATIPROC __glewVertexStream4svATI = nullptr;

PFNGLGETUNIFORMBUFFERSIZEEXTPROC __glewGetUniformBufferSizeEXT = nullptr;
PFNGLGETUNIFORMOFFSETEXTPROC __glewGetUniformOffsetEXT = nullptr;
PFNGLUNIFORMBUFFEREXTPROC __glewUniformBufferEXT = nullptr;

PFNGLBLENDCOLOREXTPROC __glewBlendColorEXT = nullptr;

PFNGLBLENDEQUATIONSEPARATEEXTPROC __glewBlendEquationSeparateEXT = nullptr;

PFNGLBLENDFUNCSEPARATEEXTPROC __glewBlendFuncSeparateEXT = nullptr;

PFNGLBLENDEQUATIONEXTPROC __glewBlendEquationEXT = nullptr;

PFNGLCOLORSUBTABLEEXTPROC __glewColorSubTableEXT = nullptr;
PFNGLCOPYCOLORSUBTABLEEXTPROC __glewCopyColorSubTableEXT = nullptr;

PFNGLLOCKARRAYSEXTPROC __glewLockArraysEXT = nullptr;
PFNGLUNLOCKARRAYSEXTPROC __glewUnlockArraysEXT = nullptr;

PFNGLCONVOLUTIONFILTER1DEXTPROC __glewConvolutionFilter1DEXT = nullptr;
PFNGLCONVOLUTIONFILTER2DEXTPROC __glewConvolutionFilter2DEXT = nullptr;
PFNGLCONVOLUTIONPARAMETERFEXTPROC __glewConvolutionParameterfEXT = nullptr;
PFNGLCONVOLUTIONPARAMETERFVEXTPROC __glewConvolutionParameterfvEXT = nullptr;
PFNGLCONVOLUTIONPARAMETERIEXTPROC __glewConvolutionParameteriEXT = nullptr;
PFNGLCONVOLUTIONPARAMETERIVEXTPROC __glewConvolutionParameterivEXT = nullptr;
PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC __glewCopyConvolutionFilter1DEXT = nullptr;
PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC __glewCopyConvolutionFilter2DEXT = nullptr;
PFNGLGETCONVOLUTIONFILTEREXTPROC __glewGetConvolutionFilterEXT = nullptr;
PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC __glewGetConvolutionParameterfvEXT = nullptr;
PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC __glewGetConvolutionParameterivEXT = nullptr;
PFNGLGETSEPARABLEFILTEREXTPROC __glewGetSeparableFilterEXT = nullptr;
PFNGLSEPARABLEFILTER2DEXTPROC __glewSeparableFilter2DEXT = nullptr;

PFNGLBINORMALPOINTEREXTPROC __glewBinormalPointerEXT = nullptr;
PFNGLTANGENTPOINTEREXTPROC __glewTangentPointerEXT = nullptr;

PFNGLCOPYTEXIMAGE1DEXTPROC __glewCopyTexImage1DEXT = nullptr;
PFNGLCOPYTEXIMAGE2DEXTPROC __glewCopyTexImage2DEXT = nullptr;
PFNGLCOPYTEXSUBIMAGE1DEXTPROC __glewCopyTexSubImage1DEXT = nullptr;
PFNGLCOPYTEXSUBIMAGE2DEXTPROC __glewCopyTexSubImage2DEXT = nullptr;
PFNGLCOPYTEXSUBIMAGE3DEXTPROC __glewCopyTexSubImage3DEXT = nullptr;

PFNGLCULLPARAMETERDVEXTPROC __glewCullParameterdvEXT = nullptr;
PFNGLCULLPARAMETERFVEXTPROC __glewCullParameterfvEXT = nullptr;

PFNGLDEPTHBOUNDSEXTPROC __glewDepthBoundsEXT = nullptr;

PFNGLBINDMULTITEXTUREEXTPROC __glewBindMultiTextureEXT = nullptr;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC __glewCheckNamedFramebufferStatusEXT = nullptr;
PFNGLCLIENTATTRIBDEFAULTEXTPROC __glewClientAttribDefaultEXT = nullptr;
PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC __glewCompressedMultiTexImage1DEXT = nullptr;
PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC __glewCompressedMultiTexImage2DEXT = nullptr;
PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC __glewCompressedMultiTexImage3DEXT = nullptr;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC __glewCompressedMultiTexSubImage1DEXT = nullptr;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC __glewCompressedMultiTexSubImage2DEXT = nullptr;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC __glewCompressedMultiTexSubImage3DEXT = nullptr;
PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC __glewCompressedTextureImage1DEXT = nullptr;
PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC __glewCompressedTextureImage2DEXT = nullptr;
PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC __glewCompressedTextureImage3DEXT = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC __glewCompressedTextureSubImage1DEXT = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC __glewCompressedTextureSubImage2DEXT = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC __glewCompressedTextureSubImage3DEXT = nullptr;
PFNGLCOPYMULTITEXIMAGE1DEXTPROC __glewCopyMultiTexImage1DEXT = nullptr;
PFNGLCOPYMULTITEXIMAGE2DEXTPROC __glewCopyMultiTexImage2DEXT = nullptr;
PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC __glewCopyMultiTexSubImage1DEXT = nullptr;
PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC __glewCopyMultiTexSubImage2DEXT = nullptr;
PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC __glewCopyMultiTexSubImage3DEXT = nullptr;
PFNGLCOPYTEXTUREIMAGE1DEXTPROC __glewCopyTextureImage1DEXT = nullptr;
PFNGLCOPYTEXTUREIMAGE2DEXTPROC __glewCopyTextureImage2DEXT = nullptr;
PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC __glewCopyTextureSubImage1DEXT = nullptr;
PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC __glewCopyTextureSubImage2DEXT = nullptr;
PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC __glewCopyTextureSubImage3DEXT = nullptr;
PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC __glewDisableClientStateIndexedEXT = nullptr;
PFNGLDISABLECLIENTSTATEIEXTPROC __glewDisableClientStateiEXT = nullptr;
PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC __glewDisableVertexArrayAttribEXT = nullptr;
PFNGLDISABLEVERTEXARRAYEXTPROC __glewDisableVertexArrayEXT = nullptr;
PFNGLENABLECLIENTSTATEINDEXEDEXTPROC __glewEnableClientStateIndexedEXT = nullptr;
PFNGLENABLECLIENTSTATEIEXTPROC __glewEnableClientStateiEXT = nullptr;
PFNGLENABLEVERTEXARRAYATTRIBEXTPROC __glewEnableVertexArrayAttribEXT = nullptr;
PFNGLENABLEVERTEXARRAYEXTPROC __glewEnableVertexArrayEXT = nullptr;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC __glewFlushMappedNamedBufferRangeEXT = nullptr;
PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC __glewFramebufferDrawBufferEXT = nullptr;
PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC __glewFramebufferDrawBuffersEXT = nullptr;
PFNGLFRAMEBUFFERREADBUFFEREXTPROC __glewFramebufferReadBufferEXT = nullptr;
PFNGLGENERATEMULTITEXMIPMAPEXTPROC __glewGenerateMultiTexMipmapEXT = nullptr;
PFNGLGENERATETEXTUREMIPMAPEXTPROC __glewGenerateTextureMipmapEXT = nullptr;
PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC __glewGetCompressedMultiTexImageEXT = nullptr;
PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC __glewGetCompressedTextureImageEXT = nullptr;
PFNGLGETDOUBLEINDEXEDVEXTPROC __glewGetDoubleIndexedvEXT = nullptr;
PFNGLGETDOUBLEI_VEXTPROC __glewGetDoublei_vEXT = nullptr;
PFNGLGETFLOATINDEXEDVEXTPROC __glewGetFloatIndexedvEXT = nullptr;
PFNGLGETFLOATI_VEXTPROC __glewGetFloati_vEXT = nullptr;
PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC __glewGetFramebufferParameterivEXT = nullptr;
PFNGLGETMULTITEXENVFVEXTPROC __glewGetMultiTexEnvfvEXT = nullptr;
PFNGLGETMULTITEXENVIVEXTPROC __glewGetMultiTexEnvivEXT = nullptr;
PFNGLGETMULTITEXGENDVEXTPROC __glewGetMultiTexGendvEXT = nullptr;
PFNGLGETMULTITEXGENFVEXTPROC __glewGetMultiTexGenfvEXT = nullptr;
PFNGLGETMULTITEXGENIVEXTPROC __glewGetMultiTexGenivEXT = nullptr;
PFNGLGETMULTITEXIMAGEEXTPROC __glewGetMultiTexImageEXT = nullptr;
PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC __glewGetMultiTexLevelParameterfvEXT = nullptr;
PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC __glewGetMultiTexLevelParameterivEXT = nullptr;
PFNGLGETMULTITEXPARAMETERIIVEXTPROC __glewGetMultiTexParameterIivEXT = nullptr;
PFNGLGETMULTITEXPARAMETERIUIVEXTPROC __glewGetMultiTexParameterIuivEXT = nullptr;
PFNGLGETMULTITEXPARAMETERFVEXTPROC __glewGetMultiTexParameterfvEXT = nullptr;
PFNGLGETMULTITEXPARAMETERIVEXTPROC __glewGetMultiTexParameterivEXT = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC __glewGetNamedBufferParameterivEXT = nullptr;
PFNGLGETNAMEDBUFFERPOINTERVEXTPROC __glewGetNamedBufferPointervEXT = nullptr;
PFNGLGETNAMEDBUFFERSUBDATAEXTPROC __glewGetNamedBufferSubDataEXT = nullptr;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetNamedFramebufferAttachmentParameterivEXT = nullptr;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC __glewGetNamedProgramLocalParameterIivEXT = nullptr;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC __glewGetNamedProgramLocalParameterIuivEXT = nullptr;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC __glewGetNamedProgramLocalParameterdvEXT = nullptr;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC __glewGetNamedProgramLocalParameterfvEXT = nullptr;
PFNGLGETNAMEDPROGRAMSTRINGEXTPROC __glewGetNamedProgramStringEXT = nullptr;
PFNGLGETNAMEDPROGRAMIVEXTPROC __glewGetNamedProgramivEXT = nullptr;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC __glewGetNamedRenderbufferParameterivEXT = nullptr;
PFNGLGETPOINTERINDEXEDVEXTPROC __glewGetPointerIndexedvEXT = nullptr;
PFNGLGETPOINTERI_VEXTPROC __glewGetPointeri_vEXT = nullptr;
PFNGLGETTEXTUREIMAGEEXTPROC __glewGetTextureImageEXT = nullptr;
PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC __glewGetTextureLevelParameterfvEXT = nullptr;
PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC __glewGetTextureLevelParameterivEXT = nullptr;
PFNGLGETTEXTUREPARAMETERIIVEXTPROC __glewGetTextureParameterIivEXT = nullptr;
PFNGLGETTEXTUREPARAMETERIUIVEXTPROC __glewGetTextureParameterIuivEXT = nullptr;
PFNGLGETTEXTUREPARAMETERFVEXTPROC __glewGetTextureParameterfvEXT = nullptr;
PFNGLGETTEXTUREPARAMETERIVEXTPROC __glewGetTextureParameterivEXT = nullptr;
PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC __glewGetVertexArrayIntegeri_vEXT = nullptr;
PFNGLGETVERTEXARRAYINTEGERVEXTPROC __glewGetVertexArrayIntegervEXT = nullptr;
PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC __glewGetVertexArrayPointeri_vEXT = nullptr;
PFNGLGETVERTEXARRAYPOINTERVEXTPROC __glewGetVertexArrayPointervEXT = nullptr;
PFNGLMAPNAMEDBUFFEREXTPROC __glewMapNamedBufferEXT = nullptr;
PFNGLMAPNAMEDBUFFERRANGEEXTPROC __glewMapNamedBufferRangeEXT = nullptr;
PFNGLMATRIXFRUSTUMEXTPROC __glewMatrixFrustumEXT = nullptr;
PFNGLMATRIXLOADIDENTITYEXTPROC __glewMatrixLoadIdentityEXT = nullptr;
PFNGLMATRIXLOADTRANSPOSEDEXTPROC __glewMatrixLoadTransposedEXT = nullptr;
PFNGLMATRIXLOADTRANSPOSEFEXTPROC __glewMatrixLoadTransposefEXT = nullptr;
PFNGLMATRIXLOADDEXTPROC __glewMatrixLoaddEXT = nullptr;
PFNGLMATRIXLOADFEXTPROC __glewMatrixLoadfEXT = nullptr;
PFNGLMATRIXMULTTRANSPOSEDEXTPROC __glewMatrixMultTransposedEXT = nullptr;
PFNGLMATRIXMULTTRANSPOSEFEXTPROC __glewMatrixMultTransposefEXT = nullptr;
PFNGLMATRIXMULTDEXTPROC __glewMatrixMultdEXT = nullptr;
PFNGLMATRIXMULTFEXTPROC __glewMatrixMultfEXT = nullptr;
PFNGLMATRIXORTHOEXTPROC __glewMatrixOrthoEXT = nullptr;
PFNGLMATRIXPOPEXTPROC __glewMatrixPopEXT = nullptr;
PFNGLMATRIXPUSHEXTPROC __glewMatrixPushEXT = nullptr;
PFNGLMATRIXROTATEDEXTPROC __glewMatrixRotatedEXT = nullptr;
PFNGLMATRIXROTATEFEXTPROC __glewMatrixRotatefEXT = nullptr;
PFNGLMATRIXSCALEDEXTPROC __glewMatrixScaledEXT = nullptr;
PFNGLMATRIXSCALEFEXTPROC __glewMatrixScalefEXT = nullptr;
PFNGLMATRIXTRANSLATEDEXTPROC __glewMatrixTranslatedEXT = nullptr;
PFNGLMATRIXTRANSLATEFEXTPROC __glewMatrixTranslatefEXT = nullptr;
PFNGLMULTITEXBUFFEREXTPROC __glewMultiTexBufferEXT = nullptr;
PFNGLMULTITEXCOORDPOINTEREXTPROC __glewMultiTexCoordPointerEXT = nullptr;
PFNGLMULTITEXENVFEXTPROC __glewMultiTexEnvfEXT = nullptr;
PFNGLMULTITEXENVFVEXTPROC __glewMultiTexEnvfvEXT = nullptr;
PFNGLMULTITEXENVIEXTPROC __glewMultiTexEnviEXT = nullptr;
PFNGLMULTITEXENVIVEXTPROC __glewMultiTexEnvivEXT = nullptr;
PFNGLMULTITEXGENDEXTPROC __glewMultiTexGendEXT = nullptr;
PFNGLMULTITEXGENDVEXTPROC __glewMultiTexGendvEXT = nullptr;
PFNGLMULTITEXGENFEXTPROC __glewMultiTexGenfEXT = nullptr;
PFNGLMULTITEXGENFVEXTPROC __glewMultiTexGenfvEXT = nullptr;
PFNGLMULTITEXGENIEXTPROC __glewMultiTexGeniEXT = nullptr;
PFNGLMULTITEXGENIVEXTPROC __glewMultiTexGenivEXT = nullptr;
PFNGLMULTITEXIMAGE1DEXTPROC __glewMultiTexImage1DEXT = nullptr;
PFNGLMULTITEXIMAGE2DEXTPROC __glewMultiTexImage2DEXT = nullptr;
PFNGLMULTITEXIMAGE3DEXTPROC __glewMultiTexImage3DEXT = nullptr;
PFNGLMULTITEXPARAMETERIIVEXTPROC __glewMultiTexParameterIivEXT = nullptr;
PFNGLMULTITEXPARAMETERIUIVEXTPROC __glewMultiTexParameterIuivEXT = nullptr;
PFNGLMULTITEXPARAMETERFEXTPROC __glewMultiTexParameterfEXT = nullptr;
PFNGLMULTITEXPARAMETERFVEXTPROC __glewMultiTexParameterfvEXT = nullptr;
PFNGLMULTITEXPARAMETERIEXTPROC __glewMultiTexParameteriEXT = nullptr;
PFNGLMULTITEXPARAMETERIVEXTPROC __glewMultiTexParameterivEXT = nullptr;
PFNGLMULTITEXRENDERBUFFEREXTPROC __glewMultiTexRenderbufferEXT = nullptr;
PFNGLMULTITEXSUBIMAGE1DEXTPROC __glewMultiTexSubImage1DEXT = nullptr;
PFNGLMULTITEXSUBIMAGE2DEXTPROC __glewMultiTexSubImage2DEXT = nullptr;
PFNGLMULTITEXSUBIMAGE3DEXTPROC __glewMultiTexSubImage3DEXT = nullptr;
PFNGLNAMEDBUFFERDATAEXTPROC __glewNamedBufferDataEXT = nullptr;
PFNGLNAMEDBUFFERSUBDATAEXTPROC __glewNamedBufferSubDataEXT = nullptr;
PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC __glewNamedCopyBufferSubDataEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC __glewNamedFramebufferRenderbufferEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC __glewNamedFramebufferTexture1DEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC __glewNamedFramebufferTexture2DEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC __glewNamedFramebufferTexture3DEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC __glewNamedFramebufferTextureEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC __glewNamedFramebufferTextureFaceEXT = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC __glewNamedFramebufferTextureLayerEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC __glewNamedProgramLocalParameter4dEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC __glewNamedProgramLocalParameter4dvEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC __glewNamedProgramLocalParameter4fEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC __glewNamedProgramLocalParameter4fvEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC __glewNamedProgramLocalParameterI4iEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC __glewNamedProgramLocalParameterI4ivEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC __glewNamedProgramLocalParameterI4uiEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC __glewNamedProgramLocalParameterI4uivEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC __glewNamedProgramLocalParameters4fvEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC __glewNamedProgramLocalParametersI4ivEXT = nullptr;
PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC __glewNamedProgramLocalParametersI4uivEXT = nullptr;
PFNGLNAMEDPROGRAMSTRINGEXTPROC __glewNamedProgramStringEXT = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC __glewNamedRenderbufferStorageEXT = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC __glewNamedRenderbufferStorageMultisampleCoverageEXT = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewNamedRenderbufferStorageMultisampleEXT = nullptr;
PFNGLPROGRAMUNIFORM1FEXTPROC __glewProgramUniform1fEXT = nullptr;
PFNGLPROGRAMUNIFORM1FVEXTPROC __glewProgramUniform1fvEXT = nullptr;
PFNGLPROGRAMUNIFORM1IEXTPROC __glewProgramUniform1iEXT = nullptr;
PFNGLPROGRAMUNIFORM1IVEXTPROC __glewProgramUniform1ivEXT = nullptr;
PFNGLPROGRAMUNIFORM1UIEXTPROC __glewProgramUniform1uiEXT = nullptr;
PFNGLPROGRAMUNIFORM1UIVEXTPROC __glewProgramUniform1uivEXT = nullptr;
PFNGLPROGRAMUNIFORM2FEXTPROC __glewProgramUniform2fEXT = nullptr;
PFNGLPROGRAMUNIFORM2FVEXTPROC __glewProgramUniform2fvEXT = nullptr;
PFNGLPROGRAMUNIFORM2IEXTPROC __glewProgramUniform2iEXT = nullptr;
PFNGLPROGRAMUNIFORM2IVEXTPROC __glewProgramUniform2ivEXT = nullptr;
PFNGLPROGRAMUNIFORM2UIEXTPROC __glewProgramUniform2uiEXT = nullptr;
PFNGLPROGRAMUNIFORM2UIVEXTPROC __glewProgramUniform2uivEXT = nullptr;
PFNGLPROGRAMUNIFORM3FEXTPROC __glewProgramUniform3fEXT = nullptr;
PFNGLPROGRAMUNIFORM3FVEXTPROC __glewProgramUniform3fvEXT = nullptr;
PFNGLPROGRAMUNIFORM3IEXTPROC __glewProgramUniform3iEXT = nullptr;
PFNGLPROGRAMUNIFORM3IVEXTPROC __glewProgramUniform3ivEXT = nullptr;
PFNGLPROGRAMUNIFORM3UIEXTPROC __glewProgramUniform3uiEXT = nullptr;
PFNGLPROGRAMUNIFORM3UIVEXTPROC __glewProgramUniform3uivEXT = nullptr;
PFNGLPROGRAMUNIFORM4FEXTPROC __glewProgramUniform4fEXT = nullptr;
PFNGLPROGRAMUNIFORM4FVEXTPROC __glewProgramUniform4fvEXT = nullptr;
PFNGLPROGRAMUNIFORM4IEXTPROC __glewProgramUniform4iEXT = nullptr;
PFNGLPROGRAMUNIFORM4IVEXTPROC __glewProgramUniform4ivEXT = nullptr;
PFNGLPROGRAMUNIFORM4UIEXTPROC __glewProgramUniform4uiEXT = nullptr;
PFNGLPROGRAMUNIFORM4UIVEXTPROC __glewProgramUniform4uivEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC __glewProgramUniformMatrix2fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC __glewProgramUniformMatrix2x3fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC __glewProgramUniformMatrix2x4fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC __glewProgramUniformMatrix3fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC __glewProgramUniformMatrix3x2fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC __glewProgramUniformMatrix3x4fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC __glewProgramUniformMatrix4fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC __glewProgramUniformMatrix4x2fvEXT = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC __glewProgramUniformMatrix4x3fvEXT = nullptr;
PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC __glewPushClientAttribDefaultEXT = nullptr;
PFNGLTEXTUREBUFFEREXTPROC __glewTextureBufferEXT = nullptr;
PFNGLTEXTUREIMAGE1DEXTPROC __glewTextureImage1DEXT = nullptr;
PFNGLTEXTUREIMAGE2DEXTPROC __glewTextureImage2DEXT = nullptr;
PFNGLTEXTUREIMAGE3DEXTPROC __glewTextureImage3DEXT = nullptr;
PFNGLTEXTUREPARAMETERIIVEXTPROC __glewTextureParameterIivEXT = nullptr;
PFNGLTEXTUREPARAMETERIUIVEXTPROC __glewTextureParameterIuivEXT = nullptr;
PFNGLTEXTUREPARAMETERFEXTPROC __glewTextureParameterfEXT = nullptr;
PFNGLTEXTUREPARAMETERFVEXTPROC __glewTextureParameterfvEXT = nullptr;
PFNGLTEXTUREPARAMETERIEXTPROC __glewTextureParameteriEXT = nullptr;
PFNGLTEXTUREPARAMETERIVEXTPROC __glewTextureParameterivEXT = nullptr;
PFNGLTEXTURERENDERBUFFEREXTPROC __glewTextureRenderbufferEXT = nullptr;
PFNGLTEXTURESUBIMAGE1DEXTPROC __glewTextureSubImage1DEXT = nullptr;
PFNGLTEXTURESUBIMAGE2DEXTPROC __glewTextureSubImage2DEXT = nullptr;
PFNGLTEXTURESUBIMAGE3DEXTPROC __glewTextureSubImage3DEXT = nullptr;
PFNGLUNMAPNAMEDBUFFEREXTPROC __glewUnmapNamedBufferEXT = nullptr;
PFNGLVERTEXARRAYCOLOROFFSETEXTPROC __glewVertexArrayColorOffsetEXT = nullptr;
PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC __glewVertexArrayEdgeFlagOffsetEXT = nullptr;
PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC __glewVertexArrayFogCoordOffsetEXT = nullptr;
PFNGLVERTEXARRAYINDEXOFFSETEXTPROC __glewVertexArrayIndexOffsetEXT = nullptr;
PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC __glewVertexArrayMultiTexCoordOffsetEXT = nullptr;
PFNGLVERTEXARRAYNORMALOFFSETEXTPROC __glewVertexArrayNormalOffsetEXT = nullptr;
PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC __glewVertexArraySecondaryColorOffsetEXT = nullptr;
PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC __glewVertexArrayTexCoordOffsetEXT = nullptr;
PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC __glewVertexArrayVertexAttribIOffsetEXT = nullptr;
PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC __glewVertexArrayVertexAttribOffsetEXT = nullptr;
PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC __glewVertexArrayVertexOffsetEXT = nullptr;

PFNGLCOLORMASKINDEXEDEXTPROC __glewColorMaskIndexedEXT = nullptr;
PFNGLDISABLEINDEXEDEXTPROC __glewDisableIndexedEXT = nullptr;
PFNGLENABLEINDEXEDEXTPROC __glewEnableIndexedEXT = nullptr;
PFNGLGETBOOLEANINDEXEDVEXTPROC __glewGetBooleanIndexedvEXT = nullptr;
PFNGLGETINTEGERINDEXEDVEXTPROC __glewGetIntegerIndexedvEXT = nullptr;
PFNGLISENABLEDINDEXEDEXTPROC __glewIsEnabledIndexedEXT = nullptr;

PFNGLDRAWARRAYSINSTANCEDEXTPROC __glewDrawArraysInstancedEXT = nullptr;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC __glewDrawElementsInstancedEXT = nullptr;

PFNGLDRAWRANGEELEMENTSEXTPROC __glewDrawRangeElementsEXT = nullptr;

PFNGLFOGCOORDPOINTEREXTPROC __glewFogCoordPointerEXT = nullptr;
PFNGLFOGCOORDDEXTPROC __glewFogCoorddEXT = nullptr;
PFNGLFOGCOORDDVEXTPROC __glewFogCoorddvEXT = nullptr;
PFNGLFOGCOORDFEXTPROC __glewFogCoordfEXT = nullptr;
PFNGLFOGCOORDFVEXTPROC __glewFogCoordfvEXT = nullptr;

PFNGLFRAGMENTCOLORMATERIALEXTPROC __glewFragmentColorMaterialEXT = nullptr;
PFNGLFRAGMENTLIGHTMODELFEXTPROC __glewFragmentLightModelfEXT = nullptr;
PFNGLFRAGMENTLIGHTMODELFVEXTPROC __glewFragmentLightModelfvEXT = nullptr;
PFNGLFRAGMENTLIGHTMODELIEXTPROC __glewFragmentLightModeliEXT = nullptr;
PFNGLFRAGMENTLIGHTMODELIVEXTPROC __glewFragmentLightModelivEXT = nullptr;
PFNGLFRAGMENTLIGHTFEXTPROC __glewFragmentLightfEXT = nullptr;
PFNGLFRAGMENTLIGHTFVEXTPROC __glewFragmentLightfvEXT = nullptr;
PFNGLFRAGMENTLIGHTIEXTPROC __glewFragmentLightiEXT = nullptr;
PFNGLFRAGMENTLIGHTIVEXTPROC __glewFragmentLightivEXT = nullptr;
PFNGLFRAGMENTMATERIALFEXTPROC __glewFragmentMaterialfEXT = nullptr;
PFNGLFRAGMENTMATERIALFVEXTPROC __glewFragmentMaterialfvEXT = nullptr;
PFNGLFRAGMENTMATERIALIEXTPROC __glewFragmentMaterialiEXT = nullptr;
PFNGLFRAGMENTMATERIALIVEXTPROC __glewFragmentMaterialivEXT = nullptr;
PFNGLGETFRAGMENTLIGHTFVEXTPROC __glewGetFragmentLightfvEXT = nullptr;
PFNGLGETFRAGMENTLIGHTIVEXTPROC __glewGetFragmentLightivEXT = nullptr;
PFNGLGETFRAGMENTMATERIALFVEXTPROC __glewGetFragmentMaterialfvEXT = nullptr;
PFNGLGETFRAGMENTMATERIALIVEXTPROC __glewGetFragmentMaterialivEXT = nullptr;
PFNGLLIGHTENVIEXTPROC __glewLightEnviEXT = nullptr;

PFNGLBLITFRAMEBUFFEREXTPROC __glewBlitFramebufferEXT = nullptr;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewRenderbufferStorageMultisampleEXT = nullptr;

PFNGLBINDFRAMEBUFFEREXTPROC __glewBindFramebufferEXT = nullptr;
PFNGLBINDRENDERBUFFEREXTPROC __glewBindRenderbufferEXT = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC __glewCheckFramebufferStatusEXT = nullptr;
PFNGLDELETEFRAMEBUFFERSEXTPROC __glewDeleteFramebuffersEXT = nullptr;
PFNGLDELETERENDERBUFFERSEXTPROC __glewDeleteRenderbuffersEXT = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC __glewFramebufferRenderbufferEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC __glewFramebufferTexture1DEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC __glewFramebufferTexture2DEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC __glewFramebufferTexture3DEXT = nullptr;
PFNGLGENFRAMEBUFFERSEXTPROC __glewGenFramebuffersEXT = nullptr;
PFNGLGENRENDERBUFFERSEXTPROC __glewGenRenderbuffersEXT = nullptr;
PFNGLGENERATEMIPMAPEXTPROC __glewGenerateMipmapEXT = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetFramebufferAttachmentParameterivEXT = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC __glewGetRenderbufferParameterivEXT = nullptr;
PFNGLISFRAMEBUFFEREXTPROC __glewIsFramebufferEXT = nullptr;
PFNGLISRENDERBUFFEREXTPROC __glewIsRenderbufferEXT = nullptr;
PFNGLRENDERBUFFERSTORAGEEXTPROC __glewRenderbufferStorageEXT = nullptr;

PFNGLFRAMEBUFFERTEXTUREEXTPROC __glewFramebufferTextureEXT = nullptr;
PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC __glewFramebufferTextureFaceEXT = nullptr;
PFNGLPROGRAMPARAMETERIEXTPROC __glewProgramParameteriEXT = nullptr;

PFNGLPROGRAMENVPARAMETERS4FVEXTPROC __glewProgramEnvParameters4fvEXT = nullptr;
PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC __glewProgramLocalParameters4fvEXT = nullptr;

PFNGLBINDFRAGDATALOCATIONEXTPROC __glewBindFragDataLocationEXT = nullptr;
PFNGLGETFRAGDATALOCATIONEXTPROC __glewGetFragDataLocationEXT = nullptr;
PFNGLGETUNIFORMUIVEXTPROC __glewGetUniformuivEXT = nullptr;
PFNGLGETVERTEXATTRIBIIVEXTPROC __glewGetVertexAttribIivEXT = nullptr;
PFNGLGETVERTEXATTRIBIUIVEXTPROC __glewGetVertexAttribIuivEXT = nullptr;
PFNGLUNIFORM1UIEXTPROC __glewUniform1uiEXT = nullptr;
PFNGLUNIFORM1UIVEXTPROC __glewUniform1uivEXT = nullptr;
PFNGLUNIFORM2UIEXTPROC __glewUniform2uiEXT = nullptr;
PFNGLUNIFORM2UIVEXTPROC __glewUniform2uivEXT = nullptr;
PFNGLUNIFORM3UIEXTPROC __glewUniform3uiEXT = nullptr;
PFNGLUNIFORM3UIVEXTPROC __glewUniform3uivEXT = nullptr;
PFNGLUNIFORM4UIEXTPROC __glewUniform4uiEXT = nullptr;
PFNGLUNIFORM4UIVEXTPROC __glewUniform4uivEXT = nullptr;
PFNGLVERTEXATTRIBI1IEXTPROC __glewVertexAttribI1iEXT = nullptr;
PFNGLVERTEXATTRIBI1IVEXTPROC __glewVertexAttribI1ivEXT = nullptr;
PFNGLVERTEXATTRIBI1UIEXTPROC __glewVertexAttribI1uiEXT = nullptr;
PFNGLVERTEXATTRIBI1UIVEXTPROC __glewVertexAttribI1uivEXT = nullptr;
PFNGLVERTEXATTRIBI2IEXTPROC __glewVertexAttribI2iEXT = nullptr;
PFNGLVERTEXATTRIBI2IVEXTPROC __glewVertexAttribI2ivEXT = nullptr;
PFNGLVERTEXATTRIBI2UIEXTPROC __glewVertexAttribI2uiEXT = nullptr;
PFNGLVERTEXATTRIBI2UIVEXTPROC __glewVertexAttribI2uivEXT = nullptr;
PFNGLVERTEXATTRIBI3IEXTPROC __glewVertexAttribI3iEXT = nullptr;
PFNGLVERTEXATTRIBI3IVEXTPROC __glewVertexAttribI3ivEXT = nullptr;
PFNGLVERTEXATTRIBI3UIEXTPROC __glewVertexAttribI3uiEXT = nullptr;
PFNGLVERTEXATTRIBI3UIVEXTPROC __glewVertexAttribI3uivEXT = nullptr;
PFNGLVERTEXATTRIBI4BVEXTPROC __glewVertexAttribI4bvEXT = nullptr;
PFNGLVERTEXATTRIBI4IEXTPROC __glewVertexAttribI4iEXT = nullptr;
PFNGLVERTEXATTRIBI4IVEXTPROC __glewVertexAttribI4ivEXT = nullptr;
PFNGLVERTEXATTRIBI4SVEXTPROC __glewVertexAttribI4svEXT = nullptr;
PFNGLVERTEXATTRIBI4UBVEXTPROC __glewVertexAttribI4ubvEXT = nullptr;
PFNGLVERTEXATTRIBI4UIEXTPROC __glewVertexAttribI4uiEXT = nullptr;
PFNGLVERTEXATTRIBI4UIVEXTPROC __glewVertexAttribI4uivEXT = nullptr;
PFNGLVERTEXATTRIBI4USVEXTPROC __glewVertexAttribI4usvEXT = nullptr;
PFNGLVERTEXATTRIBIPOINTEREXTPROC __glewVertexAttribIPointerEXT = nullptr;

PFNGLGETHISTOGRAMEXTPROC __glewGetHistogramEXT = nullptr;
PFNGLGETHISTOGRAMPARAMETERFVEXTPROC __glewGetHistogramParameterfvEXT = nullptr;
PFNGLGETHISTOGRAMPARAMETERIVEXTPROC __glewGetHistogramParameterivEXT = nullptr;
PFNGLGETMINMAXEXTPROC __glewGetMinmaxEXT = nullptr;
PFNGLGETMINMAXPARAMETERFVEXTPROC __glewGetMinmaxParameterfvEXT = nullptr;
PFNGLGETMINMAXPARAMETERIVEXTPROC __glewGetMinmaxParameterivEXT = nullptr;
PFNGLHISTOGRAMEXTPROC __glewHistogramEXT = nullptr;
PFNGLMINMAXEXTPROC __glewMinmaxEXT = nullptr;
PFNGLRESETHISTOGRAMEXTPROC __glewResetHistogramEXT = nullptr;
PFNGLRESETMINMAXEXTPROC __glewResetMinmaxEXT = nullptr;

PFNGLINDEXFUNCEXTPROC __glewIndexFuncEXT = nullptr;

PFNGLINDEXMATERIALEXTPROC __glewIndexMaterialEXT = nullptr;

PFNGLAPPLYTEXTUREEXTPROC __glewApplyTextureEXT = nullptr;
PFNGLTEXTURELIGHTEXTPROC __glewTextureLightEXT = nullptr;
PFNGLTEXTUREMATERIALEXTPROC __glewTextureMaterialEXT = nullptr;

PFNGLMULTIDRAWARRAYSEXTPROC __glewMultiDrawArraysEXT = nullptr;
PFNGLMULTIDRAWELEMENTSEXTPROC __glewMultiDrawElementsEXT = nullptr;

PFNGLSAMPLEMASKEXTPROC __glewSampleMaskEXT = nullptr;
PFNGLSAMPLEPATTERNEXTPROC __glewSamplePatternEXT = nullptr;

PFNGLCOLORTABLEEXTPROC __glewColorTableEXT = nullptr;
PFNGLGETCOLORTABLEEXTPROC __glewGetColorTableEXT = nullptr;
PFNGLGETCOLORTABLEPARAMETERFVEXTPROC __glewGetColorTableParameterfvEXT = nullptr;
PFNGLGETCOLORTABLEPARAMETERIVEXTPROC __glewGetColorTableParameterivEXT = nullptr;

PFNGLGETPIXELTRANSFORMPARAMETERFVEXTPROC __glewGetPixelTransformParameterfvEXT = nullptr;
PFNGLGETPIXELTRANSFORMPARAMETERIVEXTPROC __glewGetPixelTransformParameterivEXT = nullptr;
PFNGLPIXELTRANSFORMPARAMETERFEXTPROC __glewPixelTransformParameterfEXT = nullptr;
PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC __glewPixelTransformParameterfvEXT = nullptr;
PFNGLPIXELTRANSFORMPARAMETERIEXTPROC __glewPixelTransformParameteriEXT = nullptr;
PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC __glewPixelTransformParameterivEXT = nullptr;

PFNGLPOINTPARAMETERFEXTPROC __glewPointParameterfEXT = nullptr;
PFNGLPOINTPARAMETERFVEXTPROC __glewPointParameterfvEXT = nullptr;

PFNGLPOLYGONOFFSETEXTPROC __glewPolygonOffsetEXT = nullptr;

PFNGLPROVOKINGVERTEXEXTPROC __glewProvokingVertexEXT = nullptr;

PFNGLBEGINSCENEEXTPROC __glewBeginSceneEXT = nullptr;
PFNGLENDSCENEEXTPROC __glewEndSceneEXT = nullptr;

PFNGLSECONDARYCOLOR3BEXTPROC __glewSecondaryColor3bEXT = nullptr;
PFNGLSECONDARYCOLOR3BVEXTPROC __glewSecondaryColor3bvEXT = nullptr;
PFNGLSECONDARYCOLOR3DEXTPROC __glewSecondaryColor3dEXT = nullptr;
PFNGLSECONDARYCOLOR3DVEXTPROC __glewSecondaryColor3dvEXT = nullptr;
PFNGLSECONDARYCOLOR3FEXTPROC __glewSecondaryColor3fEXT = nullptr;
PFNGLSECONDARYCOLOR3FVEXTPROC __glewSecondaryColor3fvEXT = nullptr;
PFNGLSECONDARYCOLOR3IEXTPROC __glewSecondaryColor3iEXT = nullptr;
PFNGLSECONDARYCOLOR3IVEXTPROC __glewSecondaryColor3ivEXT = nullptr;
PFNGLSECONDARYCOLOR3SEXTPROC __glewSecondaryColor3sEXT = nullptr;
PFNGLSECONDARYCOLOR3SVEXTPROC __glewSecondaryColor3svEXT = nullptr;
PFNGLSECONDARYCOLOR3UBEXTPROC __glewSecondaryColor3ubEXT = nullptr;
PFNGLSECONDARYCOLOR3UBVEXTPROC __glewSecondaryColor3ubvEXT = nullptr;
PFNGLSECONDARYCOLOR3UIEXTPROC __glewSecondaryColor3uiEXT = nullptr;
PFNGLSECONDARYCOLOR3UIVEXTPROC __glewSecondaryColor3uivEXT = nullptr;
PFNGLSECONDARYCOLOR3USEXTPROC __glewSecondaryColor3usEXT = nullptr;
PFNGLSECONDARYCOLOR3USVEXTPROC __glewSecondaryColor3usvEXT = nullptr;
PFNGLSECONDARYCOLORPOINTEREXTPROC __glewSecondaryColorPointerEXT = nullptr;

PFNGLACTIVEPROGRAMEXTPROC __glewActiveProgramEXT = nullptr;
PFNGLCREATESHADERPROGRAMEXTPROC __glewCreateShaderProgramEXT = nullptr;
PFNGLUSESHADERPROGRAMEXTPROC __glewUseShaderProgramEXT = nullptr;

PFNGLBINDIMAGETEXTUREEXTPROC __glewBindImageTextureEXT = nullptr;
PFNGLMEMORYBARRIEREXTPROC __glewMemoryBarrierEXT = nullptr;

PFNGLACTIVESTENCILFACEEXTPROC __glewActiveStencilFaceEXT = nullptr;

PFNGLTEXSUBIMAGE1DEXTPROC __glewTexSubImage1DEXT = nullptr;
PFNGLTEXSUBIMAGE2DEXTPROC __glewTexSubImage2DEXT = nullptr;
PFNGLTEXSUBIMAGE3DEXTPROC __glewTexSubImage3DEXT = nullptr;

PFNGLTEXIMAGE3DEXTPROC __glewTexImage3DEXT = nullptr;

PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC __glewFramebufferTextureLayerEXT = nullptr;

PFNGLTEXBUFFEREXTPROC __glewTexBufferEXT = nullptr;

PFNGLCLEARCOLORIIEXTPROC __glewClearColorIiEXT = nullptr;
PFNGLCLEARCOLORIUIEXTPROC __glewClearColorIuiEXT = nullptr;
PFNGLGETTEXPARAMETERIIVEXTPROC __glewGetTexParameterIivEXT = nullptr;
PFNGLGETTEXPARAMETERIUIVEXTPROC __glewGetTexParameterIuivEXT = nullptr;
PFNGLTEXPARAMETERIIVEXTPROC __glewTexParameterIivEXT = nullptr;
PFNGLTEXPARAMETERIUIVEXTPROC __glewTexParameterIuivEXT = nullptr;

PFNGLARETEXTURESRESIDENTEXTPROC __glewAreTexturesResidentEXT = nullptr;
PFNGLBINDTEXTUREEXTPROC __glewBindTextureEXT = nullptr;
PFNGLDELETETEXTURESEXTPROC __glewDeleteTexturesEXT = nullptr;
PFNGLGENTEXTURESEXTPROC __glewGenTexturesEXT = nullptr;
PFNGLISTEXTUREEXTPROC __glewIsTextureEXT = nullptr;
PFNGLPRIORITIZETEXTURESEXTPROC __glewPrioritizeTexturesEXT = nullptr;

PFNGLTEXTURENORMALEXTPROC __glewTextureNormalEXT = nullptr;

PFNGLGETQUERYOBJECTI64VEXTPROC __glewGetQueryObjecti64vEXT = nullptr;
PFNGLGETQUERYOBJECTUI64VEXTPROC __glewGetQueryObjectui64vEXT = nullptr;

PFNGLBEGINTRANSFORMFEEDBACKEXTPROC __glewBeginTransformFeedbackEXT = nullptr;
PFNGLBINDBUFFERBASEEXTPROC __glewBindBufferBaseEXT = nullptr;
PFNGLBINDBUFFEROFFSETEXTPROC __glewBindBufferOffsetEXT = nullptr;
PFNGLBINDBUFFERRANGEEXTPROC __glewBindBufferRangeEXT = nullptr;
PFNGLENDTRANSFORMFEEDBACKEXTPROC __glewEndTransformFeedbackEXT = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC __glewGetTransformFeedbackVaryingEXT = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC __glewTransformFeedbackVaryingsEXT = nullptr;

PFNGLARRAYELEMENTEXTPROC __glewArrayElementEXT = nullptr;
PFNGLCOLORPOINTEREXTPROC __glewColorPointerEXT = nullptr;
PFNGLDRAWARRAYSEXTPROC __glewDrawArraysEXT = nullptr;
PFNGLEDGEFLAGPOINTEREXTPROC __glewEdgeFlagPointerEXT = nullptr;
PFNGLINDEXPOINTEREXTPROC __glewIndexPointerEXT = nullptr;
PFNGLNORMALPOINTEREXTPROC __glewNormalPointerEXT = nullptr;
PFNGLTEXCOORDPOINTEREXTPROC __glewTexCoordPointerEXT = nullptr;
PFNGLVERTEXPOINTEREXTPROC __glewVertexPointerEXT = nullptr;

PFNGLGETVERTEXATTRIBLDVEXTPROC __glewGetVertexAttribLdvEXT = nullptr;
PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC __glewVertexArrayVertexAttribLOffsetEXT = nullptr;
PFNGLVERTEXATTRIBL1DEXTPROC __glewVertexAttribL1dEXT = nullptr;
PFNGLVERTEXATTRIBL1DVEXTPROC __glewVertexAttribL1dvEXT = nullptr;
PFNGLVERTEXATTRIBL2DEXTPROC __glewVertexAttribL2dEXT = nullptr;
PFNGLVERTEXATTRIBL2DVEXTPROC __glewVertexAttribL2dvEXT = nullptr;
PFNGLVERTEXATTRIBL3DEXTPROC __glewVertexAttribL3dEXT = nullptr;
PFNGLVERTEXATTRIBL3DVEXTPROC __glewVertexAttribL3dvEXT = nullptr;
PFNGLVERTEXATTRIBL4DEXTPROC __glewVertexAttribL4dEXT = nullptr;
PFNGLVERTEXATTRIBL4DVEXTPROC __glewVertexAttribL4dvEXT = nullptr;
PFNGLVERTEXATTRIBLPOINTEREXTPROC __glewVertexAttribLPointerEXT = nullptr;

PFNGLBEGINVERTEXSHADEREXTPROC __glewBeginVertexShaderEXT = nullptr;
PFNGLBINDLIGHTPARAMETEREXTPROC __glewBindLightParameterEXT = nullptr;
PFNGLBINDMATERIALPARAMETEREXTPROC __glewBindMaterialParameterEXT = nullptr;
PFNGLBINDPARAMETEREXTPROC __glewBindParameterEXT = nullptr;
PFNGLBINDTEXGENPARAMETEREXTPROC __glewBindTexGenParameterEXT = nullptr;
PFNGLBINDTEXTUREUNITPARAMETEREXTPROC __glewBindTextureUnitParameterEXT = nullptr;
PFNGLBINDVERTEXSHADEREXTPROC __glewBindVertexShaderEXT = nullptr;
PFNGLDELETEVERTEXSHADEREXTPROC __glewDeleteVertexShaderEXT = nullptr;
PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC __glewDisableVariantClientStateEXT = nullptr;
PFNGLENABLEVARIANTCLIENTSTATEEXTPROC __glewEnableVariantClientStateEXT = nullptr;
PFNGLENDVERTEXSHADEREXTPROC __glewEndVertexShaderEXT = nullptr;
PFNGLEXTRACTCOMPONENTEXTPROC __glewExtractComponentEXT = nullptr;
PFNGLGENSYMBOLSEXTPROC __glewGenSymbolsEXT = nullptr;
PFNGLGENVERTEXSHADERSEXTPROC __glewGenVertexShadersEXT = nullptr;
PFNGLGETINVARIANTBOOLEANVEXTPROC __glewGetInvariantBooleanvEXT = nullptr;
PFNGLGETINVARIANTFLOATVEXTPROC __glewGetInvariantFloatvEXT = nullptr;
PFNGLGETINVARIANTINTEGERVEXTPROC __glewGetInvariantIntegervEXT = nullptr;
PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC __glewGetLocalConstantBooleanvEXT = nullptr;
PFNGLGETLOCALCONSTANTFLOATVEXTPROC __glewGetLocalConstantFloatvEXT = nullptr;
PFNGLGETLOCALCONSTANTINTEGERVEXTPROC __glewGetLocalConstantIntegervEXT = nullptr;
PFNGLGETVARIANTBOOLEANVEXTPROC __glewGetVariantBooleanvEXT = nullptr;
PFNGLGETVARIANTFLOATVEXTPROC __glewGetVariantFloatvEXT = nullptr;
PFNGLGETVARIANTINTEGERVEXTPROC __glewGetVariantIntegervEXT = nullptr;
PFNGLGETVARIANTPOINTERVEXTPROC __glewGetVariantPointervEXT = nullptr;
PFNGLINSERTCOMPONENTEXTPROC __glewInsertComponentEXT = nullptr;
PFNGLISVARIANTENABLEDEXTPROC __glewIsVariantEnabledEXT = nullptr;
PFNGLSETINVARIANTEXTPROC __glewSetInvariantEXT = nullptr;
PFNGLSETLOCALCONSTANTEXTPROC __glewSetLocalConstantEXT = nullptr;
PFNGLSHADEROP1EXTPROC __glewShaderOp1EXT = nullptr;
PFNGLSHADEROP2EXTPROC __glewShaderOp2EXT = nullptr;
PFNGLSHADEROP3EXTPROC __glewShaderOp3EXT = nullptr;
PFNGLSWIZZLEEXTPROC __glewSwizzleEXT = nullptr;
PFNGLVARIANTPOINTEREXTPROC __glewVariantPointerEXT = nullptr;
PFNGLVARIANTBVEXTPROC __glewVariantbvEXT = nullptr;
PFNGLVARIANTDVEXTPROC __glewVariantdvEXT = nullptr;
PFNGLVARIANTFVEXTPROC __glewVariantfvEXT = nullptr;
PFNGLVARIANTIVEXTPROC __glewVariantivEXT = nullptr;
PFNGLVARIANTSVEXTPROC __glewVariantsvEXT = nullptr;
PFNGLVARIANTUBVEXTPROC __glewVariantubvEXT = nullptr;
PFNGLVARIANTUIVEXTPROC __glewVariantuivEXT = nullptr;
PFNGLVARIANTUSVEXTPROC __glewVariantusvEXT = nullptr;
PFNGLWRITEMASKEXTPROC __glewWriteMaskEXT = nullptr;

PFNGLVERTEXWEIGHTPOINTEREXTPROC __glewVertexWeightPointerEXT = nullptr;
PFNGLVERTEXWEIGHTFEXTPROC __glewVertexWeightfEXT = nullptr;
PFNGLVERTEXWEIGHTFVEXTPROC __glewVertexWeightfvEXT = nullptr;

PFNGLIMPORTSYNCEXTPROC __glewImportSyncEXT = nullptr;

PFNGLFRAMETERMINATORGREMEDYPROC __glewFrameTerminatorGREMEDY = nullptr;

PFNGLSTRINGMARKERGREMEDYPROC __glewStringMarkerGREMEDY = nullptr;

PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC __glewGetImageTransformParameterfvHP = nullptr;
PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC __glewGetImageTransformParameterivHP = nullptr;
PFNGLIMAGETRANSFORMPARAMETERFHPPROC __glewImageTransformParameterfHP = nullptr;
PFNGLIMAGETRANSFORMPARAMETERFVHPPROC __glewImageTransformParameterfvHP = nullptr;
PFNGLIMAGETRANSFORMPARAMETERIHPPROC __glewImageTransformParameteriHP = nullptr;
PFNGLIMAGETRANSFORMPARAMETERIVHPPROC __glewImageTransformParameterivHP = nullptr;

PFNGLMULTIMODEDRAWARRAYSIBMPROC __glewMultiModeDrawArraysIBM = nullptr;
PFNGLMULTIMODEDRAWELEMENTSIBMPROC __glewMultiModeDrawElementsIBM = nullptr;

PFNGLCOLORPOINTERLISTIBMPROC __glewColorPointerListIBM = nullptr;
PFNGLEDGEFLAGPOINTERLISTIBMPROC __glewEdgeFlagPointerListIBM = nullptr;
PFNGLFOGCOORDPOINTERLISTIBMPROC __glewFogCoordPointerListIBM = nullptr;
PFNGLINDEXPOINTERLISTIBMPROC __glewIndexPointerListIBM = nullptr;
PFNGLNORMALPOINTERLISTIBMPROC __glewNormalPointerListIBM = nullptr;
PFNGLSECONDARYCOLORPOINTERLISTIBMPROC __glewSecondaryColorPointerListIBM = nullptr;
PFNGLTEXCOORDPOINTERLISTIBMPROC __glewTexCoordPointerListIBM = nullptr;
PFNGLVERTEXPOINTERLISTIBMPROC __glewVertexPointerListIBM = nullptr;

PFNGLCOLORPOINTERVINTELPROC __glewColorPointervINTEL = nullptr;
PFNGLNORMALPOINTERVINTELPROC __glewNormalPointervINTEL = nullptr;
PFNGLTEXCOORDPOINTERVINTELPROC __glewTexCoordPointervINTEL = nullptr;
PFNGLVERTEXPOINTERVINTELPROC __glewVertexPointervINTEL = nullptr;

PFNGLTEXSCISSORFUNCINTELPROC __glewTexScissorFuncINTEL = nullptr;
PFNGLTEXSCISSORINTELPROC __glewTexScissorINTEL = nullptr;

PFNGLBUFFERREGIONENABLEDPROC __glewBufferRegionEnabled = nullptr;
PFNGLDELETEBUFFERREGIONPROC __glewDeleteBufferRegion = nullptr;
PFNGLDRAWBUFFERREGIONPROC __glewDrawBufferRegion = nullptr;
PFNGLNEWBUFFERREGIONPROC __glewNewBufferRegion = nullptr;
PFNGLREADBUFFERREGIONPROC __glewReadBufferRegion = nullptr;

PFNGLRESIZEBUFFERSMESAPROC __glewResizeBuffersMESA = nullptr;

PFNGLWINDOWPOS2DMESAPROC __glewWindowPos2dMESA = nullptr;
PFNGLWINDOWPOS2DVMESAPROC __glewWindowPos2dvMESA = nullptr;
PFNGLWINDOWPOS2FMESAPROC __glewWindowPos2fMESA = nullptr;
PFNGLWINDOWPOS2FVMESAPROC __glewWindowPos2fvMESA = nullptr;
PFNGLWINDOWPOS2IMESAPROC __glewWindowPos2iMESA = nullptr;
PFNGLWINDOWPOS2IVMESAPROC __glewWindowPos2ivMESA = nullptr;
PFNGLWINDOWPOS2SMESAPROC __glewWindowPos2sMESA = nullptr;
PFNGLWINDOWPOS2SVMESAPROC __glewWindowPos2svMESA = nullptr;
PFNGLWINDOWPOS3DMESAPROC __glewWindowPos3dMESA = nullptr;
PFNGLWINDOWPOS3DVMESAPROC __glewWindowPos3dvMESA = nullptr;
PFNGLWINDOWPOS3FMESAPROC __glewWindowPos3fMESA = nullptr;
PFNGLWINDOWPOS3FVMESAPROC __glewWindowPos3fvMESA = nullptr;
PFNGLWINDOWPOS3IMESAPROC __glewWindowPos3iMESA = nullptr;
PFNGLWINDOWPOS3IVMESAPROC __glewWindowPos3ivMESA = nullptr;
PFNGLWINDOWPOS3SMESAPROC __glewWindowPos3sMESA = nullptr;
PFNGLWINDOWPOS3SVMESAPROC __glewWindowPos3svMESA = nullptr;
PFNGLWINDOWPOS4DMESAPROC __glewWindowPos4dMESA = nullptr;
PFNGLWINDOWPOS4DVMESAPROC __glewWindowPos4dvMESA = nullptr;
PFNGLWINDOWPOS4FMESAPROC __glewWindowPos4fMESA = nullptr;
PFNGLWINDOWPOS4FVMESAPROC __glewWindowPos4fvMESA = nullptr;
PFNGLWINDOWPOS4IMESAPROC __glewWindowPos4iMESA = nullptr;
PFNGLWINDOWPOS4IVMESAPROC __glewWindowPos4ivMESA = nullptr;
PFNGLWINDOWPOS4SMESAPROC __glewWindowPos4sMESA = nullptr;
PFNGLWINDOWPOS4SVMESAPROC __glewWindowPos4svMESA = nullptr;

PFNGLBEGINCONDITIONALRENDERNVPROC __glewBeginConditionalRenderNV = nullptr;
PFNGLENDCONDITIONALRENDERNVPROC __glewEndConditionalRenderNV = nullptr;

PFNGLCOPYIMAGESUBDATANVPROC __glewCopyImageSubDataNV = nullptr;

PFNGLCLEARDEPTHDNVPROC __glewClearDepthdNV = nullptr;
PFNGLDEPTHBOUNDSDNVPROC __glewDepthBoundsdNV = nullptr;
PFNGLDEPTHRANGEDNVPROC __glewDepthRangedNV = nullptr;

PFNGLEVALMAPSNVPROC __glewEvalMapsNV = nullptr;
PFNGLGETMAPATTRIBPARAMETERFVNVPROC __glewGetMapAttribParameterfvNV = nullptr;
PFNGLGETMAPATTRIBPARAMETERIVNVPROC __glewGetMapAttribParameterivNV = nullptr;
PFNGLGETMAPCONTROLPOINTSNVPROC __glewGetMapControlPointsNV = nullptr;
PFNGLGETMAPPARAMETERFVNVPROC __glewGetMapParameterfvNV = nullptr;
PFNGLGETMAPPARAMETERIVNVPROC __glewGetMapParameterivNV = nullptr;
PFNGLMAPCONTROLPOINTSNVPROC __glewMapControlPointsNV = nullptr;
PFNGLMAPPARAMETERFVNVPROC __glewMapParameterfvNV = nullptr;
PFNGLMAPPARAMETERIVNVPROC __glewMapParameterivNV = nullptr;

PFNGLGETMULTISAMPLEFVNVPROC __glewGetMultisamplefvNV = nullptr;
PFNGLSAMPLEMASKINDEXEDNVPROC __glewSampleMaskIndexedNV = nullptr;
PFNGLTEXRENDERBUFFERNVPROC __glewTexRenderbufferNV = nullptr;

PFNGLDELETEFENCESNVPROC __glewDeleteFencesNV = nullptr;
PFNGLFINISHFENCENVPROC __glewFinishFenceNV = nullptr;
PFNGLGENFENCESNVPROC __glewGenFencesNV = nullptr;
PFNGLGETFENCEIVNVPROC __glewGetFenceivNV = nullptr;
PFNGLISFENCENVPROC __glewIsFenceNV = nullptr;
PFNGLSETFENCENVPROC __glewSetFenceNV = nullptr;
PFNGLTESTFENCENVPROC __glewTestFenceNV = nullptr;

PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC __glewGetProgramNamedParameterdvNV = nullptr;
PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC __glewGetProgramNamedParameterfvNV = nullptr;
PFNGLPROGRAMNAMEDPARAMETER4DNVPROC __glewProgramNamedParameter4dNV = nullptr;
PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC __glewProgramNamedParameter4dvNV = nullptr;
PFNGLPROGRAMNAMEDPARAMETER4FNVPROC __glewProgramNamedParameter4fNV = nullptr;
PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC __glewProgramNamedParameter4fvNV = nullptr;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC __glewRenderbufferStorageMultisampleCoverageNV = nullptr;

PFNGLPROGRAMVERTEXLIMITNVPROC __glewProgramVertexLimitNV = nullptr;

PFNGLPROGRAMENVPARAMETERI4INVPROC __glewProgramEnvParameterI4iNV = nullptr;
PFNGLPROGRAMENVPARAMETERI4IVNVPROC __glewProgramEnvParameterI4ivNV = nullptr;
PFNGLPROGRAMENVPARAMETERI4UINVPROC __glewProgramEnvParameterI4uiNV = nullptr;
PFNGLPROGRAMENVPARAMETERI4UIVNVPROC __glewProgramEnvParameterI4uivNV = nullptr;
PFNGLPROGRAMENVPARAMETERSI4IVNVPROC __glewProgramEnvParametersI4ivNV = nullptr;
PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC __glewProgramEnvParametersI4uivNV = nullptr;
PFNGLPROGRAMLOCALPARAMETERI4INVPROC __glewProgramLocalParameterI4iNV = nullptr;
PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC __glewProgramLocalParameterI4ivNV = nullptr;
PFNGLPROGRAMLOCALPARAMETERI4UINVPROC __glewProgramLocalParameterI4uiNV = nullptr;
PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC __glewProgramLocalParameterI4uivNV = nullptr;
PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC __glewProgramLocalParametersI4ivNV = nullptr;
PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC __glewProgramLocalParametersI4uivNV = nullptr;

PFNGLGETUNIFORMI64VNVPROC __glewGetUniformi64vNV = nullptr;
PFNGLGETUNIFORMUI64VNVPROC __glewGetUniformui64vNV = nullptr;
PFNGLPROGRAMUNIFORM1I64NVPROC __glewProgramUniform1i64NV = nullptr;
PFNGLPROGRAMUNIFORM1I64VNVPROC __glewProgramUniform1i64vNV = nullptr;
PFNGLPROGRAMUNIFORM1UI64NVPROC __glewProgramUniform1ui64NV = nullptr;
PFNGLPROGRAMUNIFORM1UI64VNVPROC __glewProgramUniform1ui64vNV = nullptr;
PFNGLPROGRAMUNIFORM2I64NVPROC __glewProgramUniform2i64NV = nullptr;
PFNGLPROGRAMUNIFORM2I64VNVPROC __glewProgramUniform2i64vNV = nullptr;
PFNGLPROGRAMUNIFORM2UI64NVPROC __glewProgramUniform2ui64NV = nullptr;
PFNGLPROGRAMUNIFORM2UI64VNVPROC __glewProgramUniform2ui64vNV = nullptr;
PFNGLPROGRAMUNIFORM3I64NVPROC __glewProgramUniform3i64NV = nullptr;
PFNGLPROGRAMUNIFORM3I64VNVPROC __glewProgramUniform3i64vNV = nullptr;
PFNGLPROGRAMUNIFORM3UI64NVPROC __glewProgramUniform3ui64NV = nullptr;
PFNGLPROGRAMUNIFORM3UI64VNVPROC __glewProgramUniform3ui64vNV = nullptr;
PFNGLPROGRAMUNIFORM4I64NVPROC __glewProgramUniform4i64NV = nullptr;
PFNGLPROGRAMUNIFORM4I64VNVPROC __glewProgramUniform4i64vNV = nullptr;
PFNGLPROGRAMUNIFORM4UI64NVPROC __glewProgramUniform4ui64NV = nullptr;
PFNGLPROGRAMUNIFORM4UI64VNVPROC __glewProgramUniform4ui64vNV = nullptr;
PFNGLUNIFORM1I64NVPROC __glewUniform1i64NV = nullptr;
PFNGLUNIFORM1I64VNVPROC __glewUniform1i64vNV = nullptr;
PFNGLUNIFORM1UI64NVPROC __glewUniform1ui64NV = nullptr;
PFNGLUNIFORM1UI64VNVPROC __glewUniform1ui64vNV = nullptr;
PFNGLUNIFORM2I64NVPROC __glewUniform2i64NV = nullptr;
PFNGLUNIFORM2I64VNVPROC __glewUniform2i64vNV = nullptr;
PFNGLUNIFORM2UI64NVPROC __glewUniform2ui64NV = nullptr;
PFNGLUNIFORM2UI64VNVPROC __glewUniform2ui64vNV = nullptr;
PFNGLUNIFORM3I64NVPROC __glewUniform3i64NV = nullptr;
PFNGLUNIFORM3I64VNVPROC __glewUniform3i64vNV = nullptr;
PFNGLUNIFORM3UI64NVPROC __glewUniform3ui64NV = nullptr;
PFNGLUNIFORM3UI64VNVPROC __glewUniform3ui64vNV = nullptr;
PFNGLUNIFORM4I64NVPROC __glewUniform4i64NV = nullptr;
PFNGLUNIFORM4I64VNVPROC __glewUniform4i64vNV = nullptr;
PFNGLUNIFORM4UI64NVPROC __glewUniform4ui64NV = nullptr;
PFNGLUNIFORM4UI64VNVPROC __glewUniform4ui64vNV = nullptr;

PFNGLCOLOR3HNVPROC __glewColor3hNV = nullptr;
PFNGLCOLOR3HVNVPROC __glewColor3hvNV = nullptr;
PFNGLCOLOR4HNVPROC __glewColor4hNV = nullptr;
PFNGLCOLOR4HVNVPROC __glewColor4hvNV = nullptr;
PFNGLFOGCOORDHNVPROC __glewFogCoordhNV = nullptr;
PFNGLFOGCOORDHVNVPROC __glewFogCoordhvNV = nullptr;
PFNGLMULTITEXCOORD1HNVPROC __glewMultiTexCoord1hNV = nullptr;
PFNGLMULTITEXCOORD1HVNVPROC __glewMultiTexCoord1hvNV = nullptr;
PFNGLMULTITEXCOORD2HNVPROC __glewMultiTexCoord2hNV = nullptr;
PFNGLMULTITEXCOORD2HVNVPROC __glewMultiTexCoord2hvNV = nullptr;
PFNGLMULTITEXCOORD3HNVPROC __glewMultiTexCoord3hNV = nullptr;
PFNGLMULTITEXCOORD3HVNVPROC __glewMultiTexCoord3hvNV = nullptr;
PFNGLMULTITEXCOORD4HNVPROC __glewMultiTexCoord4hNV = nullptr;
PFNGLMULTITEXCOORD4HVNVPROC __glewMultiTexCoord4hvNV = nullptr;
PFNGLNORMAL3HNVPROC __glewNormal3hNV = nullptr;
PFNGLNORMAL3HVNVPROC __glewNormal3hvNV = nullptr;
PFNGLSECONDARYCOLOR3HNVPROC __glewSecondaryColor3hNV = nullptr;
PFNGLSECONDARYCOLOR3HVNVPROC __glewSecondaryColor3hvNV = nullptr;
PFNGLTEXCOORD1HNVPROC __glewTexCoord1hNV = nullptr;
PFNGLTEXCOORD1HVNVPROC __glewTexCoord1hvNV = nullptr;
PFNGLTEXCOORD2HNVPROC __glewTexCoord2hNV = nullptr;
PFNGLTEXCOORD2HVNVPROC __glewTexCoord2hvNV = nullptr;
PFNGLTEXCOORD3HNVPROC __glewTexCoord3hNV = nullptr;
PFNGLTEXCOORD3HVNVPROC __glewTexCoord3hvNV = nullptr;
PFNGLTEXCOORD4HNVPROC __glewTexCoord4hNV = nullptr;
PFNGLTEXCOORD4HVNVPROC __glewTexCoord4hvNV = nullptr;
PFNGLVERTEX2HNVPROC __glewVertex2hNV = nullptr;
PFNGLVERTEX2HVNVPROC __glewVertex2hvNV = nullptr;
PFNGLVERTEX3HNVPROC __glewVertex3hNV = nullptr;
PFNGLVERTEX3HVNVPROC __glewVertex3hvNV = nullptr;
PFNGLVERTEX4HNVPROC __glewVertex4hNV = nullptr;
PFNGLVERTEX4HVNVPROC __glewVertex4hvNV = nullptr;
PFNGLVERTEXATTRIB1HNVPROC __glewVertexAttrib1hNV = nullptr;
PFNGLVERTEXATTRIB1HVNVPROC __glewVertexAttrib1hvNV = nullptr;
PFNGLVERTEXATTRIB2HNVPROC __glewVertexAttrib2hNV = nullptr;
PFNGLVERTEXATTRIB2HVNVPROC __glewVertexAttrib2hvNV = nullptr;
PFNGLVERTEXATTRIB3HNVPROC __glewVertexAttrib3hNV = nullptr;
PFNGLVERTEXATTRIB3HVNVPROC __glewVertexAttrib3hvNV = nullptr;
PFNGLVERTEXATTRIB4HNVPROC __glewVertexAttrib4hNV = nullptr;
PFNGLVERTEXATTRIB4HVNVPROC __glewVertexAttrib4hvNV = nullptr;
PFNGLVERTEXATTRIBS1HVNVPROC __glewVertexAttribs1hvNV = nullptr;
PFNGLVERTEXATTRIBS2HVNVPROC __glewVertexAttribs2hvNV = nullptr;
PFNGLVERTEXATTRIBS3HVNVPROC __glewVertexAttribs3hvNV = nullptr;
PFNGLVERTEXATTRIBS4HVNVPROC __glewVertexAttribs4hvNV = nullptr;
PFNGLVERTEXWEIGHTHNVPROC __glewVertexWeighthNV = nullptr;
PFNGLVERTEXWEIGHTHVNVPROC __glewVertexWeighthvNV = nullptr;

PFNGLBEGINOCCLUSIONQUERYNVPROC __glewBeginOcclusionQueryNV = nullptr;
PFNGLDELETEOCCLUSIONQUERIESNVPROC __glewDeleteOcclusionQueriesNV = nullptr;
PFNGLENDOCCLUSIONQUERYNVPROC __glewEndOcclusionQueryNV = nullptr;
PFNGLGENOCCLUSIONQUERIESNVPROC __glewGenOcclusionQueriesNV = nullptr;
PFNGLGETOCCLUSIONQUERYIVNVPROC __glewGetOcclusionQueryivNV = nullptr;
PFNGLGETOCCLUSIONQUERYUIVNVPROC __glewGetOcclusionQueryuivNV = nullptr;
PFNGLISOCCLUSIONQUERYNVPROC __glewIsOcclusionQueryNV = nullptr;

PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC __glewProgramBufferParametersIivNV = nullptr;
PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC __glewProgramBufferParametersIuivNV = nullptr;
PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC __glewProgramBufferParametersfvNV = nullptr;

PFNGLCOPYPATHNVPROC __glewCopyPathNV = nullptr;
PFNGLCOVERFILLPATHINSTANCEDNVPROC __glewCoverFillPathInstancedNV = nullptr;
PFNGLCOVERFILLPATHNVPROC __glewCoverFillPathNV = nullptr;
PFNGLCOVERSTROKEPATHINSTANCEDNVPROC __glewCoverStrokePathInstancedNV = nullptr;
PFNGLCOVERSTROKEPATHNVPROC __glewCoverStrokePathNV = nullptr;
PFNGLDELETEPATHSNVPROC __glewDeletePathsNV = nullptr;
PFNGLGENPATHSNVPROC __glewGenPathsNV = nullptr;
PFNGLGETPATHCOLORGENFVNVPROC __glewGetPathColorGenfvNV = nullptr;
PFNGLGETPATHCOLORGENIVNVPROC __glewGetPathColorGenivNV = nullptr;
PFNGLGETPATHCOMMANDSNVPROC __glewGetPathCommandsNV = nullptr;
PFNGLGETPATHCOORDSNVPROC __glewGetPathCoordsNV = nullptr;
PFNGLGETPATHDASHARRAYNVPROC __glewGetPathDashArrayNV = nullptr;
PFNGLGETPATHLENGTHNVPROC __glewGetPathLengthNV = nullptr;
PFNGLGETPATHMETRICRANGENVPROC __glewGetPathMetricRangeNV = nullptr;
PFNGLGETPATHMETRICSNVPROC __glewGetPathMetricsNV = nullptr;
PFNGLGETPATHPARAMETERFVNVPROC __glewGetPathParameterfvNV = nullptr;
PFNGLGETPATHPARAMETERIVNVPROC __glewGetPathParameterivNV = nullptr;
PFNGLGETPATHSPACINGNVPROC __glewGetPathSpacingNV = nullptr;
PFNGLGETPATHTEXGENFVNVPROC __glewGetPathTexGenfvNV = nullptr;
PFNGLGETPATHTEXGENIVNVPROC __glewGetPathTexGenivNV = nullptr;
PFNGLINTERPOLATEPATHSNVPROC __glewInterpolatePathsNV = nullptr;
PFNGLISPATHNVPROC __glewIsPathNV = nullptr;
PFNGLISPOINTINFILLPATHNVPROC __glewIsPointInFillPathNV = nullptr;
PFNGLISPOINTINSTROKEPATHNVPROC __glewIsPointInStrokePathNV = nullptr;
PFNGLPATHCOLORGENNVPROC __glewPathColorGenNV = nullptr;
PFNGLPATHCOMMANDSNVPROC __glewPathCommandsNV = nullptr;
PFNGLPATHCOORDSNVPROC __glewPathCoordsNV = nullptr;
PFNGLPATHCOVERDEPTHFUNCNVPROC __glewPathCoverDepthFuncNV = nullptr;
PFNGLPATHDASHARRAYNVPROC __glewPathDashArrayNV = nullptr;
PFNGLPATHFOGGENNVPROC __glewPathFogGenNV = nullptr;
PFNGLPATHGLYPHRANGENVPROC __glewPathGlyphRangeNV = nullptr;
PFNGLPATHGLYPHSNVPROC __glewPathGlyphsNV = nullptr;
PFNGLPATHPARAMETERFNVPROC __glewPathParameterfNV = nullptr;
PFNGLPATHPARAMETERFVNVPROC __glewPathParameterfvNV = nullptr;
PFNGLPATHPARAMETERINVPROC __glewPathParameteriNV = nullptr;
PFNGLPATHPARAMETERIVNVPROC __glewPathParameterivNV = nullptr;
PFNGLPATHSTENCILDEPTHOFFSETNVPROC __glewPathStencilDepthOffsetNV = nullptr;
PFNGLPATHSTENCILFUNCNVPROC __glewPathStencilFuncNV = nullptr;
PFNGLPATHSTRINGNVPROC __glewPathStringNV = nullptr;
PFNGLPATHSUBCOMMANDSNVPROC __glewPathSubCommandsNV = nullptr;
PFNGLPATHSUBCOORDSNVPROC __glewPathSubCoordsNV = nullptr;
PFNGLPATHTEXGENNVPROC __glewPathTexGenNV = nullptr;
PFNGLPOINTALONGPATHNVPROC __glewPointAlongPathNV = nullptr;
PFNGLSTENCILFILLPATHINSTANCEDNVPROC __glewStencilFillPathInstancedNV = nullptr;
PFNGLSTENCILFILLPATHNVPROC __glewStencilFillPathNV = nullptr;
PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC __glewStencilStrokePathInstancedNV = nullptr;
PFNGLSTENCILSTROKEPATHNVPROC __glewStencilStrokePathNV = nullptr;
PFNGLTRANSFORMPATHNVPROC __glewTransformPathNV = nullptr;
PFNGLWEIGHTPATHSNVPROC __glewWeightPathsNV = nullptr;

PFNGLFLUSHPIXELDATARANGENVPROC __glewFlushPixelDataRangeNV = nullptr;
PFNGLPIXELDATARANGENVPROC __glewPixelDataRangeNV = nullptr;

PFNGLPOINTPARAMETERINVPROC __glewPointParameteriNV = nullptr;
PFNGLPOINTPARAMETERIVNVPROC __glewPointParameterivNV = nullptr;

PFNGLGETVIDEOI64VNVPROC __glewGetVideoi64vNV = nullptr;
PFNGLGETVIDEOIVNVPROC __glewGetVideoivNV = nullptr;
PFNGLGETVIDEOUI64VNVPROC __glewGetVideoui64vNV = nullptr;
PFNGLGETVIDEOUIVNVPROC __glewGetVideouivNV = nullptr;
PFNGLPRESENTFRAMEDUALFILLNVPROC __glewPresentFrameDualFillNV = nullptr;
PFNGLPRESENTFRAMEKEYEDNVPROC __glewPresentFrameKeyedNV = nullptr;

PFNGLPRIMITIVERESTARTINDEXNVPROC __glewPrimitiveRestartIndexNV = nullptr;
PFNGLPRIMITIVERESTARTNVPROC __glewPrimitiveRestartNV = nullptr;

PFNGLCOMBINERINPUTNVPROC __glewCombinerInputNV = nullptr;
PFNGLCOMBINEROUTPUTNVPROC __glewCombinerOutputNV = nullptr;
PFNGLCOMBINERPARAMETERFNVPROC __glewCombinerParameterfNV = nullptr;
PFNGLCOMBINERPARAMETERFVNVPROC __glewCombinerParameterfvNV = nullptr;
PFNGLCOMBINERPARAMETERINVPROC __glewCombinerParameteriNV = nullptr;
PFNGLCOMBINERPARAMETERIVNVPROC __glewCombinerParameterivNV = nullptr;
PFNGLFINALCOMBINERINPUTNVPROC __glewFinalCombinerInputNV = nullptr;
PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC __glewGetCombinerInputParameterfvNV = nullptr;
PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC __glewGetCombinerInputParameterivNV = nullptr;
PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC __glewGetCombinerOutputParameterfvNV = nullptr;
PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC __glewGetCombinerOutputParameterivNV = nullptr;
PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC __glewGetFinalCombinerInputParameterfvNV = nullptr;
PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC __glewGetFinalCombinerInputParameterivNV = nullptr;

PFNGLCOMBINERSTAGEPARAMETERFVNVPROC __glewCombinerStageParameterfvNV = nullptr;
PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC __glewGetCombinerStageParameterfvNV = nullptr;

PFNGLGETBUFFERPARAMETERUI64VNVPROC __glewGetBufferParameterui64vNV = nullptr;
PFNGLGETINTEGERUI64VNVPROC __glewGetIntegerui64vNV = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC __glewGetNamedBufferParameterui64vNV = nullptr;
PFNGLISBUFFERRESIDENTNVPROC __glewIsBufferResidentNV = nullptr;
PFNGLISNAMEDBUFFERRESIDENTNVPROC __glewIsNamedBufferResidentNV = nullptr;
PFNGLMAKEBUFFERNONRESIDENTNVPROC __glewMakeBufferNonResidentNV = nullptr;
PFNGLMAKEBUFFERRESIDENTNVPROC __glewMakeBufferResidentNV = nullptr;
PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC __glewMakeNamedBufferNonResidentNV = nullptr;
PFNGLMAKENAMEDBUFFERRESIDENTNVPROC __glewMakeNamedBufferResidentNV = nullptr;
PFNGLPROGRAMUNIFORMUI64NVPROC __glewProgramUniformui64NV = nullptr;
PFNGLPROGRAMUNIFORMUI64VNVPROC __glewProgramUniformui64vNV = nullptr;
PFNGLUNIFORMUI64NVPROC __glewUniformui64NV = nullptr;
PFNGLUNIFORMUI64VNVPROC __glewUniformui64vNV = nullptr;

PFNGLTEXTUREBARRIERNVPROC __glewTextureBarrierNV = nullptr;

PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC __glewTexImage2DMultisampleCoverageNV = nullptr;
PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC __glewTexImage3DMultisampleCoverageNV = nullptr;
PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC __glewTextureImage2DMultisampleCoverageNV = nullptr;
PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC __glewTextureImage2DMultisampleNV = nullptr;
PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC __glewTextureImage3DMultisampleCoverageNV = nullptr;
PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC __glewTextureImage3DMultisampleNV = nullptr;

PFNGLACTIVEVARYINGNVPROC __glewActiveVaryingNV = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKNVPROC __glewBeginTransformFeedbackNV = nullptr;
PFNGLBINDBUFFERBASENVPROC __glewBindBufferBaseNV = nullptr;
PFNGLBINDBUFFEROFFSETNVPROC __glewBindBufferOffsetNV = nullptr;
PFNGLBINDBUFFERRANGENVPROC __glewBindBufferRangeNV = nullptr;
PFNGLENDTRANSFORMFEEDBACKNVPROC __glewEndTransformFeedbackNV = nullptr;
PFNGLGETACTIVEVARYINGNVPROC __glewGetActiveVaryingNV = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC __glewGetTransformFeedbackVaryingNV = nullptr;
PFNGLGETVARYINGLOCATIONNVPROC __glewGetVaryingLocationNV = nullptr;
PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC __glewTransformFeedbackAttribsNV = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC __glewTransformFeedbackVaryingsNV = nullptr;

PFNGLBINDTRANSFORMFEEDBACKNVPROC __glewBindTransformFeedbackNV = nullptr;
PFNGLDELETETRANSFORMFEEDBACKSNVPROC __glewDeleteTransformFeedbacksNV = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKNVPROC __glewDrawTransformFeedbackNV = nullptr;
PFNGLGENTRANSFORMFEEDBACKSNVPROC __glewGenTransformFeedbacksNV = nullptr;
PFNGLISTRANSFORMFEEDBACKNVPROC __glewIsTransformFeedbackNV = nullptr;
PFNGLPAUSETRANSFORMFEEDBACKNVPROC __glewPauseTransformFeedbackNV = nullptr;
PFNGLRESUMETRANSFORMFEEDBACKNVPROC __glewResumeTransformFeedbackNV = nullptr;

PFNGLVDPAUFININVPROC __glewVDPAUFiniNV = nullptr;
PFNGLVDPAUGETSURFACEIVNVPROC __glewVDPAUGetSurfaceivNV = nullptr;
PFNGLVDPAUINITNVPROC __glewVDPAUInitNV = nullptr;
PFNGLVDPAUISSURFACENVPROC __glewVDPAUIsSurfaceNV = nullptr;
PFNGLVDPAUMAPSURFACESNVPROC __glewVDPAUMapSurfacesNV = nullptr;
PFNGLVDPAUREGISTEROUTPUTSURFACENVPROC __glewVDPAURegisterOutputSurfaceNV = nullptr;
PFNGLVDPAUREGISTERVIDEOSURFACENVPROC __glewVDPAURegisterVideoSurfaceNV = nullptr;
PFNGLVDPAUSURFACEACCESSNVPROC __glewVDPAUSurfaceAccessNV = nullptr;
PFNGLVDPAUUNMAPSURFACESNVPROC __glewVDPAUUnmapSurfacesNV = nullptr;
PFNGLVDPAUUNREGISTERSURFACENVPROC __glewVDPAUUnregisterSurfaceNV = nullptr;

PFNGLFLUSHVERTEXARRAYRANGENVPROC __glewFlushVertexArrayRangeNV = nullptr;
PFNGLVERTEXARRAYRANGENVPROC __glewVertexArrayRangeNV = nullptr;

PFNGLGETVERTEXATTRIBLI64VNVPROC __glewGetVertexAttribLi64vNV = nullptr;
PFNGLGETVERTEXATTRIBLUI64VNVPROC __glewGetVertexAttribLui64vNV = nullptr;
PFNGLVERTEXATTRIBL1I64NVPROC __glewVertexAttribL1i64NV = nullptr;
PFNGLVERTEXATTRIBL1I64VNVPROC __glewVertexAttribL1i64vNV = nullptr;
PFNGLVERTEXATTRIBL1UI64NVPROC __glewVertexAttribL1ui64NV = nullptr;
PFNGLVERTEXATTRIBL1UI64VNVPROC __glewVertexAttribL1ui64vNV = nullptr;
PFNGLVERTEXATTRIBL2I64NVPROC __glewVertexAttribL2i64NV = nullptr;
PFNGLVERTEXATTRIBL2I64VNVPROC __glewVertexAttribL2i64vNV = nullptr;
PFNGLVERTEXATTRIBL2UI64NVPROC __glewVertexAttribL2ui64NV = nullptr;
PFNGLVERTEXATTRIBL2UI64VNVPROC __glewVertexAttribL2ui64vNV = nullptr;
PFNGLVERTEXATTRIBL3I64NVPROC __glewVertexAttribL3i64NV = nullptr;
PFNGLVERTEXATTRIBL3I64VNVPROC __glewVertexAttribL3i64vNV = nullptr;
PFNGLVERTEXATTRIBL3UI64NVPROC __glewVertexAttribL3ui64NV = nullptr;
PFNGLVERTEXATTRIBL3UI64VNVPROC __glewVertexAttribL3ui64vNV = nullptr;
PFNGLVERTEXATTRIBL4I64NVPROC __glewVertexAttribL4i64NV = nullptr;
PFNGLVERTEXATTRIBL4I64VNVPROC __glewVertexAttribL4i64vNV = nullptr;
PFNGLVERTEXATTRIBL4UI64NVPROC __glewVertexAttribL4ui64NV = nullptr;
PFNGLVERTEXATTRIBL4UI64VNVPROC __glewVertexAttribL4ui64vNV = nullptr;
PFNGLVERTEXATTRIBLFORMATNVPROC __glewVertexAttribLFormatNV = nullptr;

PFNGLBUFFERADDRESSRANGENVPROC __glewBufferAddressRangeNV = nullptr;
PFNGLCOLORFORMATNVPROC __glewColorFormatNV = nullptr;
PFNGLEDGEFLAGFORMATNVPROC __glewEdgeFlagFormatNV = nullptr;
PFNGLFOGCOORDFORMATNVPROC __glewFogCoordFormatNV = nullptr;
PFNGLGETINTEGERUI64I_VNVPROC __glewGetIntegerui64i_vNV = nullptr;
PFNGLINDEXFORMATNVPROC __glewIndexFormatNV = nullptr;
PFNGLNORMALFORMATNVPROC __glewNormalFormatNV = nullptr;
PFNGLSECONDARYCOLORFORMATNVPROC __glewSecondaryColorFormatNV = nullptr;
PFNGLTEXCOORDFORMATNVPROC __glewTexCoordFormatNV = nullptr;
PFNGLVERTEXATTRIBFORMATNVPROC __glewVertexAttribFormatNV = nullptr;
PFNGLVERTEXATTRIBIFORMATNVPROC __glewVertexAttribIFormatNV = nullptr;
PFNGLVERTEXFORMATNVPROC __glewVertexFormatNV = nullptr;

PFNGLAREPROGRAMSRESIDENTNVPROC __glewAreProgramsResidentNV = nullptr;
PFNGLBINDPROGRAMNVPROC __glewBindProgramNV = nullptr;
PFNGLDELETEPROGRAMSNVPROC __glewDeleteProgramsNV = nullptr;
PFNGLEXECUTEPROGRAMNVPROC __glewExecuteProgramNV = nullptr;
PFNGLGENPROGRAMSNVPROC __glewGenProgramsNV = nullptr;
PFNGLGETPROGRAMPARAMETERDVNVPROC __glewGetProgramParameterdvNV = nullptr;
PFNGLGETPROGRAMPARAMETERFVNVPROC __glewGetProgramParameterfvNV = nullptr;
PFNGLGETPROGRAMSTRINGNVPROC __glewGetProgramStringNV = nullptr;
PFNGLGETPROGRAMIVNVPROC __glewGetProgramivNV = nullptr;
PFNGLGETTRACKMATRIXIVNVPROC __glewGetTrackMatrixivNV = nullptr;
PFNGLGETVERTEXATTRIBPOINTERVNVPROC __glewGetVertexAttribPointervNV = nullptr;
PFNGLGETVERTEXATTRIBDVNVPROC __glewGetVertexAttribdvNV = nullptr;
PFNGLGETVERTEXATTRIBFVNVPROC __glewGetVertexAttribfvNV = nullptr;
PFNGLGETVERTEXATTRIBIVNVPROC __glewGetVertexAttribivNV = nullptr;
PFNGLISPROGRAMNVPROC __glewIsProgramNV = nullptr;
PFNGLLOADPROGRAMNVPROC __glewLoadProgramNV = nullptr;
PFNGLPROGRAMPARAMETER4DNVPROC __glewProgramParameter4dNV = nullptr;
PFNGLPROGRAMPARAMETER4DVNVPROC __glewProgramParameter4dvNV = nullptr;
PFNGLPROGRAMPARAMETER4FNVPROC __glewProgramParameter4fNV = nullptr;
PFNGLPROGRAMPARAMETER4FVNVPROC __glewProgramParameter4fvNV = nullptr;
PFNGLPROGRAMPARAMETERS4DVNVPROC __glewProgramParameters4dvNV = nullptr;
PFNGLPROGRAMPARAMETERS4FVNVPROC __glewProgramParameters4fvNV = nullptr;
PFNGLREQUESTRESIDENTPROGRAMSNVPROC __glewRequestResidentProgramsNV = nullptr;
PFNGLTRACKMATRIXNVPROC __glewTrackMatrixNV = nullptr;
PFNGLVERTEXATTRIB1DNVPROC __glewVertexAttrib1dNV = nullptr;
PFNGLVERTEXATTRIB1DVNVPROC __glewVertexAttrib1dvNV = nullptr;
PFNGLVERTEXATTRIB1FNVPROC __glewVertexAttrib1fNV = nullptr;
PFNGLVERTEXATTRIB1FVNVPROC __glewVertexAttrib1fvNV = nullptr;
PFNGLVERTEXATTRIB1SNVPROC __glewVertexAttrib1sNV = nullptr;
PFNGLVERTEXATTRIB1SVNVPROC __glewVertexAttrib1svNV = nullptr;
PFNGLVERTEXATTRIB2DNVPROC __glewVertexAttrib2dNV = nullptr;
PFNGLVERTEXATTRIB2DVNVPROC __glewVertexAttrib2dvNV = nullptr;
PFNGLVERTEXATTRIB2FNVPROC __glewVertexAttrib2fNV = nullptr;
PFNGLVERTEXATTRIB2FVNVPROC __glewVertexAttrib2fvNV = nullptr;
PFNGLVERTEXATTRIB2SNVPROC __glewVertexAttrib2sNV = nullptr;
PFNGLVERTEXATTRIB2SVNVPROC __glewVertexAttrib2svNV = nullptr;
PFNGLVERTEXATTRIB3DNVPROC __glewVertexAttrib3dNV = nullptr;
PFNGLVERTEXATTRIB3DVNVPROC __glewVertexAttrib3dvNV = nullptr;
PFNGLVERTEXATTRIB3FNVPROC __glewVertexAttrib3fNV = nullptr;
PFNGLVERTEXATTRIB3FVNVPROC __glewVertexAttrib3fvNV = nullptr;
PFNGLVERTEXATTRIB3SNVPROC __glewVertexAttrib3sNV = nullptr;
PFNGLVERTEXATTRIB3SVNVPROC __glewVertexAttrib3svNV = nullptr;
PFNGLVERTEXATTRIB4DNVPROC __glewVertexAttrib4dNV = nullptr;
PFNGLVERTEXATTRIB4DVNVPROC __glewVertexAttrib4dvNV = nullptr;
PFNGLVERTEXATTRIB4FNVPROC __glewVertexAttrib4fNV = nullptr;
PFNGLVERTEXATTRIB4FVNVPROC __glewVertexAttrib4fvNV = nullptr;
PFNGLVERTEXATTRIB4SNVPROC __glewVertexAttrib4sNV = nullptr;
PFNGLVERTEXATTRIB4SVNVPROC __glewVertexAttrib4svNV = nullptr;
PFNGLVERTEXATTRIB4UBNVPROC __glewVertexAttrib4ubNV = nullptr;
PFNGLVERTEXATTRIB4UBVNVPROC __glewVertexAttrib4ubvNV = nullptr;
PFNGLVERTEXATTRIBPOINTERNVPROC __glewVertexAttribPointerNV = nullptr;
PFNGLVERTEXATTRIBS1DVNVPROC __glewVertexAttribs1dvNV = nullptr;
PFNGLVERTEXATTRIBS1FVNVPROC __glewVertexAttribs1fvNV = nullptr;
PFNGLVERTEXATTRIBS1SVNVPROC __glewVertexAttribs1svNV = nullptr;
PFNGLVERTEXATTRIBS2DVNVPROC __glewVertexAttribs2dvNV = nullptr;
PFNGLVERTEXATTRIBS2FVNVPROC __glewVertexAttribs2fvNV = nullptr;
PFNGLVERTEXATTRIBS2SVNVPROC __glewVertexAttribs2svNV = nullptr;
PFNGLVERTEXATTRIBS3DVNVPROC __glewVertexAttribs3dvNV = nullptr;
PFNGLVERTEXATTRIBS3FVNVPROC __glewVertexAttribs3fvNV = nullptr;
PFNGLVERTEXATTRIBS3SVNVPROC __glewVertexAttribs3svNV = nullptr;
PFNGLVERTEXATTRIBS4DVNVPROC __glewVertexAttribs4dvNV = nullptr;
PFNGLVERTEXATTRIBS4FVNVPROC __glewVertexAttribs4fvNV = nullptr;
PFNGLVERTEXATTRIBS4SVNVPROC __glewVertexAttribs4svNV = nullptr;
PFNGLVERTEXATTRIBS4UBVNVPROC __glewVertexAttribs4ubvNV = nullptr;

PFNGLBEGINVIDEOCAPTURENVPROC __glewBeginVideoCaptureNV = nullptr;
PFNGLBINDVIDEOCAPTURESTREAMBUFFERNVPROC __glewBindVideoCaptureStreamBufferNV = nullptr;
PFNGLBINDVIDEOCAPTURESTREAMTEXTURENVPROC __glewBindVideoCaptureStreamTextureNV = nullptr;
PFNGLENDVIDEOCAPTURENVPROC __glewEndVideoCaptureNV = nullptr;
PFNGLGETVIDEOCAPTURESTREAMDVNVPROC __glewGetVideoCaptureStreamdvNV = nullptr;
PFNGLGETVIDEOCAPTURESTREAMFVNVPROC __glewGetVideoCaptureStreamfvNV = nullptr;
PFNGLGETVIDEOCAPTURESTREAMIVNVPROC __glewGetVideoCaptureStreamivNV = nullptr;
PFNGLGETVIDEOCAPTUREIVNVPROC __glewGetVideoCaptureivNV = nullptr;
PFNGLVIDEOCAPTURENVPROC __glewVideoCaptureNV = nullptr;
PFNGLVIDEOCAPTURESTREAMPARAMETERDVNVPROC __glewVideoCaptureStreamParameterdvNV = nullptr;
PFNGLVIDEOCAPTURESTREAMPARAMETERFVNVPROC __glewVideoCaptureStreamParameterfvNV = nullptr;
PFNGLVIDEOCAPTURESTREAMPARAMETERIVNVPROC __glewVideoCaptureStreamParameterivNV = nullptr;

PFNGLCLEARDEPTHFOESPROC __glewClearDepthfOES = nullptr;
PFNGLCLIPPLANEFOESPROC __glewClipPlanefOES = nullptr;
PFNGLDEPTHRANGEFOESPROC __glewDepthRangefOES = nullptr;
PFNGLFRUSTUMFOESPROC __glewFrustumfOES = nullptr;
PFNGLGETCLIPPLANEFOESPROC __glewGetClipPlanefOES = nullptr;
PFNGLORTHOFOESPROC __glewOrthofOES = nullptr;

PFNGLDETAILTEXFUNCSGISPROC __glewDetailTexFuncSGIS = nullptr;
PFNGLGETDETAILTEXFUNCSGISPROC __glewGetDetailTexFuncSGIS = nullptr;

PFNGLFOGFUNCSGISPROC __glewFogFuncSGIS = nullptr;
PFNGLGETFOGFUNCSGISPROC __glewGetFogFuncSGIS = nullptr;

PFNGLSAMPLEMASKSGISPROC __glewSampleMaskSGIS = nullptr;
PFNGLSAMPLEPATTERNSGISPROC __glewSamplePatternSGIS = nullptr;

PFNGLGETSHARPENTEXFUNCSGISPROC __glewGetSharpenTexFuncSGIS = nullptr;
PFNGLSHARPENTEXFUNCSGISPROC __glewSharpenTexFuncSGIS = nullptr;

PFNGLTEXIMAGE4DSGISPROC __glewTexImage4DSGIS = nullptr;
PFNGLTEXSUBIMAGE4DSGISPROC __glewTexSubImage4DSGIS = nullptr;

PFNGLGETTEXFILTERFUNCSGISPROC __glewGetTexFilterFuncSGIS = nullptr;
PFNGLTEXFILTERFUNCSGISPROC __glewTexFilterFuncSGIS = nullptr;

PFNGLASYNCMARKERSGIXPROC __glewAsyncMarkerSGIX = nullptr;
PFNGLDELETEASYNCMARKERSSGIXPROC __glewDeleteAsyncMarkersSGIX = nullptr;
PFNGLFINISHASYNCSGIXPROC __glewFinishAsyncSGIX = nullptr;
PFNGLGENASYNCMARKERSSGIXPROC __glewGenAsyncMarkersSGIX = nullptr;
PFNGLISASYNCMARKERSGIXPROC __glewIsAsyncMarkerSGIX = nullptr;
PFNGLPOLLASYNCSGIXPROC __glewPollAsyncSGIX = nullptr;

PFNGLFLUSHRASTERSGIXPROC __glewFlushRasterSGIX = nullptr;

PFNGLTEXTUREFOGSGIXPROC __glewTextureFogSGIX = nullptr;

PFNGLFRAGMENTCOLORMATERIALSGIXPROC __glewFragmentColorMaterialSGIX = nullptr;
PFNGLFRAGMENTLIGHTMODELFSGIXPROC __glewFragmentLightModelfSGIX = nullptr;
PFNGLFRAGMENTLIGHTMODELFVSGIXPROC __glewFragmentLightModelfvSGIX = nullptr;
PFNGLFRAGMENTLIGHTMODELISGIXPROC __glewFragmentLightModeliSGIX = nullptr;
PFNGLFRAGMENTLIGHTMODELIVSGIXPROC __glewFragmentLightModelivSGIX = nullptr;
PFNGLFRAGMENTLIGHTFSGIXPROC __glewFragmentLightfSGIX = nullptr;
PFNGLFRAGMENTLIGHTFVSGIXPROC __glewFragmentLightfvSGIX = nullptr;
PFNGLFRAGMENTLIGHTISGIXPROC __glewFragmentLightiSGIX = nullptr;
PFNGLFRAGMENTLIGHTIVSGIXPROC __glewFragmentLightivSGIX = nullptr;
PFNGLFRAGMENTMATERIALFSGIXPROC __glewFragmentMaterialfSGIX = nullptr;
PFNGLFRAGMENTMATERIALFVSGIXPROC __glewFragmentMaterialfvSGIX = nullptr;
PFNGLFRAGMENTMATERIALISGIXPROC __glewFragmentMaterialiSGIX = nullptr;
PFNGLFRAGMENTMATERIALIVSGIXPROC __glewFragmentMaterialivSGIX = nullptr;
PFNGLGETFRAGMENTLIGHTFVSGIXPROC __glewGetFragmentLightfvSGIX = nullptr;
PFNGLGETFRAGMENTLIGHTIVSGIXPROC __glewGetFragmentLightivSGIX = nullptr;
PFNGLGETFRAGMENTMATERIALFVSGIXPROC __glewGetFragmentMaterialfvSGIX = nullptr;
PFNGLGETFRAGMENTMATERIALIVSGIXPROC __glewGetFragmentMaterialivSGIX = nullptr;

PFNGLFRAMEZOOMSGIXPROC __glewFrameZoomSGIX = nullptr;

PFNGLPIXELTEXGENSGIXPROC __glewPixelTexGenSGIX = nullptr;

PFNGLREFERENCEPLANESGIXPROC __glewReferencePlaneSGIX = nullptr;

PFNGLSPRITEPARAMETERFSGIXPROC __glewSpriteParameterfSGIX = nullptr;
PFNGLSPRITEPARAMETERFVSGIXPROC __glewSpriteParameterfvSGIX = nullptr;
PFNGLSPRITEPARAMETERISGIXPROC __glewSpriteParameteriSGIX = nullptr;
PFNGLSPRITEPARAMETERIVSGIXPROC __glewSpriteParameterivSGIX = nullptr;

PFNGLTAGSAMPLEBUFFERSGIXPROC __glewTagSampleBufferSGIX = nullptr;

PFNGLCOLORTABLEPARAMETERFVSGIPROC __glewColorTableParameterfvSGI = nullptr;
PFNGLCOLORTABLEPARAMETERIVSGIPROC __glewColorTableParameterivSGI = nullptr;
PFNGLCOLORTABLESGIPROC __glewColorTableSGI = nullptr;
PFNGLCOPYCOLORTABLESGIPROC __glewCopyColorTableSGI = nullptr;
PFNGLGETCOLORTABLEPARAMETERFVSGIPROC __glewGetColorTableParameterfvSGI = nullptr;
PFNGLGETCOLORTABLEPARAMETERIVSGIPROC __glewGetColorTableParameterivSGI = nullptr;
PFNGLGETCOLORTABLESGIPROC __glewGetColorTableSGI = nullptr;

PFNGLFINISHTEXTURESUNXPROC __glewFinishTextureSUNX = nullptr;

PFNGLGLOBALALPHAFACTORBSUNPROC __glewGlobalAlphaFactorbSUN = nullptr;
PFNGLGLOBALALPHAFACTORDSUNPROC __glewGlobalAlphaFactordSUN = nullptr;
PFNGLGLOBALALPHAFACTORFSUNPROC __glewGlobalAlphaFactorfSUN = nullptr;
PFNGLGLOBALALPHAFACTORISUNPROC __glewGlobalAlphaFactoriSUN = nullptr;
PFNGLGLOBALALPHAFACTORSSUNPROC __glewGlobalAlphaFactorsSUN = nullptr;
PFNGLGLOBALALPHAFACTORUBSUNPROC __glewGlobalAlphaFactorubSUN = nullptr;
PFNGLGLOBALALPHAFACTORUISUNPROC __glewGlobalAlphaFactoruiSUN = nullptr;
PFNGLGLOBALALPHAFACTORUSSUNPROC __glewGlobalAlphaFactorusSUN = nullptr;

PFNGLREADVIDEOPIXELSSUNPROC __glewReadVideoPixelsSUN = nullptr;

PFNGLREPLACEMENTCODEPOINTERSUNPROC __glewReplacementCodePointerSUN = nullptr;
PFNGLREPLACEMENTCODEUBSUNPROC __glewReplacementCodeubSUN = nullptr;
PFNGLREPLACEMENTCODEUBVSUNPROC __glewReplacementCodeubvSUN = nullptr;
PFNGLREPLACEMENTCODEUISUNPROC __glewReplacementCodeuiSUN = nullptr;
PFNGLREPLACEMENTCODEUIVSUNPROC __glewReplacementCodeuivSUN = nullptr;
PFNGLREPLACEMENTCODEUSSUNPROC __glewReplacementCodeusSUN = nullptr;
PFNGLREPLACEMENTCODEUSVSUNPROC __glewReplacementCodeusvSUN = nullptr;

PFNGLCOLOR3FVERTEX3FSUNPROC __glewColor3fVertex3fSUN = nullptr;
PFNGLCOLOR3FVERTEX3FVSUNPROC __glewColor3fVertex3fvSUN = nullptr;
PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewColor4fNormal3fVertex3fSUN = nullptr;
PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewColor4fNormal3fVertex3fvSUN = nullptr;
PFNGLCOLOR4UBVERTEX2FSUNPROC __glewColor4ubVertex2fSUN = nullptr;
PFNGLCOLOR4UBVERTEX2FVSUNPROC __glewColor4ubVertex2fvSUN = nullptr;
PFNGLCOLOR4UBVERTEX3FSUNPROC __glewColor4ubVertex3fSUN = nullptr;
PFNGLCOLOR4UBVERTEX3FVSUNPROC __glewColor4ubVertex3fvSUN = nullptr;
PFNGLNORMAL3FVERTEX3FSUNPROC __glewNormal3fVertex3fSUN = nullptr;
PFNGLNORMAL3FVERTEX3FVSUNPROC __glewNormal3fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC __glewReplacementCodeuiColor3fVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC __glewReplacementCodeuiColor3fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiColor4fNormal3fVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiColor4fNormal3fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC __glewReplacementCodeuiColor4ubVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC __glewReplacementCodeuiColor4ubVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiNormal3fVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiNormal3fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC __glewReplacementCodeuiTexCoord2fVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC __glewReplacementCodeuiTexCoord2fVertex3fvSUN = nullptr;
PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC __glewReplacementCodeuiVertex3fSUN = nullptr;
PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC __glewReplacementCodeuiVertex3fvSUN = nullptr;
PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC __glewTexCoord2fColor3fVertex3fSUN = nullptr;
PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC __glewTexCoord2fColor3fVertex3fvSUN = nullptr;
PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC __glewTexCoord2fColor4fNormal3fVertex3fSUN = nullptr;
PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC __glewTexCoord2fColor4fNormal3fVertex3fvSUN = nullptr;
PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC __glewTexCoord2fColor4ubVertex3fSUN = nullptr;
PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC __glewTexCoord2fColor4ubVertex3fvSUN = nullptr;
PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC __glewTexCoord2fNormal3fVertex3fSUN = nullptr;
PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC __glewTexCoord2fNormal3fVertex3fvSUN = nullptr;
PFNGLTEXCOORD2FVERTEX3FSUNPROC __glewTexCoord2fVertex3fSUN = nullptr;
PFNGLTEXCOORD2FVERTEX3FVSUNPROC __glewTexCoord2fVertex3fvSUN = nullptr;
PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC __glewTexCoord4fColor4fNormal3fVertex4fSUN = nullptr;
PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC __glewTexCoord4fColor4fNormal3fVertex4fvSUN = nullptr;
PFNGLTEXCOORD4FVERTEX4FSUNPROC __glewTexCoord4fVertex4fSUN = nullptr;
PFNGLTEXCOORD4FVERTEX4FVSUNPROC __glewTexCoord4fVertex4fvSUN = nullptr;

PFNGLADDSWAPHINTRECTWINPROC __glewAddSwapHintRectWIN = nullptr;

#endif /* !WIN32 || !GLEW_MX */

#if !defined(GLEW_MX)

GLboolean __GLEW_VERSION_1_1 = GL_FALSE;
GLboolean __GLEW_VERSION_1_2 = GL_FALSE;
GLboolean __GLEW_VERSION_1_2_1 = GL_FALSE;
GLboolean __GLEW_VERSION_1_3 = GL_FALSE;
GLboolean __GLEW_VERSION_1_4 = GL_FALSE;
GLboolean __GLEW_VERSION_1_5 = GL_FALSE;
GLboolean __GLEW_VERSION_2_0 = GL_FALSE;
GLboolean __GLEW_VERSION_2_1 = GL_FALSE;
GLboolean __GLEW_VERSION_3_0 = GL_FALSE;
GLboolean __GLEW_VERSION_3_1 = GL_FALSE;
GLboolean __GLEW_VERSION_3_2 = GL_FALSE;
GLboolean __GLEW_VERSION_3_3 = GL_FALSE;
GLboolean __GLEW_VERSION_4_0 = GL_FALSE;
GLboolean __GLEW_VERSION_4_1 = GL_FALSE;
GLboolean __GLEW_VERSION_4_2 = GL_FALSE;
GLboolean __GLEW_3DFX_multisample = GL_FALSE;
GLboolean __GLEW_3DFX_tbuffer = GL_FALSE;
GLboolean __GLEW_3DFX_texture_compression_FXT1 = GL_FALSE;
GLboolean __GLEW_AMD_blend_minmax_factor = GL_FALSE;
GLboolean __GLEW_AMD_conservative_depth = GL_FALSE;
GLboolean __GLEW_AMD_debug_output = GL_FALSE;
GLboolean __GLEW_AMD_depth_clamp_separate = GL_FALSE;
GLboolean __GLEW_AMD_draw_buffers_blend = GL_FALSE;
GLboolean __GLEW_AMD_multi_draw_indirect = GL_FALSE;
GLboolean __GLEW_AMD_name_gen_delete = GL_FALSE;
GLboolean __GLEW_AMD_performance_monitor = GL_FALSE;
GLboolean __GLEW_AMD_sample_positions = GL_FALSE;
GLboolean __GLEW_AMD_seamless_cubemap_per_texture = GL_FALSE;
GLboolean __GLEW_AMD_shader_stencil_export = GL_FALSE;
GLboolean __GLEW_AMD_texture_texture4 = GL_FALSE;
GLboolean __GLEW_AMD_transform_feedback3_lines_triangles = GL_FALSE;
GLboolean __GLEW_AMD_vertex_shader_tessellator = GL_FALSE;
GLboolean __GLEW_APPLE_aux_depth_stencil = GL_FALSE;
GLboolean __GLEW_APPLE_client_storage = GL_FALSE;
GLboolean __GLEW_APPLE_element_array = GL_FALSE;
GLboolean __GLEW_APPLE_fence = GL_FALSE;
GLboolean __GLEW_APPLE_float_pixels = GL_FALSE;
GLboolean __GLEW_APPLE_flush_buffer_range = GL_FALSE;
GLboolean __GLEW_APPLE_object_purgeable = GL_FALSE;
GLboolean __GLEW_APPLE_pixel_buffer = GL_FALSE;
GLboolean __GLEW_APPLE_rgb_422 = GL_FALSE;
GLboolean __GLEW_APPLE_row_bytes = GL_FALSE;
GLboolean __GLEW_APPLE_specular_vector = GL_FALSE;
GLboolean __GLEW_APPLE_texture_range = GL_FALSE;
GLboolean __GLEW_APPLE_transform_hint = GL_FALSE;
GLboolean __GLEW_APPLE_vertex_array_object = GL_FALSE;
GLboolean __GLEW_APPLE_vertex_array_range = GL_FALSE;
GLboolean __GLEW_APPLE_vertex_program_evaluators = GL_FALSE;
GLboolean __GLEW_APPLE_ycbcr_422 = GL_FALSE;
GLboolean __GLEW_ARB_ES2_compatibility = GL_FALSE;
GLboolean __GLEW_ARB_base_instance = GL_FALSE;
GLboolean __GLEW_ARB_blend_func_extended = GL_FALSE;
GLboolean __GLEW_ARB_cl_event = GL_FALSE;
GLboolean __GLEW_ARB_color_buffer_float = GL_FALSE;
GLboolean __GLEW_ARB_compatibility = GL_FALSE;
GLboolean __GLEW_ARB_compressed_texture_pixel_storage = GL_FALSE;
GLboolean __GLEW_ARB_conservative_depth = GL_FALSE;
GLboolean __GLEW_ARB_copy_buffer = GL_FALSE;
GLboolean __GLEW_ARB_debug_output = GL_FALSE;
GLboolean __GLEW_ARB_depth_buffer_float = GL_FALSE;
GLboolean __GLEW_ARB_depth_clamp = GL_FALSE;
GLboolean __GLEW_ARB_depth_texture = GL_FALSE;
GLboolean __GLEW_ARB_draw_buffers = GL_FALSE;
GLboolean __GLEW_ARB_draw_buffers_blend = GL_FALSE;
GLboolean __GLEW_ARB_draw_elements_base_vertex = GL_FALSE;
GLboolean __GLEW_ARB_draw_indirect = GL_FALSE;
GLboolean __GLEW_ARB_draw_instanced = GL_FALSE;
GLboolean __GLEW_ARB_explicit_attrib_location = GL_FALSE;
GLboolean __GLEW_ARB_fragment_coord_conventions = GL_FALSE;
GLboolean __GLEW_ARB_fragment_program = GL_FALSE;
GLboolean __GLEW_ARB_fragment_program_shadow = GL_FALSE;
GLboolean __GLEW_ARB_fragment_shader = GL_FALSE;
GLboolean __GLEW_ARB_framebuffer_object = GL_FALSE;
GLboolean __GLEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __GLEW_ARB_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_ARB_get_program_binary = GL_FALSE;
GLboolean __GLEW_ARB_gpu_shader5 = GL_FALSE;
GLboolean __GLEW_ARB_gpu_shader_fp64 = GL_FALSE;
GLboolean __GLEW_ARB_half_float_pixel = GL_FALSE;
GLboolean __GLEW_ARB_half_float_vertex = GL_FALSE;
GLboolean __GLEW_ARB_imaging = GL_FALSE;
GLboolean __GLEW_ARB_instanced_arrays = GL_FALSE;
GLboolean __GLEW_ARB_internalformat_query = GL_FALSE;
GLboolean __GLEW_ARB_map_buffer_alignment = GL_FALSE;
GLboolean __GLEW_ARB_map_buffer_range = GL_FALSE;
GLboolean __GLEW_ARB_matrix_palette = GL_FALSE;
GLboolean __GLEW_ARB_multisample = GL_FALSE;
GLboolean __GLEW_ARB_multitexture = GL_FALSE;
GLboolean __GLEW_ARB_occlusion_query = GL_FALSE;
GLboolean __GLEW_ARB_occlusion_query2 = GL_FALSE;
GLboolean __GLEW_ARB_pixel_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_point_parameters = GL_FALSE;
GLboolean __GLEW_ARB_point_sprite = GL_FALSE;
GLboolean __GLEW_ARB_provoking_vertex = GL_FALSE;
GLboolean __GLEW_ARB_robustness = GL_FALSE;
GLboolean __GLEW_ARB_sample_shading = GL_FALSE;
GLboolean __GLEW_ARB_sampler_objects = GL_FALSE;
GLboolean __GLEW_ARB_seamless_cube_map = GL_FALSE;
GLboolean __GLEW_ARB_separate_shader_objects = GL_FALSE;
GLboolean __GLEW_ARB_shader_atomic_counters = GL_FALSE;
GLboolean __GLEW_ARB_shader_bit_encoding = GL_FALSE;
GLboolean __GLEW_ARB_shader_image_load_store = GL_FALSE;
GLboolean __GLEW_ARB_shader_objects = GL_FALSE;
GLboolean __GLEW_ARB_shader_precision = GL_FALSE;
GLboolean __GLEW_ARB_shader_stencil_export = GL_FALSE;
GLboolean __GLEW_ARB_shader_subroutine = GL_FALSE;
GLboolean __GLEW_ARB_shader_texture_lod = GL_FALSE;
GLboolean __GLEW_ARB_shading_language_100 = GL_FALSE;
GLboolean __GLEW_ARB_shading_language_420pack = GL_FALSE;
GLboolean __GLEW_ARB_shading_language_include = GL_FALSE;
GLboolean __GLEW_ARB_shading_language_packing = GL_FALSE;
GLboolean __GLEW_ARB_shadow = GL_FALSE;
GLboolean __GLEW_ARB_shadow_ambient = GL_FALSE;
GLboolean __GLEW_ARB_sync = GL_FALSE;
GLboolean __GLEW_ARB_tessellation_shader = GL_FALSE;
GLboolean __GLEW_ARB_texture_border_clamp = GL_FALSE;
GLboolean __GLEW_ARB_texture_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_texture_buffer_object_rgb32 = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression_bptc = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression_rgtc = GL_FALSE;
GLboolean __GLEW_ARB_texture_cube_map = GL_FALSE;
GLboolean __GLEW_ARB_texture_cube_map_array = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_add = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_combine = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_crossbar = GL_FALSE;
GLboolean __GLEW_ARB_texture_env_dot3 = GL_FALSE;
GLboolean __GLEW_ARB_texture_float = GL_FALSE;
GLboolean __GLEW_ARB_texture_gather = GL_FALSE;
GLboolean __GLEW_ARB_texture_mirrored_repeat = GL_FALSE;
GLboolean __GLEW_ARB_texture_multisample = GL_FALSE;
GLboolean __GLEW_ARB_texture_non_power_of_two = GL_FALSE;
GLboolean __GLEW_ARB_texture_query_lod = GL_FALSE;
GLboolean __GLEW_ARB_texture_rectangle = GL_FALSE;
GLboolean __GLEW_ARB_texture_rg = GL_FALSE;
GLboolean __GLEW_ARB_texture_rgb10_a2ui = GL_FALSE;
GLboolean __GLEW_ARB_texture_storage = GL_FALSE;
GLboolean __GLEW_ARB_texture_swizzle = GL_FALSE;
GLboolean __GLEW_ARB_timer_query = GL_FALSE;
GLboolean __GLEW_ARB_transform_feedback2 = GL_FALSE;
GLboolean __GLEW_ARB_transform_feedback3 = GL_FALSE;
GLboolean __GLEW_ARB_transform_feedback_instanced = GL_FALSE;
GLboolean __GLEW_ARB_transpose_matrix = GL_FALSE;
GLboolean __GLEW_ARB_uniform_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_array_bgra = GL_FALSE;
GLboolean __GLEW_ARB_vertex_array_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_attrib_64bit = GL_FALSE;
GLboolean __GLEW_ARB_vertex_blend = GL_FALSE;
GLboolean __GLEW_ARB_vertex_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_program = GL_FALSE;
GLboolean __GLEW_ARB_vertex_shader = GL_FALSE;
GLboolean __GLEW_ARB_vertex_type_2_10_10_10_rev = GL_FALSE;
GLboolean __GLEW_ARB_viewport_array = GL_FALSE;
GLboolean __GLEW_ARB_window_pos = GL_FALSE;
GLboolean __GLEW_ATIX_point_sprites = GL_FALSE;
GLboolean __GLEW_ATIX_texture_env_combine3 = GL_FALSE;
GLboolean __GLEW_ATIX_texture_env_route = GL_FALSE;
GLboolean __GLEW_ATIX_vertex_shader_output_point_size = GL_FALSE;
GLboolean __GLEW_ATI_draw_buffers = GL_FALSE;
GLboolean __GLEW_ATI_element_array = GL_FALSE;
GLboolean __GLEW_ATI_envmap_bumpmap = GL_FALSE;
GLboolean __GLEW_ATI_fragment_shader = GL_FALSE;
GLboolean __GLEW_ATI_map_object_buffer = GL_FALSE;
GLboolean __GLEW_ATI_meminfo = GL_FALSE;
GLboolean __GLEW_ATI_pn_triangles = GL_FALSE;
GLboolean __GLEW_ATI_separate_stencil = GL_FALSE;
GLboolean __GLEW_ATI_shader_texture_lod = GL_FALSE;
GLboolean __GLEW_ATI_text_fragment_shader = GL_FALSE;
GLboolean __GLEW_ATI_texture_compression_3dc = GL_FALSE;
GLboolean __GLEW_ATI_texture_env_combine3 = GL_FALSE;
GLboolean __GLEW_ATI_texture_float = GL_FALSE;
GLboolean __GLEW_ATI_texture_mirror_once = GL_FALSE;
GLboolean __GLEW_ATI_vertex_array_object = GL_FALSE;
GLboolean __GLEW_ATI_vertex_attrib_array_object = GL_FALSE;
GLboolean __GLEW_ATI_vertex_streams = GL_FALSE;
GLboolean __GLEW_EXT_422_pixels = GL_FALSE;
GLboolean __GLEW_EXT_Cg_shader = GL_FALSE;
GLboolean __GLEW_EXT_abgr = GL_FALSE;
GLboolean __GLEW_EXT_bgra = GL_FALSE;
GLboolean __GLEW_EXT_bindable_uniform = GL_FALSE;
GLboolean __GLEW_EXT_blend_color = GL_FALSE;
GLboolean __GLEW_EXT_blend_equation_separate = GL_FALSE;
GLboolean __GLEW_EXT_blend_func_separate = GL_FALSE;
GLboolean __GLEW_EXT_blend_logic_op = GL_FALSE;
GLboolean __GLEW_EXT_blend_minmax = GL_FALSE;
GLboolean __GLEW_EXT_blend_subtract = GL_FALSE;
GLboolean __GLEW_EXT_clip_volume_hint = GL_FALSE;
GLboolean __GLEW_EXT_cmyka = GL_FALSE;
GLboolean __GLEW_EXT_color_subtable = GL_FALSE;
GLboolean __GLEW_EXT_compiled_vertex_array = GL_FALSE;
GLboolean __GLEW_EXT_convolution = GL_FALSE;
GLboolean __GLEW_EXT_coordinate_frame = GL_FALSE;
GLboolean __GLEW_EXT_copy_texture = GL_FALSE;
GLboolean __GLEW_EXT_cull_vertex = GL_FALSE;
GLboolean __GLEW_EXT_depth_bounds_test = GL_FALSE;
GLboolean __GLEW_EXT_direct_state_access = GL_FALSE;
GLboolean __GLEW_EXT_draw_buffers2 = GL_FALSE;
GLboolean __GLEW_EXT_draw_instanced = GL_FALSE;
GLboolean __GLEW_EXT_draw_range_elements = GL_FALSE;
GLboolean __GLEW_EXT_fog_coord = GL_FALSE;
GLboolean __GLEW_EXT_fragment_lighting = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_blit = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_multisample = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_multisample_blit_scaled = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_object = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __GLEW_EXT_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_EXT_gpu_program_parameters = GL_FALSE;
GLboolean __GLEW_EXT_gpu_shader4 = GL_FALSE;
GLboolean __GLEW_EXT_histogram = GL_FALSE;
GLboolean __GLEW_EXT_index_array_formats = GL_FALSE;
GLboolean __GLEW_EXT_index_func = GL_FALSE;
GLboolean __GLEW_EXT_index_material = GL_FALSE;
GLboolean __GLEW_EXT_index_texture = GL_FALSE;
GLboolean __GLEW_EXT_light_texture = GL_FALSE;
GLboolean __GLEW_EXT_misc_attribute = GL_FALSE;
GLboolean __GLEW_EXT_multi_draw_arrays = GL_FALSE;
GLboolean __GLEW_EXT_multisample = GL_FALSE;
GLboolean __GLEW_EXT_packed_depth_stencil = GL_FALSE;
GLboolean __GLEW_EXT_packed_float = GL_FALSE;
GLboolean __GLEW_EXT_packed_pixels = GL_FALSE;
GLboolean __GLEW_EXT_paletted_texture = GL_FALSE;
GLboolean __GLEW_EXT_pixel_buffer_object = GL_FALSE;
GLboolean __GLEW_EXT_pixel_transform = GL_FALSE;
GLboolean __GLEW_EXT_pixel_transform_color_table = GL_FALSE;
GLboolean __GLEW_EXT_point_parameters = GL_FALSE;
GLboolean __GLEW_EXT_polygon_offset = GL_FALSE;
GLboolean __GLEW_EXT_provoking_vertex = GL_FALSE;
GLboolean __GLEW_EXT_rescale_normal = GL_FALSE;
GLboolean __GLEW_EXT_scene_marker = GL_FALSE;
GLboolean __GLEW_EXT_secondary_color = GL_FALSE;
GLboolean __GLEW_EXT_separate_shader_objects = GL_FALSE;
GLboolean __GLEW_EXT_separate_specular_color = GL_FALSE;
GLboolean __GLEW_EXT_shader_image_load_store = GL_FALSE;
GLboolean __GLEW_EXT_shadow_funcs = GL_FALSE;
GLboolean __GLEW_EXT_shared_texture_palette = GL_FALSE;
GLboolean __GLEW_EXT_stencil_clear_tag = GL_FALSE;
GLboolean __GLEW_EXT_stencil_two_side = GL_FALSE;
GLboolean __GLEW_EXT_stencil_wrap = GL_FALSE;
GLboolean __GLEW_EXT_subtexture = GL_FALSE;
GLboolean __GLEW_EXT_texture = GL_FALSE;
GLboolean __GLEW_EXT_texture3D = GL_FALSE;
GLboolean __GLEW_EXT_texture_array = GL_FALSE;
GLboolean __GLEW_EXT_texture_buffer_object = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_dxt1 = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_latc = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_rgtc = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_s3tc = GL_FALSE;
GLboolean __GLEW_EXT_texture_cube_map = GL_FALSE;
GLboolean __GLEW_EXT_texture_edge_clamp = GL_FALSE;
GLboolean __GLEW_EXT_texture_env = GL_FALSE;
GLboolean __GLEW_EXT_texture_env_add = GL_FALSE;
GLboolean __GLEW_EXT_texture_env_combine = GL_FALSE;
GLboolean __GLEW_EXT_texture_env_dot3 = GL_FALSE;
GLboolean __GLEW_EXT_texture_filter_anisotropic = GL_FALSE;
GLboolean __GLEW_EXT_texture_integer = GL_FALSE;
GLboolean __GLEW_EXT_texture_lod_bias = GL_FALSE;
GLboolean __GLEW_EXT_texture_mirror_clamp = GL_FALSE;
GLboolean __GLEW_EXT_texture_object = GL_FALSE;
GLboolean __GLEW_EXT_texture_perturb_normal = GL_FALSE;
GLboolean __GLEW_EXT_texture_rectangle = GL_FALSE;
GLboolean __GLEW_EXT_texture_sRGB = GL_FALSE;
GLboolean __GLEW_EXT_texture_sRGB_decode = GL_FALSE;
GLboolean __GLEW_EXT_texture_shared_exponent = GL_FALSE;
GLboolean __GLEW_EXT_texture_snorm = GL_FALSE;
GLboolean __GLEW_EXT_texture_swizzle = GL_FALSE;
GLboolean __GLEW_EXT_timer_query = GL_FALSE;
GLboolean __GLEW_EXT_transform_feedback = GL_FALSE;
GLboolean __GLEW_EXT_vertex_array = GL_FALSE;
GLboolean __GLEW_EXT_vertex_array_bgra = GL_FALSE;
GLboolean __GLEW_EXT_vertex_attrib_64bit = GL_FALSE;
GLboolean __GLEW_EXT_vertex_shader = GL_FALSE;
GLboolean __GLEW_EXT_vertex_weighting = GL_FALSE;
GLboolean __GLEW_EXT_x11_sync_object = GL_FALSE;
GLboolean __GLEW_GREMEDY_frame_terminator = GL_FALSE;
GLboolean __GLEW_GREMEDY_string_marker = GL_FALSE;
GLboolean __GLEW_HP_convolution_border_modes = GL_FALSE;
GLboolean __GLEW_HP_image_transform = GL_FALSE;
GLboolean __GLEW_HP_occlusion_test = GL_FALSE;
GLboolean __GLEW_HP_texture_lighting = GL_FALSE;
GLboolean __GLEW_IBM_cull_vertex = GL_FALSE;
GLboolean __GLEW_IBM_multimode_draw_arrays = GL_FALSE;
GLboolean __GLEW_IBM_rasterpos_clip = GL_FALSE;
GLboolean __GLEW_IBM_static_data = GL_FALSE;
GLboolean __GLEW_IBM_texture_mirrored_repeat = GL_FALSE;
GLboolean __GLEW_IBM_vertex_array_lists = GL_FALSE;
GLboolean __GLEW_INGR_color_clamp = GL_FALSE;
GLboolean __GLEW_INGR_interlace_read = GL_FALSE;
GLboolean __GLEW_INTEL_parallel_arrays = GL_FALSE;
GLboolean __GLEW_INTEL_texture_scissor = GL_FALSE;
GLboolean __GLEW_KTX_buffer_region = GL_FALSE;
GLboolean __GLEW_MESAX_texture_stack = GL_FALSE;
GLboolean __GLEW_MESA_pack_invert = GL_FALSE;
GLboolean __GLEW_MESA_resize_buffers = GL_FALSE;
GLboolean __GLEW_MESA_window_pos = GL_FALSE;
GLboolean __GLEW_MESA_ycbcr_texture = GL_FALSE;
GLboolean __GLEW_NVX_gpu_memory_info = GL_FALSE;
GLboolean __GLEW_NV_blend_square = GL_FALSE;
GLboolean __GLEW_NV_conditional_render = GL_FALSE;
GLboolean __GLEW_NV_copy_depth_to_color = GL_FALSE;
GLboolean __GLEW_NV_copy_image = GL_FALSE;
GLboolean __GLEW_NV_depth_buffer_float = GL_FALSE;
GLboolean __GLEW_NV_depth_clamp = GL_FALSE;
GLboolean __GLEW_NV_depth_range_unclamped = GL_FALSE;
GLboolean __GLEW_NV_evaluators = GL_FALSE;
GLboolean __GLEW_NV_explicit_multisample = GL_FALSE;
GLboolean __GLEW_NV_fence = GL_FALSE;
GLboolean __GLEW_NV_float_buffer = GL_FALSE;
GLboolean __GLEW_NV_fog_distance = GL_FALSE;
GLboolean __GLEW_NV_fragment_program = GL_FALSE;
GLboolean __GLEW_NV_fragment_program2 = GL_FALSE;
GLboolean __GLEW_NV_fragment_program4 = GL_FALSE;
GLboolean __GLEW_NV_fragment_program_option = GL_FALSE;
GLboolean __GLEW_NV_framebuffer_multisample_coverage = GL_FALSE;
GLboolean __GLEW_NV_geometry_program4 = GL_FALSE;
GLboolean __GLEW_NV_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_NV_gpu_program4 = GL_FALSE;
GLboolean __GLEW_NV_gpu_program5 = GL_FALSE;
GLboolean __GLEW_NV_gpu_program_fp64 = GL_FALSE;
GLboolean __GLEW_NV_gpu_shader5 = GL_FALSE;
GLboolean __GLEW_NV_half_float = GL_FALSE;
GLboolean __GLEW_NV_light_max_exponent = GL_FALSE;
GLboolean __GLEW_NV_multisample_coverage = GL_FALSE;
GLboolean __GLEW_NV_multisample_filter_hint = GL_FALSE;
GLboolean __GLEW_NV_occlusion_query = GL_FALSE;
GLboolean __GLEW_NV_packed_depth_stencil = GL_FALSE;
GLboolean __GLEW_NV_parameter_buffer_object = GL_FALSE;
GLboolean __GLEW_NV_parameter_buffer_object2 = GL_FALSE;
GLboolean __GLEW_NV_path_rendering = GL_FALSE;
GLboolean __GLEW_NV_pixel_data_range = GL_FALSE;
GLboolean __GLEW_NV_point_sprite = GL_FALSE;
GLboolean __GLEW_NV_present_video = GL_FALSE;
GLboolean __GLEW_NV_primitive_restart = GL_FALSE;
GLboolean __GLEW_NV_register_combiners = GL_FALSE;
GLboolean __GLEW_NV_register_combiners2 = GL_FALSE;
GLboolean __GLEW_NV_shader_buffer_load = GL_FALSE;
GLboolean __GLEW_NV_tessellation_program5 = GL_FALSE;
GLboolean __GLEW_NV_texgen_emboss = GL_FALSE;
GLboolean __GLEW_NV_texgen_reflection = GL_FALSE;
GLboolean __GLEW_NV_texture_barrier = GL_FALSE;
GLboolean __GLEW_NV_texture_compression_vtc = GL_FALSE;
GLboolean __GLEW_NV_texture_env_combine4 = GL_FALSE;
GLboolean __GLEW_NV_texture_expand_normal = GL_FALSE;
GLboolean __GLEW_NV_texture_multisample = GL_FALSE;
GLboolean __GLEW_NV_texture_rectangle = GL_FALSE;
GLboolean __GLEW_NV_texture_shader = GL_FALSE;
GLboolean __GLEW_NV_texture_shader2 = GL_FALSE;
GLboolean __GLEW_NV_texture_shader3 = GL_FALSE;
GLboolean __GLEW_NV_transform_feedback = GL_FALSE;
GLboolean __GLEW_NV_transform_feedback2 = GL_FALSE;
GLboolean __GLEW_NV_vdpau_interop = GL_FALSE;
GLboolean __GLEW_NV_vertex_array_range = GL_FALSE;
GLboolean __GLEW_NV_vertex_array_range2 = GL_FALSE;
GLboolean __GLEW_NV_vertex_attrib_integer_64bit = GL_FALSE;
GLboolean __GLEW_NV_vertex_buffer_unified_memory = GL_FALSE;
GLboolean __GLEW_NV_vertex_program = GL_FALSE;
GLboolean __GLEW_NV_vertex_program1_1 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program2 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program2_option = GL_FALSE;
GLboolean __GLEW_NV_vertex_program3 = GL_FALSE;
GLboolean __GLEW_NV_vertex_program4 = GL_FALSE;
GLboolean __GLEW_NV_video_capture = GL_FALSE;
GLboolean __GLEW_OES_byte_coordinates = GL_FALSE;
GLboolean __GLEW_OES_compressed_paletted_texture = GL_FALSE;
GLboolean __GLEW_OES_read_format = GL_FALSE;
GLboolean __GLEW_OES_single_precision = GL_FALSE;
GLboolean __GLEW_OML_interlace = GL_FALSE;
GLboolean __GLEW_OML_resample = GL_FALSE;
GLboolean __GLEW_OML_subsample = GL_FALSE;
GLboolean __GLEW_PGI_misc_hints = GL_FALSE;
GLboolean __GLEW_PGI_vertex_hints = GL_FALSE;
GLboolean __GLEW_REND_screen_coordinates = GL_FALSE;
GLboolean __GLEW_S3_s3tc = GL_FALSE;
GLboolean __GLEW_SGIS_color_range = GL_FALSE;
GLboolean __GLEW_SGIS_detail_texture = GL_FALSE;
GLboolean __GLEW_SGIS_fog_function = GL_FALSE;
GLboolean __GLEW_SGIS_generate_mipmap = GL_FALSE;
GLboolean __GLEW_SGIS_multisample = GL_FALSE;
GLboolean __GLEW_SGIS_pixel_texture = GL_FALSE;
GLboolean __GLEW_SGIS_point_line_texgen = GL_FALSE;
GLboolean __GLEW_SGIS_sharpen_texture = GL_FALSE;
GLboolean __GLEW_SGIS_texture4D = GL_FALSE;
GLboolean __GLEW_SGIS_texture_border_clamp = GL_FALSE;
GLboolean __GLEW_SGIS_texture_edge_clamp = GL_FALSE;
GLboolean __GLEW_SGIS_texture_filter4 = GL_FALSE;
GLboolean __GLEW_SGIS_texture_lod = GL_FALSE;
GLboolean __GLEW_SGIS_texture_select = GL_FALSE;
GLboolean __GLEW_SGIX_async = GL_FALSE;
GLboolean __GLEW_SGIX_async_histogram = GL_FALSE;
GLboolean __GLEW_SGIX_async_pixel = GL_FALSE;
GLboolean __GLEW_SGIX_blend_alpha_minmax = GL_FALSE;
GLboolean __GLEW_SGIX_clipmap = GL_FALSE;
GLboolean __GLEW_SGIX_convolution_accuracy = GL_FALSE;
GLboolean __GLEW_SGIX_depth_texture = GL_FALSE;
GLboolean __GLEW_SGIX_flush_raster = GL_FALSE;
GLboolean __GLEW_SGIX_fog_offset = GL_FALSE;
GLboolean __GLEW_SGIX_fog_texture = GL_FALSE;
GLboolean __GLEW_SGIX_fragment_specular_lighting = GL_FALSE;
GLboolean __GLEW_SGIX_framezoom = GL_FALSE;
GLboolean __GLEW_SGIX_interlace = GL_FALSE;
GLboolean __GLEW_SGIX_ir_instrument1 = GL_FALSE;
GLboolean __GLEW_SGIX_list_priority = GL_FALSE;
GLboolean __GLEW_SGIX_pixel_texture = GL_FALSE;
GLboolean __GLEW_SGIX_pixel_texture_bits = GL_FALSE;
GLboolean __GLEW_SGIX_reference_plane = GL_FALSE;
GLboolean __GLEW_SGIX_resample = GL_FALSE;
GLboolean __GLEW_SGIX_shadow = GL_FALSE;
GLboolean __GLEW_SGIX_shadow_ambient = GL_FALSE;
GLboolean __GLEW_SGIX_sprite = GL_FALSE;
GLboolean __GLEW_SGIX_tag_sample_buffer = GL_FALSE;
GLboolean __GLEW_SGIX_texture_add_env = GL_FALSE;
GLboolean __GLEW_SGIX_texture_coordinate_clamp = GL_FALSE;
GLboolean __GLEW_SGIX_texture_lod_bias = GL_FALSE;
GLboolean __GLEW_SGIX_texture_multi_buffer = GL_FALSE;
GLboolean __GLEW_SGIX_texture_range = GL_FALSE;
GLboolean __GLEW_SGIX_texture_scale_bias = GL_FALSE;
GLboolean __GLEW_SGIX_vertex_preclip = GL_FALSE;
GLboolean __GLEW_SGIX_vertex_preclip_hint = GL_FALSE;
GLboolean __GLEW_SGIX_ycrcb = GL_FALSE;
GLboolean __GLEW_SGI_color_matrix = GL_FALSE;
GLboolean __GLEW_SGI_color_table = GL_FALSE;
GLboolean __GLEW_SGI_texture_color_table = GL_FALSE;
GLboolean __GLEW_SUNX_constant_data = GL_FALSE;
GLboolean __GLEW_SUN_convolution_border_modes = GL_FALSE;
GLboolean __GLEW_SUN_global_alpha = GL_FALSE;
GLboolean __GLEW_SUN_mesh_array = GL_FALSE;
GLboolean __GLEW_SUN_read_video_pixels = GL_FALSE;
GLboolean __GLEW_SUN_slice_accum = GL_FALSE;
GLboolean __GLEW_SUN_triangle_list = GL_FALSE;
GLboolean __GLEW_SUN_vertex = GL_FALSE;
GLboolean __GLEW_WIN_phong_shading = GL_FALSE;
GLboolean __GLEW_WIN_specular_fog = GL_FALSE;
GLboolean __GLEW_WIN_swap_hint = GL_FALSE;

#endif /* !GLEW_MX */

#ifdef GL_VERSION_1_2

static GLboolean _glewInit_GL_VERSION_1_2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage3D")) == nullptr) || r;
  r = ((glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElements")) == nullptr) || r;
  r = ((glTexImage3D = (PFNGLTEXIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexImage3D")) == nullptr) || r;
  r = ((glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage3D")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_1_2 */

#ifdef GL_VERSION_1_2_1

#endif /* GL_VERSION_1_2_1 */

#ifdef GL_VERSION_1_3

static GLboolean _glewInit_GL_VERSION_1_3 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveTexture = (PFNGLACTIVETEXTUREPROC)glewGetProcAddress((const GLubyte*)"glActiveTexture")) == nullptr) || r;
  r = ((glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)glewGetProcAddress((const GLubyte*)"glClientActiveTexture")) == nullptr) || r;
  r = ((glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage1D")) == nullptr) || r;
  r = ((glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage2D")) == nullptr) || r;
  r = ((glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage3D")) == nullptr) || r;
  r = ((glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage1D")) == nullptr) || r;
  r = ((glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage2D")) == nullptr) || r;
  r = ((glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage3D")) == nullptr) || r;
  r = ((glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedTexImage")) == nullptr) || r;
  r = ((glLoadTransposeMatrixd = (PFNGLLOADTRANSPOSEMATRIXDPROC)glewGetProcAddress((const GLubyte*)"glLoadTransposeMatrixd")) == nullptr) || r;
  r = ((glLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFPROC)glewGetProcAddress((const GLubyte*)"glLoadTransposeMatrixf")) == nullptr) || r;
  r = ((glMultTransposeMatrixd = (PFNGLMULTTRANSPOSEMATRIXDPROC)glewGetProcAddress((const GLubyte*)"glMultTransposeMatrixd")) == nullptr) || r;
  r = ((glMultTransposeMatrixf = (PFNGLMULTTRANSPOSEMATRIXFPROC)glewGetProcAddress((const GLubyte*)"glMultTransposeMatrixf")) == nullptr) || r;
  r = ((glMultiTexCoord1d = (PFNGLMULTITEXCOORD1DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1d")) == nullptr) || r;
  r = ((glMultiTexCoord1dv = (PFNGLMULTITEXCOORD1DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1dv")) == nullptr) || r;
  r = ((glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1f")) == nullptr) || r;
  r = ((glMultiTexCoord1fv = (PFNGLMULTITEXCOORD1FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1fv")) == nullptr) || r;
  r = ((glMultiTexCoord1i = (PFNGLMULTITEXCOORD1IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1i")) == nullptr) || r;
  r = ((glMultiTexCoord1iv = (PFNGLMULTITEXCOORD1IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1iv")) == nullptr) || r;
  r = ((glMultiTexCoord1s = (PFNGLMULTITEXCOORD1SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1s")) == nullptr) || r;
  r = ((glMultiTexCoord1sv = (PFNGLMULTITEXCOORD1SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1sv")) == nullptr) || r;
  r = ((glMultiTexCoord2d = (PFNGLMULTITEXCOORD2DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2d")) == nullptr) || r;
  r = ((glMultiTexCoord2dv = (PFNGLMULTITEXCOORD2DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2dv")) == nullptr) || r;
  r = ((glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2f")) == nullptr) || r;
  r = ((glMultiTexCoord2fv = (PFNGLMULTITEXCOORD2FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2fv")) == nullptr) || r;
  r = ((glMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2i")) == nullptr) || r;
  r = ((glMultiTexCoord2iv = (PFNGLMULTITEXCOORD2IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2iv")) == nullptr) || r;
  r = ((glMultiTexCoord2s = (PFNGLMULTITEXCOORD2SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2s")) == nullptr) || r;
  r = ((glMultiTexCoord2sv = (PFNGLMULTITEXCOORD2SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2sv")) == nullptr) || r;
  r = ((glMultiTexCoord3d = (PFNGLMULTITEXCOORD3DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3d")) == nullptr) || r;
  r = ((glMultiTexCoord3dv = (PFNGLMULTITEXCOORD3DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3dv")) == nullptr) || r;
  r = ((glMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3f")) == nullptr) || r;
  r = ((glMultiTexCoord3fv = (PFNGLMULTITEXCOORD3FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3fv")) == nullptr) || r;
  r = ((glMultiTexCoord3i = (PFNGLMULTITEXCOORD3IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3i")) == nullptr) || r;
  r = ((glMultiTexCoord3iv = (PFNGLMULTITEXCOORD3IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3iv")) == nullptr) || r;
  r = ((glMultiTexCoord3s = (PFNGLMULTITEXCOORD3SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3s")) == nullptr) || r;
  r = ((glMultiTexCoord3sv = (PFNGLMULTITEXCOORD3SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3sv")) == nullptr) || r;
  r = ((glMultiTexCoord4d = (PFNGLMULTITEXCOORD4DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4d")) == nullptr) || r;
  r = ((glMultiTexCoord4dv = (PFNGLMULTITEXCOORD4DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4dv")) == nullptr) || r;
  r = ((glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4f")) == nullptr) || r;
  r = ((glMultiTexCoord4fv = (PFNGLMULTITEXCOORD4FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4fv")) == nullptr) || r;
  r = ((glMultiTexCoord4i = (PFNGLMULTITEXCOORD4IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4i")) == nullptr) || r;
  r = ((glMultiTexCoord4iv = (PFNGLMULTITEXCOORD4IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4iv")) == nullptr) || r;
  r = ((glMultiTexCoord4s = (PFNGLMULTITEXCOORD4SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4s")) == nullptr) || r;
  r = ((glMultiTexCoord4sv = (PFNGLMULTITEXCOORD4SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4sv")) == nullptr) || r;
  r = ((glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)glewGetProcAddress((const GLubyte*)"glSampleCoverage")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_1_3 */

#ifdef GL_VERSION_1_4

static GLboolean _glewInit_GL_VERSION_1_4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendColor = (PFNGLBLENDCOLORPROC)glewGetProcAddress((const GLubyte*)"glBlendColor")) == nullptr) || r;
  r = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)glewGetProcAddress((const GLubyte*)"glBlendEquation")) == nullptr) || r;
  r = ((glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparate")) == nullptr) || r;
  r = ((glFogCoordPointer = (PFNGLFOGCOORDPOINTERPROC)glewGetProcAddress((const GLubyte*)"glFogCoordPointer")) == nullptr) || r;
  r = ((glFogCoordd = (PFNGLFOGCOORDDPROC)glewGetProcAddress((const GLubyte*)"glFogCoordd")) == nullptr) || r;
  r = ((glFogCoorddv = (PFNGLFOGCOORDDVPROC)glewGetProcAddress((const GLubyte*)"glFogCoorddv")) == nullptr) || r;
  r = ((glFogCoordf = (PFNGLFOGCOORDFPROC)glewGetProcAddress((const GLubyte*)"glFogCoordf")) == nullptr) || r;
  r = ((glFogCoordfv = (PFNGLFOGCOORDFVPROC)glewGetProcAddress((const GLubyte*)"glFogCoordfv")) == nullptr) || r;
  r = ((glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawArrays")) == nullptr) || r;
  r = ((glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElements")) == nullptr) || r;
  r = ((glPointParameterf = (PFNGLPOINTPARAMETERFPROC)glewGetProcAddress((const GLubyte*)"glPointParameterf")) == nullptr) || r;
  r = ((glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glPointParameterfv")) == nullptr) || r;
  r = ((glPointParameteri = (PFNGLPOINTPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glPointParameteri")) == nullptr) || r;
  r = ((glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glPointParameteriv")) == nullptr) || r;
  r = ((glSecondaryColor3b = (PFNGLSECONDARYCOLOR3BPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3b")) == nullptr) || r;
  r = ((glSecondaryColor3bv = (PFNGLSECONDARYCOLOR3BVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3bv")) == nullptr) || r;
  r = ((glSecondaryColor3d = (PFNGLSECONDARYCOLOR3DPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3d")) == nullptr) || r;
  r = ((glSecondaryColor3dv = (PFNGLSECONDARYCOLOR3DVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3dv")) == nullptr) || r;
  r = ((glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3f")) == nullptr) || r;
  r = ((glSecondaryColor3fv = (PFNGLSECONDARYCOLOR3FVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3fv")) == nullptr) || r;
  r = ((glSecondaryColor3i = (PFNGLSECONDARYCOLOR3IPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3i")) == nullptr) || r;
  r = ((glSecondaryColor3iv = (PFNGLSECONDARYCOLOR3IVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3iv")) == nullptr) || r;
  r = ((glSecondaryColor3s = (PFNGLSECONDARYCOLOR3SPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3s")) == nullptr) || r;
  r = ((glSecondaryColor3sv = (PFNGLSECONDARYCOLOR3SVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3sv")) == nullptr) || r;
  r = ((glSecondaryColor3ub = (PFNGLSECONDARYCOLOR3UBPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ub")) == nullptr) || r;
  r = ((glSecondaryColor3ubv = (PFNGLSECONDARYCOLOR3UBVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ubv")) == nullptr) || r;
  r = ((glSecondaryColor3ui = (PFNGLSECONDARYCOLOR3UIPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ui")) == nullptr) || r;
  r = ((glSecondaryColor3uiv = (PFNGLSECONDARYCOLOR3UIVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3uiv")) == nullptr) || r;
  r = ((glSecondaryColor3us = (PFNGLSECONDARYCOLOR3USPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3us")) == nullptr) || r;
  r = ((glSecondaryColor3usv = (PFNGLSECONDARYCOLOR3USVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3usv")) == nullptr) || r;
  r = ((glSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTERPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorPointer")) == nullptr) || r;
  r = ((glWindowPos2d = (PFNGLWINDOWPOS2DPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2d")) == nullptr) || r;
  r = ((glWindowPos2dv = (PFNGLWINDOWPOS2DVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2dv")) == nullptr) || r;
  r = ((glWindowPos2f = (PFNGLWINDOWPOS2FPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2f")) == nullptr) || r;
  r = ((glWindowPos2fv = (PFNGLWINDOWPOS2FVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2fv")) == nullptr) || r;
  r = ((glWindowPos2i = (PFNGLWINDOWPOS2IPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2i")) == nullptr) || r;
  r = ((glWindowPos2iv = (PFNGLWINDOWPOS2IVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2iv")) == nullptr) || r;
  r = ((glWindowPos2s = (PFNGLWINDOWPOS2SPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2s")) == nullptr) || r;
  r = ((glWindowPos2sv = (PFNGLWINDOWPOS2SVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2sv")) == nullptr) || r;
  r = ((glWindowPos3d = (PFNGLWINDOWPOS3DPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3d")) == nullptr) || r;
  r = ((glWindowPos3dv = (PFNGLWINDOWPOS3DVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3dv")) == nullptr) || r;
  r = ((glWindowPos3f = (PFNGLWINDOWPOS3FPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3f")) == nullptr) || r;
  r = ((glWindowPos3fv = (PFNGLWINDOWPOS3FVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3fv")) == nullptr) || r;
  r = ((glWindowPos3i = (PFNGLWINDOWPOS3IPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3i")) == nullptr) || r;
  r = ((glWindowPos3iv = (PFNGLWINDOWPOS3IVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3iv")) == nullptr) || r;
  r = ((glWindowPos3s = (PFNGLWINDOWPOS3SPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3s")) == nullptr) || r;
  r = ((glWindowPos3sv = (PFNGLWINDOWPOS3SVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3sv")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_1_4 */

#ifdef GL_VERSION_1_5

static GLboolean _glewInit_GL_VERSION_1_5 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginQuery = (PFNGLBEGINQUERYPROC)glewGetProcAddress((const GLubyte*)"glBeginQuery")) == nullptr) || r;
  r = ((glBindBuffer = (PFNGLBINDBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindBuffer")) == nullptr) || r;
  r = ((glBufferData = (PFNGLBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glBufferData")) == nullptr) || r;
  r = ((glBufferSubData = (PFNGLBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glBufferSubData")) == nullptr) || r;
  r = ((glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteBuffers")) == nullptr) || r;
  r = ((glDeleteQueries = (PFNGLDELETEQUERIESPROC)glewGetProcAddress((const GLubyte*)"glDeleteQueries")) == nullptr) || r;
  r = ((glEndQuery = (PFNGLENDQUERYPROC)glewGetProcAddress((const GLubyte*)"glEndQuery")) == nullptr) || r;
  r = ((glGenBuffers = (PFNGLGENBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glGenBuffers")) == nullptr) || r;
  r = ((glGenQueries = (PFNGLGENQUERIESPROC)glewGetProcAddress((const GLubyte*)"glGenQueries")) == nullptr) || r;
  r = ((glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetBufferParameteriv")) == nullptr) || r;
  r = ((glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)glewGetProcAddress((const GLubyte*)"glGetBufferPointerv")) == nullptr) || r;
  r = ((glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glGetBufferSubData")) == nullptr) || r;
  r = ((glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectiv")) == nullptr) || r;
  r = ((glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectuiv")) == nullptr) || r;
  r = ((glGetQueryiv = (PFNGLGETQUERYIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryiv")) == nullptr) || r;
  r = ((glIsBuffer = (PFNGLISBUFFERPROC)glewGetProcAddress((const GLubyte*)"glIsBuffer")) == nullptr) || r;
  r = ((glIsQuery = (PFNGLISQUERYPROC)glewGetProcAddress((const GLubyte*)"glIsQuery")) == nullptr) || r;
  r = ((glMapBuffer = (PFNGLMAPBUFFERPROC)glewGetProcAddress((const GLubyte*)"glMapBuffer")) == nullptr) || r;
  r = ((glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)glewGetProcAddress((const GLubyte*)"glUnmapBuffer")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_1_5 */

#ifdef GL_VERSION_2_0

static GLboolean _glewInit_GL_VERSION_2_0 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAttachShader = (PFNGLATTACHSHADERPROC)glewGetProcAddress((const GLubyte*)"glAttachShader")) == nullptr) || r;
  r = ((glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glBindAttribLocation")) == nullptr) || r;
  r = ((glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparate")) == nullptr) || r;
  r = ((glCompileShader = (PFNGLCOMPILESHADERPROC)glewGetProcAddress((const GLubyte*)"glCompileShader")) == nullptr) || r;
  r = ((glCreateProgram = (PFNGLCREATEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glCreateProgram")) == nullptr) || r;
  r = ((glCreateShader = (PFNGLCREATESHADERPROC)glewGetProcAddress((const GLubyte*)"glCreateShader")) == nullptr) || r;
  r = ((glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glDeleteProgram")) == nullptr) || r;
  r = ((glDeleteShader = (PFNGLDELETESHADERPROC)glewGetProcAddress((const GLubyte*)"glDeleteShader")) == nullptr) || r;
  r = ((glDetachShader = (PFNGLDETACHSHADERPROC)glewGetProcAddress((const GLubyte*)"glDetachShader")) == nullptr) || r;
  r = ((glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexAttribArray")) == nullptr) || r;
  r = ((glDrawBuffers = (PFNGLDRAWBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDrawBuffers")) == nullptr) || r;
  r = ((glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexAttribArray")) == nullptr) || r;
  r = ((glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)glewGetProcAddress((const GLubyte*)"glGetActiveAttrib")) == nullptr) || r;
  r = ((glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniform")) == nullptr) || r;
  r = ((glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)glewGetProcAddress((const GLubyte*)"glGetAttachedShaders")) == nullptr) || r;
  r = ((glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetAttribLocation")) == nullptr) || r;
  r = ((glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glewGetProcAddress((const GLubyte*)"glGetProgramInfoLog")) == nullptr) || r;
  r = ((glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramiv")) == nullptr) || r;
  r = ((glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glewGetProcAddress((const GLubyte*)"glGetShaderInfoLog")) == nullptr) || r;
  r = ((glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)glewGetProcAddress((const GLubyte*)"glGetShaderSource")) == nullptr) || r;
  r = ((glGetShaderiv = (PFNGLGETSHADERIVPROC)glewGetProcAddress((const GLubyte*)"glGetShaderiv")) == nullptr) || r;
  r = ((glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetUniformLocation")) == nullptr) || r;
  r = ((glGetUniformfv = (PFNGLGETUNIFORMFVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformfv")) == nullptr) || r;
  r = ((glGetUniformiv = (PFNGLGETUNIFORMIVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformiv")) == nullptr) || r;
  r = ((glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribPointerv")) == nullptr) || r;
  r = ((glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribdv")) == nullptr) || r;
  r = ((glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribfv")) == nullptr) || r;
  r = ((glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribiv")) == nullptr) || r;
  r = ((glIsProgram = (PFNGLISPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glIsProgram")) == nullptr) || r;
  r = ((glIsShader = (PFNGLISSHADERPROC)glewGetProcAddress((const GLubyte*)"glIsShader")) == nullptr) || r;
  r = ((glLinkProgram = (PFNGLLINKPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glLinkProgram")) == nullptr) || r;
  r = ((glShaderSource = (PFNGLSHADERSOURCEPROC)glewGetProcAddress((const GLubyte*)"glShaderSource")) == nullptr) || r;
  r = ((glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glStencilFuncSeparate")) == nullptr) || r;
  r = ((glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glStencilMaskSeparate")) == nullptr) || r;
  r = ((glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glStencilOpSeparate")) == nullptr) || r;
  r = ((glUniform1f = (PFNGLUNIFORM1FPROC)glewGetProcAddress((const GLubyte*)"glUniform1f")) == nullptr) || r;
  r = ((glUniform1fv = (PFNGLUNIFORM1FVPROC)glewGetProcAddress((const GLubyte*)"glUniform1fv")) == nullptr) || r;
  r = ((glUniform1i = (PFNGLUNIFORM1IPROC)glewGetProcAddress((const GLubyte*)"glUniform1i")) == nullptr) || r;
  r = ((glUniform1iv = (PFNGLUNIFORM1IVPROC)glewGetProcAddress((const GLubyte*)"glUniform1iv")) == nullptr) || r;
  r = ((glUniform2f = (PFNGLUNIFORM2FPROC)glewGetProcAddress((const GLubyte*)"glUniform2f")) == nullptr) || r;
  r = ((glUniform2fv = (PFNGLUNIFORM2FVPROC)glewGetProcAddress((const GLubyte*)"glUniform2fv")) == nullptr) || r;
  r = ((glUniform2i = (PFNGLUNIFORM2IPROC)glewGetProcAddress((const GLubyte*)"glUniform2i")) == nullptr) || r;
  r = ((glUniform2iv = (PFNGLUNIFORM2IVPROC)glewGetProcAddress((const GLubyte*)"glUniform2iv")) == nullptr) || r;
  r = ((glUniform3f = (PFNGLUNIFORM3FPROC)glewGetProcAddress((const GLubyte*)"glUniform3f")) == nullptr) || r;
  r = ((glUniform3fv = (PFNGLUNIFORM3FVPROC)glewGetProcAddress((const GLubyte*)"glUniform3fv")) == nullptr) || r;
  r = ((glUniform3i = (PFNGLUNIFORM3IPROC)glewGetProcAddress((const GLubyte*)"glUniform3i")) == nullptr) || r;
  r = ((glUniform3iv = (PFNGLUNIFORM3IVPROC)glewGetProcAddress((const GLubyte*)"glUniform3iv")) == nullptr) || r;
  r = ((glUniform4f = (PFNGLUNIFORM4FPROC)glewGetProcAddress((const GLubyte*)"glUniform4f")) == nullptr) || r;
  r = ((glUniform4fv = (PFNGLUNIFORM4FVPROC)glewGetProcAddress((const GLubyte*)"glUniform4fv")) == nullptr) || r;
  r = ((glUniform4i = (PFNGLUNIFORM4IPROC)glewGetProcAddress((const GLubyte*)"glUniform4i")) == nullptr) || r;
  r = ((glUniform4iv = (PFNGLUNIFORM4IVPROC)glewGetProcAddress((const GLubyte*)"glUniform4iv")) == nullptr) || r;
  r = ((glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2fv")) == nullptr) || r;
  r = ((glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3fv")) == nullptr) || r;
  r = ((glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4fv")) == nullptr) || r;
  r = ((glUseProgram = (PFNGLUSEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glUseProgram")) == nullptr) || r;
  r = ((glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glValidateProgram")) == nullptr) || r;
  r = ((glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1d")) == nullptr) || r;
  r = ((glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1dv")) == nullptr) || r;
  r = ((glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1f")) == nullptr) || r;
  r = ((glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1fv")) == nullptr) || r;
  r = ((glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1s")) == nullptr) || r;
  r = ((glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1sv")) == nullptr) || r;
  r = ((glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2d")) == nullptr) || r;
  r = ((glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2dv")) == nullptr) || r;
  r = ((glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2f")) == nullptr) || r;
  r = ((glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2fv")) == nullptr) || r;
  r = ((glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2s")) == nullptr) || r;
  r = ((glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2sv")) == nullptr) || r;
  r = ((glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3d")) == nullptr) || r;
  r = ((glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3dv")) == nullptr) || r;
  r = ((glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3f")) == nullptr) || r;
  r = ((glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3fv")) == nullptr) || r;
  r = ((glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3s")) == nullptr) || r;
  r = ((glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3sv")) == nullptr) || r;
  r = ((glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nbv")) == nullptr) || r;
  r = ((glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Niv")) == nullptr) || r;
  r = ((glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nsv")) == nullptr) || r;
  r = ((glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nub")) == nullptr) || r;
  r = ((glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nubv")) == nullptr) || r;
  r = ((glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nuiv")) == nullptr) || r;
  r = ((glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nusv")) == nullptr) || r;
  r = ((glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4bv")) == nullptr) || r;
  r = ((glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4d")) == nullptr) || r;
  r = ((glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4dv")) == nullptr) || r;
  r = ((glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4f")) == nullptr) || r;
  r = ((glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4fv")) == nullptr) || r;
  r = ((glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4iv")) == nullptr) || r;
  r = ((glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4s")) == nullptr) || r;
  r = ((glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4sv")) == nullptr) || r;
  r = ((glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4ubv")) == nullptr) || r;
  r = ((glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4uiv")) == nullptr) || r;
  r = ((glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4usv")) == nullptr) || r;
  r = ((glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribPointer")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_2_0 */

#ifdef GL_VERSION_2_1

static GLboolean _glewInit_GL_VERSION_2_1 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2x3fv")) == nullptr) || r;
  r = ((glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2x4fv")) == nullptr) || r;
  r = ((glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3x2fv")) == nullptr) || r;
  r = ((glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3x4fv")) == nullptr) || r;
  r = ((glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4x2fv")) == nullptr) || r;
  r = ((glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4x3fv")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_2_1 */

#ifdef GL_VERSION_3_0

static GLboolean _glewInit_GL_VERSION_3_0 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC)glewGetProcAddress((const GLubyte*)"glBeginConditionalRender")) == nullptr) || r;
  r = ((glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glBeginTransformFeedback")) == nullptr) || r;
  r = ((glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)glewGetProcAddress((const GLubyte*)"glBindFragDataLocation")) == nullptr) || r;
  r = ((glClampColor = (PFNGLCLAMPCOLORPROC)glewGetProcAddress((const GLubyte*)"glClampColor")) == nullptr) || r;
  r = ((glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)glewGetProcAddress((const GLubyte*)"glClearBufferfi")) == nullptr) || r;
  r = ((glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)glewGetProcAddress((const GLubyte*)"glClearBufferfv")) == nullptr) || r;
  r = ((glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)glewGetProcAddress((const GLubyte*)"glClearBufferiv")) == nullptr) || r;
  r = ((glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)glewGetProcAddress((const GLubyte*)"glClearBufferuiv")) == nullptr) || r;
  r = ((glColorMaski = (PFNGLCOLORMASKIPROC)glewGetProcAddress((const GLubyte*)"glColorMaski")) == nullptr) || r;
  r = ((glDisablei = (PFNGLDISABLEIPROC)glewGetProcAddress((const GLubyte*)"glDisablei")) == nullptr) || r;
  r = ((glEnablei = (PFNGLENABLEIPROC)glewGetProcAddress((const GLubyte*)"glEnablei")) == nullptr) || r;
  r = ((glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC)glewGetProcAddress((const GLubyte*)"glEndConditionalRender")) == nullptr) || r;
  r = ((glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glEndTransformFeedback")) == nullptr) || r;
  r = ((glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)glewGetProcAddress((const GLubyte*)"glGetBooleani_v")) == nullptr) || r;
  r = ((glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetFragDataLocation")) == nullptr) || r;
  r = ((glGetStringi = (PFNGLGETSTRINGIPROC)glewGetProcAddress((const GLubyte*)"glGetStringi")) == nullptr) || r;
  r = ((glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterIiv")) == nullptr) || r;
  r = ((glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterIuiv")) == nullptr) || r;
  r = ((glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbackVarying")) == nullptr) || r;
  r = ((glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformuiv")) == nullptr) || r;
  r = ((glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIiv")) == nullptr) || r;
  r = ((glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIuiv")) == nullptr) || r;
  r = ((glIsEnabledi = (PFNGLISENABLEDIPROC)glewGetProcAddress((const GLubyte*)"glIsEnabledi")) == nullptr) || r;
  r = ((glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glTexParameterIiv")) == nullptr) || r;
  r = ((glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glTexParameterIuiv")) == nullptr) || r;
  r = ((glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackVaryings")) == nullptr) || r;
  r = ((glUniform1ui = (PFNGLUNIFORM1UIPROC)glewGetProcAddress((const GLubyte*)"glUniform1ui")) == nullptr) || r;
  r = ((glUniform1uiv = (PFNGLUNIFORM1UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform1uiv")) == nullptr) || r;
  r = ((glUniform2ui = (PFNGLUNIFORM2UIPROC)glewGetProcAddress((const GLubyte*)"glUniform2ui")) == nullptr) || r;
  r = ((glUniform2uiv = (PFNGLUNIFORM2UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform2uiv")) == nullptr) || r;
  r = ((glUniform3ui = (PFNGLUNIFORM3UIPROC)glewGetProcAddress((const GLubyte*)"glUniform3ui")) == nullptr) || r;
  r = ((glUniform3uiv = (PFNGLUNIFORM3UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform3uiv")) == nullptr) || r;
  r = ((glUniform4ui = (PFNGLUNIFORM4UIPROC)glewGetProcAddress((const GLubyte*)"glUniform4ui")) == nullptr) || r;
  r = ((glUniform4uiv = (PFNGLUNIFORM4UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform4uiv")) == nullptr) || r;
  r = ((glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1i")) == nullptr) || r;
  r = ((glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1iv")) == nullptr) || r;
  r = ((glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1ui")) == nullptr) || r;
  r = ((glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1uiv")) == nullptr) || r;
  r = ((glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2i")) == nullptr) || r;
  r = ((glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2iv")) == nullptr) || r;
  r = ((glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2ui")) == nullptr) || r;
  r = ((glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2uiv")) == nullptr) || r;
  r = ((glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3i")) == nullptr) || r;
  r = ((glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3iv")) == nullptr) || r;
  r = ((glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3ui")) == nullptr) || r;
  r = ((glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3uiv")) == nullptr) || r;
  r = ((glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4bv")) == nullptr) || r;
  r = ((glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4i")) == nullptr) || r;
  r = ((glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4iv")) == nullptr) || r;
  r = ((glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4sv")) == nullptr) || r;
  r = ((glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ubv")) == nullptr) || r;
  r = ((glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ui")) == nullptr) || r;
  r = ((glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4uiv")) == nullptr) || r;
  r = ((glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4usv")) == nullptr) || r;
  r = ((glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribIPointer")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_3_0 */

#ifdef GL_VERSION_3_1

static GLboolean _glewInit_GL_VERSION_3_1 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysInstanced")) == nullptr) || r;
  r = ((glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstanced")) == nullptr) || r;
  r = ((glPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEXPROC)glewGetProcAddress((const GLubyte*)"glPrimitiveRestartIndex")) == nullptr) || r;
  r = ((glTexBuffer = (PFNGLTEXBUFFERPROC)glewGetProcAddress((const GLubyte*)"glTexBuffer")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_3_1 */

#ifdef GL_VERSION_3_2

static GLboolean _glewInit_GL_VERSION_3_2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture")) == nullptr) || r;
  r = ((glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)glewGetProcAddress((const GLubyte*)"glGetBufferParameteri64v")) == nullptr) || r;
  r = ((glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)glewGetProcAddress((const GLubyte*)"glGetInteger64i_v")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_3_2 */

#ifdef GL_VERSION_3_3

static GLboolean _glewInit_GL_VERSION_3_3 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribDivisor")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_3_3 */

#ifdef GL_VERSION_4_0

static GLboolean _glewInit_GL_VERSION_4_0 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparatei")) == nullptr) || r;
  r = ((glBlendEquationi = (PFNGLBLENDEQUATIONIPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationi")) == nullptr) || r;
  r = ((glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparatei")) == nullptr) || r;
  r = ((glBlendFunci = (PFNGLBLENDFUNCIPROC)glewGetProcAddress((const GLubyte*)"glBlendFunci")) == nullptr) || r;
  r = ((glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC)glewGetProcAddress((const GLubyte*)"glMinSampleShading")) == nullptr) || r;

  return r;
}

#endif /* GL_VERSION_4_0 */

#ifdef GL_VERSION_4_1

#endif /* GL_VERSION_4_1 */

#ifdef GL_VERSION_4_2

#endif /* GL_VERSION_4_2 */

#ifdef GL_3DFX_multisample

#endif /* GL_3DFX_multisample */

#ifdef GL_3DFX_tbuffer

static GLboolean _glewInit_GL_3DFX_tbuffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTbufferMask3DFX = (PFNGLTBUFFERMASK3DFXPROC)glewGetProcAddress((const GLubyte*)"glTbufferMask3DFX")) == nullptr) || r;

  return r;
}

#endif /* GL_3DFX_tbuffer */

#ifdef GL_3DFX_texture_compression_FXT1

#endif /* GL_3DFX_texture_compression_FXT1 */

#ifdef GL_AMD_blend_minmax_factor

#endif /* GL_AMD_blend_minmax_factor */

#ifdef GL_AMD_conservative_depth

#endif /* GL_AMD_conservative_depth */

#ifdef GL_AMD_debug_output

static GLboolean _glewInit_GL_AMD_debug_output (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageCallbackAMD")) == nullptr) || r;
  r = ((glDebugMessageEnableAMD = (PFNGLDEBUGMESSAGEENABLEAMDPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageEnableAMD")) == nullptr) || r;
  r = ((glDebugMessageInsertAMD = (PFNGLDEBUGMESSAGEINSERTAMDPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageInsertAMD")) == nullptr) || r;
  r = ((glGetDebugMessageLogAMD = (PFNGLGETDEBUGMESSAGELOGAMDPROC)glewGetProcAddress((const GLubyte*)"glGetDebugMessageLogAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_debug_output */

#ifdef GL_AMD_depth_clamp_separate

#endif /* GL_AMD_depth_clamp_separate */

#ifdef GL_AMD_draw_buffers_blend

static GLboolean _glewInit_GL_AMD_draw_buffers_blend (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationIndexedAMD = (PFNGLBLENDEQUATIONINDEXEDAMDPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationIndexedAMD")) == nullptr) || r;
  r = ((glBlendEquationSeparateIndexedAMD = (PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparateIndexedAMD")) == nullptr) || r;
  r = ((glBlendFuncIndexedAMD = (PFNGLBLENDFUNCINDEXEDAMDPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncIndexedAMD")) == nullptr) || r;
  r = ((glBlendFuncSeparateIndexedAMD = (PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparateIndexedAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_draw_buffers_blend */

#ifdef GL_AMD_multi_draw_indirect

static GLboolean _glewInit_GL_AMD_multi_draw_indirect (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMultiDrawArraysIndirectAMD = (PFNGLMULTIDRAWARRAYSINDIRECTAMDPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawArraysIndirectAMD")) == nullptr) || r;
  r = ((glMultiDrawElementsIndirectAMD = (PFNGLMULTIDRAWELEMENTSINDIRECTAMDPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElementsIndirectAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_multi_draw_indirect */

#ifdef GL_AMD_name_gen_delete

static GLboolean _glewInit_GL_AMD_name_gen_delete (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDeleteNamesAMD = (PFNGLDELETENAMESAMDPROC)glewGetProcAddress((const GLubyte*)"glDeleteNamesAMD")) == nullptr) || r;
  r = ((glGenNamesAMD = (PFNGLGENNAMESAMDPROC)glewGetProcAddress((const GLubyte*)"glGenNamesAMD")) == nullptr) || r;
  r = ((glIsNameAMD = (PFNGLISNAMEAMDPROC)glewGetProcAddress((const GLubyte*)"glIsNameAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_name_gen_delete */

#ifdef GL_AMD_performance_monitor

static GLboolean _glewInit_GL_AMD_performance_monitor (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC)glewGetProcAddress((const GLubyte*)"glBeginPerfMonitorAMD")) == nullptr) || r;
  r = ((glDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC)glewGetProcAddress((const GLubyte*)"glDeletePerfMonitorsAMD")) == nullptr) || r;
  r = ((glEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC)glewGetProcAddress((const GLubyte*)"glEndPerfMonitorAMD")) == nullptr) || r;
  r = ((glGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC)glewGetProcAddress((const GLubyte*)"glGenPerfMonitorsAMD")) == nullptr) || r;
  r = ((glGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC)glewGetProcAddress((const GLubyte*)"glGetPerfMonitorCounterDataAMD")) == nullptr) || r;
  r = ((glGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC)glewGetProcAddress((const GLubyte*)"glGetPerfMonitorCounterInfoAMD")) == nullptr) || r;
  r = ((glGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC)glewGetProcAddress((const GLubyte*)"glGetPerfMonitorCounterStringAMD")) == nullptr) || r;
  r = ((glGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC)glewGetProcAddress((const GLubyte*)"glGetPerfMonitorCountersAMD")) == nullptr) || r;
  r = ((glGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC)glewGetProcAddress((const GLubyte*)"glGetPerfMonitorGroupStringAMD")) == nullptr) || r;
  r = ((glGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC)glewGetProcAddress((const GLubyte*)"glGetPerfMonitorGroupsAMD")) == nullptr) || r;
  r = ((glSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC)glewGetProcAddress((const GLubyte*)"glSelectPerfMonitorCountersAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_performance_monitor */

#ifdef GL_AMD_sample_positions

static GLboolean _glewInit_GL_AMD_sample_positions (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSetMultisamplefvAMD = (PFNGLSETMULTISAMPLEFVAMDPROC)glewGetProcAddress((const GLubyte*)"glSetMultisamplefvAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_sample_positions */

#ifdef GL_AMD_seamless_cubemap_per_texture

#endif /* GL_AMD_seamless_cubemap_per_texture */

#ifdef GL_AMD_shader_stencil_export

#endif /* GL_AMD_shader_stencil_export */

#ifdef GL_AMD_texture_texture4

#endif /* GL_AMD_texture_texture4 */

#ifdef GL_AMD_transform_feedback3_lines_triangles

#endif /* GL_AMD_transform_feedback3_lines_triangles */

#ifdef GL_AMD_vertex_shader_tessellator

static GLboolean _glewInit_GL_AMD_vertex_shader_tessellator (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTessellationFactorAMD = (PFNGLTESSELLATIONFACTORAMDPROC)glewGetProcAddress((const GLubyte*)"glTessellationFactorAMD")) == nullptr) || r;
  r = ((glTessellationModeAMD = (PFNGLTESSELLATIONMODEAMDPROC)glewGetProcAddress((const GLubyte*)"glTessellationModeAMD")) == nullptr) || r;

  return r;
}

#endif /* GL_AMD_vertex_shader_tessellator */

#ifdef GL_APPLE_aux_depth_stencil

#endif /* GL_APPLE_aux_depth_stencil */

#ifdef GL_APPLE_client_storage

#endif /* GL_APPLE_client_storage */

#ifdef GL_APPLE_element_array

static GLboolean _glewInit_GL_APPLE_element_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawElementArrayAPPLE = (PFNGLDRAWELEMENTARRAYAPPLEPROC)glewGetProcAddress((const GLubyte*)"glDrawElementArrayAPPLE")) == nullptr) || r;
  r = ((glDrawRangeElementArrayAPPLE = (PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElementArrayAPPLE")) == nullptr) || r;
  r = ((glElementPointerAPPLE = (PFNGLELEMENTPOINTERAPPLEPROC)glewGetProcAddress((const GLubyte*)"glElementPointerAPPLE")) == nullptr) || r;
  r = ((glMultiDrawElementArrayAPPLE = (PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElementArrayAPPLE")) == nullptr) || r;
  r = ((glMultiDrawRangeElementArrayAPPLE = (PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawRangeElementArrayAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_element_array */

#ifdef GL_APPLE_fence

static GLboolean _glewInit_GL_APPLE_fence (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDeleteFencesAPPLE = (PFNGLDELETEFENCESAPPLEPROC)glewGetProcAddress((const GLubyte*)"glDeleteFencesAPPLE")) == nullptr) || r;
  r = ((glFinishFenceAPPLE = (PFNGLFINISHFENCEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glFinishFenceAPPLE")) == nullptr) || r;
  r = ((glFinishObjectAPPLE = (PFNGLFINISHOBJECTAPPLEPROC)glewGetProcAddress((const GLubyte*)"glFinishObjectAPPLE")) == nullptr) || r;
  r = ((glGenFencesAPPLE = (PFNGLGENFENCESAPPLEPROC)glewGetProcAddress((const GLubyte*)"glGenFencesAPPLE")) == nullptr) || r;
  r = ((glIsFenceAPPLE = (PFNGLISFENCEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glIsFenceAPPLE")) == nullptr) || r;
  r = ((glSetFenceAPPLE = (PFNGLSETFENCEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glSetFenceAPPLE")) == nullptr) || r;
  r = ((glTestFenceAPPLE = (PFNGLTESTFENCEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glTestFenceAPPLE")) == nullptr) || r;
  r = ((glTestObjectAPPLE = (PFNGLTESTOBJECTAPPLEPROC)glewGetProcAddress((const GLubyte*)"glTestObjectAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_fence */

#ifdef GL_APPLE_float_pixels

#endif /* GL_APPLE_float_pixels */

#ifdef GL_APPLE_flush_buffer_range

static GLboolean _glewInit_GL_APPLE_flush_buffer_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBufferParameteriAPPLE = (PFNGLBUFFERPARAMETERIAPPLEPROC)glewGetProcAddress((const GLubyte*)"glBufferParameteriAPPLE")) == nullptr) || r;
  r = ((glFlushMappedBufferRangeAPPLE = (PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glFlushMappedBufferRangeAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_flush_buffer_range */

#ifdef GL_APPLE_object_purgeable

static GLboolean _glewInit_GL_APPLE_object_purgeable (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetObjectParameterivAPPLE = (PFNGLGETOBJECTPARAMETERIVAPPLEPROC)glewGetProcAddress((const GLubyte*)"glGetObjectParameterivAPPLE")) == nullptr) || r;
  r = ((glObjectPurgeableAPPLE = (PFNGLOBJECTPURGEABLEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glObjectPurgeableAPPLE")) == nullptr) || r;
  r = ((glObjectUnpurgeableAPPLE = (PFNGLOBJECTUNPURGEABLEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glObjectUnpurgeableAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_object_purgeable */

#ifdef GL_APPLE_pixel_buffer

#endif /* GL_APPLE_pixel_buffer */

#ifdef GL_APPLE_rgb_422

#endif /* GL_APPLE_rgb_422 */

#ifdef GL_APPLE_row_bytes

#endif /* GL_APPLE_row_bytes */

#ifdef GL_APPLE_specular_vector

#endif /* GL_APPLE_specular_vector */

#ifdef GL_APPLE_texture_range

static GLboolean _glewInit_GL_APPLE_texture_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetTexParameterPointervAPPLE = (PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterPointervAPPLE")) == nullptr) || r;
  r = ((glTextureRangeAPPLE = (PFNGLTEXTURERANGEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glTextureRangeAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_texture_range */

#ifdef GL_APPLE_transform_hint

#endif /* GL_APPLE_transform_hint */

#ifdef GL_APPLE_vertex_array_object

static GLboolean _glewInit_GL_APPLE_vertex_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindVertexArrayAPPLE = (PFNGLBINDVERTEXARRAYAPPLEPROC)glewGetProcAddress((const GLubyte*)"glBindVertexArrayAPPLE")) == nullptr) || r;
  r = ((glDeleteVertexArraysAPPLE = (PFNGLDELETEVERTEXARRAYSAPPLEPROC)glewGetProcAddress((const GLubyte*)"glDeleteVertexArraysAPPLE")) == nullptr) || r;
  r = ((glGenVertexArraysAPPLE = (PFNGLGENVERTEXARRAYSAPPLEPROC)glewGetProcAddress((const GLubyte*)"glGenVertexArraysAPPLE")) == nullptr) || r;
  r = ((glIsVertexArrayAPPLE = (PFNGLISVERTEXARRAYAPPLEPROC)glewGetProcAddress((const GLubyte*)"glIsVertexArrayAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_vertex_array_object */

#ifdef GL_APPLE_vertex_array_range

static GLboolean _glewInit_GL_APPLE_vertex_array_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushVertexArrayRangeAPPLE = (PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glFlushVertexArrayRangeAPPLE")) == nullptr) || r;
  r = ((glVertexArrayParameteriAPPLE = (PFNGLVERTEXARRAYPARAMETERIAPPLEPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayParameteriAPPLE")) == nullptr) || r;
  r = ((glVertexArrayRangeAPPLE = (PFNGLVERTEXARRAYRANGEAPPLEPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayRangeAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_vertex_array_range */

#ifdef GL_APPLE_vertex_program_evaluators

static GLboolean _glewInit_GL_APPLE_vertex_program_evaluators (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDisableVertexAttribAPPLE = (PFNGLDISABLEVERTEXATTRIBAPPLEPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexAttribAPPLE")) == nullptr) || r;
  r = ((glEnableVertexAttribAPPLE = (PFNGLENABLEVERTEXATTRIBAPPLEPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexAttribAPPLE")) == nullptr) || r;
  r = ((glIsVertexAttribEnabledAPPLE = (PFNGLISVERTEXATTRIBENABLEDAPPLEPROC)glewGetProcAddress((const GLubyte*)"glIsVertexAttribEnabledAPPLE")) == nullptr) || r;
  r = ((glMapVertexAttrib1dAPPLE = (PFNGLMAPVERTEXATTRIB1DAPPLEPROC)glewGetProcAddress((const GLubyte*)"glMapVertexAttrib1dAPPLE")) == nullptr) || r;
  r = ((glMapVertexAttrib1fAPPLE = (PFNGLMAPVERTEXATTRIB1FAPPLEPROC)glewGetProcAddress((const GLubyte*)"glMapVertexAttrib1fAPPLE")) == nullptr) || r;
  r = ((glMapVertexAttrib2dAPPLE = (PFNGLMAPVERTEXATTRIB2DAPPLEPROC)glewGetProcAddress((const GLubyte*)"glMapVertexAttrib2dAPPLE")) == nullptr) || r;
  r = ((glMapVertexAttrib2fAPPLE = (PFNGLMAPVERTEXATTRIB2FAPPLEPROC)glewGetProcAddress((const GLubyte*)"glMapVertexAttrib2fAPPLE")) == nullptr) || r;

  return r;
}

#endif /* GL_APPLE_vertex_program_evaluators */

#ifdef GL_APPLE_ycbcr_422

#endif /* GL_APPLE_ycbcr_422 */

#ifdef GL_ARB_ES2_compatibility

static GLboolean _glewInit_GL_ARB_ES2_compatibility (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearDepthf = (PFNGLCLEARDEPTHFPROC)glewGetProcAddress((const GLubyte*)"glClearDepthf")) == nullptr) || r;
  r = ((glDepthRangef = (PFNGLDEPTHRANGEFPROC)glewGetProcAddress((const GLubyte*)"glDepthRangef")) == nullptr) || r;
  r = ((glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)glewGetProcAddress((const GLubyte*)"glGetShaderPrecisionFormat")) == nullptr) || r;
  r = ((glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)glewGetProcAddress((const GLubyte*)"glReleaseShaderCompiler")) == nullptr) || r;
  r = ((glShaderBinary = (PFNGLSHADERBINARYPROC)glewGetProcAddress((const GLubyte*)"glShaderBinary")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_ES2_compatibility */

#ifdef GL_ARB_base_instance

static GLboolean _glewInit_GL_ARB_base_instance (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstancedBaseInstance = (PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysInstancedBaseInstance")) == nullptr) || r;
  r = ((glDrawElementsInstancedBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedBaseInstance")) == nullptr) || r;
  r = ((glDrawElementsInstancedBaseVertexBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedBaseVertexBaseInstance")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_base_instance */

#ifdef GL_ARB_blend_func_extended

static GLboolean _glewInit_GL_ARB_blend_func_extended (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)glewGetProcAddress((const GLubyte*)"glBindFragDataLocationIndexed")) == nullptr) || r;
  r = ((glGetFragDataIndex = (PFNGLGETFRAGDATAINDEXPROC)glewGetProcAddress((const GLubyte*)"glGetFragDataIndex")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_blend_func_extended */

#ifdef GL_ARB_cl_event

static GLboolean _glewInit_GL_ARB_cl_event (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCreateSyncFromCLeventARB = (PFNGLCREATESYNCFROMCLEVENTARBPROC)glewGetProcAddress((const GLubyte*)"glCreateSyncFromCLeventARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_cl_event */

#ifdef GL_ARB_color_buffer_float

static GLboolean _glewInit_GL_ARB_color_buffer_float (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClampColorARB = (PFNGLCLAMPCOLORARBPROC)glewGetProcAddress((const GLubyte*)"glClampColorARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_color_buffer_float */

#ifdef GL_ARB_compatibility

#endif /* GL_ARB_compatibility */

#ifdef GL_ARB_compressed_texture_pixel_storage

#endif /* GL_ARB_compressed_texture_pixel_storage */

#ifdef GL_ARB_conservative_depth

#endif /* GL_ARB_conservative_depth */

#ifdef GL_ARB_copy_buffer

static GLboolean _glewInit_GL_ARB_copy_buffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glCopyBufferSubData")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_copy_buffer */

#ifdef GL_ARB_debug_output

static GLboolean _glewInit_GL_ARB_debug_output (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageCallbackARB")) == nullptr) || r;
  r = ((glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageControlARB")) == nullptr) || r;
  r = ((glDebugMessageInsertARB = (PFNGLDEBUGMESSAGEINSERTARBPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageInsertARB")) == nullptr) || r;
  r = ((glGetDebugMessageLogARB = (PFNGLGETDEBUGMESSAGELOGARBPROC)glewGetProcAddress((const GLubyte*)"glGetDebugMessageLogARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_debug_output */

#ifdef GL_ARB_depth_buffer_float

#endif /* GL_ARB_depth_buffer_float */

#ifdef GL_ARB_depth_clamp

#endif /* GL_ARB_depth_clamp */

#ifdef GL_ARB_depth_texture

#endif /* GL_ARB_depth_texture */

#ifdef GL_ARB_draw_buffers

static GLboolean _glewInit_GL_ARB_draw_buffers (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawBuffersARB = (PFNGLDRAWBUFFERSARBPROC)glewGetProcAddress((const GLubyte*)"glDrawBuffersARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_draw_buffers */

#ifdef GL_ARB_draw_buffers_blend

static GLboolean _glewInit_GL_ARB_draw_buffers_blend (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationSeparateiARB = (PFNGLBLENDEQUATIONSEPARATEIARBPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparateiARB")) == nullptr) || r;
  r = ((glBlendEquationiARB = (PFNGLBLENDEQUATIONIARBPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationiARB")) == nullptr) || r;
  r = ((glBlendFuncSeparateiARB = (PFNGLBLENDFUNCSEPARATEIARBPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparateiARB")) == nullptr) || r;
  r = ((glBlendFunciARB = (PFNGLBLENDFUNCIARBPROC)glewGetProcAddress((const GLubyte*)"glBlendFunciARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_draw_buffers_blend */

#ifdef GL_ARB_draw_elements_base_vertex

static GLboolean _glewInit_GL_ARB_draw_elements_base_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsBaseVertex")) == nullptr) || r;
  r = ((glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedBaseVertex")) == nullptr) || r;
  r = ((glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElementsBaseVertex")) == nullptr) || r;
  r = ((glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElementsBaseVertex")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_draw_elements_base_vertex */

#ifdef GL_ARB_draw_indirect

static GLboolean _glewInit_GL_ARB_draw_indirect (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysIndirect")) == nullptr) || r;
  r = ((glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsIndirect")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_draw_indirect */

#ifdef GL_ARB_draw_instanced

#endif /* GL_ARB_draw_instanced */

#ifdef GL_ARB_explicit_attrib_location

#endif /* GL_ARB_explicit_attrib_location */

#ifdef GL_ARB_fragment_coord_conventions

#endif /* GL_ARB_fragment_coord_conventions */

#ifdef GL_ARB_fragment_program

#endif /* GL_ARB_fragment_program */

#ifdef GL_ARB_fragment_program_shadow

#endif /* GL_ARB_fragment_program_shadow */

#ifdef GL_ARB_fragment_shader

#endif /* GL_ARB_fragment_shader */

#ifdef GL_ARB_framebuffer_object

static GLboolean _glewInit_GL_ARB_framebuffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindFramebuffer")) == nullptr) || r;
  r = ((glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindRenderbuffer")) == nullptr) || r;
  r = ((glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBlitFramebuffer")) == nullptr) || r;
  r = ((glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glewGetProcAddress((const GLubyte*)"glCheckFramebufferStatus")) == nullptr) || r;
  r = ((glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteFramebuffers")) == nullptr) || r;
  r = ((glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteRenderbuffers")) == nullptr) || r;
  r = ((glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glFramebufferRenderbuffer")) == nullptr) || r;
  r = ((glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture1D")) == nullptr) || r;
  r = ((glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture2D")) == nullptr) || r;
  r = ((glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture3D")) == nullptr) || r;
  r = ((glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureLayer")) == nullptr) || r;
  r = ((glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glGenFramebuffers")) == nullptr) || r;
  r = ((glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glGenRenderbuffers")) == nullptr) || r;
  r = ((glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)glewGetProcAddress((const GLubyte*)"glGenerateMipmap")) == nullptr) || r;
  r = ((glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetFramebufferAttachmentParameteriv")) == nullptr) || r;
  r = ((glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetRenderbufferParameteriv")) == nullptr) || r;
  r = ((glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glIsFramebuffer")) == nullptr) || r;
  r = ((glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glIsRenderbuffer")) == nullptr) || r;
  r = ((glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorage")) == nullptr) || r;
  r = ((glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisample")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_framebuffer_object */

#ifdef GL_ARB_framebuffer_sRGB

#endif /* GL_ARB_framebuffer_sRGB */

#ifdef GL_ARB_geometry_shader4

static GLboolean _glewInit_GL_ARB_geometry_shader4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureARB = (PFNGLFRAMEBUFFERTEXTUREARBPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureARB")) == nullptr) || r;
  r = ((glFramebufferTextureFaceARB = (PFNGLFRAMEBUFFERTEXTUREFACEARBPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureFaceARB")) == nullptr) || r;
  r = ((glFramebufferTextureLayerARB = (PFNGLFRAMEBUFFERTEXTURELAYERARBPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureLayerARB")) == nullptr) || r;
  r = ((glProgramParameteriARB = (PFNGLPROGRAMPARAMETERIARBPROC)glewGetProcAddress((const GLubyte*)"glProgramParameteriARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_geometry_shader4 */

#ifdef GL_ARB_get_program_binary

static GLboolean _glewInit_GL_ARB_get_program_binary (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)glewGetProcAddress((const GLubyte*)"glGetProgramBinary")) == nullptr) || r;
  r = ((glProgramBinary = (PFNGLPROGRAMBINARYPROC)glewGetProcAddress((const GLubyte*)"glProgramBinary")) == nullptr) || r;
  r = ((glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glProgramParameteri")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_get_program_binary */

#ifdef GL_ARB_gpu_shader5

#endif /* GL_ARB_gpu_shader5 */

#ifdef GL_ARB_gpu_shader_fp64

static GLboolean _glewInit_GL_ARB_gpu_shader_fp64 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetUniformdv = (PFNGLGETUNIFORMDVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformdv")) == nullptr) || r;
  r = ((glProgramUniform1dEXT = (PFNGLPROGRAMUNIFORM1DEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1dEXT")) == nullptr) || r;
  r = ((glProgramUniform1dvEXT = (PFNGLPROGRAMUNIFORM1DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1dvEXT")) == nullptr) || r;
  r = ((glProgramUniform2dEXT = (PFNGLPROGRAMUNIFORM2DEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2dEXT")) == nullptr) || r;
  r = ((glProgramUniform2dvEXT = (PFNGLPROGRAMUNIFORM2DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2dvEXT")) == nullptr) || r;
  r = ((glProgramUniform3dEXT = (PFNGLPROGRAMUNIFORM3DEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3dEXT")) == nullptr) || r;
  r = ((glProgramUniform3dvEXT = (PFNGLPROGRAMUNIFORM3DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3dvEXT")) == nullptr) || r;
  r = ((glProgramUniform4dEXT = (PFNGLPROGRAMUNIFORM4DEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4dEXT")) == nullptr) || r;
  r = ((glProgramUniform4dvEXT = (PFNGLPROGRAMUNIFORM4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix2dvEXT = (PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x3dvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x3dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x4dvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x4dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix3dvEXT = (PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x2dvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x2dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x4dvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x4dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix4dvEXT = (PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x2dvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x2dvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x3dvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x3dvEXT")) == nullptr) || r;
  r = ((glUniform1d = (PFNGLUNIFORM1DPROC)glewGetProcAddress((const GLubyte*)"glUniform1d")) == nullptr) || r;
  r = ((glUniform1dv = (PFNGLUNIFORM1DVPROC)glewGetProcAddress((const GLubyte*)"glUniform1dv")) == nullptr) || r;
  r = ((glUniform2d = (PFNGLUNIFORM2DPROC)glewGetProcAddress((const GLubyte*)"glUniform2d")) == nullptr) || r;
  r = ((glUniform2dv = (PFNGLUNIFORM2DVPROC)glewGetProcAddress((const GLubyte*)"glUniform2dv")) == nullptr) || r;
  r = ((glUniform3d = (PFNGLUNIFORM3DPROC)glewGetProcAddress((const GLubyte*)"glUniform3d")) == nullptr) || r;
  r = ((glUniform3dv = (PFNGLUNIFORM3DVPROC)glewGetProcAddress((const GLubyte*)"glUniform3dv")) == nullptr) || r;
  r = ((glUniform4d = (PFNGLUNIFORM4DPROC)glewGetProcAddress((const GLubyte*)"glUniform4d")) == nullptr) || r;
  r = ((glUniform4dv = (PFNGLUNIFORM4DVPROC)glewGetProcAddress((const GLubyte*)"glUniform4dv")) == nullptr) || r;
  r = ((glUniformMatrix2dv = (PFNGLUNIFORMMATRIX2DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2dv")) == nullptr) || r;
  r = ((glUniformMatrix2x3dv = (PFNGLUNIFORMMATRIX2X3DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2x3dv")) == nullptr) || r;
  r = ((glUniformMatrix2x4dv = (PFNGLUNIFORMMATRIX2X4DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2x4dv")) == nullptr) || r;
  r = ((glUniformMatrix3dv = (PFNGLUNIFORMMATRIX3DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3dv")) == nullptr) || r;
  r = ((glUniformMatrix3x2dv = (PFNGLUNIFORMMATRIX3X2DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3x2dv")) == nullptr) || r;
  r = ((glUniformMatrix3x4dv = (PFNGLUNIFORMMATRIX3X4DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3x4dv")) == nullptr) || r;
  r = ((glUniformMatrix4dv = (PFNGLUNIFORMMATRIX4DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4dv")) == nullptr) || r;
  r = ((glUniformMatrix4x2dv = (PFNGLUNIFORMMATRIX4X2DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4x2dv")) == nullptr) || r;
  r = ((glUniformMatrix4x3dv = (PFNGLUNIFORMMATRIX4X3DVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4x3dv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_gpu_shader_fp64 */

#ifdef GL_ARB_half_float_pixel

#endif /* GL_ARB_half_float_pixel */

#ifdef GL_ARB_half_float_vertex

#endif /* GL_ARB_half_float_vertex */

#ifdef GL_ARB_imaging

static GLboolean _glewInit_GL_ARB_imaging (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)glewGetProcAddress((const GLubyte*)"glBlendEquation")) == nullptr) || r;
  r = ((glColorSubTable = (PFNGLCOLORSUBTABLEPROC)glewGetProcAddress((const GLubyte*)"glColorSubTable")) == nullptr) || r;
  r = ((glColorTable = (PFNGLCOLORTABLEPROC)glewGetProcAddress((const GLubyte*)"glColorTable")) == nullptr) || r;
  r = ((glColorTableParameterfv = (PFNGLCOLORTABLEPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glColorTableParameterfv")) == nullptr) || r;
  r = ((glColorTableParameteriv = (PFNGLCOLORTABLEPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glColorTableParameteriv")) == nullptr) || r;
  r = ((glConvolutionFilter1D = (PFNGLCONVOLUTIONFILTER1DPROC)glewGetProcAddress((const GLubyte*)"glConvolutionFilter1D")) == nullptr) || r;
  r = ((glConvolutionFilter2D = (PFNGLCONVOLUTIONFILTER2DPROC)glewGetProcAddress((const GLubyte*)"glConvolutionFilter2D")) == nullptr) || r;
  r = ((glConvolutionParameterf = (PFNGLCONVOLUTIONPARAMETERFPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterf")) == nullptr) || r;
  r = ((glConvolutionParameterfv = (PFNGLCONVOLUTIONPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterfv")) == nullptr) || r;
  r = ((glConvolutionParameteri = (PFNGLCONVOLUTIONPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameteri")) == nullptr) || r;
  r = ((glConvolutionParameteriv = (PFNGLCONVOLUTIONPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameteriv")) == nullptr) || r;
  r = ((glCopyColorSubTable = (PFNGLCOPYCOLORSUBTABLEPROC)glewGetProcAddress((const GLubyte*)"glCopyColorSubTable")) == nullptr) || r;
  r = ((glCopyColorTable = (PFNGLCOPYCOLORTABLEPROC)glewGetProcAddress((const GLubyte*)"glCopyColorTable")) == nullptr) || r;
  r = ((glCopyConvolutionFilter1D = (PFNGLCOPYCONVOLUTIONFILTER1DPROC)glewGetProcAddress((const GLubyte*)"glCopyConvolutionFilter1D")) == nullptr) || r;
  r = ((glCopyConvolutionFilter2D = (PFNGLCOPYCONVOLUTIONFILTER2DPROC)glewGetProcAddress((const GLubyte*)"glCopyConvolutionFilter2D")) == nullptr) || r;
  r = ((glGetColorTable = (PFNGLGETCOLORTABLEPROC)glewGetProcAddress((const GLubyte*)"glGetColorTable")) == nullptr) || r;
  r = ((glGetColorTableParameterfv = (PFNGLGETCOLORTABLEPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameterfv")) == nullptr) || r;
  r = ((glGetColorTableParameteriv = (PFNGLGETCOLORTABLEPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameteriv")) == nullptr) || r;
  r = ((glGetConvolutionFilter = (PFNGLGETCONVOLUTIONFILTERPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionFilter")) == nullptr) || r;
  r = ((glGetConvolutionParameterfv = (PFNGLGETCONVOLUTIONPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionParameterfv")) == nullptr) || r;
  r = ((glGetConvolutionParameteriv = (PFNGLGETCONVOLUTIONPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionParameteriv")) == nullptr) || r;
  r = ((glGetHistogram = (PFNGLGETHISTOGRAMPROC)glewGetProcAddress((const GLubyte*)"glGetHistogram")) == nullptr) || r;
  r = ((glGetHistogramParameterfv = (PFNGLGETHISTOGRAMPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramParameterfv")) == nullptr) || r;
  r = ((glGetHistogramParameteriv = (PFNGLGETHISTOGRAMPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramParameteriv")) == nullptr) || r;
  r = ((glGetMinmax = (PFNGLGETMINMAXPROC)glewGetProcAddress((const GLubyte*)"glGetMinmax")) == nullptr) || r;
  r = ((glGetMinmaxParameterfv = (PFNGLGETMINMAXPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxParameterfv")) == nullptr) || r;
  r = ((glGetMinmaxParameteriv = (PFNGLGETMINMAXPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxParameteriv")) == nullptr) || r;
  r = ((glGetSeparableFilter = (PFNGLGETSEPARABLEFILTERPROC)glewGetProcAddress((const GLubyte*)"glGetSeparableFilter")) == nullptr) || r;
  r = ((glHistogram = (PFNGLHISTOGRAMPROC)glewGetProcAddress((const GLubyte*)"glHistogram")) == nullptr) || r;
  r = ((glMinmax = (PFNGLMINMAXPROC)glewGetProcAddress((const GLubyte*)"glMinmax")) == nullptr) || r;
  r = ((glResetHistogram = (PFNGLRESETHISTOGRAMPROC)glewGetProcAddress((const GLubyte*)"glResetHistogram")) == nullptr) || r;
  r = ((glResetMinmax = (PFNGLRESETMINMAXPROC)glewGetProcAddress((const GLubyte*)"glResetMinmax")) == nullptr) || r;
  r = ((glSeparableFilter2D = (PFNGLSEPARABLEFILTER2DPROC)glewGetProcAddress((const GLubyte*)"glSeparableFilter2D")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_imaging */

#ifdef GL_ARB_instanced_arrays

static GLboolean _glewInit_GL_ARB_instanced_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstancedARB = (PFNGLDRAWARRAYSINSTANCEDARBPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysInstancedARB")) == nullptr) || r;
  r = ((glDrawElementsInstancedARB = (PFNGLDRAWELEMENTSINSTANCEDARBPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedARB")) == nullptr) || r;
  r = ((glVertexAttribDivisorARB = (PFNGLVERTEXATTRIBDIVISORARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribDivisorARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_instanced_arrays */

#ifdef GL_ARB_internalformat_query

static GLboolean _glewInit_GL_ARB_internalformat_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)glewGetProcAddress((const GLubyte*)"glGetInternalformativ")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_internalformat_query */

#ifdef GL_ARB_map_buffer_alignment

#endif /* GL_ARB_map_buffer_alignment */

#ifdef GL_ARB_map_buffer_range

static GLboolean _glewInit_GL_ARB_map_buffer_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glFlushMappedBufferRange")) == nullptr) || r;
  r = ((glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glMapBufferRange")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_map_buffer_range */

#ifdef GL_ARB_matrix_palette

static GLboolean _glewInit_GL_ARB_matrix_palette (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCurrentPaletteMatrixARB = (PFNGLCURRENTPALETTEMATRIXARBPROC)glewGetProcAddress((const GLubyte*)"glCurrentPaletteMatrixARB")) == nullptr) || r;
  r = ((glMatrixIndexPointerARB = (PFNGLMATRIXINDEXPOINTERARBPROC)glewGetProcAddress((const GLubyte*)"glMatrixIndexPointerARB")) == nullptr) || r;
  r = ((glMatrixIndexubvARB = (PFNGLMATRIXINDEXUBVARBPROC)glewGetProcAddress((const GLubyte*)"glMatrixIndexubvARB")) == nullptr) || r;
  r = ((glMatrixIndexuivARB = (PFNGLMATRIXINDEXUIVARBPROC)glewGetProcAddress((const GLubyte*)"glMatrixIndexuivARB")) == nullptr) || r;
  r = ((glMatrixIndexusvARB = (PFNGLMATRIXINDEXUSVARBPROC)glewGetProcAddress((const GLubyte*)"glMatrixIndexusvARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_matrix_palette */

#ifdef GL_ARB_multisample

static GLboolean _glewInit_GL_ARB_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSampleCoverageARB = (PFNGLSAMPLECOVERAGEARBPROC)glewGetProcAddress((const GLubyte*)"glSampleCoverageARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_multisample */

#ifdef GL_ARB_multitexture

static GLboolean _glewInit_GL_ARB_multitexture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)glewGetProcAddress((const GLubyte*)"glActiveTextureARB")) == nullptr) || r;
  r = ((glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)glewGetProcAddress((const GLubyte*)"glClientActiveTextureARB")) == nullptr) || r;
  r = ((glMultiTexCoord1dARB = (PFNGLMULTITEXCOORD1DARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1dARB")) == nullptr) || r;
  r = ((glMultiTexCoord1dvARB = (PFNGLMULTITEXCOORD1DVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1dvARB")) == nullptr) || r;
  r = ((glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1fARB")) == nullptr) || r;
  r = ((glMultiTexCoord1fvARB = (PFNGLMULTITEXCOORD1FVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1fvARB")) == nullptr) || r;
  r = ((glMultiTexCoord1iARB = (PFNGLMULTITEXCOORD1IARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1iARB")) == nullptr) || r;
  r = ((glMultiTexCoord1ivARB = (PFNGLMULTITEXCOORD1IVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1ivARB")) == nullptr) || r;
  r = ((glMultiTexCoord1sARB = (PFNGLMULTITEXCOORD1SARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1sARB")) == nullptr) || r;
  r = ((glMultiTexCoord1svARB = (PFNGLMULTITEXCOORD1SVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1svARB")) == nullptr) || r;
  r = ((glMultiTexCoord2dARB = (PFNGLMULTITEXCOORD2DARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2dARB")) == nullptr) || r;
  r = ((glMultiTexCoord2dvARB = (PFNGLMULTITEXCOORD2DVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2dvARB")) == nullptr) || r;
  r = ((glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2fARB")) == nullptr) || r;
  r = ((glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2fvARB")) == nullptr) || r;
  r = ((glMultiTexCoord2iARB = (PFNGLMULTITEXCOORD2IARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2iARB")) == nullptr) || r;
  r = ((glMultiTexCoord2ivARB = (PFNGLMULTITEXCOORD2IVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2ivARB")) == nullptr) || r;
  r = ((glMultiTexCoord2sARB = (PFNGLMULTITEXCOORD2SARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2sARB")) == nullptr) || r;
  r = ((glMultiTexCoord2svARB = (PFNGLMULTITEXCOORD2SVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2svARB")) == nullptr) || r;
  r = ((glMultiTexCoord3dARB = (PFNGLMULTITEXCOORD3DARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3dARB")) == nullptr) || r;
  r = ((glMultiTexCoord3dvARB = (PFNGLMULTITEXCOORD3DVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3dvARB")) == nullptr) || r;
  r = ((glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3fARB")) == nullptr) || r;
  r = ((glMultiTexCoord3fvARB = (PFNGLMULTITEXCOORD3FVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3fvARB")) == nullptr) || r;
  r = ((glMultiTexCoord3iARB = (PFNGLMULTITEXCOORD3IARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3iARB")) == nullptr) || r;
  r = ((glMultiTexCoord3ivARB = (PFNGLMULTITEXCOORD3IVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3ivARB")) == nullptr) || r;
  r = ((glMultiTexCoord3sARB = (PFNGLMULTITEXCOORD3SARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3sARB")) == nullptr) || r;
  r = ((glMultiTexCoord3svARB = (PFNGLMULTITEXCOORD3SVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3svARB")) == nullptr) || r;
  r = ((glMultiTexCoord4dARB = (PFNGLMULTITEXCOORD4DARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4dARB")) == nullptr) || r;
  r = ((glMultiTexCoord4dvARB = (PFNGLMULTITEXCOORD4DVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4dvARB")) == nullptr) || r;
  r = ((glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4fARB")) == nullptr) || r;
  r = ((glMultiTexCoord4fvARB = (PFNGLMULTITEXCOORD4FVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4fvARB")) == nullptr) || r;
  r = ((glMultiTexCoord4iARB = (PFNGLMULTITEXCOORD4IARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4iARB")) == nullptr) || r;
  r = ((glMultiTexCoord4ivARB = (PFNGLMULTITEXCOORD4IVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4ivARB")) == nullptr) || r;
  r = ((glMultiTexCoord4sARB = (PFNGLMULTITEXCOORD4SARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4sARB")) == nullptr) || r;
  r = ((glMultiTexCoord4svARB = (PFNGLMULTITEXCOORD4SVARBPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4svARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_multitexture */

#ifdef GL_ARB_occlusion_query

static GLboolean _glewInit_GL_ARB_occlusion_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)glewGetProcAddress((const GLubyte*)"glBeginQueryARB")) == nullptr) || r;
  r = ((glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)glewGetProcAddress((const GLubyte*)"glDeleteQueriesARB")) == nullptr) || r;
  r = ((glEndQueryARB = (PFNGLENDQUERYARBPROC)glewGetProcAddress((const GLubyte*)"glEndQueryARB")) == nullptr) || r;
  r = ((glGenQueriesARB = (PFNGLGENQUERIESARBPROC)glewGetProcAddress((const GLubyte*)"glGenQueriesARB")) == nullptr) || r;
  r = ((glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectivARB")) == nullptr) || r;
  r = ((glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectuivARB")) == nullptr) || r;
  r = ((glGetQueryivARB = (PFNGLGETQUERYIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetQueryivARB")) == nullptr) || r;
  r = ((glIsQueryARB = (PFNGLISQUERYARBPROC)glewGetProcAddress((const GLubyte*)"glIsQueryARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_occlusion_query */

#ifdef GL_ARB_occlusion_query2

#endif /* GL_ARB_occlusion_query2 */

#ifdef GL_ARB_pixel_buffer_object

#endif /* GL_ARB_pixel_buffer_object */

#ifdef GL_ARB_point_parameters

static GLboolean _glewInit_GL_ARB_point_parameters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC)glewGetProcAddress((const GLubyte*)"glPointParameterfARB")) == nullptr) || r;
  r = ((glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)glewGetProcAddress((const GLubyte*)"glPointParameterfvARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_point_parameters */

#ifdef GL_ARB_point_sprite

#endif /* GL_ARB_point_sprite */

#ifdef GL_ARB_provoking_vertex

static GLboolean _glewInit_GL_ARB_provoking_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC)glewGetProcAddress((const GLubyte*)"glProvokingVertex")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_provoking_vertex */

#ifdef GL_ARB_robustness

static GLboolean _glewInit_GL_ARB_robustness (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetGraphicsResetStatusARB = (PFNGLGETGRAPHICSRESETSTATUSARBPROC)glewGetProcAddress((const GLubyte*)"glGetGraphicsResetStatusARB")) == nullptr) || r;
  r = ((glGetnColorTableARB = (PFNGLGETNCOLORTABLEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnColorTableARB")) == nullptr) || r;
  r = ((glGetnCompressedTexImageARB = (PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnCompressedTexImageARB")) == nullptr) || r;
  r = ((glGetnConvolutionFilterARB = (PFNGLGETNCONVOLUTIONFILTERARBPROC)glewGetProcAddress((const GLubyte*)"glGetnConvolutionFilterARB")) == nullptr) || r;
  r = ((glGetnHistogramARB = (PFNGLGETNHISTOGRAMARBPROC)glewGetProcAddress((const GLubyte*)"glGetnHistogramARB")) == nullptr) || r;
  r = ((glGetnMapdvARB = (PFNGLGETNMAPDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMapdvARB")) == nullptr) || r;
  r = ((glGetnMapfvARB = (PFNGLGETNMAPFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMapfvARB")) == nullptr) || r;
  r = ((glGetnMapivARB = (PFNGLGETNMAPIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMapivARB")) == nullptr) || r;
  r = ((glGetnMinmaxARB = (PFNGLGETNMINMAXARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMinmaxARB")) == nullptr) || r;
  r = ((glGetnPixelMapfvARB = (PFNGLGETNPIXELMAPFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPixelMapfvARB")) == nullptr) || r;
  r = ((glGetnPixelMapuivARB = (PFNGLGETNPIXELMAPUIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPixelMapuivARB")) == nullptr) || r;
  r = ((glGetnPixelMapusvARB = (PFNGLGETNPIXELMAPUSVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPixelMapusvARB")) == nullptr) || r;
  r = ((glGetnPolygonStippleARB = (PFNGLGETNPOLYGONSTIPPLEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPolygonStippleARB")) == nullptr) || r;
  r = ((glGetnSeparableFilterARB = (PFNGLGETNSEPARABLEFILTERARBPROC)glewGetProcAddress((const GLubyte*)"glGetnSeparableFilterARB")) == nullptr) || r;
  r = ((glGetnTexImageARB = (PFNGLGETNTEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnTexImageARB")) == nullptr) || r;
  r = ((glGetnUniformdvARB = (PFNGLGETNUNIFORMDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformdvARB")) == nullptr) || r;
  r = ((glGetnUniformfvARB = (PFNGLGETNUNIFORMFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformfvARB")) == nullptr) || r;
  r = ((glGetnUniformivARB = (PFNGLGETNUNIFORMIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformivARB")) == nullptr) || r;
  r = ((glGetnUniformuivARB = (PFNGLGETNUNIFORMUIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformuivARB")) == nullptr) || r;
  r = ((glReadnPixelsARB = (PFNGLREADNPIXELSARBPROC)glewGetProcAddress((const GLubyte*)"glReadnPixelsARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_robustness */

#ifdef GL_ARB_sample_shading

static GLboolean _glewInit_GL_ARB_sample_shading (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMinSampleShadingARB = (PFNGLMINSAMPLESHADINGARBPROC)glewGetProcAddress((const GLubyte*)"glMinSampleShadingARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_sample_shading */

#ifdef GL_ARB_sampler_objects

static GLboolean _glewInit_GL_ARB_sampler_objects (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindSampler = (PFNGLBINDSAMPLERPROC)glewGetProcAddress((const GLubyte*)"glBindSampler")) == nullptr) || r;
  r = ((glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteSamplers")) == nullptr) || r;
  r = ((glGenSamplers = (PFNGLGENSAMPLERSPROC)glewGetProcAddress((const GLubyte*)"glGenSamplers")) == nullptr) || r;
  r = ((glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glGetSamplerParameterIiv")) == nullptr) || r;
  r = ((glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glGetSamplerParameterIuiv")) == nullptr) || r;
  r = ((glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetSamplerParameterfv")) == nullptr) || r;
  r = ((glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetSamplerParameteriv")) == nullptr) || r;
  r = ((glIsSampler = (PFNGLISSAMPLERPROC)glewGetProcAddress((const GLubyte*)"glIsSampler")) == nullptr) || r;
  r = ((glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glSamplerParameterIiv")) == nullptr) || r;
  r = ((glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glSamplerParameterIuiv")) == nullptr) || r;
  r = ((glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)glewGetProcAddress((const GLubyte*)"glSamplerParameterf")) == nullptr) || r;
  r = ((glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glSamplerParameterfv")) == nullptr) || r;
  r = ((glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glSamplerParameteri")) == nullptr) || r;
  r = ((glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glSamplerParameteriv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_sampler_objects */

#ifdef GL_ARB_seamless_cube_map

#endif /* GL_ARB_seamless_cube_map */

#ifdef GL_ARB_separate_shader_objects

static GLboolean _glewInit_GL_ARB_separate_shader_objects (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glActiveShaderProgram")) == nullptr) || r;
  r = ((glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC)glewGetProcAddress((const GLubyte*)"glBindProgramPipeline")) == nullptr) || r;
  r = ((glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)glewGetProcAddress((const GLubyte*)"glCreateShaderProgramv")) == nullptr) || r;
  r = ((glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC)glewGetProcAddress((const GLubyte*)"glDeleteProgramPipelines")) == nullptr) || r;
  r = ((glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC)glewGetProcAddress((const GLubyte*)"glGenProgramPipelines")) == nullptr) || r;
  r = ((glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)glewGetProcAddress((const GLubyte*)"glGetProgramPipelineInfoLog")) == nullptr) || r;
  r = ((glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramPipelineiv")) == nullptr) || r;
  r = ((glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC)glewGetProcAddress((const GLubyte*)"glIsProgramPipeline")) == nullptr) || r;
  r = ((glProgramUniform1d = (PFNGLPROGRAMUNIFORM1DPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1d")) == nullptr) || r;
  r = ((glProgramUniform1dv = (PFNGLPROGRAMUNIFORM1DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1dv")) == nullptr) || r;
  r = ((glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1f")) == nullptr) || r;
  r = ((glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1fv")) == nullptr) || r;
  r = ((glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1i")) == nullptr) || r;
  r = ((glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1iv")) == nullptr) || r;
  r = ((glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ui")) == nullptr) || r;
  r = ((glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1uiv")) == nullptr) || r;
  r = ((glProgramUniform2d = (PFNGLPROGRAMUNIFORM2DPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2d")) == nullptr) || r;
  r = ((glProgramUniform2dv = (PFNGLPROGRAMUNIFORM2DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2dv")) == nullptr) || r;
  r = ((glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2f")) == nullptr) || r;
  r = ((glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2fv")) == nullptr) || r;
  r = ((glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2i")) == nullptr) || r;
  r = ((glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2iv")) == nullptr) || r;
  r = ((glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ui")) == nullptr) || r;
  r = ((glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2uiv")) == nullptr) || r;
  r = ((glProgramUniform3d = (PFNGLPROGRAMUNIFORM3DPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3d")) == nullptr) || r;
  r = ((glProgramUniform3dv = (PFNGLPROGRAMUNIFORM3DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3dv")) == nullptr) || r;
  r = ((glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3f")) == nullptr) || r;
  r = ((glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3fv")) == nullptr) || r;
  r = ((glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3i")) == nullptr) || r;
  r = ((glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3iv")) == nullptr) || r;
  r = ((glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ui")) == nullptr) || r;
  r = ((glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3uiv")) == nullptr) || r;
  r = ((glProgramUniform4d = (PFNGLPROGRAMUNIFORM4DPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4d")) == nullptr) || r;
  r = ((glProgramUniform4dv = (PFNGLPROGRAMUNIFORM4DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4dv")) == nullptr) || r;
  r = ((glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4f")) == nullptr) || r;
  r = ((glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4fv")) == nullptr) || r;
  r = ((glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4i")) == nullptr) || r;
  r = ((glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4iv")) == nullptr) || r;
  r = ((glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ui")) == nullptr) || r;
  r = ((glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4uiv")) == nullptr) || r;
  r = ((glProgramUniformMatrix2dv = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x3dv = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x3dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x3fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x4dv = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x4dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x4fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix3dv = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x2dv = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x2dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x2fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x4dv = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x4dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x4fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix4dv = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x2dv = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x2dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x2fv")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x3dv = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x3dv")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x3fv")) == nullptr) || r;
  r = ((glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC)glewGetProcAddress((const GLubyte*)"glUseProgramStages")) == nullptr) || r;
  r = ((glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC)glewGetProcAddress((const GLubyte*)"glValidateProgramPipeline")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_separate_shader_objects */

#ifdef GL_ARB_shader_atomic_counters

static GLboolean _glewInit_GL_ARB_shader_atomic_counters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetActiveAtomicCounterBufferiv = (PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveAtomicCounterBufferiv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_shader_atomic_counters */

#ifdef GL_ARB_shader_bit_encoding

#endif /* GL_ARB_shader_bit_encoding */

#ifdef GL_ARB_shader_image_load_store

static GLboolean _glewInit_GL_ARB_shader_image_load_store (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)glewGetProcAddress((const GLubyte*)"glBindImageTexture")) == nullptr) || r;
  r = ((glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)glewGetProcAddress((const GLubyte*)"glMemoryBarrier")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_shader_image_load_store */

#ifdef GL_ARB_shader_objects

static GLboolean _glewInit_GL_ARB_shader_objects (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)glewGetProcAddress((const GLubyte*)"glAttachObjectARB")) == nullptr) || r;
  r = ((glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)glewGetProcAddress((const GLubyte*)"glCompileShaderARB")) == nullptr) || r;
  r = ((glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)glewGetProcAddress((const GLubyte*)"glCreateProgramObjectARB")) == nullptr) || r;
  r = ((glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)glewGetProcAddress((const GLubyte*)"glCreateShaderObjectARB")) == nullptr) || r;
  r = ((glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)glewGetProcAddress((const GLubyte*)"glDeleteObjectARB")) == nullptr) || r;
  r = ((glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)glewGetProcAddress((const GLubyte*)"glDetachObjectARB")) == nullptr) || r;
  r = ((glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformARB")) == nullptr) || r;
  r = ((glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)glewGetProcAddress((const GLubyte*)"glGetAttachedObjectsARB")) == nullptr) || r;
  r = ((glGetHandleARB = (PFNGLGETHANDLEARBPROC)glewGetProcAddress((const GLubyte*)"glGetHandleARB")) == nullptr) || r;
  r = ((glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)glewGetProcAddress((const GLubyte*)"glGetInfoLogARB")) == nullptr) || r;
  r = ((glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetObjectParameterfvARB")) == nullptr) || r;
  r = ((glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetObjectParameterivARB")) == nullptr) || r;
  r = ((glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)glewGetProcAddress((const GLubyte*)"glGetShaderSourceARB")) == nullptr) || r;
  r = ((glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)glewGetProcAddress((const GLubyte*)"glGetUniformLocationARB")) == nullptr) || r;
  r = ((glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetUniformfvARB")) == nullptr) || r;
  r = ((glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetUniformivARB")) == nullptr) || r;
  r = ((glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)glewGetProcAddress((const GLubyte*)"glLinkProgramARB")) == nullptr) || r;
  r = ((glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)glewGetProcAddress((const GLubyte*)"glShaderSourceARB")) == nullptr) || r;
  r = ((glUniform1fARB = (PFNGLUNIFORM1FARBPROC)glewGetProcAddress((const GLubyte*)"glUniform1fARB")) == nullptr) || r;
  r = ((glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform1fvARB")) == nullptr) || r;
  r = ((glUniform1iARB = (PFNGLUNIFORM1IARBPROC)glewGetProcAddress((const GLubyte*)"glUniform1iARB")) == nullptr) || r;
  r = ((glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform1ivARB")) == nullptr) || r;
  r = ((glUniform2fARB = (PFNGLUNIFORM2FARBPROC)glewGetProcAddress((const GLubyte*)"glUniform2fARB")) == nullptr) || r;
  r = ((glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform2fvARB")) == nullptr) || r;
  r = ((glUniform2iARB = (PFNGLUNIFORM2IARBPROC)glewGetProcAddress((const GLubyte*)"glUniform2iARB")) == nullptr) || r;
  r = ((glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform2ivARB")) == nullptr) || r;
  r = ((glUniform3fARB = (PFNGLUNIFORM3FARBPROC)glewGetProcAddress((const GLubyte*)"glUniform3fARB")) == nullptr) || r;
  r = ((glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform3fvARB")) == nullptr) || r;
  r = ((glUniform3iARB = (PFNGLUNIFORM3IARBPROC)glewGetProcAddress((const GLubyte*)"glUniform3iARB")) == nullptr) || r;
  r = ((glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform3ivARB")) == nullptr) || r;
  r = ((glUniform4fARB = (PFNGLUNIFORM4FARBPROC)glewGetProcAddress((const GLubyte*)"glUniform4fARB")) == nullptr) || r;
  r = ((glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform4fvARB")) == nullptr) || r;
  r = ((glUniform4iARB = (PFNGLUNIFORM4IARBPROC)glewGetProcAddress((const GLubyte*)"glUniform4iARB")) == nullptr) || r;
  r = ((glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)glewGetProcAddress((const GLubyte*)"glUniform4ivARB")) == nullptr) || r;
  r = ((glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2fvARB")) == nullptr) || r;
  r = ((glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3fvARB")) == nullptr) || r;
  r = ((glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4fvARB")) == nullptr) || r;
  r = ((glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)glewGetProcAddress((const GLubyte*)"glUseProgramObjectARB")) == nullptr) || r;
  r = ((glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)glewGetProcAddress((const GLubyte*)"glValidateProgramARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_shader_objects */

#ifdef GL_ARB_shader_precision

#endif /* GL_ARB_shader_precision */

#ifdef GL_ARB_shader_stencil_export

#endif /* GL_ARB_shader_stencil_export */

#ifdef GL_ARB_shader_subroutine

static GLboolean _glewInit_GL_ARB_shader_subroutine (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetActiveSubroutineName = (PFNGLGETACTIVESUBROUTINENAMEPROC)glewGetProcAddress((const GLubyte*)"glGetActiveSubroutineName")) == nullptr) || r;
  r = ((glGetActiveSubroutineUniformName = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)glewGetProcAddress((const GLubyte*)"glGetActiveSubroutineUniformName")) == nullptr) || r;
  r = ((glGetActiveSubroutineUniformiv = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveSubroutineUniformiv")) == nullptr) || r;
  r = ((glGetProgramStageiv = (PFNGLGETPROGRAMSTAGEIVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramStageiv")) == nullptr) || r;
  r = ((glGetSubroutineIndex = (PFNGLGETSUBROUTINEINDEXPROC)glewGetProcAddress((const GLubyte*)"glGetSubroutineIndex")) == nullptr) || r;
  r = ((glGetSubroutineUniformLocation = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetSubroutineUniformLocation")) == nullptr) || r;
  r = ((glGetUniformSubroutineuiv = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformSubroutineuiv")) == nullptr) || r;
  r = ((glUniformSubroutinesuiv = (PFNGLUNIFORMSUBROUTINESUIVPROC)glewGetProcAddress((const GLubyte*)"glUniformSubroutinesuiv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_shader_subroutine */

#ifdef GL_ARB_shader_texture_lod

#endif /* GL_ARB_shader_texture_lod */

#ifdef GL_ARB_shading_language_100

#endif /* GL_ARB_shading_language_100 */

#ifdef GL_ARB_shading_language_420pack

#endif /* GL_ARB_shading_language_420pack */

#ifdef GL_ARB_shading_language_include

static GLboolean _glewInit_GL_ARB_shading_language_include (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCompileShaderIncludeARB = (PFNGLCOMPILESHADERINCLUDEARBPROC)glewGetProcAddress((const GLubyte*)"glCompileShaderIncludeARB")) == nullptr) || r;
  r = ((glDeleteNamedStringARB = (PFNGLDELETENAMEDSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"glDeleteNamedStringARB")) == nullptr) || r;
  r = ((glGetNamedStringARB = (PFNGLGETNAMEDSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"glGetNamedStringARB")) == nullptr) || r;
  r = ((glGetNamedStringivARB = (PFNGLGETNAMEDSTRINGIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetNamedStringivARB")) == nullptr) || r;
  r = ((glIsNamedStringARB = (PFNGLISNAMEDSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"glIsNamedStringARB")) == nullptr) || r;
  r = ((glNamedStringARB = (PFNGLNAMEDSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"glNamedStringARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_shading_language_include */

#ifdef GL_ARB_shading_language_packing

#endif /* GL_ARB_shading_language_packing */

#ifdef GL_ARB_shadow

#endif /* GL_ARB_shadow */

#ifdef GL_ARB_shadow_ambient

#endif /* GL_ARB_shadow_ambient */

#ifdef GL_ARB_sync

static GLboolean _glewInit_GL_ARB_sync (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)glewGetProcAddress((const GLubyte*)"glClientWaitSync")) == nullptr) || r;
  r = ((glDeleteSync = (PFNGLDELETESYNCPROC)glewGetProcAddress((const GLubyte*)"glDeleteSync")) == nullptr) || r;
  r = ((glFenceSync = (PFNGLFENCESYNCPROC)glewGetProcAddress((const GLubyte*)"glFenceSync")) == nullptr) || r;
  r = ((glGetInteger64v = (PFNGLGETINTEGER64VPROC)glewGetProcAddress((const GLubyte*)"glGetInteger64v")) == nullptr) || r;
  r = ((glGetSynciv = (PFNGLGETSYNCIVPROC)glewGetProcAddress((const GLubyte*)"glGetSynciv")) == nullptr) || r;
  r = ((glIsSync = (PFNGLISSYNCPROC)glewGetProcAddress((const GLubyte*)"glIsSync")) == nullptr) || r;
  r = ((glWaitSync = (PFNGLWAITSYNCPROC)glewGetProcAddress((const GLubyte*)"glWaitSync")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_sync */

#ifdef GL_ARB_tessellation_shader

static GLboolean _glewInit_GL_ARB_tessellation_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glPatchParameterfv")) == nullptr) || r;
  r = ((glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glPatchParameteri")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_tessellation_shader */

#ifdef GL_ARB_texture_border_clamp

#endif /* GL_ARB_texture_border_clamp */

#ifdef GL_ARB_texture_buffer_object

static GLboolean _glewInit_GL_ARB_texture_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexBufferARB = (PFNGLTEXBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"glTexBufferARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_texture_buffer_object */

#ifdef GL_ARB_texture_buffer_object_rgb32

#endif /* GL_ARB_texture_buffer_object_rgb32 */

#ifdef GL_ARB_texture_compression

static GLboolean _glewInit_GL_ARB_texture_compression (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage1DARB")) == nullptr) || r;
  r = ((glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage2DARB")) == nullptr) || r;
  r = ((glCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage3DARB")) == nullptr) || r;
  r = ((glCompressedTexSubImage1DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage1DARB")) == nullptr) || r;
  r = ((glCompressedTexSubImage2DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage2DARB")) == nullptr) || r;
  r = ((glCompressedTexSubImage3DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage3DARB")) == nullptr) || r;
  r = ((glGetCompressedTexImageARB = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedTexImageARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_texture_compression */

#ifdef GL_ARB_texture_compression_bptc

#endif /* GL_ARB_texture_compression_bptc */

#ifdef GL_ARB_texture_compression_rgtc

#endif /* GL_ARB_texture_compression_rgtc */

#ifdef GL_ARB_texture_cube_map

#endif /* GL_ARB_texture_cube_map */

#ifdef GL_ARB_texture_cube_map_array

#endif /* GL_ARB_texture_cube_map_array */

#ifdef GL_ARB_texture_env_add

#endif /* GL_ARB_texture_env_add */

#ifdef GL_ARB_texture_env_combine

#endif /* GL_ARB_texture_env_combine */

#ifdef GL_ARB_texture_env_crossbar

#endif /* GL_ARB_texture_env_crossbar */

#ifdef GL_ARB_texture_env_dot3

#endif /* GL_ARB_texture_env_dot3 */

#ifdef GL_ARB_texture_float

#endif /* GL_ARB_texture_float */

#ifdef GL_ARB_texture_gather

#endif /* GL_ARB_texture_gather */

#ifdef GL_ARB_texture_mirrored_repeat

#endif /* GL_ARB_texture_mirrored_repeat */

#ifdef GL_ARB_texture_multisample

static GLboolean _glewInit_GL_ARB_texture_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)glewGetProcAddress((const GLubyte*)"glGetMultisamplefv")) == nullptr) || r;
  r = ((glSampleMaski = (PFNGLSAMPLEMASKIPROC)glewGetProcAddress((const GLubyte*)"glSampleMaski")) == nullptr) || r;
  r = ((glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glTexImage2DMultisample")) == nullptr) || r;
  r = ((glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glTexImage3DMultisample")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_texture_multisample */

#ifdef GL_ARB_texture_non_power_of_two

#endif /* GL_ARB_texture_non_power_of_two */

#ifdef GL_ARB_texture_query_lod

#endif /* GL_ARB_texture_query_lod */

#ifdef GL_ARB_texture_rectangle

#endif /* GL_ARB_texture_rectangle */

#ifdef GL_ARB_texture_rg

#endif /* GL_ARB_texture_rg */

#ifdef GL_ARB_texture_rgb10_a2ui

#endif /* GL_ARB_texture_rgb10_a2ui */

#ifdef GL_ARB_texture_storage

static GLboolean _glewInit_GL_ARB_texture_storage (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexStorage1D = (PFNGLTEXSTORAGE1DPROC)glewGetProcAddress((const GLubyte*)"glTexStorage1D")) == nullptr) || r;
  r = ((glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)glewGetProcAddress((const GLubyte*)"glTexStorage2D")) == nullptr) || r;
  r = ((glTexStorage3D = (PFNGLTEXSTORAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexStorage3D")) == nullptr) || r;
  r = ((glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage1DEXT")) == nullptr) || r;
  r = ((glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage2DEXT")) == nullptr) || r;
  r = ((glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage3DEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_texture_storage */

#ifdef GL_ARB_texture_swizzle

#endif /* GL_ARB_texture_swizzle */

#ifdef GL_ARB_timer_query

static GLboolean _glewInit_GL_ARB_timer_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjecti64v")) == nullptr) || r;
  r = ((glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectui64v")) == nullptr) || r;
  r = ((glQueryCounter = (PFNGLQUERYCOUNTERPROC)glewGetProcAddress((const GLubyte*)"glQueryCounter")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_timer_query */

#ifdef GL_ARB_transform_feedback2

static GLboolean _glewInit_GL_ARB_transform_feedback2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glBindTransformFeedback")) == nullptr) || r;
  r = ((glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)glewGetProcAddress((const GLubyte*)"glDeleteTransformFeedbacks")) == nullptr) || r;
  r = ((glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glDrawTransformFeedback")) == nullptr) || r;
  r = ((glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)glewGetProcAddress((const GLubyte*)"glGenTransformFeedbacks")) == nullptr) || r;
  r = ((glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glIsTransformFeedback")) == nullptr) || r;
  r = ((glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glPauseTransformFeedback")) == nullptr) || r;
  r = ((glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glResumeTransformFeedback")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_transform_feedback2 */

#ifdef GL_ARB_transform_feedback3

static GLboolean _glewInit_GL_ARB_transform_feedback3 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginQueryIndexed = (PFNGLBEGINQUERYINDEXEDPROC)glewGetProcAddress((const GLubyte*)"glBeginQueryIndexed")) == nullptr) || r;
  r = ((glDrawTransformFeedbackStream = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)glewGetProcAddress((const GLubyte*)"glDrawTransformFeedbackStream")) == nullptr) || r;
  r = ((glEndQueryIndexed = (PFNGLENDQUERYINDEXEDPROC)glewGetProcAddress((const GLubyte*)"glEndQueryIndexed")) == nullptr) || r;
  r = ((glGetQueryIndexediv = (PFNGLGETQUERYINDEXEDIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryIndexediv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_transform_feedback3 */

#ifdef GL_ARB_transform_feedback_instanced

static GLboolean _glewInit_GL_ARB_transform_feedback_instanced (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawTransformFeedbackInstanced = (PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC)glewGetProcAddress((const GLubyte*)"glDrawTransformFeedbackInstanced")) == nullptr) || r;
  r = ((glDrawTransformFeedbackStreamInstanced = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC)glewGetProcAddress((const GLubyte*)"glDrawTransformFeedbackStreamInstanced")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_transform_feedback_instanced */

#ifdef GL_ARB_transpose_matrix

static GLboolean _glewInit_GL_ARB_transpose_matrix (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glLoadTransposeMatrixdARB = (PFNGLLOADTRANSPOSEMATRIXDARBPROC)glewGetProcAddress((const GLubyte*)"glLoadTransposeMatrixdARB")) == nullptr) || r;
  r = ((glLoadTransposeMatrixfARB = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)glewGetProcAddress((const GLubyte*)"glLoadTransposeMatrixfARB")) == nullptr) || r;
  r = ((glMultTransposeMatrixdARB = (PFNGLMULTTRANSPOSEMATRIXDARBPROC)glewGetProcAddress((const GLubyte*)"glMultTransposeMatrixdARB")) == nullptr) || r;
  r = ((glMultTransposeMatrixfARB = (PFNGLMULTTRANSPOSEMATRIXFARBPROC)glewGetProcAddress((const GLubyte*)"glMultTransposeMatrixfARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_transpose_matrix */

#ifdef GL_ARB_uniform_buffer_object

static GLboolean _glewInit_GL_ARB_uniform_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)glewGetProcAddress((const GLubyte*)"glBindBufferBase")) == nullptr) || r;
  r = ((glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glBindBufferRange")) == nullptr) || r;
  r = ((glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformBlockName")) == nullptr) || r;
  r = ((glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformBlockiv")) == nullptr) || r;
  r = ((glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformName")) == nullptr) || r;
  r = ((glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformsiv")) == nullptr) || r;
  r = ((glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)glewGetProcAddress((const GLubyte*)"glGetIntegeri_v")) == nullptr) || r;
  r = ((glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)glewGetProcAddress((const GLubyte*)"glGetUniformBlockIndex")) == nullptr) || r;
  r = ((glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)glewGetProcAddress((const GLubyte*)"glGetUniformIndices")) == nullptr) || r;
  r = ((glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)glewGetProcAddress((const GLubyte*)"glUniformBlockBinding")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_uniform_buffer_object */

#ifdef GL_ARB_vertex_array_bgra

#endif /* GL_ARB_vertex_array_bgra */

#ifdef GL_ARB_vertex_array_object

static GLboolean _glewInit_GL_ARB_vertex_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glewGetProcAddress((const GLubyte*)"glBindVertexArray")) == nullptr) || r;
  r = ((glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glewGetProcAddress((const GLubyte*)"glDeleteVertexArrays")) == nullptr) || r;
  r = ((glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glewGetProcAddress((const GLubyte*)"glGenVertexArrays")) == nullptr) || r;
  r = ((glIsVertexArray = (PFNGLISVERTEXARRAYPROC)glewGetProcAddress((const GLubyte*)"glIsVertexArray")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_array_object */

#ifdef GL_ARB_vertex_attrib_64bit

static GLboolean _glewInit_GL_ARB_vertex_attrib_64bit (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVertexAttribLdv = (PFNGLGETVERTEXATTRIBLDVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribLdv")) == nullptr) || r;
  r = ((glVertexAttribL1d = (PFNGLVERTEXATTRIBL1DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1d")) == nullptr) || r;
  r = ((glVertexAttribL1dv = (PFNGLVERTEXATTRIBL1DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1dv")) == nullptr) || r;
  r = ((glVertexAttribL2d = (PFNGLVERTEXATTRIBL2DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2d")) == nullptr) || r;
  r = ((glVertexAttribL2dv = (PFNGLVERTEXATTRIBL2DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2dv")) == nullptr) || r;
  r = ((glVertexAttribL3d = (PFNGLVERTEXATTRIBL3DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3d")) == nullptr) || r;
  r = ((glVertexAttribL3dv = (PFNGLVERTEXATTRIBL3DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3dv")) == nullptr) || r;
  r = ((glVertexAttribL4d = (PFNGLVERTEXATTRIBL4DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4d")) == nullptr) || r;
  r = ((glVertexAttribL4dv = (PFNGLVERTEXATTRIBL4DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4dv")) == nullptr) || r;
  r = ((glVertexAttribLPointer = (PFNGLVERTEXATTRIBLPOINTERPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribLPointer")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_attrib_64bit */

#ifdef GL_ARB_vertex_blend

static GLboolean _glewInit_GL_ARB_vertex_blend (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexBlendARB = (PFNGLVERTEXBLENDARBPROC)glewGetProcAddress((const GLubyte*)"glVertexBlendARB")) == nullptr) || r;
  r = ((glWeightPointerARB = (PFNGLWEIGHTPOINTERARBPROC)glewGetProcAddress((const GLubyte*)"glWeightPointerARB")) == nullptr) || r;
  r = ((glWeightbvARB = (PFNGLWEIGHTBVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightbvARB")) == nullptr) || r;
  r = ((glWeightdvARB = (PFNGLWEIGHTDVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightdvARB")) == nullptr) || r;
  r = ((glWeightfvARB = (PFNGLWEIGHTFVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightfvARB")) == nullptr) || r;
  r = ((glWeightivARB = (PFNGLWEIGHTIVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightivARB")) == nullptr) || r;
  r = ((glWeightsvARB = (PFNGLWEIGHTSVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightsvARB")) == nullptr) || r;
  r = ((glWeightubvARB = (PFNGLWEIGHTUBVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightubvARB")) == nullptr) || r;
  r = ((glWeightuivARB = (PFNGLWEIGHTUIVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightuivARB")) == nullptr) || r;
  r = ((glWeightusvARB = (PFNGLWEIGHTUSVARBPROC)glewGetProcAddress((const GLubyte*)"glWeightusvARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_blend */

#ifdef GL_ARB_vertex_buffer_object

static GLboolean _glewInit_GL_ARB_vertex_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindBufferARB = (PFNGLBINDBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"glBindBufferARB")) == nullptr) || r;
  r = ((glBufferDataARB = (PFNGLBUFFERDATAARBPROC)glewGetProcAddress((const GLubyte*)"glBufferDataARB")) == nullptr) || r;
  r = ((glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)glewGetProcAddress((const GLubyte*)"glBufferSubDataARB")) == nullptr) || r;
  r = ((glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)glewGetProcAddress((const GLubyte*)"glDeleteBuffersARB")) == nullptr) || r;
  r = ((glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)glewGetProcAddress((const GLubyte*)"glGenBuffersARB")) == nullptr) || r;
  r = ((glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetBufferParameterivARB")) == nullptr) || r;
  r = ((glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)glewGetProcAddress((const GLubyte*)"glGetBufferPointervARB")) == nullptr) || r;
  r = ((glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)glewGetProcAddress((const GLubyte*)"glGetBufferSubDataARB")) == nullptr) || r;
  r = ((glIsBufferARB = (PFNGLISBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"glIsBufferARB")) == nullptr) || r;
  r = ((glMapBufferARB = (PFNGLMAPBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"glMapBufferARB")) == nullptr) || r;
  r = ((glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"glUnmapBufferARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_buffer_object */

#ifdef GL_ARB_vertex_program

static GLboolean _glewInit_GL_ARB_vertex_program (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)glewGetProcAddress((const GLubyte*)"glBindProgramARB")) == nullptr) || r;
  r = ((glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)glewGetProcAddress((const GLubyte*)"glDeleteProgramsARB")) == nullptr) || r;
  r = ((glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexAttribArrayARB")) == nullptr) || r;
  r = ((glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexAttribArrayARB")) == nullptr) || r;
  r = ((glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)glewGetProcAddress((const GLubyte*)"glGenProgramsARB")) == nullptr) || r;
  r = ((glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetProgramEnvParameterdvARB")) == nullptr) || r;
  r = ((glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetProgramEnvParameterfvARB")) == nullptr) || r;
  r = ((glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetProgramLocalParameterdvARB")) == nullptr) || r;
  r = ((glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetProgramLocalParameterfvARB")) == nullptr) || r;
  r = ((glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"glGetProgramStringARB")) == nullptr) || r;
  r = ((glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetProgramivARB")) == nullptr) || r;
  r = ((glGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribPointervARB")) == nullptr) || r;
  r = ((glGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribdvARB")) == nullptr) || r;
  r = ((glGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribfvARB")) == nullptr) || r;
  r = ((glGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribivARB")) == nullptr) || r;
  r = ((glIsProgramARB = (PFNGLISPROGRAMARBPROC)glewGetProcAddress((const GLubyte*)"glIsProgramARB")) == nullptr) || r;
  r = ((glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameter4dARB")) == nullptr) || r;
  r = ((glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameter4dvARB")) == nullptr) || r;
  r = ((glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameter4fARB")) == nullptr) || r;
  r = ((glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameter4fvARB")) == nullptr) || r;
  r = ((glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameter4dARB")) == nullptr) || r;
  r = ((glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameter4dvARB")) == nullptr) || r;
  r = ((glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameter4fARB")) == nullptr) || r;
  r = ((glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameter4fvARB")) == nullptr) || r;
  r = ((glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"glProgramStringARB")) == nullptr) || r;
  r = ((glVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1dARB")) == nullptr) || r;
  r = ((glVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1dvARB")) == nullptr) || r;
  r = ((glVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1fARB")) == nullptr) || r;
  r = ((glVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1fvARB")) == nullptr) || r;
  r = ((glVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1sARB")) == nullptr) || r;
  r = ((glVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1svARB")) == nullptr) || r;
  r = ((glVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2dARB")) == nullptr) || r;
  r = ((glVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2dvARB")) == nullptr) || r;
  r = ((glVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2fARB")) == nullptr) || r;
  r = ((glVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2fvARB")) == nullptr) || r;
  r = ((glVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2sARB")) == nullptr) || r;
  r = ((glVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2svARB")) == nullptr) || r;
  r = ((glVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3dARB")) == nullptr) || r;
  r = ((glVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3dvARB")) == nullptr) || r;
  r = ((glVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3fARB")) == nullptr) || r;
  r = ((glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3fvARB")) == nullptr) || r;
  r = ((glVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3sARB")) == nullptr) || r;
  r = ((glVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3svARB")) == nullptr) || r;
  r = ((glVertexAttrib4NbvARB = (PFNGLVERTEXATTRIB4NBVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NbvARB")) == nullptr) || r;
  r = ((glVertexAttrib4NivARB = (PFNGLVERTEXATTRIB4NIVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NivARB")) == nullptr) || r;
  r = ((glVertexAttrib4NsvARB = (PFNGLVERTEXATTRIB4NSVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NsvARB")) == nullptr) || r;
  r = ((glVertexAttrib4NubARB = (PFNGLVERTEXATTRIB4NUBARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NubARB")) == nullptr) || r;
  r = ((glVertexAttrib4NubvARB = (PFNGLVERTEXATTRIB4NUBVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NubvARB")) == nullptr) || r;
  r = ((glVertexAttrib4NuivARB = (PFNGLVERTEXATTRIB4NUIVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NuivARB")) == nullptr) || r;
  r = ((glVertexAttrib4NusvARB = (PFNGLVERTEXATTRIB4NUSVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4NusvARB")) == nullptr) || r;
  r = ((glVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4bvARB")) == nullptr) || r;
  r = ((glVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4dARB")) == nullptr) || r;
  r = ((glVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4dvARB")) == nullptr) || r;
  r = ((glVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4fARB")) == nullptr) || r;
  r = ((glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4fvARB")) == nullptr) || r;
  r = ((glVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4ivARB")) == nullptr) || r;
  r = ((glVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4sARB")) == nullptr) || r;
  r = ((glVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4svARB")) == nullptr) || r;
  r = ((glVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4ubvARB")) == nullptr) || r;
  r = ((glVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4uivARB")) == nullptr) || r;
  r = ((glVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4usvARB")) == nullptr) || r;
  r = ((glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribPointerARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_program */

#ifdef GL_ARB_vertex_shader

static GLboolean _glewInit_GL_ARB_vertex_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)glewGetProcAddress((const GLubyte*)"glBindAttribLocationARB")) == nullptr) || r;
  r = ((glGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)glewGetProcAddress((const GLubyte*)"glGetActiveAttribARB")) == nullptr) || r;
  r = ((glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)glewGetProcAddress((const GLubyte*)"glGetAttribLocationARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_shader */

#ifdef GL_ARB_vertex_type_2_10_10_10_rev

static GLboolean _glewInit_GL_ARB_vertex_type_2_10_10_10_rev (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorP3ui = (PFNGLCOLORP3UIPROC)glewGetProcAddress((const GLubyte*)"glColorP3ui")) == nullptr) || r;
  r = ((glColorP3uiv = (PFNGLCOLORP3UIVPROC)glewGetProcAddress((const GLubyte*)"glColorP3uiv")) == nullptr) || r;
  r = ((glColorP4ui = (PFNGLCOLORP4UIPROC)glewGetProcAddress((const GLubyte*)"glColorP4ui")) == nullptr) || r;
  r = ((glColorP4uiv = (PFNGLCOLORP4UIVPROC)glewGetProcAddress((const GLubyte*)"glColorP4uiv")) == nullptr) || r;
  r = ((glMultiTexCoordP1ui = (PFNGLMULTITEXCOORDP1UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP1ui")) == nullptr) || r;
  r = ((glMultiTexCoordP1uiv = (PFNGLMULTITEXCOORDP1UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP1uiv")) == nullptr) || r;
  r = ((glMultiTexCoordP2ui = (PFNGLMULTITEXCOORDP2UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP2ui")) == nullptr) || r;
  r = ((glMultiTexCoordP2uiv = (PFNGLMULTITEXCOORDP2UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP2uiv")) == nullptr) || r;
  r = ((glMultiTexCoordP3ui = (PFNGLMULTITEXCOORDP3UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP3ui")) == nullptr) || r;
  r = ((glMultiTexCoordP3uiv = (PFNGLMULTITEXCOORDP3UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP3uiv")) == nullptr) || r;
  r = ((glMultiTexCoordP4ui = (PFNGLMULTITEXCOORDP4UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP4ui")) == nullptr) || r;
  r = ((glMultiTexCoordP4uiv = (PFNGLMULTITEXCOORDP4UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP4uiv")) == nullptr) || r;
  r = ((glNormalP3ui = (PFNGLNORMALP3UIPROC)glewGetProcAddress((const GLubyte*)"glNormalP3ui")) == nullptr) || r;
  r = ((glNormalP3uiv = (PFNGLNORMALP3UIVPROC)glewGetProcAddress((const GLubyte*)"glNormalP3uiv")) == nullptr) || r;
  r = ((glSecondaryColorP3ui = (PFNGLSECONDARYCOLORP3UIPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorP3ui")) == nullptr) || r;
  r = ((glSecondaryColorP3uiv = (PFNGLSECONDARYCOLORP3UIVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorP3uiv")) == nullptr) || r;
  r = ((glTexCoordP1ui = (PFNGLTEXCOORDP1UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP1ui")) == nullptr) || r;
  r = ((glTexCoordP1uiv = (PFNGLTEXCOORDP1UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP1uiv")) == nullptr) || r;
  r = ((glTexCoordP2ui = (PFNGLTEXCOORDP2UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP2ui")) == nullptr) || r;
  r = ((glTexCoordP2uiv = (PFNGLTEXCOORDP2UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP2uiv")) == nullptr) || r;
  r = ((glTexCoordP3ui = (PFNGLTEXCOORDP3UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP3ui")) == nullptr) || r;
  r = ((glTexCoordP3uiv = (PFNGLTEXCOORDP3UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP3uiv")) == nullptr) || r;
  r = ((glTexCoordP4ui = (PFNGLTEXCOORDP4UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP4ui")) == nullptr) || r;
  r = ((glTexCoordP4uiv = (PFNGLTEXCOORDP4UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP4uiv")) == nullptr) || r;
  r = ((glVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP1ui")) == nullptr) || r;
  r = ((glVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP1uiv")) == nullptr) || r;
  r = ((glVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP2ui")) == nullptr) || r;
  r = ((glVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP2uiv")) == nullptr) || r;
  r = ((glVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP3ui")) == nullptr) || r;
  r = ((glVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP3uiv")) == nullptr) || r;
  r = ((glVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP4ui")) == nullptr) || r;
  r = ((glVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP4uiv")) == nullptr) || r;
  r = ((glVertexP2ui = (PFNGLVERTEXP2UIPROC)glewGetProcAddress((const GLubyte*)"glVertexP2ui")) == nullptr) || r;
  r = ((glVertexP2uiv = (PFNGLVERTEXP2UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexP2uiv")) == nullptr) || r;
  r = ((glVertexP3ui = (PFNGLVERTEXP3UIPROC)glewGetProcAddress((const GLubyte*)"glVertexP3ui")) == nullptr) || r;
  r = ((glVertexP3uiv = (PFNGLVERTEXP3UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexP3uiv")) == nullptr) || r;
  r = ((glVertexP4ui = (PFNGLVERTEXP4UIPROC)glewGetProcAddress((const GLubyte*)"glVertexP4ui")) == nullptr) || r;
  r = ((glVertexP4uiv = (PFNGLVERTEXP4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexP4uiv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_vertex_type_2_10_10_10_rev */

#ifdef GL_ARB_viewport_array

static GLboolean _glewInit_GL_ARB_viewport_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDepthRangeArrayv = (PFNGLDEPTHRANGEARRAYVPROC)glewGetProcAddress((const GLubyte*)"glDepthRangeArrayv")) == nullptr) || r;
  r = ((glDepthRangeIndexed = (PFNGLDEPTHRANGEINDEXEDPROC)glewGetProcAddress((const GLubyte*)"glDepthRangeIndexed")) == nullptr) || r;
  r = ((glGetDoublei_v = (PFNGLGETDOUBLEI_VPROC)glewGetProcAddress((const GLubyte*)"glGetDoublei_v")) == nullptr) || r;
  r = ((glGetFloati_v = (PFNGLGETFLOATI_VPROC)glewGetProcAddress((const GLubyte*)"glGetFloati_v")) == nullptr) || r;
  r = ((glScissorArrayv = (PFNGLSCISSORARRAYVPROC)glewGetProcAddress((const GLubyte*)"glScissorArrayv")) == nullptr) || r;
  r = ((glScissorIndexed = (PFNGLSCISSORINDEXEDPROC)glewGetProcAddress((const GLubyte*)"glScissorIndexed")) == nullptr) || r;
  r = ((glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC)glewGetProcAddress((const GLubyte*)"glScissorIndexedv")) == nullptr) || r;
  r = ((glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC)glewGetProcAddress((const GLubyte*)"glViewportArrayv")) == nullptr) || r;
  r = ((glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC)glewGetProcAddress((const GLubyte*)"glViewportIndexedf")) == nullptr) || r;
  r = ((glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC)glewGetProcAddress((const GLubyte*)"glViewportIndexedfv")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_viewport_array */

#ifdef GL_ARB_window_pos

static GLboolean _glewInit_GL_ARB_window_pos (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glWindowPos2dARB = (PFNGLWINDOWPOS2DARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2dARB")) == nullptr) || r;
  r = ((glWindowPos2dvARB = (PFNGLWINDOWPOS2DVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2dvARB")) == nullptr) || r;
  r = ((glWindowPos2fARB = (PFNGLWINDOWPOS2FARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2fARB")) == nullptr) || r;
  r = ((glWindowPos2fvARB = (PFNGLWINDOWPOS2FVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2fvARB")) == nullptr) || r;
  r = ((glWindowPos2iARB = (PFNGLWINDOWPOS2IARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2iARB")) == nullptr) || r;
  r = ((glWindowPos2ivARB = (PFNGLWINDOWPOS2IVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2ivARB")) == nullptr) || r;
  r = ((glWindowPos2sARB = (PFNGLWINDOWPOS2SARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2sARB")) == nullptr) || r;
  r = ((glWindowPos2svARB = (PFNGLWINDOWPOS2SVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2svARB")) == nullptr) || r;
  r = ((glWindowPos3dARB = (PFNGLWINDOWPOS3DARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3dARB")) == nullptr) || r;
  r = ((glWindowPos3dvARB = (PFNGLWINDOWPOS3DVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3dvARB")) == nullptr) || r;
  r = ((glWindowPos3fARB = (PFNGLWINDOWPOS3FARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3fARB")) == nullptr) || r;
  r = ((glWindowPos3fvARB = (PFNGLWINDOWPOS3FVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3fvARB")) == nullptr) || r;
  r = ((glWindowPos3iARB = (PFNGLWINDOWPOS3IARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3iARB")) == nullptr) || r;
  r = ((glWindowPos3ivARB = (PFNGLWINDOWPOS3IVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3ivARB")) == nullptr) || r;
  r = ((glWindowPos3sARB = (PFNGLWINDOWPOS3SARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3sARB")) == nullptr) || r;
  r = ((glWindowPos3svARB = (PFNGLWINDOWPOS3SVARBPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3svARB")) == nullptr) || r;

  return r;
}

#endif /* GL_ARB_window_pos */

#ifdef GL_ATIX_point_sprites

#endif /* GL_ATIX_point_sprites */

#ifdef GL_ATIX_texture_env_combine3

#endif /* GL_ATIX_texture_env_combine3 */

#ifdef GL_ATIX_texture_env_route

#endif /* GL_ATIX_texture_env_route */

#ifdef GL_ATIX_vertex_shader_output_point_size

#endif /* GL_ATIX_vertex_shader_output_point_size */

#ifdef GL_ATI_draw_buffers

static GLboolean _glewInit_GL_ATI_draw_buffers (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawBuffersATI = (PFNGLDRAWBUFFERSATIPROC)glewGetProcAddress((const GLubyte*)"glDrawBuffersATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_draw_buffers */

#ifdef GL_ATI_element_array

static GLboolean _glewInit_GL_ATI_element_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawElementArrayATI = (PFNGLDRAWELEMENTARRAYATIPROC)glewGetProcAddress((const GLubyte*)"glDrawElementArrayATI")) == nullptr) || r;
  r = ((glDrawRangeElementArrayATI = (PFNGLDRAWRANGEELEMENTARRAYATIPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElementArrayATI")) == nullptr) || r;
  r = ((glElementPointerATI = (PFNGLELEMENTPOINTERATIPROC)glewGetProcAddress((const GLubyte*)"glElementPointerATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_element_array */

#ifdef GL_ATI_envmap_bumpmap

static GLboolean _glewInit_GL_ATI_envmap_bumpmap (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetTexBumpParameterfvATI = (PFNGLGETTEXBUMPPARAMETERFVATIPROC)glewGetProcAddress((const GLubyte*)"glGetTexBumpParameterfvATI")) == nullptr) || r;
  r = ((glGetTexBumpParameterivATI = (PFNGLGETTEXBUMPPARAMETERIVATIPROC)glewGetProcAddress((const GLubyte*)"glGetTexBumpParameterivATI")) == nullptr) || r;
  r = ((glTexBumpParameterfvATI = (PFNGLTEXBUMPPARAMETERFVATIPROC)glewGetProcAddress((const GLubyte*)"glTexBumpParameterfvATI")) == nullptr) || r;
  r = ((glTexBumpParameterivATI = (PFNGLTEXBUMPPARAMETERIVATIPROC)glewGetProcAddress((const GLubyte*)"glTexBumpParameterivATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_envmap_bumpmap */

#ifdef GL_ATI_fragment_shader

static GLboolean _glewInit_GL_ATI_fragment_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAlphaFragmentOp1ATI = (PFNGLALPHAFRAGMENTOP1ATIPROC)glewGetProcAddress((const GLubyte*)"glAlphaFragmentOp1ATI")) == nullptr) || r;
  r = ((glAlphaFragmentOp2ATI = (PFNGLALPHAFRAGMENTOP2ATIPROC)glewGetProcAddress((const GLubyte*)"glAlphaFragmentOp2ATI")) == nullptr) || r;
  r = ((glAlphaFragmentOp3ATI = (PFNGLALPHAFRAGMENTOP3ATIPROC)glewGetProcAddress((const GLubyte*)"glAlphaFragmentOp3ATI")) == nullptr) || r;
  r = ((glBeginFragmentShaderATI = (PFNGLBEGINFRAGMENTSHADERATIPROC)glewGetProcAddress((const GLubyte*)"glBeginFragmentShaderATI")) == nullptr) || r;
  r = ((glBindFragmentShaderATI = (PFNGLBINDFRAGMENTSHADERATIPROC)glewGetProcAddress((const GLubyte*)"glBindFragmentShaderATI")) == nullptr) || r;
  r = ((glColorFragmentOp1ATI = (PFNGLCOLORFRAGMENTOP1ATIPROC)glewGetProcAddress((const GLubyte*)"glColorFragmentOp1ATI")) == nullptr) || r;
  r = ((glColorFragmentOp2ATI = (PFNGLCOLORFRAGMENTOP2ATIPROC)glewGetProcAddress((const GLubyte*)"glColorFragmentOp2ATI")) == nullptr) || r;
  r = ((glColorFragmentOp3ATI = (PFNGLCOLORFRAGMENTOP3ATIPROC)glewGetProcAddress((const GLubyte*)"glColorFragmentOp3ATI")) == nullptr) || r;
  r = ((glDeleteFragmentShaderATI = (PFNGLDELETEFRAGMENTSHADERATIPROC)glewGetProcAddress((const GLubyte*)"glDeleteFragmentShaderATI")) == nullptr) || r;
  r = ((glEndFragmentShaderATI = (PFNGLENDFRAGMENTSHADERATIPROC)glewGetProcAddress((const GLubyte*)"glEndFragmentShaderATI")) == nullptr) || r;
  r = ((glGenFragmentShadersATI = (PFNGLGENFRAGMENTSHADERSATIPROC)glewGetProcAddress((const GLubyte*)"glGenFragmentShadersATI")) == nullptr) || r;
  r = ((glPassTexCoordATI = (PFNGLPASSTEXCOORDATIPROC)glewGetProcAddress((const GLubyte*)"glPassTexCoordATI")) == nullptr) || r;
  r = ((glSampleMapATI = (PFNGLSAMPLEMAPATIPROC)glewGetProcAddress((const GLubyte*)"glSampleMapATI")) == nullptr) || r;
  r = ((glSetFragmentShaderConstantATI = (PFNGLSETFRAGMENTSHADERCONSTANTATIPROC)glewGetProcAddress((const GLubyte*)"glSetFragmentShaderConstantATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_fragment_shader */

#ifdef GL_ATI_map_object_buffer

static GLboolean _glewInit_GL_ATI_map_object_buffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMapObjectBufferATI = (PFNGLMAPOBJECTBUFFERATIPROC)glewGetProcAddress((const GLubyte*)"glMapObjectBufferATI")) == nullptr) || r;
  r = ((glUnmapObjectBufferATI = (PFNGLUNMAPOBJECTBUFFERATIPROC)glewGetProcAddress((const GLubyte*)"glUnmapObjectBufferATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_map_object_buffer */

#ifdef GL_ATI_meminfo

#endif /* GL_ATI_meminfo */

#ifdef GL_ATI_pn_triangles

static GLboolean _glewInit_GL_ATI_pn_triangles (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPNTrianglesfATI = (PFNGLPNTRIANGLESFATIPROC)glewGetProcAddress((const GLubyte*)"glPNTrianglesfATI")) == nullptr) || r;
  r = ((glPNTrianglesiATI = (PFNGLPNTRIANGLESIATIPROC)glewGetProcAddress((const GLubyte*)"glPNTrianglesiATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_pn_triangles */

#ifdef GL_ATI_separate_stencil

static GLboolean _glewInit_GL_ATI_separate_stencil (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glStencilFuncSeparateATI = (PFNGLSTENCILFUNCSEPARATEATIPROC)glewGetProcAddress((const GLubyte*)"glStencilFuncSeparateATI")) == nullptr) || r;
  r = ((glStencilOpSeparateATI = (PFNGLSTENCILOPSEPARATEATIPROC)glewGetProcAddress((const GLubyte*)"glStencilOpSeparateATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_separate_stencil */

#ifdef GL_ATI_shader_texture_lod

#endif /* GL_ATI_shader_texture_lod */

#ifdef GL_ATI_text_fragment_shader

#endif /* GL_ATI_text_fragment_shader */

#ifdef GL_ATI_texture_compression_3dc

#endif /* GL_ATI_texture_compression_3dc */

#ifdef GL_ATI_texture_env_combine3

#endif /* GL_ATI_texture_env_combine3 */

#ifdef GL_ATI_texture_float

#endif /* GL_ATI_texture_float */

#ifdef GL_ATI_texture_mirror_once

#endif /* GL_ATI_texture_mirror_once */

#ifdef GL_ATI_vertex_array_object

static GLboolean _glewInit_GL_ATI_vertex_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glArrayObjectATI = (PFNGLARRAYOBJECTATIPROC)glewGetProcAddress((const GLubyte*)"glArrayObjectATI")) == nullptr) || r;
  r = ((glFreeObjectBufferATI = (PFNGLFREEOBJECTBUFFERATIPROC)glewGetProcAddress((const GLubyte*)"glFreeObjectBufferATI")) == nullptr) || r;
  r = ((glGetArrayObjectfvATI = (PFNGLGETARRAYOBJECTFVATIPROC)glewGetProcAddress((const GLubyte*)"glGetArrayObjectfvATI")) == nullptr) || r;
  r = ((glGetArrayObjectivATI = (PFNGLGETARRAYOBJECTIVATIPROC)glewGetProcAddress((const GLubyte*)"glGetArrayObjectivATI")) == nullptr) || r;
  r = ((glGetObjectBufferfvATI = (PFNGLGETOBJECTBUFFERFVATIPROC)glewGetProcAddress((const GLubyte*)"glGetObjectBufferfvATI")) == nullptr) || r;
  r = ((glGetObjectBufferivATI = (PFNGLGETOBJECTBUFFERIVATIPROC)glewGetProcAddress((const GLubyte*)"glGetObjectBufferivATI")) == nullptr) || r;
  r = ((glGetVariantArrayObjectfvATI = (PFNGLGETVARIANTARRAYOBJECTFVATIPROC)glewGetProcAddress((const GLubyte*)"glGetVariantArrayObjectfvATI")) == nullptr) || r;
  r = ((glGetVariantArrayObjectivATI = (PFNGLGETVARIANTARRAYOBJECTIVATIPROC)glewGetProcAddress((const GLubyte*)"glGetVariantArrayObjectivATI")) == nullptr) || r;
  r = ((glIsObjectBufferATI = (PFNGLISOBJECTBUFFERATIPROC)glewGetProcAddress((const GLubyte*)"glIsObjectBufferATI")) == nullptr) || r;
  r = ((glNewObjectBufferATI = (PFNGLNEWOBJECTBUFFERATIPROC)glewGetProcAddress((const GLubyte*)"glNewObjectBufferATI")) == nullptr) || r;
  r = ((glUpdateObjectBufferATI = (PFNGLUPDATEOBJECTBUFFERATIPROC)glewGetProcAddress((const GLubyte*)"glUpdateObjectBufferATI")) == nullptr) || r;
  r = ((glVariantArrayObjectATI = (PFNGLVARIANTARRAYOBJECTATIPROC)glewGetProcAddress((const GLubyte*)"glVariantArrayObjectATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_vertex_array_object */

#ifdef GL_ATI_vertex_attrib_array_object

static GLboolean _glewInit_GL_ATI_vertex_attrib_array_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVertexAttribArrayObjectfvATI = (PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribArrayObjectfvATI")) == nullptr) || r;
  r = ((glGetVertexAttribArrayObjectivATI = (PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribArrayObjectivATI")) == nullptr) || r;
  r = ((glVertexAttribArrayObjectATI = (PFNGLVERTEXATTRIBARRAYOBJECTATIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribArrayObjectATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_vertex_attrib_array_object */

#ifdef GL_ATI_vertex_streams

static GLboolean _glewInit_GL_ATI_vertex_streams (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClientActiveVertexStreamATI = (PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC)glewGetProcAddress((const GLubyte*)"glClientActiveVertexStreamATI")) == nullptr) || r;
  r = ((glNormalStream3bATI = (PFNGLNORMALSTREAM3BATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3bATI")) == nullptr) || r;
  r = ((glNormalStream3bvATI = (PFNGLNORMALSTREAM3BVATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3bvATI")) == nullptr) || r;
  r = ((glNormalStream3dATI = (PFNGLNORMALSTREAM3DATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3dATI")) == nullptr) || r;
  r = ((glNormalStream3dvATI = (PFNGLNORMALSTREAM3DVATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3dvATI")) == nullptr) || r;
  r = ((glNormalStream3fATI = (PFNGLNORMALSTREAM3FATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3fATI")) == nullptr) || r;
  r = ((glNormalStream3fvATI = (PFNGLNORMALSTREAM3FVATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3fvATI")) == nullptr) || r;
  r = ((glNormalStream3iATI = (PFNGLNORMALSTREAM3IATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3iATI")) == nullptr) || r;
  r = ((glNormalStream3ivATI = (PFNGLNORMALSTREAM3IVATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3ivATI")) == nullptr) || r;
  r = ((glNormalStream3sATI = (PFNGLNORMALSTREAM3SATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3sATI")) == nullptr) || r;
  r = ((glNormalStream3svATI = (PFNGLNORMALSTREAM3SVATIPROC)glewGetProcAddress((const GLubyte*)"glNormalStream3svATI")) == nullptr) || r;
  r = ((glVertexBlendEnvfATI = (PFNGLVERTEXBLENDENVFATIPROC)glewGetProcAddress((const GLubyte*)"glVertexBlendEnvfATI")) == nullptr) || r;
  r = ((glVertexBlendEnviATI = (PFNGLVERTEXBLENDENVIATIPROC)glewGetProcAddress((const GLubyte*)"glVertexBlendEnviATI")) == nullptr) || r;
  r = ((glVertexStream2dATI = (PFNGLVERTEXSTREAM2DATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2dATI")) == nullptr) || r;
  r = ((glVertexStream2dvATI = (PFNGLVERTEXSTREAM2DVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2dvATI")) == nullptr) || r;
  r = ((glVertexStream2fATI = (PFNGLVERTEXSTREAM2FATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2fATI")) == nullptr) || r;
  r = ((glVertexStream2fvATI = (PFNGLVERTEXSTREAM2FVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2fvATI")) == nullptr) || r;
  r = ((glVertexStream2iATI = (PFNGLVERTEXSTREAM2IATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2iATI")) == nullptr) || r;
  r = ((glVertexStream2ivATI = (PFNGLVERTEXSTREAM2IVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2ivATI")) == nullptr) || r;
  r = ((glVertexStream2sATI = (PFNGLVERTEXSTREAM2SATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2sATI")) == nullptr) || r;
  r = ((glVertexStream2svATI = (PFNGLVERTEXSTREAM2SVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream2svATI")) == nullptr) || r;
  r = ((glVertexStream3dATI = (PFNGLVERTEXSTREAM3DATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3dATI")) == nullptr) || r;
  r = ((glVertexStream3dvATI = (PFNGLVERTEXSTREAM3DVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3dvATI")) == nullptr) || r;
  r = ((glVertexStream3fATI = (PFNGLVERTEXSTREAM3FATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3fATI")) == nullptr) || r;
  r = ((glVertexStream3fvATI = (PFNGLVERTEXSTREAM3FVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3fvATI")) == nullptr) || r;
  r = ((glVertexStream3iATI = (PFNGLVERTEXSTREAM3IATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3iATI")) == nullptr) || r;
  r = ((glVertexStream3ivATI = (PFNGLVERTEXSTREAM3IVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3ivATI")) == nullptr) || r;
  r = ((glVertexStream3sATI = (PFNGLVERTEXSTREAM3SATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3sATI")) == nullptr) || r;
  r = ((glVertexStream3svATI = (PFNGLVERTEXSTREAM3SVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream3svATI")) == nullptr) || r;
  r = ((glVertexStream4dATI = (PFNGLVERTEXSTREAM4DATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4dATI")) == nullptr) || r;
  r = ((glVertexStream4dvATI = (PFNGLVERTEXSTREAM4DVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4dvATI")) == nullptr) || r;
  r = ((glVertexStream4fATI = (PFNGLVERTEXSTREAM4FATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4fATI")) == nullptr) || r;
  r = ((glVertexStream4fvATI = (PFNGLVERTEXSTREAM4FVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4fvATI")) == nullptr) || r;
  r = ((glVertexStream4iATI = (PFNGLVERTEXSTREAM4IATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4iATI")) == nullptr) || r;
  r = ((glVertexStream4ivATI = (PFNGLVERTEXSTREAM4IVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4ivATI")) == nullptr) || r;
  r = ((glVertexStream4sATI = (PFNGLVERTEXSTREAM4SATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4sATI")) == nullptr) || r;
  r = ((glVertexStream4svATI = (PFNGLVERTEXSTREAM4SVATIPROC)glewGetProcAddress((const GLubyte*)"glVertexStream4svATI")) == nullptr) || r;

  return r;
}

#endif /* GL_ATI_vertex_streams */

#ifdef GL_EXT_422_pixels

#endif /* GL_EXT_422_pixels */

#ifdef GL_EXT_Cg_shader

#endif /* GL_EXT_Cg_shader */

#ifdef GL_EXT_abgr

#endif /* GL_EXT_abgr */

#ifdef GL_EXT_bgra

#endif /* GL_EXT_bgra */

#ifdef GL_EXT_bindable_uniform

static GLboolean _glewInit_GL_EXT_bindable_uniform (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetUniformBufferSizeEXT = (PFNGLGETUNIFORMBUFFERSIZEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetUniformBufferSizeEXT")) == nullptr) || r;
  r = ((glGetUniformOffsetEXT = (PFNGLGETUNIFORMOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glGetUniformOffsetEXT")) == nullptr) || r;
  r = ((glUniformBufferEXT = (PFNGLUNIFORMBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glUniformBufferEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_bindable_uniform */

#ifdef GL_EXT_blend_color

static GLboolean _glewInit_GL_EXT_blend_color (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendColorEXT = (PFNGLBLENDCOLOREXTPROC)glewGetProcAddress((const GLubyte*)"glBlendColorEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_blend_color */

#ifdef GL_EXT_blend_equation_separate

static GLboolean _glewInit_GL_EXT_blend_equation_separate (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationSeparateEXT = (PFNGLBLENDEQUATIONSEPARATEEXTPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparateEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_blend_equation_separate */

#ifdef GL_EXT_blend_func_separate

static GLboolean _glewInit_GL_EXT_blend_func_separate (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparateEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_blend_func_separate */

#ifdef GL_EXT_blend_logic_op

#endif /* GL_EXT_blend_logic_op */

#ifdef GL_EXT_blend_minmax

static GLboolean _glewInit_GL_EXT_blend_minmax (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationEXT = (PFNGLBLENDEQUATIONEXTPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_blend_minmax */

#ifdef GL_EXT_blend_subtract

#endif /* GL_EXT_blend_subtract */

#ifdef GL_EXT_clip_volume_hint

#endif /* GL_EXT_clip_volume_hint */

#ifdef GL_EXT_cmyka

#endif /* GL_EXT_cmyka */

#ifdef GL_EXT_color_subtable

static GLboolean _glewInit_GL_EXT_color_subtable (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorSubTableEXT = (PFNGLCOLORSUBTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"glColorSubTableEXT")) == nullptr) || r;
  r = ((glCopyColorSubTableEXT = (PFNGLCOPYCOLORSUBTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyColorSubTableEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_color_subtable */

#ifdef GL_EXT_compiled_vertex_array

static GLboolean _glewInit_GL_EXT_compiled_vertex_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glLockArraysEXT = (PFNGLLOCKARRAYSEXTPROC)glewGetProcAddress((const GLubyte*)"glLockArraysEXT")) == nullptr) || r;
  r = ((glUnlockArraysEXT = (PFNGLUNLOCKARRAYSEXTPROC)glewGetProcAddress((const GLubyte*)"glUnlockArraysEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_compiled_vertex_array */

#ifdef GL_EXT_convolution

static GLboolean _glewInit_GL_EXT_convolution (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glConvolutionFilter1DEXT = (PFNGLCONVOLUTIONFILTER1DEXTPROC)glewGetProcAddress((const GLubyte*)"glConvolutionFilter1DEXT")) == nullptr) || r;
  r = ((glConvolutionFilter2DEXT = (PFNGLCONVOLUTIONFILTER2DEXTPROC)glewGetProcAddress((const GLubyte*)"glConvolutionFilter2DEXT")) == nullptr) || r;
  r = ((glConvolutionParameterfEXT = (PFNGLCONVOLUTIONPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterfEXT")) == nullptr) || r;
  r = ((glConvolutionParameterfvEXT = (PFNGLCONVOLUTIONPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterfvEXT")) == nullptr) || r;
  r = ((glConvolutionParameteriEXT = (PFNGLCONVOLUTIONPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameteriEXT")) == nullptr) || r;
  r = ((glConvolutionParameterivEXT = (PFNGLCONVOLUTIONPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterivEXT")) == nullptr) || r;
  r = ((glCopyConvolutionFilter1DEXT = (PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyConvolutionFilter1DEXT")) == nullptr) || r;
  r = ((glCopyConvolutionFilter2DEXT = (PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyConvolutionFilter2DEXT")) == nullptr) || r;
  r = ((glGetConvolutionFilterEXT = (PFNGLGETCONVOLUTIONFILTEREXTPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionFilterEXT")) == nullptr) || r;
  r = ((glGetConvolutionParameterfvEXT = (PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionParameterfvEXT")) == nullptr) || r;
  r = ((glGetConvolutionParameterivEXT = (PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionParameterivEXT")) == nullptr) || r;
  r = ((glGetSeparableFilterEXT = (PFNGLGETSEPARABLEFILTEREXTPROC)glewGetProcAddress((const GLubyte*)"glGetSeparableFilterEXT")) == nullptr) || r;
  r = ((glSeparableFilter2DEXT = (PFNGLSEPARABLEFILTER2DEXTPROC)glewGetProcAddress((const GLubyte*)"glSeparableFilter2DEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_convolution */

#ifdef GL_EXT_coordinate_frame

static GLboolean _glewInit_GL_EXT_coordinate_frame (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBinormalPointerEXT = (PFNGLBINORMALPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glBinormalPointerEXT")) == nullptr) || r;
  r = ((glTangentPointerEXT = (PFNGLTANGENTPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glTangentPointerEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_coordinate_frame */

#ifdef GL_EXT_copy_texture

static GLboolean _glewInit_GL_EXT_copy_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyTexImage1DEXT = (PFNGLCOPYTEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTexImage1DEXT")) == nullptr) || r;
  r = ((glCopyTexImage2DEXT = (PFNGLCOPYTEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTexImage2DEXT")) == nullptr) || r;
  r = ((glCopyTexSubImage1DEXT = (PFNGLCOPYTEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage1DEXT")) == nullptr) || r;
  r = ((glCopyTexSubImage2DEXT = (PFNGLCOPYTEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage2DEXT")) == nullptr) || r;
  r = ((glCopyTexSubImage3DEXT = (PFNGLCOPYTEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage3DEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_copy_texture */

#ifdef GL_EXT_cull_vertex

static GLboolean _glewInit_GL_EXT_cull_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCullParameterdvEXT = (PFNGLCULLPARAMETERDVEXTPROC)glewGetProcAddress((const GLubyte*)"glCullParameterdvEXT")) == nullptr) || r;
  r = ((glCullParameterfvEXT = (PFNGLCULLPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glCullParameterfvEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_cull_vertex */

#ifdef GL_EXT_depth_bounds_test

static GLboolean _glewInit_GL_EXT_depth_bounds_test (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)glewGetProcAddress((const GLubyte*)"glDepthBoundsEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_depth_bounds_test */

#ifdef GL_EXT_direct_state_access

static GLboolean _glewInit_GL_EXT_direct_state_access (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindMultiTextureEXT = (PFNGLBINDMULTITEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glBindMultiTextureEXT")) == nullptr) || r;
  r = ((glCheckNamedFramebufferStatusEXT = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC)glewGetProcAddress((const GLubyte*)"glCheckNamedFramebufferStatusEXT")) == nullptr) || r;
  r = ((glClientAttribDefaultEXT = (PFNGLCLIENTATTRIBDEFAULTEXTPROC)glewGetProcAddress((const GLubyte*)"glClientAttribDefaultEXT")) == nullptr) || r;
  r = ((glCompressedMultiTexImage1DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexImage1DEXT")) == nullptr) || r;
  r = ((glCompressedMultiTexImage2DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexImage2DEXT")) == nullptr) || r;
  r = ((glCompressedMultiTexImage3DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexImage3DEXT")) == nullptr) || r;
  r = ((glCompressedMultiTexSubImage1DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexSubImage1DEXT")) == nullptr) || r;
  r = ((glCompressedMultiTexSubImage2DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexSubImage2DEXT")) == nullptr) || r;
  r = ((glCompressedMultiTexSubImage3DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexSubImage3DEXT")) == nullptr) || r;
  r = ((glCompressedTextureImage1DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureImage1DEXT")) == nullptr) || r;
  r = ((glCompressedTextureImage2DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureImage2DEXT")) == nullptr) || r;
  r = ((glCompressedTextureImage3DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureImage3DEXT")) == nullptr) || r;
  r = ((glCompressedTextureSubImage1DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage1DEXT")) == nullptr) || r;
  r = ((glCompressedTextureSubImage2DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage2DEXT")) == nullptr) || r;
  r = ((glCompressedTextureSubImage3DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage3DEXT")) == nullptr) || r;
  r = ((glCopyMultiTexImage1DEXT = (PFNGLCOPYMULTITEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexImage1DEXT")) == nullptr) || r;
  r = ((glCopyMultiTexImage2DEXT = (PFNGLCOPYMULTITEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexImage2DEXT")) == nullptr) || r;
  r = ((glCopyMultiTexSubImage1DEXT = (PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexSubImage1DEXT")) == nullptr) || r;
  r = ((glCopyMultiTexSubImage2DEXT = (PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexSubImage2DEXT")) == nullptr) || r;
  r = ((glCopyMultiTexSubImage3DEXT = (PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexSubImage3DEXT")) == nullptr) || r;
  r = ((glCopyTextureImage1DEXT = (PFNGLCOPYTEXTUREIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureImage1DEXT")) == nullptr) || r;
  r = ((glCopyTextureImage2DEXT = (PFNGLCOPYTEXTUREIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureImage2DEXT")) == nullptr) || r;
  r = ((glCopyTextureSubImage1DEXT = (PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage1DEXT")) == nullptr) || r;
  r = ((glCopyTextureSubImage2DEXT = (PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage2DEXT")) == nullptr) || r;
  r = ((glCopyTextureSubImage3DEXT = (PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage3DEXT")) == nullptr) || r;
  r = ((glDisableClientStateIndexedEXT = (PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableClientStateIndexedEXT")) == nullptr) || r;
  r = ((glDisableClientStateiEXT = (PFNGLDISABLECLIENTSTATEIEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableClientStateiEXT")) == nullptr) || r;
  r = ((glDisableVertexArrayAttribEXT = (PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexArrayAttribEXT")) == nullptr) || r;
  r = ((glDisableVertexArrayEXT = (PFNGLDISABLEVERTEXARRAYEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexArrayEXT")) == nullptr) || r;
  r = ((glEnableClientStateIndexedEXT = (PFNGLENABLECLIENTSTATEINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableClientStateIndexedEXT")) == nullptr) || r;
  r = ((glEnableClientStateiEXT = (PFNGLENABLECLIENTSTATEIEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableClientStateiEXT")) == nullptr) || r;
  r = ((glEnableVertexArrayAttribEXT = (PFNGLENABLEVERTEXARRAYATTRIBEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexArrayAttribEXT")) == nullptr) || r;
  r = ((glEnableVertexArrayEXT = (PFNGLENABLEVERTEXARRAYEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexArrayEXT")) == nullptr) || r;
  r = ((glFlushMappedNamedBufferRangeEXT = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC)glewGetProcAddress((const GLubyte*)"glFlushMappedNamedBufferRangeEXT")) == nullptr) || r;
  r = ((glFramebufferDrawBufferEXT = (PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferDrawBufferEXT")) == nullptr) || r;
  r = ((glFramebufferDrawBuffersEXT = (PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferDrawBuffersEXT")) == nullptr) || r;
  r = ((glFramebufferReadBufferEXT = (PFNGLFRAMEBUFFERREADBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferReadBufferEXT")) == nullptr) || r;
  r = ((glGenerateMultiTexMipmapEXT = (PFNGLGENERATEMULTITEXMIPMAPEXTPROC)glewGetProcAddress((const GLubyte*)"glGenerateMultiTexMipmapEXT")) == nullptr) || r;
  r = ((glGenerateTextureMipmapEXT = (PFNGLGENERATETEXTUREMIPMAPEXTPROC)glewGetProcAddress((const GLubyte*)"glGenerateTextureMipmapEXT")) == nullptr) || r;
  r = ((glGetCompressedMultiTexImageEXT = (PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedMultiTexImageEXT")) == nullptr) || r;
  r = ((glGetCompressedTextureImageEXT = (PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedTextureImageEXT")) == nullptr) || r;
  r = ((glGetDoubleIndexedvEXT = (PFNGLGETDOUBLEINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetDoubleIndexedvEXT")) == nullptr) || r;
  r = ((glGetDoublei_vEXT = (PFNGLGETDOUBLEI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetDoublei_vEXT")) == nullptr) || r;
  r = ((glGetFloatIndexedvEXT = (PFNGLGETFLOATINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFloatIndexedvEXT")) == nullptr) || r;
  r = ((glGetFloati_vEXT = (PFNGLGETFLOATI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFloati_vEXT")) == nullptr) || r;
  r = ((glGetFramebufferParameterivEXT = (PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFramebufferParameterivEXT")) == nullptr) || r;
  r = ((glGetMultiTexEnvfvEXT = (PFNGLGETMULTITEXENVFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexEnvfvEXT")) == nullptr) || r;
  r = ((glGetMultiTexEnvivEXT = (PFNGLGETMULTITEXENVIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexEnvivEXT")) == nullptr) || r;
  r = ((glGetMultiTexGendvEXT = (PFNGLGETMULTITEXGENDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexGendvEXT")) == nullptr) || r;
  r = ((glGetMultiTexGenfvEXT = (PFNGLGETMULTITEXGENFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexGenfvEXT")) == nullptr) || r;
  r = ((glGetMultiTexGenivEXT = (PFNGLGETMULTITEXGENIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexGenivEXT")) == nullptr) || r;
  r = ((glGetMultiTexImageEXT = (PFNGLGETMULTITEXIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexImageEXT")) == nullptr) || r;
  r = ((glGetMultiTexLevelParameterfvEXT = (PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexLevelParameterfvEXT")) == nullptr) || r;
  r = ((glGetMultiTexLevelParameterivEXT = (PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexLevelParameterivEXT")) == nullptr) || r;
  r = ((glGetMultiTexParameterIivEXT = (PFNGLGETMULTITEXPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterIivEXT")) == nullptr) || r;
  r = ((glGetMultiTexParameterIuivEXT = (PFNGLGETMULTITEXPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterIuivEXT")) == nullptr) || r;
  r = ((glGetMultiTexParameterfvEXT = (PFNGLGETMULTITEXPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterfvEXT")) == nullptr) || r;
  r = ((glGetMultiTexParameterivEXT = (PFNGLGETMULTITEXPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterivEXT")) == nullptr) || r;
  r = ((glGetNamedBufferParameterivEXT = (PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferParameterivEXT")) == nullptr) || r;
  r = ((glGetNamedBufferPointervEXT = (PFNGLGETNAMEDBUFFERPOINTERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferPointervEXT")) == nullptr) || r;
  r = ((glGetNamedBufferSubDataEXT = (PFNGLGETNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferSubDataEXT")) == nullptr) || r;
  r = ((glGetNamedFramebufferAttachmentParameterivEXT = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedFramebufferAttachmentParameterivEXT")) == nullptr) || r;
  r = ((glGetNamedProgramLocalParameterIivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterIivEXT")) == nullptr) || r;
  r = ((glGetNamedProgramLocalParameterIuivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterIuivEXT")) == nullptr) || r;
  r = ((glGetNamedProgramLocalParameterdvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterdvEXT")) == nullptr) || r;
  r = ((glGetNamedProgramLocalParameterfvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterfvEXT")) == nullptr) || r;
  r = ((glGetNamedProgramStringEXT = (PFNGLGETNAMEDPROGRAMSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramStringEXT")) == nullptr) || r;
  r = ((glGetNamedProgramivEXT = (PFNGLGETNAMEDPROGRAMIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramivEXT")) == nullptr) || r;
  r = ((glGetNamedRenderbufferParameterivEXT = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedRenderbufferParameterivEXT")) == nullptr) || r;
  r = ((glGetPointerIndexedvEXT = (PFNGLGETPOINTERINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetPointerIndexedvEXT")) == nullptr) || r;
  r = ((glGetPointeri_vEXT = (PFNGLGETPOINTERI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetPointeri_vEXT")) == nullptr) || r;
  r = ((glGetTextureImageEXT = (PFNGLGETTEXTUREIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureImageEXT")) == nullptr) || r;
  r = ((glGetTextureLevelParameterfvEXT = (PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureLevelParameterfvEXT")) == nullptr) || r;
  r = ((glGetTextureLevelParameterivEXT = (PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureLevelParameterivEXT")) == nullptr) || r;
  r = ((glGetTextureParameterIivEXT = (PFNGLGETTEXTUREPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterIivEXT")) == nullptr) || r;
  r = ((glGetTextureParameterIuivEXT = (PFNGLGETTEXTUREPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterIuivEXT")) == nullptr) || r;
  r = ((glGetTextureParameterfvEXT = (PFNGLGETTEXTUREPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterfvEXT")) == nullptr) || r;
  r = ((glGetTextureParameterivEXT = (PFNGLGETTEXTUREPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterivEXT")) == nullptr) || r;
  r = ((glGetVertexArrayIntegeri_vEXT = (PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayIntegeri_vEXT")) == nullptr) || r;
  r = ((glGetVertexArrayIntegervEXT = (PFNGLGETVERTEXARRAYINTEGERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayIntegervEXT")) == nullptr) || r;
  r = ((glGetVertexArrayPointeri_vEXT = (PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayPointeri_vEXT")) == nullptr) || r;
  r = ((glGetVertexArrayPointervEXT = (PFNGLGETVERTEXARRAYPOINTERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayPointervEXT")) == nullptr) || r;
  r = ((glMapNamedBufferEXT = (PFNGLMAPNAMEDBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glMapNamedBufferEXT")) == nullptr) || r;
  r = ((glMapNamedBufferRangeEXT = (PFNGLMAPNAMEDBUFFERRANGEEXTPROC)glewGetProcAddress((const GLubyte*)"glMapNamedBufferRangeEXT")) == nullptr) || r;
  r = ((glMatrixFrustumEXT = (PFNGLMATRIXFRUSTUMEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixFrustumEXT")) == nullptr) || r;
  r = ((glMatrixLoadIdentityEXT = (PFNGLMATRIXLOADIDENTITYEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadIdentityEXT")) == nullptr) || r;
  r = ((glMatrixLoadTransposedEXT = (PFNGLMATRIXLOADTRANSPOSEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadTransposedEXT")) == nullptr) || r;
  r = ((glMatrixLoadTransposefEXT = (PFNGLMATRIXLOADTRANSPOSEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadTransposefEXT")) == nullptr) || r;
  r = ((glMatrixLoaddEXT = (PFNGLMATRIXLOADDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoaddEXT")) == nullptr) || r;
  r = ((glMatrixLoadfEXT = (PFNGLMATRIXLOADFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadfEXT")) == nullptr) || r;
  r = ((glMatrixMultTransposedEXT = (PFNGLMATRIXMULTTRANSPOSEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultTransposedEXT")) == nullptr) || r;
  r = ((glMatrixMultTransposefEXT = (PFNGLMATRIXMULTTRANSPOSEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultTransposefEXT")) == nullptr) || r;
  r = ((glMatrixMultdEXT = (PFNGLMATRIXMULTDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultdEXT")) == nullptr) || r;
  r = ((glMatrixMultfEXT = (PFNGLMATRIXMULTFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultfEXT")) == nullptr) || r;
  r = ((glMatrixOrthoEXT = (PFNGLMATRIXORTHOEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixOrthoEXT")) == nullptr) || r;
  r = ((glMatrixPopEXT = (PFNGLMATRIXPOPEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixPopEXT")) == nullptr) || r;
  r = ((glMatrixPushEXT = (PFNGLMATRIXPUSHEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixPushEXT")) == nullptr) || r;
  r = ((glMatrixRotatedEXT = (PFNGLMATRIXROTATEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixRotatedEXT")) == nullptr) || r;
  r = ((glMatrixRotatefEXT = (PFNGLMATRIXROTATEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixRotatefEXT")) == nullptr) || r;
  r = ((glMatrixScaledEXT = (PFNGLMATRIXSCALEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixScaledEXT")) == nullptr) || r;
  r = ((glMatrixScalefEXT = (PFNGLMATRIXSCALEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixScalefEXT")) == nullptr) || r;
  r = ((glMatrixTranslatedEXT = (PFNGLMATRIXTRANSLATEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixTranslatedEXT")) == nullptr) || r;
  r = ((glMatrixTranslatefEXT = (PFNGLMATRIXTRANSLATEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixTranslatefEXT")) == nullptr) || r;
  r = ((glMultiTexBufferEXT = (PFNGLMULTITEXBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexBufferEXT")) == nullptr) || r;
  r = ((glMultiTexCoordPointerEXT = (PFNGLMULTITEXCOORDPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordPointerEXT")) == nullptr) || r;
  r = ((glMultiTexEnvfEXT = (PFNGLMULTITEXENVFEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnvfEXT")) == nullptr) || r;
  r = ((glMultiTexEnvfvEXT = (PFNGLMULTITEXENVFVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnvfvEXT")) == nullptr) || r;
  r = ((glMultiTexEnviEXT = (PFNGLMULTITEXENVIEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnviEXT")) == nullptr) || r;
  r = ((glMultiTexEnvivEXT = (PFNGLMULTITEXENVIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnvivEXT")) == nullptr) || r;
  r = ((glMultiTexGendEXT = (PFNGLMULTITEXGENDEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGendEXT")) == nullptr) || r;
  r = ((glMultiTexGendvEXT = (PFNGLMULTITEXGENDVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGendvEXT")) == nullptr) || r;
  r = ((glMultiTexGenfEXT = (PFNGLMULTITEXGENFEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGenfEXT")) == nullptr) || r;
  r = ((glMultiTexGenfvEXT = (PFNGLMULTITEXGENFVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGenfvEXT")) == nullptr) || r;
  r = ((glMultiTexGeniEXT = (PFNGLMULTITEXGENIEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGeniEXT")) == nullptr) || r;
  r = ((glMultiTexGenivEXT = (PFNGLMULTITEXGENIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGenivEXT")) == nullptr) || r;
  r = ((glMultiTexImage1DEXT = (PFNGLMULTITEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexImage1DEXT")) == nullptr) || r;
  r = ((glMultiTexImage2DEXT = (PFNGLMULTITEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexImage2DEXT")) == nullptr) || r;
  r = ((glMultiTexImage3DEXT = (PFNGLMULTITEXIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexImage3DEXT")) == nullptr) || r;
  r = ((glMultiTexParameterIivEXT = (PFNGLMULTITEXPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterIivEXT")) == nullptr) || r;
  r = ((glMultiTexParameterIuivEXT = (PFNGLMULTITEXPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterIuivEXT")) == nullptr) || r;
  r = ((glMultiTexParameterfEXT = (PFNGLMULTITEXPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterfEXT")) == nullptr) || r;
  r = ((glMultiTexParameterfvEXT = (PFNGLMULTITEXPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterfvEXT")) == nullptr) || r;
  r = ((glMultiTexParameteriEXT = (PFNGLMULTITEXPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameteriEXT")) == nullptr) || r;
  r = ((glMultiTexParameterivEXT = (PFNGLMULTITEXPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterivEXT")) == nullptr) || r;
  r = ((glMultiTexRenderbufferEXT = (PFNGLMULTITEXRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexRenderbufferEXT")) == nullptr) || r;
  r = ((glMultiTexSubImage1DEXT = (PFNGLMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexSubImage1DEXT")) == nullptr) || r;
  r = ((glMultiTexSubImage2DEXT = (PFNGLMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexSubImage2DEXT")) == nullptr) || r;
  r = ((glMultiTexSubImage3DEXT = (PFNGLMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexSubImage3DEXT")) == nullptr) || r;
  r = ((glNamedBufferDataEXT = (PFNGLNAMEDBUFFERDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferDataEXT")) == nullptr) || r;
  r = ((glNamedBufferSubDataEXT = (PFNGLNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferSubDataEXT")) == nullptr) || r;
  r = ((glNamedCopyBufferSubDataEXT = (PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedCopyBufferSubDataEXT")) == nullptr) || r;
  r = ((glNamedFramebufferRenderbufferEXT = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferRenderbufferEXT")) == nullptr) || r;
  r = ((glNamedFramebufferTexture1DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture1DEXT")) == nullptr) || r;
  r = ((glNamedFramebufferTexture2DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture2DEXT")) == nullptr) || r;
  r = ((glNamedFramebufferTexture3DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture3DEXT")) == nullptr) || r;
  r = ((glNamedFramebufferTextureEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureEXT")) == nullptr) || r;
  r = ((glNamedFramebufferTextureFaceEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureFaceEXT")) == nullptr) || r;
  r = ((glNamedFramebufferTextureLayerEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureLayerEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameter4dEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4dEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameter4dvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4dvEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameter4fEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4fEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameter4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4fvEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameterI4iEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4iEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameterI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4ivEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameterI4uiEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4uiEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameterI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4uivEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParameters4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameters4fvEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParametersI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParametersI4ivEXT")) == nullptr) || r;
  r = ((glNamedProgramLocalParametersI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParametersI4uivEXT")) == nullptr) || r;
  r = ((glNamedProgramStringEXT = (PFNGLNAMEDPROGRAMSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramStringEXT")) == nullptr) || r;
  r = ((glNamedRenderbufferStorageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageEXT")) == nullptr) || r;
  r = ((glNamedRenderbufferStorageMultisampleCoverageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageMultisampleCoverageEXT")) == nullptr) || r;
  r = ((glNamedRenderbufferStorageMultisampleEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageMultisampleEXT")) == nullptr) || r;
  r = ((glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1fEXT")) == nullptr) || r;
  r = ((glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1fvEXT")) == nullptr) || r;
  r = ((glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1iEXT")) == nullptr) || r;
  r = ((glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ivEXT")) == nullptr) || r;
  r = ((glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1uiEXT")) == nullptr) || r;
  r = ((glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1uivEXT")) == nullptr) || r;
  r = ((glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2fEXT")) == nullptr) || r;
  r = ((glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2fvEXT")) == nullptr) || r;
  r = ((glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2iEXT")) == nullptr) || r;
  r = ((glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ivEXT")) == nullptr) || r;
  r = ((glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2uiEXT")) == nullptr) || r;
  r = ((glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2uivEXT")) == nullptr) || r;
  r = ((glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3fEXT")) == nullptr) || r;
  r = ((glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3fvEXT")) == nullptr) || r;
  r = ((glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3iEXT")) == nullptr) || r;
  r = ((glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ivEXT")) == nullptr) || r;
  r = ((glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3uiEXT")) == nullptr) || r;
  r = ((glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3uivEXT")) == nullptr) || r;
  r = ((glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4fEXT")) == nullptr) || r;
  r = ((glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4fvEXT")) == nullptr) || r;
  r = ((glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4iEXT")) == nullptr) || r;
  r = ((glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ivEXT")) == nullptr) || r;
  r = ((glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4uiEXT")) == nullptr) || r;
  r = ((glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4uivEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x3fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x4fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x2fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x4fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x2fvEXT")) == nullptr) || r;
  r = ((glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x3fvEXT")) == nullptr) || r;
  r = ((glPushClientAttribDefaultEXT = (PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC)glewGetProcAddress((const GLubyte*)"glPushClientAttribDefaultEXT")) == nullptr) || r;
  r = ((glTextureBufferEXT = (PFNGLTEXTUREBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glTextureBufferEXT")) == nullptr) || r;
  r = ((glTextureImage1DEXT = (PFNGLTEXTUREIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureImage1DEXT")) == nullptr) || r;
  r = ((glTextureImage2DEXT = (PFNGLTEXTUREIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureImage2DEXT")) == nullptr) || r;
  r = ((glTextureImage3DEXT = (PFNGLTEXTUREIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureImage3DEXT")) == nullptr) || r;
  r = ((glTextureParameterIivEXT = (PFNGLTEXTUREPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterIivEXT")) == nullptr) || r;
  r = ((glTextureParameterIuivEXT = (PFNGLTEXTUREPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterIuivEXT")) == nullptr) || r;
  r = ((glTextureParameterfEXT = (PFNGLTEXTUREPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterfEXT")) == nullptr) || r;
  r = ((glTextureParameterfvEXT = (PFNGLTEXTUREPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterfvEXT")) == nullptr) || r;
  r = ((glTextureParameteriEXT = (PFNGLTEXTUREPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameteriEXT")) == nullptr) || r;
  r = ((glTextureParameterivEXT = (PFNGLTEXTUREPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterivEXT")) == nullptr) || r;
  r = ((glTextureRenderbufferEXT = (PFNGLTEXTURERENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glTextureRenderbufferEXT")) == nullptr) || r;
  r = ((glTextureSubImage1DEXT = (PFNGLTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage1DEXT")) == nullptr) || r;
  r = ((glTextureSubImage2DEXT = (PFNGLTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage2DEXT")) == nullptr) || r;
  r = ((glTextureSubImage3DEXT = (PFNGLTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage3DEXT")) == nullptr) || r;
  r = ((glUnmapNamedBufferEXT = (PFNGLUNMAPNAMEDBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glUnmapNamedBufferEXT")) == nullptr) || r;
  r = ((glVertexArrayColorOffsetEXT = (PFNGLVERTEXARRAYCOLOROFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayColorOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayEdgeFlagOffsetEXT = (PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayEdgeFlagOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayFogCoordOffsetEXT = (PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayFogCoordOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayIndexOffsetEXT = (PFNGLVERTEXARRAYINDEXOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayIndexOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayMultiTexCoordOffsetEXT = (PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayMultiTexCoordOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayNormalOffsetEXT = (PFNGLVERTEXARRAYNORMALOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayNormalOffsetEXT")) == nullptr) || r;
  r = ((glVertexArraySecondaryColorOffsetEXT = (PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArraySecondaryColorOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayTexCoordOffsetEXT = (PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayTexCoordOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayVertexAttribIOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribIOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayVertexAttribOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribOffsetEXT")) == nullptr) || r;
  r = ((glVertexArrayVertexOffsetEXT = (PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexOffsetEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_direct_state_access */

#ifdef GL_EXT_draw_buffers2

static GLboolean _glewInit_GL_EXT_draw_buffers2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorMaskIndexedEXT = (PFNGLCOLORMASKINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glColorMaskIndexedEXT")) == nullptr) || r;
  r = ((glDisableIndexedEXT = (PFNGLDISABLEINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableIndexedEXT")) == nullptr) || r;
  r = ((glEnableIndexedEXT = (PFNGLENABLEINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableIndexedEXT")) == nullptr) || r;
  r = ((glGetBooleanIndexedvEXT = (PFNGLGETBOOLEANINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetBooleanIndexedvEXT")) == nullptr) || r;
  r = ((glGetIntegerIndexedvEXT = (PFNGLGETINTEGERINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetIntegerIndexedvEXT")) == nullptr) || r;
  r = ((glIsEnabledIndexedEXT = (PFNGLISENABLEDINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glIsEnabledIndexedEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_draw_buffers2 */

#ifdef GL_EXT_draw_instanced

static GLboolean _glewInit_GL_EXT_draw_instanced (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysInstancedEXT")) == nullptr) || r;
  r = ((glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_draw_instanced */

#ifdef GL_EXT_draw_range_elements

static GLboolean _glewInit_GL_EXT_draw_range_elements (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElementsEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_draw_range_elements */

#ifdef GL_EXT_fog_coord

static GLboolean _glewInit_GL_EXT_fog_coord (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glFogCoordPointerEXT")) == nullptr) || r;
  r = ((glFogCoorddEXT = (PFNGLFOGCOORDDEXTPROC)glewGetProcAddress((const GLubyte*)"glFogCoorddEXT")) == nullptr) || r;
  r = ((glFogCoorddvEXT = (PFNGLFOGCOORDDVEXTPROC)glewGetProcAddress((const GLubyte*)"glFogCoorddvEXT")) == nullptr) || r;
  r = ((glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)glewGetProcAddress((const GLubyte*)"glFogCoordfEXT")) == nullptr) || r;
  r = ((glFogCoordfvEXT = (PFNGLFOGCOORDFVEXTPROC)glewGetProcAddress((const GLubyte*)"glFogCoordfvEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_fog_coord */

#ifdef GL_EXT_fragment_lighting

static GLboolean _glewInit_GL_EXT_fragment_lighting (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFragmentColorMaterialEXT = (PFNGLFRAGMENTCOLORMATERIALEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentColorMaterialEXT")) == nullptr) || r;
  r = ((glFragmentLightModelfEXT = (PFNGLFRAGMENTLIGHTMODELFEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModelfEXT")) == nullptr) || r;
  r = ((glFragmentLightModelfvEXT = (PFNGLFRAGMENTLIGHTMODELFVEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModelfvEXT")) == nullptr) || r;
  r = ((glFragmentLightModeliEXT = (PFNGLFRAGMENTLIGHTMODELIEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModeliEXT")) == nullptr) || r;
  r = ((glFragmentLightModelivEXT = (PFNGLFRAGMENTLIGHTMODELIVEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModelivEXT")) == nullptr) || r;
  r = ((glFragmentLightfEXT = (PFNGLFRAGMENTLIGHTFEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightfEXT")) == nullptr) || r;
  r = ((glFragmentLightfvEXT = (PFNGLFRAGMENTLIGHTFVEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightfvEXT")) == nullptr) || r;
  r = ((glFragmentLightiEXT = (PFNGLFRAGMENTLIGHTIEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightiEXT")) == nullptr) || r;
  r = ((glFragmentLightivEXT = (PFNGLFRAGMENTLIGHTIVEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightivEXT")) == nullptr) || r;
  r = ((glFragmentMaterialfEXT = (PFNGLFRAGMENTMATERIALFEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialfEXT")) == nullptr) || r;
  r = ((glFragmentMaterialfvEXT = (PFNGLFRAGMENTMATERIALFVEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialfvEXT")) == nullptr) || r;
  r = ((glFragmentMaterialiEXT = (PFNGLFRAGMENTMATERIALIEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialiEXT")) == nullptr) || r;
  r = ((glFragmentMaterialivEXT = (PFNGLFRAGMENTMATERIALIVEXTPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialivEXT")) == nullptr) || r;
  r = ((glGetFragmentLightfvEXT = (PFNGLGETFRAGMENTLIGHTFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentLightfvEXT")) == nullptr) || r;
  r = ((glGetFragmentLightivEXT = (PFNGLGETFRAGMENTLIGHTIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentLightivEXT")) == nullptr) || r;
  r = ((glGetFragmentMaterialfvEXT = (PFNGLGETFRAGMENTMATERIALFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentMaterialfvEXT")) == nullptr) || r;
  r = ((glGetFragmentMaterialivEXT = (PFNGLGETFRAGMENTMATERIALIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentMaterialivEXT")) == nullptr) || r;
  r = ((glLightEnviEXT = (PFNGLLIGHTENVIEXTPROC)glewGetProcAddress((const GLubyte*)"glLightEnviEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_fragment_lighting */

#ifdef GL_EXT_framebuffer_blit

static GLboolean _glewInit_GL_EXT_framebuffer_blit (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glBlitFramebufferEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_blit */

#ifdef GL_EXT_framebuffer_multisample

static GLboolean _glewInit_GL_EXT_framebuffer_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisampleEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_multisample */

#ifdef GL_EXT_framebuffer_multisample_blit_scaled

#endif /* GL_EXT_framebuffer_multisample_blit_scaled */

#ifdef GL_EXT_framebuffer_object

static GLboolean _glewInit_GL_EXT_framebuffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindFramebufferEXT")) == nullptr) || r;
  r = ((glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindRenderbufferEXT")) == nullptr) || r;
  r = ((glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glewGetProcAddress((const GLubyte*)"glCheckFramebufferStatusEXT")) == nullptr) || r;
  r = ((glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glDeleteFramebuffersEXT")) == nullptr) || r;
  r = ((glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glDeleteRenderbuffersEXT")) == nullptr) || r;
  r = ((glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferRenderbufferEXT")) == nullptr) || r;
  r = ((glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture1DEXT")) == nullptr) || r;
  r = ((glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture2DEXT")) == nullptr) || r;
  r = ((glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture3DEXT")) == nullptr) || r;
  r = ((glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glGenFramebuffersEXT")) == nullptr) || r;
  r = ((glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glGenRenderbuffersEXT")) == nullptr) || r;
  r = ((glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)glewGetProcAddress((const GLubyte*)"glGenerateMipmapEXT")) == nullptr) || r;
  r = ((glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFramebufferAttachmentParameterivEXT")) == nullptr) || r;
  r = ((glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetRenderbufferParameterivEXT")) == nullptr) || r;
  r = ((glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glIsFramebufferEXT")) == nullptr) || r;
  r = ((glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glIsRenderbufferEXT")) == nullptr) || r;
  r = ((glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_object */

#ifdef GL_EXT_framebuffer_sRGB

#endif /* GL_EXT_framebuffer_sRGB */

#ifdef GL_EXT_geometry_shader4

static GLboolean _glewInit_GL_EXT_geometry_shader4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureEXT = (PFNGLFRAMEBUFFERTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureEXT")) == nullptr) || r;
  r = ((glFramebufferTextureFaceEXT = (PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureFaceEXT")) == nullptr) || r;
  r = ((glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramParameteriEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_geometry_shader4 */

#ifdef GL_EXT_gpu_program_parameters

static GLboolean _glewInit_GL_EXT_gpu_program_parameters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramEnvParameters4fvEXT = (PFNGLPROGRAMENVPARAMETERS4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameters4fvEXT")) == nullptr) || r;
  r = ((glProgramLocalParameters4fvEXT = (PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameters4fvEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_gpu_program_parameters */

#ifdef GL_EXT_gpu_shader4

static GLboolean _glewInit_GL_EXT_gpu_shader4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFragDataLocationEXT = (PFNGLBINDFRAGDATALOCATIONEXTPROC)glewGetProcAddress((const GLubyte*)"glBindFragDataLocationEXT")) == nullptr) || r;
  r = ((glGetFragDataLocationEXT = (PFNGLGETFRAGDATALOCATIONEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFragDataLocationEXT")) == nullptr) || r;
  r = ((glGetUniformuivEXT = (PFNGLGETUNIFORMUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetUniformuivEXT")) == nullptr) || r;
  r = ((glGetVertexAttribIivEXT = (PFNGLGETVERTEXATTRIBIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIivEXT")) == nullptr) || r;
  r = ((glGetVertexAttribIuivEXT = (PFNGLGETVERTEXATTRIBIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIuivEXT")) == nullptr) || r;
  r = ((glUniform1uiEXT = (PFNGLUNIFORM1UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform1uiEXT")) == nullptr) || r;
  r = ((glUniform1uivEXT = (PFNGLUNIFORM1UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform1uivEXT")) == nullptr) || r;
  r = ((glUniform2uiEXT = (PFNGLUNIFORM2UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform2uiEXT")) == nullptr) || r;
  r = ((glUniform2uivEXT = (PFNGLUNIFORM2UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform2uivEXT")) == nullptr) || r;
  r = ((glUniform3uiEXT = (PFNGLUNIFORM3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform3uiEXT")) == nullptr) || r;
  r = ((glUniform3uivEXT = (PFNGLUNIFORM3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform3uivEXT")) == nullptr) || r;
  r = ((glUniform4uiEXT = (PFNGLUNIFORM4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform4uiEXT")) == nullptr) || r;
  r = ((glUniform4uivEXT = (PFNGLUNIFORM4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform4uivEXT")) == nullptr) || r;
  r = ((glVertexAttribI1iEXT = (PFNGLVERTEXATTRIBI1IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1iEXT")) == nullptr) || r;
  r = ((glVertexAttribI1ivEXT = (PFNGLVERTEXATTRIBI1IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1ivEXT")) == nullptr) || r;
  r = ((glVertexAttribI1uiEXT = (PFNGLVERTEXATTRIBI1UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1uiEXT")) == nullptr) || r;
  r = ((glVertexAttribI1uivEXT = (PFNGLVERTEXATTRIBI1UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1uivEXT")) == nullptr) || r;
  r = ((glVertexAttribI2iEXT = (PFNGLVERTEXATTRIBI2IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2iEXT")) == nullptr) || r;
  r = ((glVertexAttribI2ivEXT = (PFNGLVERTEXATTRIBI2IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2ivEXT")) == nullptr) || r;
  r = ((glVertexAttribI2uiEXT = (PFNGLVERTEXATTRIBI2UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2uiEXT")) == nullptr) || r;
  r = ((glVertexAttribI2uivEXT = (PFNGLVERTEXATTRIBI2UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2uivEXT")) == nullptr) || r;
  r = ((glVertexAttribI3iEXT = (PFNGLVERTEXATTRIBI3IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3iEXT")) == nullptr) || r;
  r = ((glVertexAttribI3ivEXT = (PFNGLVERTEXATTRIBI3IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3ivEXT")) == nullptr) || r;
  r = ((glVertexAttribI3uiEXT = (PFNGLVERTEXATTRIBI3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3uiEXT")) == nullptr) || r;
  r = ((glVertexAttribI3uivEXT = (PFNGLVERTEXATTRIBI3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3uivEXT")) == nullptr) || r;
  r = ((glVertexAttribI4bvEXT = (PFNGLVERTEXATTRIBI4BVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4bvEXT")) == nullptr) || r;
  r = ((glVertexAttribI4iEXT = (PFNGLVERTEXATTRIBI4IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4iEXT")) == nullptr) || r;
  r = ((glVertexAttribI4ivEXT = (PFNGLVERTEXATTRIBI4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ivEXT")) == nullptr) || r;
  r = ((glVertexAttribI4svEXT = (PFNGLVERTEXATTRIBI4SVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4svEXT")) == nullptr) || r;
  r = ((glVertexAttribI4ubvEXT = (PFNGLVERTEXATTRIBI4UBVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ubvEXT")) == nullptr) || r;
  r = ((glVertexAttribI4uiEXT = (PFNGLVERTEXATTRIBI4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4uiEXT")) == nullptr) || r;
  r = ((glVertexAttribI4uivEXT = (PFNGLVERTEXATTRIBI4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4uivEXT")) == nullptr) || r;
  r = ((glVertexAttribI4usvEXT = (PFNGLVERTEXATTRIBI4USVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4usvEXT")) == nullptr) || r;
  r = ((glVertexAttribIPointerEXT = (PFNGLVERTEXATTRIBIPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribIPointerEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_gpu_shader4 */

#ifdef GL_EXT_histogram

static GLboolean _glewInit_GL_EXT_histogram (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetHistogramEXT = (PFNGLGETHISTOGRAMEXTPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramEXT")) == nullptr) || r;
  r = ((glGetHistogramParameterfvEXT = (PFNGLGETHISTOGRAMPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramParameterfvEXT")) == nullptr) || r;
  r = ((glGetHistogramParameterivEXT = (PFNGLGETHISTOGRAMPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramParameterivEXT")) == nullptr) || r;
  r = ((glGetMinmaxEXT = (PFNGLGETMINMAXEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxEXT")) == nullptr) || r;
  r = ((glGetMinmaxParameterfvEXT = (PFNGLGETMINMAXPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxParameterfvEXT")) == nullptr) || r;
  r = ((glGetMinmaxParameterivEXT = (PFNGLGETMINMAXPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxParameterivEXT")) == nullptr) || r;
  r = ((glHistogramEXT = (PFNGLHISTOGRAMEXTPROC)glewGetProcAddress((const GLubyte*)"glHistogramEXT")) == nullptr) || r;
  r = ((glMinmaxEXT = (PFNGLMINMAXEXTPROC)glewGetProcAddress((const GLubyte*)"glMinmaxEXT")) == nullptr) || r;
  r = ((glResetHistogramEXT = (PFNGLRESETHISTOGRAMEXTPROC)glewGetProcAddress((const GLubyte*)"glResetHistogramEXT")) == nullptr) || r;
  r = ((glResetMinmaxEXT = (PFNGLRESETMINMAXEXTPROC)glewGetProcAddress((const GLubyte*)"glResetMinmaxEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_histogram */

#ifdef GL_EXT_index_array_formats

#endif /* GL_EXT_index_array_formats */

#ifdef GL_EXT_index_func

static GLboolean _glewInit_GL_EXT_index_func (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glIndexFuncEXT = (PFNGLINDEXFUNCEXTPROC)glewGetProcAddress((const GLubyte*)"glIndexFuncEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_index_func */

#ifdef GL_EXT_index_material

static GLboolean _glewInit_GL_EXT_index_material (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glIndexMaterialEXT = (PFNGLINDEXMATERIALEXTPROC)glewGetProcAddress((const GLubyte*)"glIndexMaterialEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_index_material */

#ifdef GL_EXT_index_texture

#endif /* GL_EXT_index_texture */

#ifdef GL_EXT_light_texture

static GLboolean _glewInit_GL_EXT_light_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glApplyTextureEXT = (PFNGLAPPLYTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glApplyTextureEXT")) == nullptr) || r;
  r = ((glTextureLightEXT = (PFNGLTEXTURELIGHTEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureLightEXT")) == nullptr) || r;
  r = ((glTextureMaterialEXT = (PFNGLTEXTUREMATERIALEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureMaterialEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_light_texture */

#ifdef GL_EXT_misc_attribute

#endif /* GL_EXT_misc_attribute */

#ifdef GL_EXT_multi_draw_arrays

static GLboolean _glewInit_GL_EXT_multi_draw_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawArraysEXT")) == nullptr) || r;
  r = ((glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElementsEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_multi_draw_arrays */

#ifdef GL_EXT_multisample

static GLboolean _glewInit_GL_EXT_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSampleMaskEXT = (PFNGLSAMPLEMASKEXTPROC)glewGetProcAddress((const GLubyte*)"glSampleMaskEXT")) == nullptr) || r;
  r = ((glSamplePatternEXT = (PFNGLSAMPLEPATTERNEXTPROC)glewGetProcAddress((const GLubyte*)"glSamplePatternEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_multisample */

#ifdef GL_EXT_packed_depth_stencil

#endif /* GL_EXT_packed_depth_stencil */

#ifdef GL_EXT_packed_float

#endif /* GL_EXT_packed_float */

#ifdef GL_EXT_packed_pixels

#endif /* GL_EXT_packed_pixels */

#ifdef GL_EXT_paletted_texture

static GLboolean _glewInit_GL_EXT_paletted_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorTableEXT = (PFNGLCOLORTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"glColorTableEXT")) == nullptr) || r;
  r = ((glGetColorTableEXT = (PFNGLGETCOLORTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableEXT")) == nullptr) || r;
  r = ((glGetColorTableParameterfvEXT = (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameterfvEXT")) == nullptr) || r;
  r = ((glGetColorTableParameterivEXT = (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameterivEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_paletted_texture */

#ifdef GL_EXT_pixel_buffer_object

#endif /* GL_EXT_pixel_buffer_object */

#ifdef GL_EXT_pixel_transform

static GLboolean _glewInit_GL_EXT_pixel_transform (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetPixelTransformParameterfvEXT = (PFNGLGETPIXELTRANSFORMPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetPixelTransformParameterfvEXT")) == nullptr) || r;
  r = ((glGetPixelTransformParameterivEXT = (PFNGLGETPIXELTRANSFORMPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetPixelTransformParameterivEXT")) == nullptr) || r;
  r = ((glPixelTransformParameterfEXT = (PFNGLPIXELTRANSFORMPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glPixelTransformParameterfEXT")) == nullptr) || r;
  r = ((glPixelTransformParameterfvEXT = (PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glPixelTransformParameterfvEXT")) == nullptr) || r;
  r = ((glPixelTransformParameteriEXT = (PFNGLPIXELTRANSFORMPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glPixelTransformParameteriEXT")) == nullptr) || r;
  r = ((glPixelTransformParameterivEXT = (PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glPixelTransformParameterivEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_pixel_transform */

#ifdef GL_EXT_pixel_transform_color_table

#endif /* GL_EXT_pixel_transform_color_table */

#ifdef GL_EXT_point_parameters

static GLboolean _glewInit_GL_EXT_point_parameters (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glPointParameterfEXT")) == nullptr) || r;
  r = ((glPointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glPointParameterfvEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_point_parameters */

#ifdef GL_EXT_polygon_offset

static GLboolean _glewInit_GL_EXT_polygon_offset (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPolygonOffsetEXT = (PFNGLPOLYGONOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glPolygonOffsetEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_polygon_offset */

#ifdef GL_EXT_provoking_vertex

static GLboolean _glewInit_GL_EXT_provoking_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProvokingVertexEXT = (PFNGLPROVOKINGVERTEXEXTPROC)glewGetProcAddress((const GLubyte*)"glProvokingVertexEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_provoking_vertex */

#ifdef GL_EXT_rescale_normal

#endif /* GL_EXT_rescale_normal */

#ifdef GL_EXT_scene_marker

static GLboolean _glewInit_GL_EXT_scene_marker (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginSceneEXT = (PFNGLBEGINSCENEEXTPROC)glewGetProcAddress((const GLubyte*)"glBeginSceneEXT")) == nullptr) || r;
  r = ((glEndSceneEXT = (PFNGLENDSCENEEXTPROC)glewGetProcAddress((const GLubyte*)"glEndSceneEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_scene_marker */

#ifdef GL_EXT_secondary_color

static GLboolean _glewInit_GL_EXT_secondary_color (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSecondaryColor3bEXT = (PFNGLSECONDARYCOLOR3BEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3bEXT")) == nullptr) || r;
  r = ((glSecondaryColor3bvEXT = (PFNGLSECONDARYCOLOR3BVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3bvEXT")) == nullptr) || r;
  r = ((glSecondaryColor3dEXT = (PFNGLSECONDARYCOLOR3DEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3dEXT")) == nullptr) || r;
  r = ((glSecondaryColor3dvEXT = (PFNGLSECONDARYCOLOR3DVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3dvEXT")) == nullptr) || r;
  r = ((glSecondaryColor3fEXT = (PFNGLSECONDARYCOLOR3FEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3fEXT")) == nullptr) || r;
  r = ((glSecondaryColor3fvEXT = (PFNGLSECONDARYCOLOR3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3fvEXT")) == nullptr) || r;
  r = ((glSecondaryColor3iEXT = (PFNGLSECONDARYCOLOR3IEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3iEXT")) == nullptr) || r;
  r = ((glSecondaryColor3ivEXT = (PFNGLSECONDARYCOLOR3IVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ivEXT")) == nullptr) || r;
  r = ((glSecondaryColor3sEXT = (PFNGLSECONDARYCOLOR3SEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3sEXT")) == nullptr) || r;
  r = ((glSecondaryColor3svEXT = (PFNGLSECONDARYCOLOR3SVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3svEXT")) == nullptr) || r;
  r = ((glSecondaryColor3ubEXT = (PFNGLSECONDARYCOLOR3UBEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ubEXT")) == nullptr) || r;
  r = ((glSecondaryColor3ubvEXT = (PFNGLSECONDARYCOLOR3UBVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ubvEXT")) == nullptr) || r;
  r = ((glSecondaryColor3uiEXT = (PFNGLSECONDARYCOLOR3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3uiEXT")) == nullptr) || r;
  r = ((glSecondaryColor3uivEXT = (PFNGLSECONDARYCOLOR3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3uivEXT")) == nullptr) || r;
  r = ((glSecondaryColor3usEXT = (PFNGLSECONDARYCOLOR3USEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3usEXT")) == nullptr) || r;
  r = ((glSecondaryColor3usvEXT = (PFNGLSECONDARYCOLOR3USVEXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3usvEXT")) == nullptr) || r;
  r = ((glSecondaryColorPointerEXT = (PFNGLSECONDARYCOLORPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorPointerEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_secondary_color */

#ifdef GL_EXT_separate_shader_objects

static GLboolean _glewInit_GL_EXT_separate_shader_objects (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveProgramEXT = (PFNGLACTIVEPROGRAMEXTPROC)glewGetProcAddress((const GLubyte*)"glActiveProgramEXT")) == nullptr) || r;
  r = ((glCreateShaderProgramEXT = (PFNGLCREATESHADERPROGRAMEXTPROC)glewGetProcAddress((const GLubyte*)"glCreateShaderProgramEXT")) == nullptr) || r;
  r = ((glUseShaderProgramEXT = (PFNGLUSESHADERPROGRAMEXTPROC)glewGetProcAddress((const GLubyte*)"glUseShaderProgramEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_separate_shader_objects */

#ifdef GL_EXT_separate_specular_color

#endif /* GL_EXT_separate_specular_color */

#ifdef GL_EXT_shader_image_load_store

static GLboolean _glewInit_GL_EXT_shader_image_load_store (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindImageTextureEXT = (PFNGLBINDIMAGETEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glBindImageTextureEXT")) == nullptr) || r;
  r = ((glMemoryBarrierEXT = (PFNGLMEMORYBARRIEREXTPROC)glewGetProcAddress((const GLubyte*)"glMemoryBarrierEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_shader_image_load_store */

#ifdef GL_EXT_shadow_funcs

#endif /* GL_EXT_shadow_funcs */

#ifdef GL_EXT_shared_texture_palette

#endif /* GL_EXT_shared_texture_palette */

#ifdef GL_EXT_stencil_clear_tag

#endif /* GL_EXT_stencil_clear_tag */

#ifdef GL_EXT_stencil_two_side

static GLboolean _glewInit_GL_EXT_stencil_two_side (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC)glewGetProcAddress((const GLubyte*)"glActiveStencilFaceEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_stencil_two_side */

#ifdef GL_EXT_stencil_wrap

#endif /* GL_EXT_stencil_wrap */

#ifdef GL_EXT_subtexture

static GLboolean _glewInit_GL_EXT_subtexture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexSubImage1DEXT = (PFNGLTEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage1DEXT")) == nullptr) || r;
  r = ((glTexSubImage2DEXT = (PFNGLTEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage2DEXT")) == nullptr) || r;
  r = ((glTexSubImage3DEXT = (PFNGLTEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage3DEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_subtexture */

#ifdef GL_EXT_texture

#endif /* GL_EXT_texture */

#ifdef GL_EXT_texture3D

static GLboolean _glewInit_GL_EXT_texture3D (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexImage3DEXT = (PFNGLTEXIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexImage3DEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_texture3D */

#ifdef GL_EXT_texture_array

static GLboolean _glewInit_GL_EXT_texture_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureLayerEXT = (PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureLayerEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_texture_array */

#ifdef GL_EXT_texture_buffer_object

static GLboolean _glewInit_GL_EXT_texture_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexBufferEXT = (PFNGLTEXBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glTexBufferEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_texture_buffer_object */

#ifdef GL_EXT_texture_compression_dxt1

#endif /* GL_EXT_texture_compression_dxt1 */

#ifdef GL_EXT_texture_compression_latc

#endif /* GL_EXT_texture_compression_latc */

#ifdef GL_EXT_texture_compression_rgtc

#endif /* GL_EXT_texture_compression_rgtc */

#ifdef GL_EXT_texture_compression_s3tc

#endif /* GL_EXT_texture_compression_s3tc */

#ifdef GL_EXT_texture_cube_map

#endif /* GL_EXT_texture_cube_map */

#ifdef GL_EXT_texture_edge_clamp

#endif /* GL_EXT_texture_edge_clamp */

#ifdef GL_EXT_texture_env

#endif /* GL_EXT_texture_env */

#ifdef GL_EXT_texture_env_add

#endif /* GL_EXT_texture_env_add */

#ifdef GL_EXT_texture_env_combine

#endif /* GL_EXT_texture_env_combine */

#ifdef GL_EXT_texture_env_dot3

#endif /* GL_EXT_texture_env_dot3 */

#ifdef GL_EXT_texture_filter_anisotropic

#endif /* GL_EXT_texture_filter_anisotropic */

#ifdef GL_EXT_texture_integer

static GLboolean _glewInit_GL_EXT_texture_integer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearColorIiEXT = (PFNGLCLEARCOLORIIEXTPROC)glewGetProcAddress((const GLubyte*)"glClearColorIiEXT")) == nullptr) || r;
  r = ((glClearColorIuiEXT = (PFNGLCLEARCOLORIUIEXTPROC)glewGetProcAddress((const GLubyte*)"glClearColorIuiEXT")) == nullptr) || r;
  r = ((glGetTexParameterIivEXT = (PFNGLGETTEXPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterIivEXT")) == nullptr) || r;
  r = ((glGetTexParameterIuivEXT = (PFNGLGETTEXPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterIuivEXT")) == nullptr) || r;
  r = ((glTexParameterIivEXT = (PFNGLTEXPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTexParameterIivEXT")) == nullptr) || r;
  r = ((glTexParameterIuivEXT = (PFNGLTEXPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTexParameterIuivEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_texture_integer */

#ifdef GL_EXT_texture_lod_bias

#endif /* GL_EXT_texture_lod_bias */

#ifdef GL_EXT_texture_mirror_clamp

#endif /* GL_EXT_texture_mirror_clamp */

#ifdef GL_EXT_texture_object

static GLboolean _glewInit_GL_EXT_texture_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAreTexturesResidentEXT = (PFNGLARETEXTURESRESIDENTEXTPROC)glewGetProcAddress((const GLubyte*)"glAreTexturesResidentEXT")) == nullptr) || r;
  r = ((glBindTextureEXT = (PFNGLBINDTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glBindTextureEXT")) == nullptr) || r;
  r = ((glDeleteTexturesEXT = (PFNGLDELETETEXTURESEXTPROC)glewGetProcAddress((const GLubyte*)"glDeleteTexturesEXT")) == nullptr) || r;
  r = ((glGenTexturesEXT = (PFNGLGENTEXTURESEXTPROC)glewGetProcAddress((const GLubyte*)"glGenTexturesEXT")) == nullptr) || r;
  r = ((glIsTextureEXT = (PFNGLISTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glIsTextureEXT")) == nullptr) || r;
  r = ((glPrioritizeTexturesEXT = (PFNGLPRIORITIZETEXTURESEXTPROC)glewGetProcAddress((const GLubyte*)"glPrioritizeTexturesEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_texture_object */

#ifdef GL_EXT_texture_perturb_normal

static GLboolean _glewInit_GL_EXT_texture_perturb_normal (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTextureNormalEXT = (PFNGLTEXTURENORMALEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureNormalEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_texture_perturb_normal */

#ifdef GL_EXT_texture_rectangle

#endif /* GL_EXT_texture_rectangle */

#ifdef GL_EXT_texture_sRGB

#endif /* GL_EXT_texture_sRGB */

#ifdef GL_EXT_texture_sRGB_decode

#endif /* GL_EXT_texture_sRGB_decode */

#ifdef GL_EXT_texture_shared_exponent

#endif /* GL_EXT_texture_shared_exponent */

#ifdef GL_EXT_texture_snorm

#endif /* GL_EXT_texture_snorm */

#ifdef GL_EXT_texture_swizzle

#endif /* GL_EXT_texture_swizzle */

#ifdef GL_EXT_timer_query

static GLboolean _glewInit_GL_EXT_timer_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjecti64vEXT")) == nullptr) || r;
  r = ((glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectui64vEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_timer_query */

#ifdef GL_EXT_transform_feedback

static GLboolean _glewInit_GL_EXT_transform_feedback (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginTransformFeedbackEXT = (PFNGLBEGINTRANSFORMFEEDBACKEXTPROC)glewGetProcAddress((const GLubyte*)"glBeginTransformFeedbackEXT")) == nullptr) || r;
  r = ((glBindBufferBaseEXT = (PFNGLBINDBUFFERBASEEXTPROC)glewGetProcAddress((const GLubyte*)"glBindBufferBaseEXT")) == nullptr) || r;
  r = ((glBindBufferOffsetEXT = (PFNGLBINDBUFFEROFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glBindBufferOffsetEXT")) == nullptr) || r;
  r = ((glBindBufferRangeEXT = (PFNGLBINDBUFFERRANGEEXTPROC)glewGetProcAddress((const GLubyte*)"glBindBufferRangeEXT")) == nullptr) || r;
  r = ((glEndTransformFeedbackEXT = (PFNGLENDTRANSFORMFEEDBACKEXTPROC)glewGetProcAddress((const GLubyte*)"glEndTransformFeedbackEXT")) == nullptr) || r;
  r = ((glGetTransformFeedbackVaryingEXT = (PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbackVaryingEXT")) == nullptr) || r;
  r = ((glTransformFeedbackVaryingsEXT = (PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackVaryingsEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_transform_feedback */

#ifdef GL_EXT_vertex_array

static GLboolean _glewInit_GL_EXT_vertex_array (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glArrayElementEXT = (PFNGLARRAYELEMENTEXTPROC)glewGetProcAddress((const GLubyte*)"glArrayElementEXT")) == nullptr) || r;
  r = ((glColorPointerEXT = (PFNGLCOLORPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glColorPointerEXT")) == nullptr) || r;
  r = ((glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysEXT")) == nullptr) || r;
  r = ((glEdgeFlagPointerEXT = (PFNGLEDGEFLAGPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glEdgeFlagPointerEXT")) == nullptr) || r;
  r = ((glIndexPointerEXT = (PFNGLINDEXPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glIndexPointerEXT")) == nullptr) || r;
  r = ((glNormalPointerEXT = (PFNGLNORMALPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glNormalPointerEXT")) == nullptr) || r;
  r = ((glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glTexCoordPointerEXT")) == nullptr) || r;
  r = ((glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexPointerEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_vertex_array */

#ifdef GL_EXT_vertex_array_bgra

#endif /* GL_EXT_vertex_array_bgra */

#ifdef GL_EXT_vertex_attrib_64bit

static GLboolean _glewInit_GL_EXT_vertex_attrib_64bit (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVertexAttribLdvEXT = (PFNGLGETVERTEXATTRIBLDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribLdvEXT")) == nullptr) || r;
  r = ((glVertexArrayVertexAttribLOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribLOffsetEXT")) == nullptr) || r;
  r = ((glVertexAttribL1dEXT = (PFNGLVERTEXATTRIBL1DEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1dEXT")) == nullptr) || r;
  r = ((glVertexAttribL1dvEXT = (PFNGLVERTEXATTRIBL1DVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1dvEXT")) == nullptr) || r;
  r = ((glVertexAttribL2dEXT = (PFNGLVERTEXATTRIBL2DEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2dEXT")) == nullptr) || r;
  r = ((glVertexAttribL2dvEXT = (PFNGLVERTEXATTRIBL2DVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2dvEXT")) == nullptr) || r;
  r = ((glVertexAttribL3dEXT = (PFNGLVERTEXATTRIBL3DEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3dEXT")) == nullptr) || r;
  r = ((glVertexAttribL3dvEXT = (PFNGLVERTEXATTRIBL3DVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3dvEXT")) == nullptr) || r;
  r = ((glVertexAttribL4dEXT = (PFNGLVERTEXATTRIBL4DEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4dEXT")) == nullptr) || r;
  r = ((glVertexAttribL4dvEXT = (PFNGLVERTEXATTRIBL4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4dvEXT")) == nullptr) || r;
  r = ((glVertexAttribLPointerEXT = (PFNGLVERTEXATTRIBLPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribLPointerEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_vertex_attrib_64bit */

#ifdef GL_EXT_vertex_shader

static GLboolean _glewInit_GL_EXT_vertex_shader (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginVertexShaderEXT = (PFNGLBEGINVERTEXSHADEREXTPROC)glewGetProcAddress((const GLubyte*)"glBeginVertexShaderEXT")) == nullptr) || r;
  r = ((glBindLightParameterEXT = (PFNGLBINDLIGHTPARAMETEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindLightParameterEXT")) == nullptr) || r;
  r = ((glBindMaterialParameterEXT = (PFNGLBINDMATERIALPARAMETEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindMaterialParameterEXT")) == nullptr) || r;
  r = ((glBindParameterEXT = (PFNGLBINDPARAMETEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindParameterEXT")) == nullptr) || r;
  r = ((glBindTexGenParameterEXT = (PFNGLBINDTEXGENPARAMETEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindTexGenParameterEXT")) == nullptr) || r;
  r = ((glBindTextureUnitParameterEXT = (PFNGLBINDTEXTUREUNITPARAMETEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindTextureUnitParameterEXT")) == nullptr) || r;
  r = ((glBindVertexShaderEXT = (PFNGLBINDVERTEXSHADEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindVertexShaderEXT")) == nullptr) || r;
  r = ((glDeleteVertexShaderEXT = (PFNGLDELETEVERTEXSHADEREXTPROC)glewGetProcAddress((const GLubyte*)"glDeleteVertexShaderEXT")) == nullptr) || r;
  r = ((glDisableVariantClientStateEXT = (PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableVariantClientStateEXT")) == nullptr) || r;
  r = ((glEnableVariantClientStateEXT = (PFNGLENABLEVARIANTCLIENTSTATEEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableVariantClientStateEXT")) == nullptr) || r;
  r = ((glEndVertexShaderEXT = (PFNGLENDVERTEXSHADEREXTPROC)glewGetProcAddress((const GLubyte*)"glEndVertexShaderEXT")) == nullptr) || r;
  r = ((glExtractComponentEXT = (PFNGLEXTRACTCOMPONENTEXTPROC)glewGetProcAddress((const GLubyte*)"glExtractComponentEXT")) == nullptr) || r;
  r = ((glGenSymbolsEXT = (PFNGLGENSYMBOLSEXTPROC)glewGetProcAddress((const GLubyte*)"glGenSymbolsEXT")) == nullptr) || r;
  r = ((glGenVertexShadersEXT = (PFNGLGENVERTEXSHADERSEXTPROC)glewGetProcAddress((const GLubyte*)"glGenVertexShadersEXT")) == nullptr) || r;
  r = ((glGetInvariantBooleanvEXT = (PFNGLGETINVARIANTBOOLEANVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetInvariantBooleanvEXT")) == nullptr) || r;
  r = ((glGetInvariantFloatvEXT = (PFNGLGETINVARIANTFLOATVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetInvariantFloatvEXT")) == nullptr) || r;
  r = ((glGetInvariantIntegervEXT = (PFNGLGETINVARIANTINTEGERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetInvariantIntegervEXT")) == nullptr) || r;
  r = ((glGetLocalConstantBooleanvEXT = (PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetLocalConstantBooleanvEXT")) == nullptr) || r;
  r = ((glGetLocalConstantFloatvEXT = (PFNGLGETLOCALCONSTANTFLOATVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetLocalConstantFloatvEXT")) == nullptr) || r;
  r = ((glGetLocalConstantIntegervEXT = (PFNGLGETLOCALCONSTANTINTEGERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetLocalConstantIntegervEXT")) == nullptr) || r;
  r = ((glGetVariantBooleanvEXT = (PFNGLGETVARIANTBOOLEANVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVariantBooleanvEXT")) == nullptr) || r;
  r = ((glGetVariantFloatvEXT = (PFNGLGETVARIANTFLOATVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVariantFloatvEXT")) == nullptr) || r;
  r = ((glGetVariantIntegervEXT = (PFNGLGETVARIANTINTEGERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVariantIntegervEXT")) == nullptr) || r;
  r = ((glGetVariantPointervEXT = (PFNGLGETVARIANTPOINTERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVariantPointervEXT")) == nullptr) || r;
  r = ((glInsertComponentEXT = (PFNGLINSERTCOMPONENTEXTPROC)glewGetProcAddress((const GLubyte*)"glInsertComponentEXT")) == nullptr) || r;
  r = ((glIsVariantEnabledEXT = (PFNGLISVARIANTENABLEDEXTPROC)glewGetProcAddress((const GLubyte*)"glIsVariantEnabledEXT")) == nullptr) || r;
  r = ((glSetInvariantEXT = (PFNGLSETINVARIANTEXTPROC)glewGetProcAddress((const GLubyte*)"glSetInvariantEXT")) == nullptr) || r;
  r = ((glSetLocalConstantEXT = (PFNGLSETLOCALCONSTANTEXTPROC)glewGetProcAddress((const GLubyte*)"glSetLocalConstantEXT")) == nullptr) || r;
  r = ((glShaderOp1EXT = (PFNGLSHADEROP1EXTPROC)glewGetProcAddress((const GLubyte*)"glShaderOp1EXT")) == nullptr) || r;
  r = ((glShaderOp2EXT = (PFNGLSHADEROP2EXTPROC)glewGetProcAddress((const GLubyte*)"glShaderOp2EXT")) == nullptr) || r;
  r = ((glShaderOp3EXT = (PFNGLSHADEROP3EXTPROC)glewGetProcAddress((const GLubyte*)"glShaderOp3EXT")) == nullptr) || r;
  r = ((glSwizzleEXT = (PFNGLSWIZZLEEXTPROC)glewGetProcAddress((const GLubyte*)"glSwizzleEXT")) == nullptr) || r;
  r = ((glVariantPointerEXT = (PFNGLVARIANTPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glVariantPointerEXT")) == nullptr) || r;
  r = ((glVariantbvEXT = (PFNGLVARIANTBVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantbvEXT")) == nullptr) || r;
  r = ((glVariantdvEXT = (PFNGLVARIANTDVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantdvEXT")) == nullptr) || r;
  r = ((glVariantfvEXT = (PFNGLVARIANTFVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantfvEXT")) == nullptr) || r;
  r = ((glVariantivEXT = (PFNGLVARIANTIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantivEXT")) == nullptr) || r;
  r = ((glVariantsvEXT = (PFNGLVARIANTSVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantsvEXT")) == nullptr) || r;
  r = ((glVariantubvEXT = (PFNGLVARIANTUBVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantubvEXT")) == nullptr) || r;
  r = ((glVariantuivEXT = (PFNGLVARIANTUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantuivEXT")) == nullptr) || r;
  r = ((glVariantusvEXT = (PFNGLVARIANTUSVEXTPROC)glewGetProcAddress((const GLubyte*)"glVariantusvEXT")) == nullptr) || r;
  r = ((glWriteMaskEXT = (PFNGLWRITEMASKEXTPROC)glewGetProcAddress((const GLubyte*)"glWriteMaskEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_vertex_shader */

#ifdef GL_EXT_vertex_weighting

static GLboolean _glewInit_GL_EXT_vertex_weighting (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexWeightPointerEXT = (PFNGLVERTEXWEIGHTPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexWeightPointerEXT")) == nullptr) || r;
  r = ((glVertexWeightfEXT = (PFNGLVERTEXWEIGHTFEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexWeightfEXT")) == nullptr) || r;
  r = ((glVertexWeightfvEXT = (PFNGLVERTEXWEIGHTFVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexWeightfvEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_vertex_weighting */

#ifdef GL_EXT_x11_sync_object

static GLboolean _glewInit_GL_EXT_x11_sync_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glImportSyncEXT = (PFNGLIMPORTSYNCEXTPROC)glewGetProcAddress((const GLubyte*)"glImportSyncEXT")) == nullptr) || r;

  return r;
}

#endif /* GL_EXT_x11_sync_object */

#ifdef GL_GREMEDY_frame_terminator

static GLboolean _glewInit_GL_GREMEDY_frame_terminator (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)glewGetProcAddress((const GLubyte*)"glFrameTerminatorGREMEDY")) == nullptr) || r;

  return r;
}

#endif /* GL_GREMEDY_frame_terminator */

#ifdef GL_GREMEDY_string_marker

static GLboolean _glewInit_GL_GREMEDY_string_marker (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)glewGetProcAddress((const GLubyte*)"glStringMarkerGREMEDY")) == nullptr) || r;

  return r;
}

#endif /* GL_GREMEDY_string_marker */

#ifdef GL_HP_convolution_border_modes

#endif /* GL_HP_convolution_border_modes */

#ifdef GL_HP_image_transform

static GLboolean _glewInit_GL_HP_image_transform (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetImageTransformParameterfvHP = (PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC)glewGetProcAddress((const GLubyte*)"glGetImageTransformParameterfvHP")) == nullptr) || r;
  r = ((glGetImageTransformParameterivHP = (PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC)glewGetProcAddress((const GLubyte*)"glGetImageTransformParameterivHP")) == nullptr) || r;
  r = ((glImageTransformParameterfHP = (PFNGLIMAGETRANSFORMPARAMETERFHPPROC)glewGetProcAddress((const GLubyte*)"glImageTransformParameterfHP")) == nullptr) || r;
  r = ((glImageTransformParameterfvHP = (PFNGLIMAGETRANSFORMPARAMETERFVHPPROC)glewGetProcAddress((const GLubyte*)"glImageTransformParameterfvHP")) == nullptr) || r;
  r = ((glImageTransformParameteriHP = (PFNGLIMAGETRANSFORMPARAMETERIHPPROC)glewGetProcAddress((const GLubyte*)"glImageTransformParameteriHP")) == nullptr) || r;
  r = ((glImageTransformParameterivHP = (PFNGLIMAGETRANSFORMPARAMETERIVHPPROC)glewGetProcAddress((const GLubyte*)"glImageTransformParameterivHP")) == nullptr) || r;

  return r;
}

#endif /* GL_HP_image_transform */

#ifdef GL_HP_occlusion_test

#endif /* GL_HP_occlusion_test */

#ifdef GL_HP_texture_lighting

#endif /* GL_HP_texture_lighting */

#ifdef GL_IBM_cull_vertex

#endif /* GL_IBM_cull_vertex */

#ifdef GL_IBM_multimode_draw_arrays

static GLboolean _glewInit_GL_IBM_multimode_draw_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glMultiModeDrawArraysIBM = (PFNGLMULTIMODEDRAWARRAYSIBMPROC)glewGetProcAddress((const GLubyte*)"glMultiModeDrawArraysIBM")) == nullptr) || r;
  r = ((glMultiModeDrawElementsIBM = (PFNGLMULTIMODEDRAWELEMENTSIBMPROC)glewGetProcAddress((const GLubyte*)"glMultiModeDrawElementsIBM")) == nullptr) || r;

  return r;
}

#endif /* GL_IBM_multimode_draw_arrays */

#ifdef GL_IBM_rasterpos_clip

#endif /* GL_IBM_rasterpos_clip */

#ifdef GL_IBM_static_data

#endif /* GL_IBM_static_data */

#ifdef GL_IBM_texture_mirrored_repeat

#endif /* GL_IBM_texture_mirrored_repeat */

#ifdef GL_IBM_vertex_array_lists

static GLboolean _glewInit_GL_IBM_vertex_array_lists (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorPointerListIBM = (PFNGLCOLORPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glColorPointerListIBM")) == nullptr) || r;
  r = ((glEdgeFlagPointerListIBM = (PFNGLEDGEFLAGPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glEdgeFlagPointerListIBM")) == nullptr) || r;
  r = ((glFogCoordPointerListIBM = (PFNGLFOGCOORDPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glFogCoordPointerListIBM")) == nullptr) || r;
  r = ((glIndexPointerListIBM = (PFNGLINDEXPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glIndexPointerListIBM")) == nullptr) || r;
  r = ((glNormalPointerListIBM = (PFNGLNORMALPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glNormalPointerListIBM")) == nullptr) || r;
  r = ((glSecondaryColorPointerListIBM = (PFNGLSECONDARYCOLORPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorPointerListIBM")) == nullptr) || r;
  r = ((glTexCoordPointerListIBM = (PFNGLTEXCOORDPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glTexCoordPointerListIBM")) == nullptr) || r;
  r = ((glVertexPointerListIBM = (PFNGLVERTEXPOINTERLISTIBMPROC)glewGetProcAddress((const GLubyte*)"glVertexPointerListIBM")) == nullptr) || r;

  return r;
}

#endif /* GL_IBM_vertex_array_lists */

#ifdef GL_INGR_color_clamp

#endif /* GL_INGR_color_clamp */

#ifdef GL_INGR_interlace_read

#endif /* GL_INGR_interlace_read */

#ifdef GL_INTEL_parallel_arrays

static GLboolean _glewInit_GL_INTEL_parallel_arrays (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorPointervINTEL = (PFNGLCOLORPOINTERVINTELPROC)glewGetProcAddress((const GLubyte*)"glColorPointervINTEL")) == nullptr) || r;
  r = ((glNormalPointervINTEL = (PFNGLNORMALPOINTERVINTELPROC)glewGetProcAddress((const GLubyte*)"glNormalPointervINTEL")) == nullptr) || r;
  r = ((glTexCoordPointervINTEL = (PFNGLTEXCOORDPOINTERVINTELPROC)glewGetProcAddress((const GLubyte*)"glTexCoordPointervINTEL")) == nullptr) || r;
  r = ((glVertexPointervINTEL = (PFNGLVERTEXPOINTERVINTELPROC)glewGetProcAddress((const GLubyte*)"glVertexPointervINTEL")) == nullptr) || r;

  return r;
}

#endif /* GL_INTEL_parallel_arrays */

#ifdef GL_INTEL_texture_scissor

static GLboolean _glewInit_GL_INTEL_texture_scissor (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexScissorFuncINTEL = (PFNGLTEXSCISSORFUNCINTELPROC)glewGetProcAddress((const GLubyte*)"glTexScissorFuncINTEL")) == nullptr) || r;
  r = ((glTexScissorINTEL = (PFNGLTEXSCISSORINTELPROC)glewGetProcAddress((const GLubyte*)"glTexScissorINTEL")) == nullptr) || r;

  return r;
}

#endif /* GL_INTEL_texture_scissor */

#ifdef GL_KTX_buffer_region

static GLboolean _glewInit_GL_KTX_buffer_region (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBufferRegionEnabled = (PFNGLBUFFERREGIONENABLEDPROC)glewGetProcAddress((const GLubyte*)"glBufferRegionEnabled")) == nullptr) || r;
  r = ((glDeleteBufferRegion = (PFNGLDELETEBUFFERREGIONPROC)glewGetProcAddress((const GLubyte*)"glDeleteBufferRegion")) == nullptr) || r;
  r = ((glDrawBufferRegion = (PFNGLDRAWBUFFERREGIONPROC)glewGetProcAddress((const GLubyte*)"glDrawBufferRegion")) == nullptr) || r;
  r = ((glNewBufferRegion = (PFNGLNEWBUFFERREGIONPROC)glewGetProcAddress((const GLubyte*)"glNewBufferRegion")) == nullptr) || r;
  r = ((glReadBufferRegion = (PFNGLREADBUFFERREGIONPROC)glewGetProcAddress((const GLubyte*)"glReadBufferRegion")) == nullptr) || r;

  return r;
}

#endif /* GL_KTX_buffer_region */

#ifdef GL_MESAX_texture_stack

#endif /* GL_MESAX_texture_stack */

#ifdef GL_MESA_pack_invert

#endif /* GL_MESA_pack_invert */

#ifdef GL_MESA_resize_buffers

static GLboolean _glewInit_GL_MESA_resize_buffers (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glResizeBuffersMESA = (PFNGLRESIZEBUFFERSMESAPROC)glewGetProcAddress((const GLubyte*)"glResizeBuffersMESA")) == nullptr) || r;

  return r;
}

#endif /* GL_MESA_resize_buffers */

#ifdef GL_MESA_window_pos

static GLboolean _glewInit_GL_MESA_window_pos (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glWindowPos2dMESA = (PFNGLWINDOWPOS2DMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2dMESA")) == nullptr) || r;
  r = ((glWindowPos2dvMESA = (PFNGLWINDOWPOS2DVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2dvMESA")) == nullptr) || r;
  r = ((glWindowPos2fMESA = (PFNGLWINDOWPOS2FMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2fMESA")) == nullptr) || r;
  r = ((glWindowPos2fvMESA = (PFNGLWINDOWPOS2FVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2fvMESA")) == nullptr) || r;
  r = ((glWindowPos2iMESA = (PFNGLWINDOWPOS2IMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2iMESA")) == nullptr) || r;
  r = ((glWindowPos2ivMESA = (PFNGLWINDOWPOS2IVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2ivMESA")) == nullptr) || r;
  r = ((glWindowPos2sMESA = (PFNGLWINDOWPOS2SMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2sMESA")) == nullptr) || r;
  r = ((glWindowPos2svMESA = (PFNGLWINDOWPOS2SVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2svMESA")) == nullptr) || r;
  r = ((glWindowPos3dMESA = (PFNGLWINDOWPOS3DMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3dMESA")) == nullptr) || r;
  r = ((glWindowPos3dvMESA = (PFNGLWINDOWPOS3DVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3dvMESA")) == nullptr) || r;
  r = ((glWindowPos3fMESA = (PFNGLWINDOWPOS3FMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3fMESA")) == nullptr) || r;
  r = ((glWindowPos3fvMESA = (PFNGLWINDOWPOS3FVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3fvMESA")) == nullptr) || r;
  r = ((glWindowPos3iMESA = (PFNGLWINDOWPOS3IMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3iMESA")) == nullptr) || r;
  r = ((glWindowPos3ivMESA = (PFNGLWINDOWPOS3IVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3ivMESA")) == nullptr) || r;
  r = ((glWindowPos3sMESA = (PFNGLWINDOWPOS3SMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3sMESA")) == nullptr) || r;
  r = ((glWindowPos3svMESA = (PFNGLWINDOWPOS3SVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3svMESA")) == nullptr) || r;
  r = ((glWindowPos4dMESA = (PFNGLWINDOWPOS4DMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4dMESA")) == nullptr) || r;
  r = ((glWindowPos4dvMESA = (PFNGLWINDOWPOS4DVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4dvMESA")) == nullptr) || r;
  r = ((glWindowPos4fMESA = (PFNGLWINDOWPOS4FMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4fMESA")) == nullptr) || r;
  r = ((glWindowPos4fvMESA = (PFNGLWINDOWPOS4FVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4fvMESA")) == nullptr) || r;
  r = ((glWindowPos4iMESA = (PFNGLWINDOWPOS4IMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4iMESA")) == nullptr) || r;
  r = ((glWindowPos4ivMESA = (PFNGLWINDOWPOS4IVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4ivMESA")) == nullptr) || r;
  r = ((glWindowPos4sMESA = (PFNGLWINDOWPOS4SMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4sMESA")) == nullptr) || r;
  r = ((glWindowPos4svMESA = (PFNGLWINDOWPOS4SVMESAPROC)glewGetProcAddress((const GLubyte*)"glWindowPos4svMESA")) == nullptr) || r;

  return r;
}

#endif /* GL_MESA_window_pos */

#ifdef GL_MESA_ycbcr_texture

#endif /* GL_MESA_ycbcr_texture */

#ifdef GL_NVX_gpu_memory_info

#endif /* GL_NVX_gpu_memory_info */

#ifdef GL_NV_blend_square

#endif /* GL_NV_blend_square */

#ifdef GL_NV_conditional_render

static GLboolean _glewInit_GL_NV_conditional_render (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginConditionalRenderNV = (PFNGLBEGINCONDITIONALRENDERNVPROC)glewGetProcAddress((const GLubyte*)"glBeginConditionalRenderNV")) == nullptr) || r;
  r = ((glEndConditionalRenderNV = (PFNGLENDCONDITIONALRENDERNVPROC)glewGetProcAddress((const GLubyte*)"glEndConditionalRenderNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_conditional_render */

#ifdef GL_NV_copy_depth_to_color

#endif /* GL_NV_copy_depth_to_color */

#ifdef GL_NV_copy_image

static GLboolean _glewInit_GL_NV_copy_image (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyImageSubDataNV = (PFNGLCOPYIMAGESUBDATANVPROC)glewGetProcAddress((const GLubyte*)"glCopyImageSubDataNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_copy_image */

#ifdef GL_NV_depth_buffer_float

static GLboolean _glewInit_GL_NV_depth_buffer_float (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearDepthdNV = (PFNGLCLEARDEPTHDNVPROC)glewGetProcAddress((const GLubyte*)"glClearDepthdNV")) == nullptr) || r;
  r = ((glDepthBoundsdNV = (PFNGLDEPTHBOUNDSDNVPROC)glewGetProcAddress((const GLubyte*)"glDepthBoundsdNV")) == nullptr) || r;
  r = ((glDepthRangedNV = (PFNGLDEPTHRANGEDNVPROC)glewGetProcAddress((const GLubyte*)"glDepthRangedNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_depth_buffer_float */

#ifdef GL_NV_depth_clamp

#endif /* GL_NV_depth_clamp */

#ifdef GL_NV_depth_range_unclamped

#endif /* GL_NV_depth_range_unclamped */

#ifdef GL_NV_evaluators

static GLboolean _glewInit_GL_NV_evaluators (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glEvalMapsNV = (PFNGLEVALMAPSNVPROC)glewGetProcAddress((const GLubyte*)"glEvalMapsNV")) == nullptr) || r;
  r = ((glGetMapAttribParameterfvNV = (PFNGLGETMAPATTRIBPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetMapAttribParameterfvNV")) == nullptr) || r;
  r = ((glGetMapAttribParameterivNV = (PFNGLGETMAPATTRIBPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetMapAttribParameterivNV")) == nullptr) || r;
  r = ((glGetMapControlPointsNV = (PFNGLGETMAPCONTROLPOINTSNVPROC)glewGetProcAddress((const GLubyte*)"glGetMapControlPointsNV")) == nullptr) || r;
  r = ((glGetMapParameterfvNV = (PFNGLGETMAPPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetMapParameterfvNV")) == nullptr) || r;
  r = ((glGetMapParameterivNV = (PFNGLGETMAPPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetMapParameterivNV")) == nullptr) || r;
  r = ((glMapControlPointsNV = (PFNGLMAPCONTROLPOINTSNVPROC)glewGetProcAddress((const GLubyte*)"glMapControlPointsNV")) == nullptr) || r;
  r = ((glMapParameterfvNV = (PFNGLMAPPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glMapParameterfvNV")) == nullptr) || r;
  r = ((glMapParameterivNV = (PFNGLMAPPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glMapParameterivNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_evaluators */

#ifdef GL_NV_explicit_multisample

static GLboolean _glewInit_GL_NV_explicit_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetMultisamplefvNV = (PFNGLGETMULTISAMPLEFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetMultisamplefvNV")) == nullptr) || r;
  r = ((glSampleMaskIndexedNV = (PFNGLSAMPLEMASKINDEXEDNVPROC)glewGetProcAddress((const GLubyte*)"glSampleMaskIndexedNV")) == nullptr) || r;
  r = ((glTexRenderbufferNV = (PFNGLTEXRENDERBUFFERNVPROC)glewGetProcAddress((const GLubyte*)"glTexRenderbufferNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_explicit_multisample */

#ifdef GL_NV_fence

static GLboolean _glewInit_GL_NV_fence (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDeleteFencesNV = (PFNGLDELETEFENCESNVPROC)glewGetProcAddress((const GLubyte*)"glDeleteFencesNV")) == nullptr) || r;
  r = ((glFinishFenceNV = (PFNGLFINISHFENCENVPROC)glewGetProcAddress((const GLubyte*)"glFinishFenceNV")) == nullptr) || r;
  r = ((glGenFencesNV = (PFNGLGENFENCESNVPROC)glewGetProcAddress((const GLubyte*)"glGenFencesNV")) == nullptr) || r;
  r = ((glGetFenceivNV = (PFNGLGETFENCEIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetFenceivNV")) == nullptr) || r;
  r = ((glIsFenceNV = (PFNGLISFENCENVPROC)glewGetProcAddress((const GLubyte*)"glIsFenceNV")) == nullptr) || r;
  r = ((glSetFenceNV = (PFNGLSETFENCENVPROC)glewGetProcAddress((const GLubyte*)"glSetFenceNV")) == nullptr) || r;
  r = ((glTestFenceNV = (PFNGLTESTFENCENVPROC)glewGetProcAddress((const GLubyte*)"glTestFenceNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_fence */

#ifdef GL_NV_float_buffer

#endif /* GL_NV_float_buffer */

#ifdef GL_NV_fog_distance

#endif /* GL_NV_fog_distance */

#ifdef GL_NV_fragment_program

static GLboolean _glewInit_GL_NV_fragment_program (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetProgramNamedParameterdvNV = (PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramNamedParameterdvNV")) == nullptr) || r;
  r = ((glGetProgramNamedParameterfvNV = (PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramNamedParameterfvNV")) == nullptr) || r;
  r = ((glProgramNamedParameter4dNV = (PFNGLPROGRAMNAMEDPARAMETER4DNVPROC)glewGetProcAddress((const GLubyte*)"glProgramNamedParameter4dNV")) == nullptr) || r;
  r = ((glProgramNamedParameter4dvNV = (PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramNamedParameter4dvNV")) == nullptr) || r;
  r = ((glProgramNamedParameter4fNV = (PFNGLPROGRAMNAMEDPARAMETER4FNVPROC)glewGetProcAddress((const GLubyte*)"glProgramNamedParameter4fNV")) == nullptr) || r;
  r = ((glProgramNamedParameter4fvNV = (PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramNamedParameter4fvNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_fragment_program */

#ifdef GL_NV_fragment_program2

#endif /* GL_NV_fragment_program2 */

#ifdef GL_NV_fragment_program4

#endif /* GL_NV_fragment_program4 */

#ifdef GL_NV_fragment_program_option

#endif /* GL_NV_fragment_program_option */

#ifdef GL_NV_framebuffer_multisample_coverage

static GLboolean _glewInit_GL_NV_framebuffer_multisample_coverage (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glRenderbufferStorageMultisampleCoverageNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisampleCoverageNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_framebuffer_multisample_coverage */

#ifdef GL_NV_geometry_program4

static GLboolean _glewInit_GL_NV_geometry_program4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramVertexLimitNV = (PFNGLPROGRAMVERTEXLIMITNVPROC)glewGetProcAddress((const GLubyte*)"glProgramVertexLimitNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_geometry_program4 */

#ifdef GL_NV_geometry_shader4

#endif /* GL_NV_geometry_shader4 */

#ifdef GL_NV_gpu_program4

static GLboolean _glewInit_GL_NV_gpu_program4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramEnvParameterI4iNV = (PFNGLPROGRAMENVPARAMETERI4INVPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameterI4iNV")) == nullptr) || r;
  r = ((glProgramEnvParameterI4ivNV = (PFNGLPROGRAMENVPARAMETERI4IVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameterI4ivNV")) == nullptr) || r;
  r = ((glProgramEnvParameterI4uiNV = (PFNGLPROGRAMENVPARAMETERI4UINVPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameterI4uiNV")) == nullptr) || r;
  r = ((glProgramEnvParameterI4uivNV = (PFNGLPROGRAMENVPARAMETERI4UIVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParameterI4uivNV")) == nullptr) || r;
  r = ((glProgramEnvParametersI4ivNV = (PFNGLPROGRAMENVPARAMETERSI4IVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParametersI4ivNV")) == nullptr) || r;
  r = ((glProgramEnvParametersI4uivNV = (PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramEnvParametersI4uivNV")) == nullptr) || r;
  r = ((glProgramLocalParameterI4iNV = (PFNGLPROGRAMLOCALPARAMETERI4INVPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameterI4iNV")) == nullptr) || r;
  r = ((glProgramLocalParameterI4ivNV = (PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameterI4ivNV")) == nullptr) || r;
  r = ((glProgramLocalParameterI4uiNV = (PFNGLPROGRAMLOCALPARAMETERI4UINVPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameterI4uiNV")) == nullptr) || r;
  r = ((glProgramLocalParameterI4uivNV = (PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParameterI4uivNV")) == nullptr) || r;
  r = ((glProgramLocalParametersI4ivNV = (PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParametersI4ivNV")) == nullptr) || r;
  r = ((glProgramLocalParametersI4uivNV = (PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramLocalParametersI4uivNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_gpu_program4 */

#ifdef GL_NV_gpu_program5

#endif /* GL_NV_gpu_program5 */

#ifdef GL_NV_gpu_program_fp64

#endif /* GL_NV_gpu_program_fp64 */

#ifdef GL_NV_gpu_shader5

static GLboolean _glewInit_GL_NV_gpu_shader5 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetUniformi64vNV = (PFNGLGETUNIFORMI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformi64vNV")) == nullptr) || r;
  r = ((glGetUniformui64vNV = (PFNGLGETUNIFORMUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformui64vNV")) == nullptr) || r;
  r = ((glProgramUniform1i64NV = (PFNGLPROGRAMUNIFORM1I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1i64NV")) == nullptr) || r;
  r = ((glProgramUniform1i64vNV = (PFNGLPROGRAMUNIFORM1I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1i64vNV")) == nullptr) || r;
  r = ((glProgramUniform1ui64NV = (PFNGLPROGRAMUNIFORM1UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ui64NV")) == nullptr) || r;
  r = ((glProgramUniform1ui64vNV = (PFNGLPROGRAMUNIFORM1UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ui64vNV")) == nullptr) || r;
  r = ((glProgramUniform2i64NV = (PFNGLPROGRAMUNIFORM2I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2i64NV")) == nullptr) || r;
  r = ((glProgramUniform2i64vNV = (PFNGLPROGRAMUNIFORM2I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2i64vNV")) == nullptr) || r;
  r = ((glProgramUniform2ui64NV = (PFNGLPROGRAMUNIFORM2UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ui64NV")) == nullptr) || r;
  r = ((glProgramUniform2ui64vNV = (PFNGLPROGRAMUNIFORM2UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ui64vNV")) == nullptr) || r;
  r = ((glProgramUniform3i64NV = (PFNGLPROGRAMUNIFORM3I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3i64NV")) == nullptr) || r;
  r = ((glProgramUniform3i64vNV = (PFNGLPROGRAMUNIFORM3I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3i64vNV")) == nullptr) || r;
  r = ((glProgramUniform3ui64NV = (PFNGLPROGRAMUNIFORM3UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ui64NV")) == nullptr) || r;
  r = ((glProgramUniform3ui64vNV = (PFNGLPROGRAMUNIFORM3UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ui64vNV")) == nullptr) || r;
  r = ((glProgramUniform4i64NV = (PFNGLPROGRAMUNIFORM4I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4i64NV")) == nullptr) || r;
  r = ((glProgramUniform4i64vNV = (PFNGLPROGRAMUNIFORM4I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4i64vNV")) == nullptr) || r;
  r = ((glProgramUniform4ui64NV = (PFNGLPROGRAMUNIFORM4UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ui64NV")) == nullptr) || r;
  r = ((glProgramUniform4ui64vNV = (PFNGLPROGRAMUNIFORM4UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ui64vNV")) == nullptr) || r;
  r = ((glUniform1i64NV = (PFNGLUNIFORM1I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform1i64NV")) == nullptr) || r;
  r = ((glUniform1i64vNV = (PFNGLUNIFORM1I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform1i64vNV")) == nullptr) || r;
  r = ((glUniform1ui64NV = (PFNGLUNIFORM1UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform1ui64NV")) == nullptr) || r;
  r = ((glUniform1ui64vNV = (PFNGLUNIFORM1UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform1ui64vNV")) == nullptr) || r;
  r = ((glUniform2i64NV = (PFNGLUNIFORM2I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform2i64NV")) == nullptr) || r;
  r = ((glUniform2i64vNV = (PFNGLUNIFORM2I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform2i64vNV")) == nullptr) || r;
  r = ((glUniform2ui64NV = (PFNGLUNIFORM2UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform2ui64NV")) == nullptr) || r;
  r = ((glUniform2ui64vNV = (PFNGLUNIFORM2UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform2ui64vNV")) == nullptr) || r;
  r = ((glUniform3i64NV = (PFNGLUNIFORM3I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform3i64NV")) == nullptr) || r;
  r = ((glUniform3i64vNV = (PFNGLUNIFORM3I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform3i64vNV")) == nullptr) || r;
  r = ((glUniform3ui64NV = (PFNGLUNIFORM3UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform3ui64NV")) == nullptr) || r;
  r = ((glUniform3ui64vNV = (PFNGLUNIFORM3UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform3ui64vNV")) == nullptr) || r;
  r = ((glUniform4i64NV = (PFNGLUNIFORM4I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform4i64NV")) == nullptr) || r;
  r = ((glUniform4i64vNV = (PFNGLUNIFORM4I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform4i64vNV")) == nullptr) || r;
  r = ((glUniform4ui64NV = (PFNGLUNIFORM4UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform4ui64NV")) == nullptr) || r;
  r = ((glUniform4ui64vNV = (PFNGLUNIFORM4UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform4ui64vNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_gpu_shader5 */

#ifdef GL_NV_half_float

static GLboolean _glewInit_GL_NV_half_float (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColor3hNV = (PFNGLCOLOR3HNVPROC)glewGetProcAddress((const GLubyte*)"glColor3hNV")) == nullptr) || r;
  r = ((glColor3hvNV = (PFNGLCOLOR3HVNVPROC)glewGetProcAddress((const GLubyte*)"glColor3hvNV")) == nullptr) || r;
  r = ((glColor4hNV = (PFNGLCOLOR4HNVPROC)glewGetProcAddress((const GLubyte*)"glColor4hNV")) == nullptr) || r;
  r = ((glColor4hvNV = (PFNGLCOLOR4HVNVPROC)glewGetProcAddress((const GLubyte*)"glColor4hvNV")) == nullptr) || r;
  r = ((glFogCoordhNV = (PFNGLFOGCOORDHNVPROC)glewGetProcAddress((const GLubyte*)"glFogCoordhNV")) == nullptr) || r;
  r = ((glFogCoordhvNV = (PFNGLFOGCOORDHVNVPROC)glewGetProcAddress((const GLubyte*)"glFogCoordhvNV")) == nullptr) || r;
  r = ((glMultiTexCoord1hNV = (PFNGLMULTITEXCOORD1HNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1hNV")) == nullptr) || r;
  r = ((glMultiTexCoord1hvNV = (PFNGLMULTITEXCOORD1HVNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1hvNV")) == nullptr) || r;
  r = ((glMultiTexCoord2hNV = (PFNGLMULTITEXCOORD2HNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2hNV")) == nullptr) || r;
  r = ((glMultiTexCoord2hvNV = (PFNGLMULTITEXCOORD2HVNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2hvNV")) == nullptr) || r;
  r = ((glMultiTexCoord3hNV = (PFNGLMULTITEXCOORD3HNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3hNV")) == nullptr) || r;
  r = ((glMultiTexCoord3hvNV = (PFNGLMULTITEXCOORD3HVNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3hvNV")) == nullptr) || r;
  r = ((glMultiTexCoord4hNV = (PFNGLMULTITEXCOORD4HNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4hNV")) == nullptr) || r;
  r = ((glMultiTexCoord4hvNV = (PFNGLMULTITEXCOORD4HVNVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4hvNV")) == nullptr) || r;
  r = ((glNormal3hNV = (PFNGLNORMAL3HNVPROC)glewGetProcAddress((const GLubyte*)"glNormal3hNV")) == nullptr) || r;
  r = ((glNormal3hvNV = (PFNGLNORMAL3HVNVPROC)glewGetProcAddress((const GLubyte*)"glNormal3hvNV")) == nullptr) || r;
  r = ((glSecondaryColor3hNV = (PFNGLSECONDARYCOLOR3HNVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3hNV")) == nullptr) || r;
  r = ((glSecondaryColor3hvNV = (PFNGLSECONDARYCOLOR3HVNVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3hvNV")) == nullptr) || r;
  r = ((glTexCoord1hNV = (PFNGLTEXCOORD1HNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord1hNV")) == nullptr) || r;
  r = ((glTexCoord1hvNV = (PFNGLTEXCOORD1HVNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord1hvNV")) == nullptr) || r;
  r = ((glTexCoord2hNV = (PFNGLTEXCOORD2HNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2hNV")) == nullptr) || r;
  r = ((glTexCoord2hvNV = (PFNGLTEXCOORD2HVNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2hvNV")) == nullptr) || r;
  r = ((glTexCoord3hNV = (PFNGLTEXCOORD3HNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord3hNV")) == nullptr) || r;
  r = ((glTexCoord3hvNV = (PFNGLTEXCOORD3HVNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord3hvNV")) == nullptr) || r;
  r = ((glTexCoord4hNV = (PFNGLTEXCOORD4HNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord4hNV")) == nullptr) || r;
  r = ((glTexCoord4hvNV = (PFNGLTEXCOORD4HVNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoord4hvNV")) == nullptr) || r;
  r = ((glVertex2hNV = (PFNGLVERTEX2HNVPROC)glewGetProcAddress((const GLubyte*)"glVertex2hNV")) == nullptr) || r;
  r = ((glVertex2hvNV = (PFNGLVERTEX2HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertex2hvNV")) == nullptr) || r;
  r = ((glVertex3hNV = (PFNGLVERTEX3HNVPROC)glewGetProcAddress((const GLubyte*)"glVertex3hNV")) == nullptr) || r;
  r = ((glVertex3hvNV = (PFNGLVERTEX3HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertex3hvNV")) == nullptr) || r;
  r = ((glVertex4hNV = (PFNGLVERTEX4HNVPROC)glewGetProcAddress((const GLubyte*)"glVertex4hNV")) == nullptr) || r;
  r = ((glVertex4hvNV = (PFNGLVERTEX4HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertex4hvNV")) == nullptr) || r;
  r = ((glVertexAttrib1hNV = (PFNGLVERTEXATTRIB1HNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1hNV")) == nullptr) || r;
  r = ((glVertexAttrib1hvNV = (PFNGLVERTEXATTRIB1HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1hvNV")) == nullptr) || r;
  r = ((glVertexAttrib2hNV = (PFNGLVERTEXATTRIB2HNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2hNV")) == nullptr) || r;
  r = ((glVertexAttrib2hvNV = (PFNGLVERTEXATTRIB2HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2hvNV")) == nullptr) || r;
  r = ((glVertexAttrib3hNV = (PFNGLVERTEXATTRIB3HNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3hNV")) == nullptr) || r;
  r = ((glVertexAttrib3hvNV = (PFNGLVERTEXATTRIB3HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3hvNV")) == nullptr) || r;
  r = ((glVertexAttrib4hNV = (PFNGLVERTEXATTRIB4HNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4hNV")) == nullptr) || r;
  r = ((glVertexAttrib4hvNV = (PFNGLVERTEXATTRIB4HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4hvNV")) == nullptr) || r;
  r = ((glVertexAttribs1hvNV = (PFNGLVERTEXATTRIBS1HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs1hvNV")) == nullptr) || r;
  r = ((glVertexAttribs2hvNV = (PFNGLVERTEXATTRIBS2HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs2hvNV")) == nullptr) || r;
  r = ((glVertexAttribs3hvNV = (PFNGLVERTEXATTRIBS3HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs3hvNV")) == nullptr) || r;
  r = ((glVertexAttribs4hvNV = (PFNGLVERTEXATTRIBS4HVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs4hvNV")) == nullptr) || r;
  r = ((glVertexWeighthNV = (PFNGLVERTEXWEIGHTHNVPROC)glewGetProcAddress((const GLubyte*)"glVertexWeighthNV")) == nullptr) || r;
  r = ((glVertexWeighthvNV = (PFNGLVERTEXWEIGHTHVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexWeighthvNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_half_float */

#ifdef GL_NV_light_max_exponent

#endif /* GL_NV_light_max_exponent */

#ifdef GL_NV_multisample_coverage

#endif /* GL_NV_multisample_coverage */

#ifdef GL_NV_multisample_filter_hint

#endif /* GL_NV_multisample_filter_hint */

#ifdef GL_NV_occlusion_query

static GLboolean _glewInit_GL_NV_occlusion_query (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginOcclusionQueryNV = (PFNGLBEGINOCCLUSIONQUERYNVPROC)glewGetProcAddress((const GLubyte*)"glBeginOcclusionQueryNV")) == nullptr) || r;
  r = ((glDeleteOcclusionQueriesNV = (PFNGLDELETEOCCLUSIONQUERIESNVPROC)glewGetProcAddress((const GLubyte*)"glDeleteOcclusionQueriesNV")) == nullptr) || r;
  r = ((glEndOcclusionQueryNV = (PFNGLENDOCCLUSIONQUERYNVPROC)glewGetProcAddress((const GLubyte*)"glEndOcclusionQueryNV")) == nullptr) || r;
  r = ((glGenOcclusionQueriesNV = (PFNGLGENOCCLUSIONQUERIESNVPROC)glewGetProcAddress((const GLubyte*)"glGenOcclusionQueriesNV")) == nullptr) || r;
  r = ((glGetOcclusionQueryivNV = (PFNGLGETOCCLUSIONQUERYIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetOcclusionQueryivNV")) == nullptr) || r;
  r = ((glGetOcclusionQueryuivNV = (PFNGLGETOCCLUSIONQUERYUIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetOcclusionQueryuivNV")) == nullptr) || r;
  r = ((glIsOcclusionQueryNV = (PFNGLISOCCLUSIONQUERYNVPROC)glewGetProcAddress((const GLubyte*)"glIsOcclusionQueryNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_occlusion_query */

#ifdef GL_NV_packed_depth_stencil

#endif /* GL_NV_packed_depth_stencil */

#ifdef GL_NV_parameter_buffer_object

static GLboolean _glewInit_GL_NV_parameter_buffer_object (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glProgramBufferParametersIivNV = (PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramBufferParametersIivNV")) == nullptr) || r;
  r = ((glProgramBufferParametersIuivNV = (PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramBufferParametersIuivNV")) == nullptr) || r;
  r = ((glProgramBufferParametersfvNV = (PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramBufferParametersfvNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_parameter_buffer_object */

#ifdef GL_NV_parameter_buffer_object2

#endif /* GL_NV_parameter_buffer_object2 */

#ifdef GL_NV_path_rendering

static GLboolean _glewInit_GL_NV_path_rendering (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyPathNV = (PFNGLCOPYPATHNVPROC)glewGetProcAddress((const GLubyte*)"glCopyPathNV")) == nullptr) || r;
  r = ((glCoverFillPathInstancedNV = (PFNGLCOVERFILLPATHINSTANCEDNVPROC)glewGetProcAddress((const GLubyte*)"glCoverFillPathInstancedNV")) == nullptr) || r;
  r = ((glCoverFillPathNV = (PFNGLCOVERFILLPATHNVPROC)glewGetProcAddress((const GLubyte*)"glCoverFillPathNV")) == nullptr) || r;
  r = ((glCoverStrokePathInstancedNV = (PFNGLCOVERSTROKEPATHINSTANCEDNVPROC)glewGetProcAddress((const GLubyte*)"glCoverStrokePathInstancedNV")) == nullptr) || r;
  r = ((glCoverStrokePathNV = (PFNGLCOVERSTROKEPATHNVPROC)glewGetProcAddress((const GLubyte*)"glCoverStrokePathNV")) == nullptr) || r;
  r = ((glDeletePathsNV = (PFNGLDELETEPATHSNVPROC)glewGetProcAddress((const GLubyte*)"glDeletePathsNV")) == nullptr) || r;
  r = ((glGenPathsNV = (PFNGLGENPATHSNVPROC)glewGetProcAddress((const GLubyte*)"glGenPathsNV")) == nullptr) || r;
  r = ((glGetPathColorGenfvNV = (PFNGLGETPATHCOLORGENFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathColorGenfvNV")) == nullptr) || r;
  r = ((glGetPathColorGenivNV = (PFNGLGETPATHCOLORGENIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathColorGenivNV")) == nullptr) || r;
  r = ((glGetPathCommandsNV = (PFNGLGETPATHCOMMANDSNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathCommandsNV")) == nullptr) || r;
  r = ((glGetPathCoordsNV = (PFNGLGETPATHCOORDSNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathCoordsNV")) == nullptr) || r;
  r = ((glGetPathDashArrayNV = (PFNGLGETPATHDASHARRAYNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathDashArrayNV")) == nullptr) || r;
  r = ((glGetPathLengthNV = (PFNGLGETPATHLENGTHNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathLengthNV")) == nullptr) || r;
  r = ((glGetPathMetricRangeNV = (PFNGLGETPATHMETRICRANGENVPROC)glewGetProcAddress((const GLubyte*)"glGetPathMetricRangeNV")) == nullptr) || r;
  r = ((glGetPathMetricsNV = (PFNGLGETPATHMETRICSNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathMetricsNV")) == nullptr) || r;
  r = ((glGetPathParameterfvNV = (PFNGLGETPATHPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathParameterfvNV")) == nullptr) || r;
  r = ((glGetPathParameterivNV = (PFNGLGETPATHPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathParameterivNV")) == nullptr) || r;
  r = ((glGetPathSpacingNV = (PFNGLGETPATHSPACINGNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathSpacingNV")) == nullptr) || r;
  r = ((glGetPathTexGenfvNV = (PFNGLGETPATHTEXGENFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathTexGenfvNV")) == nullptr) || r;
  r = ((glGetPathTexGenivNV = (PFNGLGETPATHTEXGENIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetPathTexGenivNV")) == nullptr) || r;
  r = ((glInterpolatePathsNV = (PFNGLINTERPOLATEPATHSNVPROC)glewGetProcAddress((const GLubyte*)"glInterpolatePathsNV")) == nullptr) || r;
  r = ((glIsPathNV = (PFNGLISPATHNVPROC)glewGetProcAddress((const GLubyte*)"glIsPathNV")) == nullptr) || r;
  r = ((glIsPointInFillPathNV = (PFNGLISPOINTINFILLPATHNVPROC)glewGetProcAddress((const GLubyte*)"glIsPointInFillPathNV")) == nullptr) || r;
  r = ((glIsPointInStrokePathNV = (PFNGLISPOINTINSTROKEPATHNVPROC)glewGetProcAddress((const GLubyte*)"glIsPointInStrokePathNV")) == nullptr) || r;
  r = ((glPathColorGenNV = (PFNGLPATHCOLORGENNVPROC)glewGetProcAddress((const GLubyte*)"glPathColorGenNV")) == nullptr) || r;
  r = ((glPathCommandsNV = (PFNGLPATHCOMMANDSNVPROC)glewGetProcAddress((const GLubyte*)"glPathCommandsNV")) == nullptr) || r;
  r = ((glPathCoordsNV = (PFNGLPATHCOORDSNVPROC)glewGetProcAddress((const GLubyte*)"glPathCoordsNV")) == nullptr) || r;
  r = ((glPathCoverDepthFuncNV = (PFNGLPATHCOVERDEPTHFUNCNVPROC)glewGetProcAddress((const GLubyte*)"glPathCoverDepthFuncNV")) == nullptr) || r;
  r = ((glPathDashArrayNV = (PFNGLPATHDASHARRAYNVPROC)glewGetProcAddress((const GLubyte*)"glPathDashArrayNV")) == nullptr) || r;
  r = ((glPathFogGenNV = (PFNGLPATHFOGGENNVPROC)glewGetProcAddress((const GLubyte*)"glPathFogGenNV")) == nullptr) || r;
  r = ((glPathGlyphRangeNV = (PFNGLPATHGLYPHRANGENVPROC)glewGetProcAddress((const GLubyte*)"glPathGlyphRangeNV")) == nullptr) || r;
  r = ((glPathGlyphsNV = (PFNGLPATHGLYPHSNVPROC)glewGetProcAddress((const GLubyte*)"glPathGlyphsNV")) == nullptr) || r;
  r = ((glPathParameterfNV = (PFNGLPATHPARAMETERFNVPROC)glewGetProcAddress((const GLubyte*)"glPathParameterfNV")) == nullptr) || r;
  r = ((glPathParameterfvNV = (PFNGLPATHPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glPathParameterfvNV")) == nullptr) || r;
  r = ((glPathParameteriNV = (PFNGLPATHPARAMETERINVPROC)glewGetProcAddress((const GLubyte*)"glPathParameteriNV")) == nullptr) || r;
  r = ((glPathParameterivNV = (PFNGLPATHPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glPathParameterivNV")) == nullptr) || r;
  r = ((glPathStencilDepthOffsetNV = (PFNGLPATHSTENCILDEPTHOFFSETNVPROC)glewGetProcAddress((const GLubyte*)"glPathStencilDepthOffsetNV")) == nullptr) || r;
  r = ((glPathStencilFuncNV = (PFNGLPATHSTENCILFUNCNVPROC)glewGetProcAddress((const GLubyte*)"glPathStencilFuncNV")) == nullptr) || r;
  r = ((glPathStringNV = (PFNGLPATHSTRINGNVPROC)glewGetProcAddress((const GLubyte*)"glPathStringNV")) == nullptr) || r;
  r = ((glPathSubCommandsNV = (PFNGLPATHSUBCOMMANDSNVPROC)glewGetProcAddress((const GLubyte*)"glPathSubCommandsNV")) == nullptr) || r;
  r = ((glPathSubCoordsNV = (PFNGLPATHSUBCOORDSNVPROC)glewGetProcAddress((const GLubyte*)"glPathSubCoordsNV")) == nullptr) || r;
  r = ((glPathTexGenNV = (PFNGLPATHTEXGENNVPROC)glewGetProcAddress((const GLubyte*)"glPathTexGenNV")) == nullptr) || r;
  r = ((glPointAlongPathNV = (PFNGLPOINTALONGPATHNVPROC)glewGetProcAddress((const GLubyte*)"glPointAlongPathNV")) == nullptr) || r;
  r = ((glStencilFillPathInstancedNV = (PFNGLSTENCILFILLPATHINSTANCEDNVPROC)glewGetProcAddress((const GLubyte*)"glStencilFillPathInstancedNV")) == nullptr) || r;
  r = ((glStencilFillPathNV = (PFNGLSTENCILFILLPATHNVPROC)glewGetProcAddress((const GLubyte*)"glStencilFillPathNV")) == nullptr) || r;
  r = ((glStencilStrokePathInstancedNV = (PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC)glewGetProcAddress((const GLubyte*)"glStencilStrokePathInstancedNV")) == nullptr) || r;
  r = ((glStencilStrokePathNV = (PFNGLSTENCILSTROKEPATHNVPROC)glewGetProcAddress((const GLubyte*)"glStencilStrokePathNV")) == nullptr) || r;
  r = ((glTransformPathNV = (PFNGLTRANSFORMPATHNVPROC)glewGetProcAddress((const GLubyte*)"glTransformPathNV")) == nullptr) || r;
  r = ((glWeightPathsNV = (PFNGLWEIGHTPATHSNVPROC)glewGetProcAddress((const GLubyte*)"glWeightPathsNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_path_rendering */

#ifdef GL_NV_pixel_data_range

static GLboolean _glewInit_GL_NV_pixel_data_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushPixelDataRangeNV = (PFNGLFLUSHPIXELDATARANGENVPROC)glewGetProcAddress((const GLubyte*)"glFlushPixelDataRangeNV")) == nullptr) || r;
  r = ((glPixelDataRangeNV = (PFNGLPIXELDATARANGENVPROC)glewGetProcAddress((const GLubyte*)"glPixelDataRangeNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_pixel_data_range */

#ifdef GL_NV_point_sprite

static GLboolean _glewInit_GL_NV_point_sprite (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPointParameteriNV = (PFNGLPOINTPARAMETERINVPROC)glewGetProcAddress((const GLubyte*)"glPointParameteriNV")) == nullptr) || r;
  r = ((glPointParameterivNV = (PFNGLPOINTPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glPointParameterivNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_point_sprite */

#ifdef GL_NV_present_video

static GLboolean _glewInit_GL_NV_present_video (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVideoi64vNV = (PFNGLGETVIDEOI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoi64vNV")) == nullptr) || r;
  r = ((glGetVideoivNV = (PFNGLGETVIDEOIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoivNV")) == nullptr) || r;
  r = ((glGetVideoui64vNV = (PFNGLGETVIDEOUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoui64vNV")) == nullptr) || r;
  r = ((glGetVideouivNV = (PFNGLGETVIDEOUIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideouivNV")) == nullptr) || r;
  r = ((glPresentFrameDualFillNV = (PFNGLPRESENTFRAMEDUALFILLNVPROC)glewGetProcAddress((const GLubyte*)"glPresentFrameDualFillNV")) == nullptr) || r;
  r = ((glPresentFrameKeyedNV = (PFNGLPRESENTFRAMEKEYEDNVPROC)glewGetProcAddress((const GLubyte*)"glPresentFrameKeyedNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_present_video */

#ifdef GL_NV_primitive_restart

static GLboolean _glewInit_GL_NV_primitive_restart (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPrimitiveRestartIndexNV = (PFNGLPRIMITIVERESTARTINDEXNVPROC)glewGetProcAddress((const GLubyte*)"glPrimitiveRestartIndexNV")) == nullptr) || r;
  r = ((glPrimitiveRestartNV = (PFNGLPRIMITIVERESTARTNVPROC)glewGetProcAddress((const GLubyte*)"glPrimitiveRestartNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_primitive_restart */

#ifdef GL_NV_register_combiners

static GLboolean _glewInit_GL_NV_register_combiners (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCombinerInputNV = (PFNGLCOMBINERINPUTNVPROC)glewGetProcAddress((const GLubyte*)"glCombinerInputNV")) == nullptr) || r;
  r = ((glCombinerOutputNV = (PFNGLCOMBINEROUTPUTNVPROC)glewGetProcAddress((const GLubyte*)"glCombinerOutputNV")) == nullptr) || r;
  r = ((glCombinerParameterfNV = (PFNGLCOMBINERPARAMETERFNVPROC)glewGetProcAddress((const GLubyte*)"glCombinerParameterfNV")) == nullptr) || r;
  r = ((glCombinerParameterfvNV = (PFNGLCOMBINERPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glCombinerParameterfvNV")) == nullptr) || r;
  r = ((glCombinerParameteriNV = (PFNGLCOMBINERPARAMETERINVPROC)glewGetProcAddress((const GLubyte*)"glCombinerParameteriNV")) == nullptr) || r;
  r = ((glCombinerParameterivNV = (PFNGLCOMBINERPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glCombinerParameterivNV")) == nullptr) || r;
  r = ((glFinalCombinerInputNV = (PFNGLFINALCOMBINERINPUTNVPROC)glewGetProcAddress((const GLubyte*)"glFinalCombinerInputNV")) == nullptr) || r;
  r = ((glGetCombinerInputParameterfvNV = (PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetCombinerInputParameterfvNV")) == nullptr) || r;
  r = ((glGetCombinerInputParameterivNV = (PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetCombinerInputParameterivNV")) == nullptr) || r;
  r = ((glGetCombinerOutputParameterfvNV = (PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetCombinerOutputParameterfvNV")) == nullptr) || r;
  r = ((glGetCombinerOutputParameterivNV = (PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetCombinerOutputParameterivNV")) == nullptr) || r;
  r = ((glGetFinalCombinerInputParameterfvNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetFinalCombinerInputParameterfvNV")) == nullptr) || r;
  r = ((glGetFinalCombinerInputParameterivNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetFinalCombinerInputParameterivNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_register_combiners */

#ifdef GL_NV_register_combiners2

static GLboolean _glewInit_GL_NV_register_combiners2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glCombinerStageParameterfvNV = (PFNGLCOMBINERSTAGEPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glCombinerStageParameterfvNV")) == nullptr) || r;
  r = ((glGetCombinerStageParameterfvNV = (PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetCombinerStageParameterfvNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_register_combiners2 */

#ifdef GL_NV_shader_buffer_load

static GLboolean _glewInit_GL_NV_shader_buffer_load (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetBufferParameterui64vNV = (PFNGLGETBUFFERPARAMETERUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetBufferParameterui64vNV")) == nullptr) || r;
  r = ((glGetIntegerui64vNV = (PFNGLGETINTEGERUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetIntegerui64vNV")) == nullptr) || r;
  r = ((glGetNamedBufferParameterui64vNV = (PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferParameterui64vNV")) == nullptr) || r;
  r = ((glIsBufferResidentNV = (PFNGLISBUFFERRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glIsBufferResidentNV")) == nullptr) || r;
  r = ((glIsNamedBufferResidentNV = (PFNGLISNAMEDBUFFERRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glIsNamedBufferResidentNV")) == nullptr) || r;
  r = ((glMakeBufferNonResidentNV = (PFNGLMAKEBUFFERNONRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glMakeBufferNonResidentNV")) == nullptr) || r;
  r = ((glMakeBufferResidentNV = (PFNGLMAKEBUFFERRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glMakeBufferResidentNV")) == nullptr) || r;
  r = ((glMakeNamedBufferNonResidentNV = (PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glMakeNamedBufferNonResidentNV")) == nullptr) || r;
  r = ((glMakeNamedBufferResidentNV = (PFNGLMAKENAMEDBUFFERRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glMakeNamedBufferResidentNV")) == nullptr) || r;
  r = ((glProgramUniformui64NV = (PFNGLPROGRAMUNIFORMUI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformui64NV")) == nullptr) || r;
  r = ((glProgramUniformui64vNV = (PFNGLPROGRAMUNIFORMUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformui64vNV")) == nullptr) || r;
  r = ((glUniformui64NV = (PFNGLUNIFORMUI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniformui64NV")) == nullptr) || r;
  r = ((glUniformui64vNV = (PFNGLUNIFORMUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniformui64vNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_shader_buffer_load */

#ifdef GL_NV_tessellation_program5

#endif /* GL_NV_tessellation_program5 */

#ifdef GL_NV_texgen_emboss

#endif /* GL_NV_texgen_emboss */

#ifdef GL_NV_texgen_reflection

#endif /* GL_NV_texgen_reflection */

#ifdef GL_NV_texture_barrier

static GLboolean _glewInit_GL_NV_texture_barrier (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTextureBarrierNV = (PFNGLTEXTUREBARRIERNVPROC)glewGetProcAddress((const GLubyte*)"glTextureBarrierNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_texture_barrier */

#ifdef GL_NV_texture_compression_vtc

#endif /* GL_NV_texture_compression_vtc */

#ifdef GL_NV_texture_env_combine4

#endif /* GL_NV_texture_env_combine4 */

#ifdef GL_NV_texture_expand_normal

#endif /* GL_NV_texture_expand_normal */

#ifdef GL_NV_texture_multisample

static GLboolean _glewInit_GL_NV_texture_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexImage2DMultisampleCoverageNV = (PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC)glewGetProcAddress((const GLubyte*)"glTexImage2DMultisampleCoverageNV")) == nullptr) || r;
  r = ((glTexImage3DMultisampleCoverageNV = (PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC)glewGetProcAddress((const GLubyte*)"glTexImage3DMultisampleCoverageNV")) == nullptr) || r;
  r = ((glTextureImage2DMultisampleCoverageNV = (PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC)glewGetProcAddress((const GLubyte*)"glTextureImage2DMultisampleCoverageNV")) == nullptr) || r;
  r = ((glTextureImage2DMultisampleNV = (PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC)glewGetProcAddress((const GLubyte*)"glTextureImage2DMultisampleNV")) == nullptr) || r;
  r = ((glTextureImage3DMultisampleCoverageNV = (PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC)glewGetProcAddress((const GLubyte*)"glTextureImage3DMultisampleCoverageNV")) == nullptr) || r;
  r = ((glTextureImage3DMultisampleNV = (PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC)glewGetProcAddress((const GLubyte*)"glTextureImage3DMultisampleNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_texture_multisample */

#ifdef GL_NV_texture_rectangle

#endif /* GL_NV_texture_rectangle */

#ifdef GL_NV_texture_shader

#endif /* GL_NV_texture_shader */

#ifdef GL_NV_texture_shader2

#endif /* GL_NV_texture_shader2 */

#ifdef GL_NV_texture_shader3

#endif /* GL_NV_texture_shader3 */

#ifdef GL_NV_transform_feedback

static GLboolean _glewInit_GL_NV_transform_feedback (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glActiveVaryingNV = (PFNGLACTIVEVARYINGNVPROC)glewGetProcAddress((const GLubyte*)"glActiveVaryingNV")) == nullptr) || r;
  r = ((glBeginTransformFeedbackNV = (PFNGLBEGINTRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glBeginTransformFeedbackNV")) == nullptr) || r;
  r = ((glBindBufferBaseNV = (PFNGLBINDBUFFERBASENVPROC)glewGetProcAddress((const GLubyte*)"glBindBufferBaseNV")) == nullptr) || r;
  r = ((glBindBufferOffsetNV = (PFNGLBINDBUFFEROFFSETNVPROC)glewGetProcAddress((const GLubyte*)"glBindBufferOffsetNV")) == nullptr) || r;
  r = ((glBindBufferRangeNV = (PFNGLBINDBUFFERRANGENVPROC)glewGetProcAddress((const GLubyte*)"glBindBufferRangeNV")) == nullptr) || r;
  r = ((glEndTransformFeedbackNV = (PFNGLENDTRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glEndTransformFeedbackNV")) == nullptr) || r;
  r = ((glGetActiveVaryingNV = (PFNGLGETACTIVEVARYINGNVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveVaryingNV")) == nullptr) || r;
  r = ((glGetTransformFeedbackVaryingNV = (PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbackVaryingNV")) == nullptr) || r;
  r = ((glGetVaryingLocationNV = (PFNGLGETVARYINGLOCATIONNVPROC)glewGetProcAddress((const GLubyte*)"glGetVaryingLocationNV")) == nullptr) || r;
  r = ((glTransformFeedbackAttribsNV = (PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackAttribsNV")) == nullptr) || r;
  r = ((glTransformFeedbackVaryingsNV = (PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackVaryingsNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_transform_feedback */

#ifdef GL_NV_transform_feedback2

static GLboolean _glewInit_GL_NV_transform_feedback2 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBindTransformFeedbackNV = (PFNGLBINDTRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glBindTransformFeedbackNV")) == nullptr) || r;
  r = ((glDeleteTransformFeedbacksNV = (PFNGLDELETETRANSFORMFEEDBACKSNVPROC)glewGetProcAddress((const GLubyte*)"glDeleteTransformFeedbacksNV")) == nullptr) || r;
  r = ((glDrawTransformFeedbackNV = (PFNGLDRAWTRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glDrawTransformFeedbackNV")) == nullptr) || r;
  r = ((glGenTransformFeedbacksNV = (PFNGLGENTRANSFORMFEEDBACKSNVPROC)glewGetProcAddress((const GLubyte*)"glGenTransformFeedbacksNV")) == nullptr) || r;
  r = ((glIsTransformFeedbackNV = (PFNGLISTRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glIsTransformFeedbackNV")) == nullptr) || r;
  r = ((glPauseTransformFeedbackNV = (PFNGLPAUSETRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glPauseTransformFeedbackNV")) == nullptr) || r;
  r = ((glResumeTransformFeedbackNV = (PFNGLRESUMETRANSFORMFEEDBACKNVPROC)glewGetProcAddress((const GLubyte*)"glResumeTransformFeedbackNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_transform_feedback2 */

#ifdef GL_NV_vdpau_interop

static GLboolean _glewInit_GL_NV_vdpau_interop (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glVDPAUFiniNV = (PFNGLVDPAUFININVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUFiniNV")) == nullptr) || r;
  r = ((glVDPAUGetSurfaceivNV = (PFNGLVDPAUGETSURFACEIVNVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUGetSurfaceivNV")) == nullptr) || r;
  r = ((glVDPAUInitNV = (PFNGLVDPAUINITNVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUInitNV")) == nullptr) || r;
  r = ((glVDPAUIsSurfaceNV = (PFNGLVDPAUISSURFACENVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUIsSurfaceNV")) == nullptr) || r;
  r = ((glVDPAUMapSurfacesNV = (PFNGLVDPAUMAPSURFACESNVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUMapSurfacesNV")) == nullptr) || r;
  r = ((glVDPAURegisterOutputSurfaceNV = (PFNGLVDPAUREGISTEROUTPUTSURFACENVPROC)glewGetProcAddress((const GLubyte*)"glVDPAURegisterOutputSurfaceNV")) == nullptr) || r;
  r = ((glVDPAURegisterVideoSurfaceNV = (PFNGLVDPAUREGISTERVIDEOSURFACENVPROC)glewGetProcAddress((const GLubyte*)"glVDPAURegisterVideoSurfaceNV")) == nullptr) || r;
  r = ((glVDPAUSurfaceAccessNV = (PFNGLVDPAUSURFACEACCESSNVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUSurfaceAccessNV")) == nullptr) || r;
  r = ((glVDPAUUnmapSurfacesNV = (PFNGLVDPAUUNMAPSURFACESNVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUUnmapSurfacesNV")) == nullptr) || r;
  r = ((glVDPAUUnregisterSurfaceNV = (PFNGLVDPAUUNREGISTERSURFACENVPROC)glewGetProcAddress((const GLubyte*)"glVDPAUUnregisterSurfaceNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_vdpau_interop */

#ifdef GL_NV_vertex_array_range

static GLboolean _glewInit_GL_NV_vertex_array_range (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushVertexArrayRangeNV = (PFNGLFLUSHVERTEXARRAYRANGENVPROC)glewGetProcAddress((const GLubyte*)"glFlushVertexArrayRangeNV")) == nullptr) || r;
  r = ((glVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayRangeNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_vertex_array_range */

#ifdef GL_NV_vertex_array_range2

#endif /* GL_NV_vertex_array_range2 */

#ifdef GL_NV_vertex_attrib_integer_64bit

static GLboolean _glewInit_GL_NV_vertex_attrib_integer_64bit (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetVertexAttribLi64vNV = (PFNGLGETVERTEXATTRIBLI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribLi64vNV")) == nullptr) || r;
  r = ((glGetVertexAttribLui64vNV = (PFNGLGETVERTEXATTRIBLUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribLui64vNV")) == nullptr) || r;
  r = ((glVertexAttribL1i64NV = (PFNGLVERTEXATTRIBL1I64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1i64NV")) == nullptr) || r;
  r = ((glVertexAttribL1i64vNV = (PFNGLVERTEXATTRIBL1I64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1i64vNV")) == nullptr) || r;
  r = ((glVertexAttribL1ui64NV = (PFNGLVERTEXATTRIBL1UI64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1ui64NV")) == nullptr) || r;
  r = ((glVertexAttribL1ui64vNV = (PFNGLVERTEXATTRIBL1UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL1ui64vNV")) == nullptr) || r;
  r = ((glVertexAttribL2i64NV = (PFNGLVERTEXATTRIBL2I64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2i64NV")) == nullptr) || r;
  r = ((glVertexAttribL2i64vNV = (PFNGLVERTEXATTRIBL2I64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2i64vNV")) == nullptr) || r;
  r = ((glVertexAttribL2ui64NV = (PFNGLVERTEXATTRIBL2UI64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2ui64NV")) == nullptr) || r;
  r = ((glVertexAttribL2ui64vNV = (PFNGLVERTEXATTRIBL2UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL2ui64vNV")) == nullptr) || r;
  r = ((glVertexAttribL3i64NV = (PFNGLVERTEXATTRIBL3I64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3i64NV")) == nullptr) || r;
  r = ((glVertexAttribL3i64vNV = (PFNGLVERTEXATTRIBL3I64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3i64vNV")) == nullptr) || r;
  r = ((glVertexAttribL3ui64NV = (PFNGLVERTEXATTRIBL3UI64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3ui64NV")) == nullptr) || r;
  r = ((glVertexAttribL3ui64vNV = (PFNGLVERTEXATTRIBL3UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL3ui64vNV")) == nullptr) || r;
  r = ((glVertexAttribL4i64NV = (PFNGLVERTEXATTRIBL4I64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4i64NV")) == nullptr) || r;
  r = ((glVertexAttribL4i64vNV = (PFNGLVERTEXATTRIBL4I64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4i64vNV")) == nullptr) || r;
  r = ((glVertexAttribL4ui64NV = (PFNGLVERTEXATTRIBL4UI64NVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4ui64NV")) == nullptr) || r;
  r = ((glVertexAttribL4ui64vNV = (PFNGLVERTEXATTRIBL4UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribL4ui64vNV")) == nullptr) || r;
  r = ((glVertexAttribLFormatNV = (PFNGLVERTEXATTRIBLFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribLFormatNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_vertex_attrib_integer_64bit */

#ifdef GL_NV_vertex_buffer_unified_memory

static GLboolean _glewInit_GL_NV_vertex_buffer_unified_memory (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBufferAddressRangeNV = (PFNGLBUFFERADDRESSRANGENVPROC)glewGetProcAddress((const GLubyte*)"glBufferAddressRangeNV")) == nullptr) || r;
  r = ((glColorFormatNV = (PFNGLCOLORFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glColorFormatNV")) == nullptr) || r;
  r = ((glEdgeFlagFormatNV = (PFNGLEDGEFLAGFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glEdgeFlagFormatNV")) == nullptr) || r;
  r = ((glFogCoordFormatNV = (PFNGLFOGCOORDFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glFogCoordFormatNV")) == nullptr) || r;
  r = ((glGetIntegerui64i_vNV = (PFNGLGETINTEGERUI64I_VNVPROC)glewGetProcAddress((const GLubyte*)"glGetIntegerui64i_vNV")) == nullptr) || r;
  r = ((glIndexFormatNV = (PFNGLINDEXFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glIndexFormatNV")) == nullptr) || r;
  r = ((glNormalFormatNV = (PFNGLNORMALFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glNormalFormatNV")) == nullptr) || r;
  r = ((glSecondaryColorFormatNV = (PFNGLSECONDARYCOLORFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorFormatNV")) == nullptr) || r;
  r = ((glTexCoordFormatNV = (PFNGLTEXCOORDFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordFormatNV")) == nullptr) || r;
  r = ((glVertexAttribFormatNV = (PFNGLVERTEXATTRIBFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribFormatNV")) == nullptr) || r;
  r = ((glVertexAttribIFormatNV = (PFNGLVERTEXATTRIBIFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribIFormatNV")) == nullptr) || r;
  r = ((glVertexFormatNV = (PFNGLVERTEXFORMATNVPROC)glewGetProcAddress((const GLubyte*)"glVertexFormatNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_vertex_buffer_unified_memory */

#ifdef GL_NV_vertex_program

static GLboolean _glewInit_GL_NV_vertex_program (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAreProgramsResidentNV = (PFNGLAREPROGRAMSRESIDENTNVPROC)glewGetProcAddress((const GLubyte*)"glAreProgramsResidentNV")) == nullptr) || r;
  r = ((glBindProgramNV = (PFNGLBINDPROGRAMNVPROC)glewGetProcAddress((const GLubyte*)"glBindProgramNV")) == nullptr) || r;
  r = ((glDeleteProgramsNV = (PFNGLDELETEPROGRAMSNVPROC)glewGetProcAddress((const GLubyte*)"glDeleteProgramsNV")) == nullptr) || r;
  r = ((glExecuteProgramNV = (PFNGLEXECUTEPROGRAMNVPROC)glewGetProcAddress((const GLubyte*)"glExecuteProgramNV")) == nullptr) || r;
  r = ((glGenProgramsNV = (PFNGLGENPROGRAMSNVPROC)glewGetProcAddress((const GLubyte*)"glGenProgramsNV")) == nullptr) || r;
  r = ((glGetProgramParameterdvNV = (PFNGLGETPROGRAMPARAMETERDVNVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramParameterdvNV")) == nullptr) || r;
  r = ((glGetProgramParameterfvNV = (PFNGLGETPROGRAMPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramParameterfvNV")) == nullptr) || r;
  r = ((glGetProgramStringNV = (PFNGLGETPROGRAMSTRINGNVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramStringNV")) == nullptr) || r;
  r = ((glGetProgramivNV = (PFNGLGETPROGRAMIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramivNV")) == nullptr) || r;
  r = ((glGetTrackMatrixivNV = (PFNGLGETTRACKMATRIXIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetTrackMatrixivNV")) == nullptr) || r;
  r = ((glGetVertexAttribPointervNV = (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribPointervNV")) == nullptr) || r;
  r = ((glGetVertexAttribdvNV = (PFNGLGETVERTEXATTRIBDVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribdvNV")) == nullptr) || r;
  r = ((glGetVertexAttribfvNV = (PFNGLGETVERTEXATTRIBFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribfvNV")) == nullptr) || r;
  r = ((glGetVertexAttribivNV = (PFNGLGETVERTEXATTRIBIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribivNV")) == nullptr) || r;
  r = ((glIsProgramNV = (PFNGLISPROGRAMNVPROC)glewGetProcAddress((const GLubyte*)"glIsProgramNV")) == nullptr) || r;
  r = ((glLoadProgramNV = (PFNGLLOADPROGRAMNVPROC)glewGetProcAddress((const GLubyte*)"glLoadProgramNV")) == nullptr) || r;
  r = ((glProgramParameter4dNV = (PFNGLPROGRAMPARAMETER4DNVPROC)glewGetProcAddress((const GLubyte*)"glProgramParameter4dNV")) == nullptr) || r;
  r = ((glProgramParameter4dvNV = (PFNGLPROGRAMPARAMETER4DVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramParameter4dvNV")) == nullptr) || r;
  r = ((glProgramParameter4fNV = (PFNGLPROGRAMPARAMETER4FNVPROC)glewGetProcAddress((const GLubyte*)"glProgramParameter4fNV")) == nullptr) || r;
  r = ((glProgramParameter4fvNV = (PFNGLPROGRAMPARAMETER4FVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramParameter4fvNV")) == nullptr) || r;
  r = ((glProgramParameters4dvNV = (PFNGLPROGRAMPARAMETERS4DVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramParameters4dvNV")) == nullptr) || r;
  r = ((glProgramParameters4fvNV = (PFNGLPROGRAMPARAMETERS4FVNVPROC)glewGetProcAddress((const GLubyte*)"glProgramParameters4fvNV")) == nullptr) || r;
  r = ((glRequestResidentProgramsNV = (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)glewGetProcAddress((const GLubyte*)"glRequestResidentProgramsNV")) == nullptr) || r;
  r = ((glTrackMatrixNV = (PFNGLTRACKMATRIXNVPROC)glewGetProcAddress((const GLubyte*)"glTrackMatrixNV")) == nullptr) || r;
  r = ((glVertexAttrib1dNV = (PFNGLVERTEXATTRIB1DNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1dNV")) == nullptr) || r;
  r = ((glVertexAttrib1dvNV = (PFNGLVERTEXATTRIB1DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1dvNV")) == nullptr) || r;
  r = ((glVertexAttrib1fNV = (PFNGLVERTEXATTRIB1FNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1fNV")) == nullptr) || r;
  r = ((glVertexAttrib1fvNV = (PFNGLVERTEXATTRIB1FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1fvNV")) == nullptr) || r;
  r = ((glVertexAttrib1sNV = (PFNGLVERTEXATTRIB1SNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1sNV")) == nullptr) || r;
  r = ((glVertexAttrib1svNV = (PFNGLVERTEXATTRIB1SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1svNV")) == nullptr) || r;
  r = ((glVertexAttrib2dNV = (PFNGLVERTEXATTRIB2DNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2dNV")) == nullptr) || r;
  r = ((glVertexAttrib2dvNV = (PFNGLVERTEXATTRIB2DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2dvNV")) == nullptr) || r;
  r = ((glVertexAttrib2fNV = (PFNGLVERTEXATTRIB2FNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2fNV")) == nullptr) || r;
  r = ((glVertexAttrib2fvNV = (PFNGLVERTEXATTRIB2FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2fvNV")) == nullptr) || r;
  r = ((glVertexAttrib2sNV = (PFNGLVERTEXATTRIB2SNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2sNV")) == nullptr) || r;
  r = ((glVertexAttrib2svNV = (PFNGLVERTEXATTRIB2SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2svNV")) == nullptr) || r;
  r = ((glVertexAttrib3dNV = (PFNGLVERTEXATTRIB3DNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3dNV")) == nullptr) || r;
  r = ((glVertexAttrib3dvNV = (PFNGLVERTEXATTRIB3DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3dvNV")) == nullptr) || r;
  r = ((glVertexAttrib3fNV = (PFNGLVERTEXATTRIB3FNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3fNV")) == nullptr) || r;
  r = ((glVertexAttrib3fvNV = (PFNGLVERTEXATTRIB3FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3fvNV")) == nullptr) || r;
  r = ((glVertexAttrib3sNV = (PFNGLVERTEXATTRIB3SNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3sNV")) == nullptr) || r;
  r = ((glVertexAttrib3svNV = (PFNGLVERTEXATTRIB3SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3svNV")) == nullptr) || r;
  r = ((glVertexAttrib4dNV = (PFNGLVERTEXATTRIB4DNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4dNV")) == nullptr) || r;
  r = ((glVertexAttrib4dvNV = (PFNGLVERTEXATTRIB4DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4dvNV")) == nullptr) || r;
  r = ((glVertexAttrib4fNV = (PFNGLVERTEXATTRIB4FNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4fNV")) == nullptr) || r;
  r = ((glVertexAttrib4fvNV = (PFNGLVERTEXATTRIB4FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4fvNV")) == nullptr) || r;
  r = ((glVertexAttrib4sNV = (PFNGLVERTEXATTRIB4SNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4sNV")) == nullptr) || r;
  r = ((glVertexAttrib4svNV = (PFNGLVERTEXATTRIB4SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4svNV")) == nullptr) || r;
  r = ((glVertexAttrib4ubNV = (PFNGLVERTEXATTRIB4UBNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4ubNV")) == nullptr) || r;
  r = ((glVertexAttrib4ubvNV = (PFNGLVERTEXATTRIB4UBVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4ubvNV")) == nullptr) || r;
  r = ((glVertexAttribPointerNV = (PFNGLVERTEXATTRIBPOINTERNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribPointerNV")) == nullptr) || r;
  r = ((glVertexAttribs1dvNV = (PFNGLVERTEXATTRIBS1DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs1dvNV")) == nullptr) || r;
  r = ((glVertexAttribs1fvNV = (PFNGLVERTEXATTRIBS1FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs1fvNV")) == nullptr) || r;
  r = ((glVertexAttribs1svNV = (PFNGLVERTEXATTRIBS1SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs1svNV")) == nullptr) || r;
  r = ((glVertexAttribs2dvNV = (PFNGLVERTEXATTRIBS2DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs2dvNV")) == nullptr) || r;
  r = ((glVertexAttribs2fvNV = (PFNGLVERTEXATTRIBS2FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs2fvNV")) == nullptr) || r;
  r = ((glVertexAttribs2svNV = (PFNGLVERTEXATTRIBS2SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs2svNV")) == nullptr) || r;
  r = ((glVertexAttribs3dvNV = (PFNGLVERTEXATTRIBS3DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs3dvNV")) == nullptr) || r;
  r = ((glVertexAttribs3fvNV = (PFNGLVERTEXATTRIBS3FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs3fvNV")) == nullptr) || r;
  r = ((glVertexAttribs3svNV = (PFNGLVERTEXATTRIBS3SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs3svNV")) == nullptr) || r;
  r = ((glVertexAttribs4dvNV = (PFNGLVERTEXATTRIBS4DVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs4dvNV")) == nullptr) || r;
  r = ((glVertexAttribs4fvNV = (PFNGLVERTEXATTRIBS4FVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs4fvNV")) == nullptr) || r;
  r = ((glVertexAttribs4svNV = (PFNGLVERTEXATTRIBS4SVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs4svNV")) == nullptr) || r;
  r = ((glVertexAttribs4ubvNV = (PFNGLVERTEXATTRIBS4UBVNVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribs4ubvNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_vertex_program */

#ifdef GL_NV_vertex_program1_1

#endif /* GL_NV_vertex_program1_1 */

#ifdef GL_NV_vertex_program2

#endif /* GL_NV_vertex_program2 */

#ifdef GL_NV_vertex_program2_option

#endif /* GL_NV_vertex_program2_option */

#ifdef GL_NV_vertex_program3

#endif /* GL_NV_vertex_program3 */

#ifdef GL_NV_vertex_program4

#endif /* GL_NV_vertex_program4 */

#ifdef GL_NV_video_capture

static GLboolean _glewInit_GL_NV_video_capture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glBeginVideoCaptureNV = (PFNGLBEGINVIDEOCAPTURENVPROC)glewGetProcAddress((const GLubyte*)"glBeginVideoCaptureNV")) == nullptr) || r;
  r = ((glBindVideoCaptureStreamBufferNV = (PFNGLBINDVIDEOCAPTURESTREAMBUFFERNVPROC)glewGetProcAddress((const GLubyte*)"glBindVideoCaptureStreamBufferNV")) == nullptr) || r;
  r = ((glBindVideoCaptureStreamTextureNV = (PFNGLBINDVIDEOCAPTURESTREAMTEXTURENVPROC)glewGetProcAddress((const GLubyte*)"glBindVideoCaptureStreamTextureNV")) == nullptr) || r;
  r = ((glEndVideoCaptureNV = (PFNGLENDVIDEOCAPTURENVPROC)glewGetProcAddress((const GLubyte*)"glEndVideoCaptureNV")) == nullptr) || r;
  r = ((glGetVideoCaptureStreamdvNV = (PFNGLGETVIDEOCAPTURESTREAMDVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoCaptureStreamdvNV")) == nullptr) || r;
  r = ((glGetVideoCaptureStreamfvNV = (PFNGLGETVIDEOCAPTURESTREAMFVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoCaptureStreamfvNV")) == nullptr) || r;
  r = ((glGetVideoCaptureStreamivNV = (PFNGLGETVIDEOCAPTURESTREAMIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoCaptureStreamivNV")) == nullptr) || r;
  r = ((glGetVideoCaptureivNV = (PFNGLGETVIDEOCAPTUREIVNVPROC)glewGetProcAddress((const GLubyte*)"glGetVideoCaptureivNV")) == nullptr) || r;
  r = ((glVideoCaptureNV = (PFNGLVIDEOCAPTURENVPROC)glewGetProcAddress((const GLubyte*)"glVideoCaptureNV")) == nullptr) || r;
  r = ((glVideoCaptureStreamParameterdvNV = (PFNGLVIDEOCAPTURESTREAMPARAMETERDVNVPROC)glewGetProcAddress((const GLubyte*)"glVideoCaptureStreamParameterdvNV")) == nullptr) || r;
  r = ((glVideoCaptureStreamParameterfvNV = (PFNGLVIDEOCAPTURESTREAMPARAMETERFVNVPROC)glewGetProcAddress((const GLubyte*)"glVideoCaptureStreamParameterfvNV")) == nullptr) || r;
  r = ((glVideoCaptureStreamParameterivNV = (PFNGLVIDEOCAPTURESTREAMPARAMETERIVNVPROC)glewGetProcAddress((const GLubyte*)"glVideoCaptureStreamParameterivNV")) == nullptr) || r;

  return r;
}

#endif /* GL_NV_video_capture */

#ifdef GL_OES_byte_coordinates

#endif /* GL_OES_byte_coordinates */

#ifdef GL_OES_compressed_paletted_texture

#endif /* GL_OES_compressed_paletted_texture */

#ifdef GL_OES_read_format

#endif /* GL_OES_read_format */

#ifdef GL_OES_single_precision

static GLboolean _glewInit_GL_OES_single_precision (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glClearDepthfOES = (PFNGLCLEARDEPTHFOESPROC)glewGetProcAddress((const GLubyte*)"glClearDepthfOES")) == nullptr) || r;
  r = ((glClipPlanefOES = (PFNGLCLIPPLANEFOESPROC)glewGetProcAddress((const GLubyte*)"glClipPlanefOES")) == nullptr) || r;
  r = ((glDepthRangefOES = (PFNGLDEPTHRANGEFOESPROC)glewGetProcAddress((const GLubyte*)"glDepthRangefOES")) == nullptr) || r;
  r = ((glFrustumfOES = (PFNGLFRUSTUMFOESPROC)glewGetProcAddress((const GLubyte*)"glFrustumfOES")) == nullptr) || r;
  r = ((glGetClipPlanefOES = (PFNGLGETCLIPPLANEFOESPROC)glewGetProcAddress((const GLubyte*)"glGetClipPlanefOES")) == nullptr) || r;
  r = ((glOrthofOES = (PFNGLORTHOFOESPROC)glewGetProcAddress((const GLubyte*)"glOrthofOES")) == nullptr) || r;

  return r;
}

#endif /* GL_OES_single_precision */

#ifdef GL_OML_interlace

#endif /* GL_OML_interlace */

#ifdef GL_OML_resample

#endif /* GL_OML_resample */

#ifdef GL_OML_subsample

#endif /* GL_OML_subsample */

#ifdef GL_PGI_misc_hints

#endif /* GL_PGI_misc_hints */

#ifdef GL_PGI_vertex_hints

#endif /* GL_PGI_vertex_hints */

#ifdef GL_REND_screen_coordinates

#endif /* GL_REND_screen_coordinates */

#ifdef GL_S3_s3tc

#endif /* GL_S3_s3tc */

#ifdef GL_SGIS_color_range

#endif /* GL_SGIS_color_range */

#ifdef GL_SGIS_detail_texture

static GLboolean _glewInit_GL_SGIS_detail_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glDetailTexFuncSGIS = (PFNGLDETAILTEXFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glDetailTexFuncSGIS")) == nullptr) || r;
  r = ((glGetDetailTexFuncSGIS = (PFNGLGETDETAILTEXFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glGetDetailTexFuncSGIS")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIS_detail_texture */

#ifdef GL_SGIS_fog_function

static GLboolean _glewInit_GL_SGIS_fog_function (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFogFuncSGIS = (PFNGLFOGFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glFogFuncSGIS")) == nullptr) || r;
  r = ((glGetFogFuncSGIS = (PFNGLGETFOGFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glGetFogFuncSGIS")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIS_fog_function */

#ifdef GL_SGIS_generate_mipmap

#endif /* GL_SGIS_generate_mipmap */

#ifdef GL_SGIS_multisample

static GLboolean _glewInit_GL_SGIS_multisample (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSampleMaskSGIS = (PFNGLSAMPLEMASKSGISPROC)glewGetProcAddress((const GLubyte*)"glSampleMaskSGIS")) == nullptr) || r;
  r = ((glSamplePatternSGIS = (PFNGLSAMPLEPATTERNSGISPROC)glewGetProcAddress((const GLubyte*)"glSamplePatternSGIS")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIS_multisample */

#ifdef GL_SGIS_pixel_texture

#endif /* GL_SGIS_pixel_texture */

#ifdef GL_SGIS_point_line_texgen

#endif /* GL_SGIS_point_line_texgen */

#ifdef GL_SGIS_sharpen_texture

static GLboolean _glewInit_GL_SGIS_sharpen_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetSharpenTexFuncSGIS = (PFNGLGETSHARPENTEXFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glGetSharpenTexFuncSGIS")) == nullptr) || r;
  r = ((glSharpenTexFuncSGIS = (PFNGLSHARPENTEXFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glSharpenTexFuncSGIS")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIS_sharpen_texture */

#ifdef GL_SGIS_texture4D

static GLboolean _glewInit_GL_SGIS_texture4D (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTexImage4DSGIS = (PFNGLTEXIMAGE4DSGISPROC)glewGetProcAddress((const GLubyte*)"glTexImage4DSGIS")) == nullptr) || r;
  r = ((glTexSubImage4DSGIS = (PFNGLTEXSUBIMAGE4DSGISPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage4DSGIS")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIS_texture4D */

#ifdef GL_SGIS_texture_border_clamp

#endif /* GL_SGIS_texture_border_clamp */

#ifdef GL_SGIS_texture_edge_clamp

#endif /* GL_SGIS_texture_edge_clamp */

#ifdef GL_SGIS_texture_filter4

static GLboolean _glewInit_GL_SGIS_texture_filter4 (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGetTexFilterFuncSGIS = (PFNGLGETTEXFILTERFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glGetTexFilterFuncSGIS")) == nullptr) || r;
  r = ((glTexFilterFuncSGIS = (PFNGLTEXFILTERFUNCSGISPROC)glewGetProcAddress((const GLubyte*)"glTexFilterFuncSGIS")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIS_texture_filter4 */

#ifdef GL_SGIS_texture_lod

#endif /* GL_SGIS_texture_lod */

#ifdef GL_SGIS_texture_select

#endif /* GL_SGIS_texture_select */

#ifdef GL_SGIX_async

static GLboolean _glewInit_GL_SGIX_async (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAsyncMarkerSGIX = (PFNGLASYNCMARKERSGIXPROC)glewGetProcAddress((const GLubyte*)"glAsyncMarkerSGIX")) == nullptr) || r;
  r = ((glDeleteAsyncMarkersSGIX = (PFNGLDELETEASYNCMARKERSSGIXPROC)glewGetProcAddress((const GLubyte*)"glDeleteAsyncMarkersSGIX")) == nullptr) || r;
  r = ((glFinishAsyncSGIX = (PFNGLFINISHASYNCSGIXPROC)glewGetProcAddress((const GLubyte*)"glFinishAsyncSGIX")) == nullptr) || r;
  r = ((glGenAsyncMarkersSGIX = (PFNGLGENASYNCMARKERSSGIXPROC)glewGetProcAddress((const GLubyte*)"glGenAsyncMarkersSGIX")) == nullptr) || r;
  r = ((glIsAsyncMarkerSGIX = (PFNGLISASYNCMARKERSGIXPROC)glewGetProcAddress((const GLubyte*)"glIsAsyncMarkerSGIX")) == nullptr) || r;
  r = ((glPollAsyncSGIX = (PFNGLPOLLASYNCSGIXPROC)glewGetProcAddress((const GLubyte*)"glPollAsyncSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_async */

#ifdef GL_SGIX_async_histogram

#endif /* GL_SGIX_async_histogram */

#ifdef GL_SGIX_async_pixel

#endif /* GL_SGIX_async_pixel */

#ifdef GL_SGIX_blend_alpha_minmax

#endif /* GL_SGIX_blend_alpha_minmax */

#ifdef GL_SGIX_clipmap

#endif /* GL_SGIX_clipmap */

#ifdef GL_SGIX_convolution_accuracy

#endif /* GL_SGIX_convolution_accuracy */

#ifdef GL_SGIX_depth_texture

#endif /* GL_SGIX_depth_texture */

#ifdef GL_SGIX_flush_raster

static GLboolean _glewInit_GL_SGIX_flush_raster (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushRasterSGIX = (PFNGLFLUSHRASTERSGIXPROC)glewGetProcAddress((const GLubyte*)"glFlushRasterSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_flush_raster */

#ifdef GL_SGIX_fog_offset

#endif /* GL_SGIX_fog_offset */

#ifdef GL_SGIX_fog_texture

static GLboolean _glewInit_GL_SGIX_fog_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTextureFogSGIX = (PFNGLTEXTUREFOGSGIXPROC)glewGetProcAddress((const GLubyte*)"glTextureFogSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_fog_texture */

#ifdef GL_SGIX_fragment_specular_lighting

static GLboolean _glewInit_GL_SGIX_fragment_specular_lighting (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFragmentColorMaterialSGIX = (PFNGLFRAGMENTCOLORMATERIALSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentColorMaterialSGIX")) == nullptr) || r;
  r = ((glFragmentLightModelfSGIX = (PFNGLFRAGMENTLIGHTMODELFSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModelfSGIX")) == nullptr) || r;
  r = ((glFragmentLightModelfvSGIX = (PFNGLFRAGMENTLIGHTMODELFVSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModelfvSGIX")) == nullptr) || r;
  r = ((glFragmentLightModeliSGIX = (PFNGLFRAGMENTLIGHTMODELISGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModeliSGIX")) == nullptr) || r;
  r = ((glFragmentLightModelivSGIX = (PFNGLFRAGMENTLIGHTMODELIVSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightModelivSGIX")) == nullptr) || r;
  r = ((glFragmentLightfSGIX = (PFNGLFRAGMENTLIGHTFSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightfSGIX")) == nullptr) || r;
  r = ((glFragmentLightfvSGIX = (PFNGLFRAGMENTLIGHTFVSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightfvSGIX")) == nullptr) || r;
  r = ((glFragmentLightiSGIX = (PFNGLFRAGMENTLIGHTISGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightiSGIX")) == nullptr) || r;
  r = ((glFragmentLightivSGIX = (PFNGLFRAGMENTLIGHTIVSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentLightivSGIX")) == nullptr) || r;
  r = ((glFragmentMaterialfSGIX = (PFNGLFRAGMENTMATERIALFSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialfSGIX")) == nullptr) || r;
  r = ((glFragmentMaterialfvSGIX = (PFNGLFRAGMENTMATERIALFVSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialfvSGIX")) == nullptr) || r;
  r = ((glFragmentMaterialiSGIX = (PFNGLFRAGMENTMATERIALISGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialiSGIX")) == nullptr) || r;
  r = ((glFragmentMaterialivSGIX = (PFNGLFRAGMENTMATERIALIVSGIXPROC)glewGetProcAddress((const GLubyte*)"glFragmentMaterialivSGIX")) == nullptr) || r;
  r = ((glGetFragmentLightfvSGIX = (PFNGLGETFRAGMENTLIGHTFVSGIXPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentLightfvSGIX")) == nullptr) || r;
  r = ((glGetFragmentLightivSGIX = (PFNGLGETFRAGMENTLIGHTIVSGIXPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentLightivSGIX")) == nullptr) || r;
  r = ((glGetFragmentMaterialfvSGIX = (PFNGLGETFRAGMENTMATERIALFVSGIXPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentMaterialfvSGIX")) == nullptr) || r;
  r = ((glGetFragmentMaterialivSGIX = (PFNGLGETFRAGMENTMATERIALIVSGIXPROC)glewGetProcAddress((const GLubyte*)"glGetFragmentMaterialivSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_fragment_specular_lighting */

#ifdef GL_SGIX_framezoom

static GLboolean _glewInit_GL_SGIX_framezoom (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFrameZoomSGIX = (PFNGLFRAMEZOOMSGIXPROC)glewGetProcAddress((const GLubyte*)"glFrameZoomSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_framezoom */

#ifdef GL_SGIX_interlace

#endif /* GL_SGIX_interlace */

#ifdef GL_SGIX_ir_instrument1

#endif /* GL_SGIX_ir_instrument1 */

#ifdef GL_SGIX_list_priority

#endif /* GL_SGIX_list_priority */

#ifdef GL_SGIX_pixel_texture

static GLboolean _glewInit_GL_SGIX_pixel_texture (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glPixelTexGenSGIX = (PFNGLPIXELTEXGENSGIXPROC)glewGetProcAddress((const GLubyte*)"glPixelTexGenSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_pixel_texture */

#ifdef GL_SGIX_pixel_texture_bits

#endif /* GL_SGIX_pixel_texture_bits */

#ifdef GL_SGIX_reference_plane

static GLboolean _glewInit_GL_SGIX_reference_plane (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glReferencePlaneSGIX = (PFNGLREFERENCEPLANESGIXPROC)glewGetProcAddress((const GLubyte*)"glReferencePlaneSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_reference_plane */

#ifdef GL_SGIX_resample

#endif /* GL_SGIX_resample */

#ifdef GL_SGIX_shadow

#endif /* GL_SGIX_shadow */

#ifdef GL_SGIX_shadow_ambient

#endif /* GL_SGIX_shadow_ambient */

#ifdef GL_SGIX_sprite

static GLboolean _glewInit_GL_SGIX_sprite (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glSpriteParameterfSGIX = (PFNGLSPRITEPARAMETERFSGIXPROC)glewGetProcAddress((const GLubyte*)"glSpriteParameterfSGIX")) == nullptr) || r;
  r = ((glSpriteParameterfvSGIX = (PFNGLSPRITEPARAMETERFVSGIXPROC)glewGetProcAddress((const GLubyte*)"glSpriteParameterfvSGIX")) == nullptr) || r;
  r = ((glSpriteParameteriSGIX = (PFNGLSPRITEPARAMETERISGIXPROC)glewGetProcAddress((const GLubyte*)"glSpriteParameteriSGIX")) == nullptr) || r;
  r = ((glSpriteParameterivSGIX = (PFNGLSPRITEPARAMETERIVSGIXPROC)glewGetProcAddress((const GLubyte*)"glSpriteParameterivSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_sprite */

#ifdef GL_SGIX_tag_sample_buffer

static GLboolean _glewInit_GL_SGIX_tag_sample_buffer (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glTagSampleBufferSGIX = (PFNGLTAGSAMPLEBUFFERSGIXPROC)glewGetProcAddress((const GLubyte*)"glTagSampleBufferSGIX")) == nullptr) || r;

  return r;
}

#endif /* GL_SGIX_tag_sample_buffer */

#ifdef GL_SGIX_texture_add_env

#endif /* GL_SGIX_texture_add_env */

#ifdef GL_SGIX_texture_coordinate_clamp

#endif /* GL_SGIX_texture_coordinate_clamp */

#ifdef GL_SGIX_texture_lod_bias

#endif /* GL_SGIX_texture_lod_bias */

#ifdef GL_SGIX_texture_multi_buffer

#endif /* GL_SGIX_texture_multi_buffer */

#ifdef GL_SGIX_texture_range

#endif /* GL_SGIX_texture_range */

#ifdef GL_SGIX_texture_scale_bias

#endif /* GL_SGIX_texture_scale_bias */

#ifdef GL_SGIX_vertex_preclip

#endif /* GL_SGIX_vertex_preclip */

#ifdef GL_SGIX_vertex_preclip_hint

#endif /* GL_SGIX_vertex_preclip_hint */

#ifdef GL_SGIX_ycrcb

#endif /* GL_SGIX_ycrcb */

#ifdef GL_SGI_color_matrix

#endif /* GL_SGI_color_matrix */

#ifdef GL_SGI_color_table

static GLboolean _glewInit_GL_SGI_color_table (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColorTableParameterfvSGI = (PFNGLCOLORTABLEPARAMETERFVSGIPROC)glewGetProcAddress((const GLubyte*)"glColorTableParameterfvSGI")) == nullptr) || r;
  r = ((glColorTableParameterivSGI = (PFNGLCOLORTABLEPARAMETERIVSGIPROC)glewGetProcAddress((const GLubyte*)"glColorTableParameterivSGI")) == nullptr) || r;
  r = ((glColorTableSGI = (PFNGLCOLORTABLESGIPROC)glewGetProcAddress((const GLubyte*)"glColorTableSGI")) == nullptr) || r;
  r = ((glCopyColorTableSGI = (PFNGLCOPYCOLORTABLESGIPROC)glewGetProcAddress((const GLubyte*)"glCopyColorTableSGI")) == nullptr) || r;
  r = ((glGetColorTableParameterfvSGI = (PFNGLGETCOLORTABLEPARAMETERFVSGIPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameterfvSGI")) == nullptr) || r;
  r = ((glGetColorTableParameterivSGI = (PFNGLGETCOLORTABLEPARAMETERIVSGIPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameterivSGI")) == nullptr) || r;
  r = ((glGetColorTableSGI = (PFNGLGETCOLORTABLESGIPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableSGI")) == nullptr) || r;

  return r;
}

#endif /* GL_SGI_color_table */

#ifdef GL_SGI_texture_color_table

#endif /* GL_SGI_texture_color_table */

#ifdef GL_SUNX_constant_data

static GLboolean _glewInit_GL_SUNX_constant_data (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glFinishTextureSUNX = (PFNGLFINISHTEXTURESUNXPROC)glewGetProcAddress((const GLubyte*)"glFinishTextureSUNX")) == nullptr) || r;

  return r;
}

#endif /* GL_SUNX_constant_data */

#ifdef GL_SUN_convolution_border_modes

#endif /* GL_SUN_convolution_border_modes */

#ifdef GL_SUN_global_alpha

static GLboolean _glewInit_GL_SUN_global_alpha (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glGlobalAlphaFactorbSUN = (PFNGLGLOBALALPHAFACTORBSUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactorbSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactordSUN = (PFNGLGLOBALALPHAFACTORDSUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactordSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactorfSUN = (PFNGLGLOBALALPHAFACTORFSUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactorfSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactoriSUN = (PFNGLGLOBALALPHAFACTORISUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactoriSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactorsSUN = (PFNGLGLOBALALPHAFACTORSSUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactorsSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactorubSUN = (PFNGLGLOBALALPHAFACTORUBSUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactorubSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactoruiSUN = (PFNGLGLOBALALPHAFACTORUISUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactoruiSUN")) == nullptr) || r;
  r = ((glGlobalAlphaFactorusSUN = (PFNGLGLOBALALPHAFACTORUSSUNPROC)glewGetProcAddress((const GLubyte*)"glGlobalAlphaFactorusSUN")) == nullptr) || r;

  return r;
}

#endif /* GL_SUN_global_alpha */

#ifdef GL_SUN_mesh_array

#endif /* GL_SUN_mesh_array */

#ifdef GL_SUN_read_video_pixels

static GLboolean _glewInit_GL_SUN_read_video_pixels (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glReadVideoPixelsSUN = (PFNGLREADVIDEOPIXELSSUNPROC)glewGetProcAddress((const GLubyte*)"glReadVideoPixelsSUN")) == nullptr) || r;

  return r;
}

#endif /* GL_SUN_read_video_pixels */

#ifdef GL_SUN_slice_accum

#endif /* GL_SUN_slice_accum */

#ifdef GL_SUN_triangle_list

static GLboolean _glewInit_GL_SUN_triangle_list (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glReplacementCodePointerSUN = (PFNGLREPLACEMENTCODEPOINTERSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodePointerSUN")) == nullptr) || r;
  r = ((glReplacementCodeubSUN = (PFNGLREPLACEMENTCODEUBSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeubSUN")) == nullptr) || r;
  r = ((glReplacementCodeubvSUN = (PFNGLREPLACEMENTCODEUBVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeubvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiSUN = (PFNGLREPLACEMENTCODEUISUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiSUN")) == nullptr) || r;
  r = ((glReplacementCodeuivSUN = (PFNGLREPLACEMENTCODEUIVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuivSUN")) == nullptr) || r;
  r = ((glReplacementCodeusSUN = (PFNGLREPLACEMENTCODEUSSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeusSUN")) == nullptr) || r;
  r = ((glReplacementCodeusvSUN = (PFNGLREPLACEMENTCODEUSVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeusvSUN")) == nullptr) || r;

  return r;
}

#endif /* GL_SUN_triangle_list */

#ifdef GL_SUN_vertex

static GLboolean _glewInit_GL_SUN_vertex (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glColor3fVertex3fSUN = (PFNGLCOLOR3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glColor3fVertex3fSUN")) == nullptr) || r;
  r = ((glColor3fVertex3fvSUN = (PFNGLCOLOR3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glColor3fVertex3fvSUN")) == nullptr) || r;
  r = ((glColor4fNormal3fVertex3fSUN = (PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glColor4fNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glColor4fNormal3fVertex3fvSUN = (PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glColor4fNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glColor4ubVertex2fSUN = (PFNGLCOLOR4UBVERTEX2FSUNPROC)glewGetProcAddress((const GLubyte*)"glColor4ubVertex2fSUN")) == nullptr) || r;
  r = ((glColor4ubVertex2fvSUN = (PFNGLCOLOR4UBVERTEX2FVSUNPROC)glewGetProcAddress((const GLubyte*)"glColor4ubVertex2fvSUN")) == nullptr) || r;
  r = ((glColor4ubVertex3fSUN = (PFNGLCOLOR4UBVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glColor4ubVertex3fSUN")) == nullptr) || r;
  r = ((glColor4ubVertex3fvSUN = (PFNGLCOLOR4UBVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glColor4ubVertex3fvSUN")) == nullptr) || r;
  r = ((glNormal3fVertex3fSUN = (PFNGLNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glNormal3fVertex3fvSUN = (PFNGLNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiColor3fVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiColor3fVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiColor3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiColor3fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiColor4fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiColor4fNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiColor4fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiColor4fNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiColor4ubVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiColor4ubVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiColor4ubVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiColor4ubVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiTexCoord2fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiTexCoord2fVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiTexCoord2fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiTexCoord2fVertex3fvSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiVertex3fSUN = (PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiVertex3fSUN")) == nullptr) || r;
  r = ((glReplacementCodeuiVertex3fvSUN = (PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glReplacementCodeuiVertex3fvSUN")) == nullptr) || r;
  r = ((glTexCoord2fColor3fVertex3fSUN = (PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fColor3fVertex3fSUN")) == nullptr) || r;
  r = ((glTexCoord2fColor3fVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fColor3fVertex3fvSUN")) == nullptr) || r;
  r = ((glTexCoord2fColor4fNormal3fVertex3fSUN = (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fColor4fNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glTexCoord2fColor4fNormal3fVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fColor4fNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glTexCoord2fColor4ubVertex3fSUN = (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fColor4ubVertex3fSUN")) == nullptr) || r;
  r = ((glTexCoord2fColor4ubVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fColor4ubVertex3fvSUN")) == nullptr) || r;
  r = ((glTexCoord2fNormal3fVertex3fSUN = (PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fNormal3fVertex3fSUN")) == nullptr) || r;
  r = ((glTexCoord2fNormal3fVertex3fvSUN = (PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fNormal3fVertex3fvSUN")) == nullptr) || r;
  r = ((glTexCoord2fVertex3fSUN = (PFNGLTEXCOORD2FVERTEX3FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fVertex3fSUN")) == nullptr) || r;
  r = ((glTexCoord2fVertex3fvSUN = (PFNGLTEXCOORD2FVERTEX3FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord2fVertex3fvSUN")) == nullptr) || r;
  r = ((glTexCoord4fColor4fNormal3fVertex4fSUN = (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord4fColor4fNormal3fVertex4fSUN")) == nullptr) || r;
  r = ((glTexCoord4fColor4fNormal3fVertex4fvSUN = (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord4fColor4fNormal3fVertex4fvSUN")) == nullptr) || r;
  r = ((glTexCoord4fVertex4fSUN = (PFNGLTEXCOORD4FVERTEX4FSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord4fVertex4fSUN")) == nullptr) || r;
  r = ((glTexCoord4fVertex4fvSUN = (PFNGLTEXCOORD4FVERTEX4FVSUNPROC)glewGetProcAddress((const GLubyte*)"glTexCoord4fVertex4fvSUN")) == nullptr) || r;

  return r;
}

#endif /* GL_SUN_vertex */

#ifdef GL_WIN_phong_shading

#endif /* GL_WIN_phong_shading */

#ifdef GL_WIN_specular_fog

#endif /* GL_WIN_specular_fog */

#ifdef GL_WIN_swap_hint

static GLboolean _glewInit_GL_WIN_swap_hint (GLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glAddSwapHintRectWIN = (PFNGLADDSWAPHINTRECTWINPROC)glewGetProcAddress((const GLubyte*)"glAddSwapHintRectWIN")) == nullptr) || r;

  return r;
}

#endif /* GL_WIN_swap_hint */

/* ------------------------------------------------------------------------- */

GLboolean glewGetExtension (const char* name)
{    
  const GLubyte* start;
  const GLubyte* end;
  start = (const GLubyte*)glGetString(GL_EXTENSIONS);
  if (start == 0)
    return GL_FALSE;
  end = start + _glewStrLen(start);
  return _glewSearchExtension(name, start, end);
}

/* ------------------------------------------------------------------------- */

#ifndef GLEW_MX
static
#endif
GLenum glewContextInit (GLEW_CONTEXT_ARG_DEF_LIST)
{
  const GLubyte* s;
  GLuint dot;
  GLint major, minor;
  const GLubyte* extStart;
  const GLubyte* extEnd;
  /* query opengl version */
  s = glGetString(GL_VERSION);
  dot = _glewStrCLen(s, '.');
  if (dot == 0)
    return GLEW_ERROR_NO_GL_VERSION;
  
  major = s[dot-1]-'0';
  minor = s[dot+1]-'0';

  if (minor < 0 || minor > 9)
    minor = 0;
  if (major<0 || major>9)
    return GLEW_ERROR_NO_GL_VERSION;
  

  if (major == 1 && minor == 0)
  {
    return GLEW_ERROR_GL_VERSION_10_ONLY;
  }
  else
  {
    CONST_CAST(GLEW_VERSION_4_2)   = ( major > 4 )                 || ( major == 4 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_4_1)   = GLEW_VERSION_4_2   == GL_TRUE || ( major == 4 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_4_0)   = GLEW_VERSION_4_1   == GL_TRUE || ( major == 4               ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_3_3)   = GLEW_VERSION_4_0   == GL_TRUE || ( major == 3 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_3_2)   = GLEW_VERSION_3_3   == GL_TRUE || ( major == 3 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_3_1)   = GLEW_VERSION_3_2   == GL_TRUE || ( major == 3 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_3_0)   = GLEW_VERSION_3_1   == GL_TRUE || ( major == 3               ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_2_1)   = GLEW_VERSION_3_0   == GL_TRUE || ( major == 2 && minor >= 1 ) ? GL_TRUE : GL_FALSE;    
    CONST_CAST(GLEW_VERSION_2_0)   = GLEW_VERSION_2_1   == GL_TRUE || ( major == 2               ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_5)   = GLEW_VERSION_2_0   == GL_TRUE || ( major == 1 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_4)   = GLEW_VERSION_1_5   == GL_TRUE || ( major == 1 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_3)   = GLEW_VERSION_1_4   == GL_TRUE || ( major == 1 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_2_1) = GLEW_VERSION_1_3   == GL_TRUE                                 ? GL_TRUE : GL_FALSE; 
    CONST_CAST(GLEW_VERSION_1_2)   = GLEW_VERSION_1_2_1 == GL_TRUE || ( major == 1 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    CONST_CAST(GLEW_VERSION_1_1)   = GLEW_VERSION_1_2   == GL_TRUE || ( major == 1 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
  }

  /* query opengl extensions string */
  extStart = glGetString(GL_EXTENSIONS);
  if (extStart == 0)
    extStart = (const GLubyte*)"";
  extEnd = extStart + _glewStrLen(extStart);

  /* initialize extensions */
#ifdef GL_VERSION_1_2
  if (glewExperimental || GLEW_VERSION_1_2) CONST_CAST(GLEW_VERSION_1_2) = !_glewInit_GL_VERSION_1_2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_2 */
#ifdef GL_VERSION_1_2_1
#endif /* GL_VERSION_1_2_1 */
#ifdef GL_VERSION_1_3
  if (glewExperimental || GLEW_VERSION_1_3) CONST_CAST(GLEW_VERSION_1_3) = !_glewInit_GL_VERSION_1_3(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_3 */
#ifdef GL_VERSION_1_4
  if (glewExperimental || GLEW_VERSION_1_4) CONST_CAST(GLEW_VERSION_1_4) = !_glewInit_GL_VERSION_1_4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_4 */
#ifdef GL_VERSION_1_5
  if (glewExperimental || GLEW_VERSION_1_5) CONST_CAST(GLEW_VERSION_1_5) = !_glewInit_GL_VERSION_1_5(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_1_5 */
#ifdef GL_VERSION_2_0
  if (glewExperimental || GLEW_VERSION_2_0) CONST_CAST(GLEW_VERSION_2_0) = !_glewInit_GL_VERSION_2_0(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_2_0 */
#ifdef GL_VERSION_2_1
  if (glewExperimental || GLEW_VERSION_2_1) CONST_CAST(GLEW_VERSION_2_1) = !_glewInit_GL_VERSION_2_1(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_2_1 */
#ifdef GL_VERSION_3_0
  if (glewExperimental || GLEW_VERSION_3_0) CONST_CAST(GLEW_VERSION_3_0) = !_glewInit_GL_VERSION_3_0(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_3_0 */
#ifdef GL_VERSION_3_1
  if (glewExperimental || GLEW_VERSION_3_1) CONST_CAST(GLEW_VERSION_3_1) = !_glewInit_GL_VERSION_3_1(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_3_1 */
#ifdef GL_VERSION_3_2
  if (glewExperimental || GLEW_VERSION_3_2) CONST_CAST(GLEW_VERSION_3_2) = !_glewInit_GL_VERSION_3_2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_3_2 */
#ifdef GL_VERSION_3_3
  if (glewExperimental || GLEW_VERSION_3_3) CONST_CAST(GLEW_VERSION_3_3) = !_glewInit_GL_VERSION_3_3(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_3_3 */
#ifdef GL_VERSION_4_0
  if (glewExperimental || GLEW_VERSION_4_0) CONST_CAST(GLEW_VERSION_4_0) = !_glewInit_GL_VERSION_4_0(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_VERSION_4_0 */
#ifdef GL_VERSION_4_1
#endif /* GL_VERSION_4_1 */
#ifdef GL_VERSION_4_2
#endif /* GL_VERSION_4_2 */
#ifdef GL_3DFX_multisample
  CONST_CAST(GLEW_3DFX_multisample) = _glewSearchExtension("GL_3DFX_multisample", extStart, extEnd);
#endif /* GL_3DFX_multisample */
#ifdef GL_3DFX_tbuffer
  CONST_CAST(GLEW_3DFX_tbuffer) = _glewSearchExtension("GL_3DFX_tbuffer", extStart, extEnd);
  if (glewExperimental || GLEW_3DFX_tbuffer) CONST_CAST(GLEW_3DFX_tbuffer) = !_glewInit_GL_3DFX_tbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_3DFX_tbuffer */
#ifdef GL_3DFX_texture_compression_FXT1
  CONST_CAST(GLEW_3DFX_texture_compression_FXT1) = _glewSearchExtension("GL_3DFX_texture_compression_FXT1", extStart, extEnd);
#endif /* GL_3DFX_texture_compression_FXT1 */
#ifdef GL_AMD_blend_minmax_factor
  CONST_CAST(GLEW_AMD_blend_minmax_factor) = _glewSearchExtension("GL_AMD_blend_minmax_factor", extStart, extEnd);
#endif /* GL_AMD_blend_minmax_factor */
#ifdef GL_AMD_conservative_depth
  CONST_CAST(GLEW_AMD_conservative_depth) = _glewSearchExtension("GL_AMD_conservative_depth", extStart, extEnd);
#endif /* GL_AMD_conservative_depth */
#ifdef GL_AMD_debug_output
  CONST_CAST(GLEW_AMD_debug_output) = _glewSearchExtension("GL_AMD_debug_output", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_debug_output) CONST_CAST(GLEW_AMD_debug_output) = !_glewInit_GL_AMD_debug_output(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_debug_output */
#ifdef GL_AMD_depth_clamp_separate
  CONST_CAST(GLEW_AMD_depth_clamp_separate) = _glewSearchExtension("GL_AMD_depth_clamp_separate", extStart, extEnd);
#endif /* GL_AMD_depth_clamp_separate */
#ifdef GL_AMD_draw_buffers_blend
  CONST_CAST(GLEW_AMD_draw_buffers_blend) = _glewSearchExtension("GL_AMD_draw_buffers_blend", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_draw_buffers_blend) CONST_CAST(GLEW_AMD_draw_buffers_blend) = !_glewInit_GL_AMD_draw_buffers_blend(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_draw_buffers_blend */
#ifdef GL_AMD_multi_draw_indirect
  CONST_CAST(GLEW_AMD_multi_draw_indirect) = _glewSearchExtension("GL_AMD_multi_draw_indirect", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_multi_draw_indirect) CONST_CAST(GLEW_AMD_multi_draw_indirect) = !_glewInit_GL_AMD_multi_draw_indirect(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_multi_draw_indirect */
#ifdef GL_AMD_name_gen_delete
  CONST_CAST(GLEW_AMD_name_gen_delete) = _glewSearchExtension("GL_AMD_name_gen_delete", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_name_gen_delete) CONST_CAST(GLEW_AMD_name_gen_delete) = !_glewInit_GL_AMD_name_gen_delete(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_name_gen_delete */
#ifdef GL_AMD_performance_monitor
  CONST_CAST(GLEW_AMD_performance_monitor) = _glewSearchExtension("GL_AMD_performance_monitor", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_performance_monitor) CONST_CAST(GLEW_AMD_performance_monitor) = !_glewInit_GL_AMD_performance_monitor(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_performance_monitor */
#ifdef GL_AMD_sample_positions
  CONST_CAST(GLEW_AMD_sample_positions) = _glewSearchExtension("GL_AMD_sample_positions", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_sample_positions) CONST_CAST(GLEW_AMD_sample_positions) = !_glewInit_GL_AMD_sample_positions(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_sample_positions */
#ifdef GL_AMD_seamless_cubemap_per_texture
  CONST_CAST(GLEW_AMD_seamless_cubemap_per_texture) = _glewSearchExtension("GL_AMD_seamless_cubemap_per_texture", extStart, extEnd);
#endif /* GL_AMD_seamless_cubemap_per_texture */
#ifdef GL_AMD_shader_stencil_export
  CONST_CAST(GLEW_AMD_shader_stencil_export) = _glewSearchExtension("GL_AMD_shader_stencil_export", extStart, extEnd);
#endif /* GL_AMD_shader_stencil_export */
#ifdef GL_AMD_texture_texture4
  CONST_CAST(GLEW_AMD_texture_texture4) = _glewSearchExtension("GL_AMD_texture_texture4", extStart, extEnd);
#endif /* GL_AMD_texture_texture4 */
#ifdef GL_AMD_transform_feedback3_lines_triangles
  CONST_CAST(GLEW_AMD_transform_feedback3_lines_triangles) = _glewSearchExtension("GL_AMD_transform_feedback3_lines_triangles", extStart, extEnd);
#endif /* GL_AMD_transform_feedback3_lines_triangles */
#ifdef GL_AMD_vertex_shader_tessellator
  CONST_CAST(GLEW_AMD_vertex_shader_tessellator) = _glewSearchExtension("GL_AMD_vertex_shader_tessellator", extStart, extEnd);
  if (glewExperimental || GLEW_AMD_vertex_shader_tessellator) CONST_CAST(GLEW_AMD_vertex_shader_tessellator) = !_glewInit_GL_AMD_vertex_shader_tessellator(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_AMD_vertex_shader_tessellator */
#ifdef GL_APPLE_aux_depth_stencil
  CONST_CAST(GLEW_APPLE_aux_depth_stencil) = _glewSearchExtension("GL_APPLE_aux_depth_stencil", extStart, extEnd);
#endif /* GL_APPLE_aux_depth_stencil */
#ifdef GL_APPLE_client_storage
  CONST_CAST(GLEW_APPLE_client_storage) = _glewSearchExtension("GL_APPLE_client_storage", extStart, extEnd);
#endif /* GL_APPLE_client_storage */
#ifdef GL_APPLE_element_array
  CONST_CAST(GLEW_APPLE_element_array) = _glewSearchExtension("GL_APPLE_element_array", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_element_array) CONST_CAST(GLEW_APPLE_element_array) = !_glewInit_GL_APPLE_element_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_element_array */
#ifdef GL_APPLE_fence
  CONST_CAST(GLEW_APPLE_fence) = _glewSearchExtension("GL_APPLE_fence", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_fence) CONST_CAST(GLEW_APPLE_fence) = !_glewInit_GL_APPLE_fence(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_fence */
#ifdef GL_APPLE_float_pixels
  CONST_CAST(GLEW_APPLE_float_pixels) = _glewSearchExtension("GL_APPLE_float_pixels", extStart, extEnd);
#endif /* GL_APPLE_float_pixels */
#ifdef GL_APPLE_flush_buffer_range
  CONST_CAST(GLEW_APPLE_flush_buffer_range) = _glewSearchExtension("GL_APPLE_flush_buffer_range", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_flush_buffer_range) CONST_CAST(GLEW_APPLE_flush_buffer_range) = !_glewInit_GL_APPLE_flush_buffer_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_flush_buffer_range */
#ifdef GL_APPLE_object_purgeable
  CONST_CAST(GLEW_APPLE_object_purgeable) = _glewSearchExtension("GL_APPLE_object_purgeable", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_object_purgeable) CONST_CAST(GLEW_APPLE_object_purgeable) = !_glewInit_GL_APPLE_object_purgeable(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_object_purgeable */
#ifdef GL_APPLE_pixel_buffer
  CONST_CAST(GLEW_APPLE_pixel_buffer) = _glewSearchExtension("GL_APPLE_pixel_buffer", extStart, extEnd);
#endif /* GL_APPLE_pixel_buffer */
#ifdef GL_APPLE_rgb_422
  CONST_CAST(GLEW_APPLE_rgb_422) = _glewSearchExtension("GL_APPLE_rgb_422", extStart, extEnd);
#endif /* GL_APPLE_rgb_422 */
#ifdef GL_APPLE_row_bytes
  CONST_CAST(GLEW_APPLE_row_bytes) = _glewSearchExtension("GL_APPLE_row_bytes", extStart, extEnd);
#endif /* GL_APPLE_row_bytes */
#ifdef GL_APPLE_specular_vector
  CONST_CAST(GLEW_APPLE_specular_vector) = _glewSearchExtension("GL_APPLE_specular_vector", extStart, extEnd);
#endif /* GL_APPLE_specular_vector */
#ifdef GL_APPLE_texture_range
  CONST_CAST(GLEW_APPLE_texture_range) = _glewSearchExtension("GL_APPLE_texture_range", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_texture_range) CONST_CAST(GLEW_APPLE_texture_range) = !_glewInit_GL_APPLE_texture_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_texture_range */
#ifdef GL_APPLE_transform_hint
  CONST_CAST(GLEW_APPLE_transform_hint) = _glewSearchExtension("GL_APPLE_transform_hint", extStart, extEnd);
#endif /* GL_APPLE_transform_hint */
#ifdef GL_APPLE_vertex_array_object
  CONST_CAST(GLEW_APPLE_vertex_array_object) = _glewSearchExtension("GL_APPLE_vertex_array_object", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_vertex_array_object) CONST_CAST(GLEW_APPLE_vertex_array_object) = !_glewInit_GL_APPLE_vertex_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_vertex_array_object */
#ifdef GL_APPLE_vertex_array_range
  CONST_CAST(GLEW_APPLE_vertex_array_range) = _glewSearchExtension("GL_APPLE_vertex_array_range", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_vertex_array_range) CONST_CAST(GLEW_APPLE_vertex_array_range) = !_glewInit_GL_APPLE_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_vertex_array_range */
#ifdef GL_APPLE_vertex_program_evaluators
  CONST_CAST(GLEW_APPLE_vertex_program_evaluators) = _glewSearchExtension("GL_APPLE_vertex_program_evaluators", extStart, extEnd);
  if (glewExperimental || GLEW_APPLE_vertex_program_evaluators) CONST_CAST(GLEW_APPLE_vertex_program_evaluators) = !_glewInit_GL_APPLE_vertex_program_evaluators(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_APPLE_vertex_program_evaluators */
#ifdef GL_APPLE_ycbcr_422
  CONST_CAST(GLEW_APPLE_ycbcr_422) = _glewSearchExtension("GL_APPLE_ycbcr_422", extStart, extEnd);
#endif /* GL_APPLE_ycbcr_422 */
#ifdef GL_ARB_ES2_compatibility
  CONST_CAST(GLEW_ARB_ES2_compatibility) = _glewSearchExtension("GL_ARB_ES2_compatibility", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_ES2_compatibility) CONST_CAST(GLEW_ARB_ES2_compatibility) = !_glewInit_GL_ARB_ES2_compatibility(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_ES2_compatibility */
#ifdef GL_ARB_base_instance
  CONST_CAST(GLEW_ARB_base_instance) = _glewSearchExtension("GL_ARB_base_instance", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_base_instance) CONST_CAST(GLEW_ARB_base_instance) = !_glewInit_GL_ARB_base_instance(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_base_instance */
#ifdef GL_ARB_blend_func_extended
  CONST_CAST(GLEW_ARB_blend_func_extended) = _glewSearchExtension("GL_ARB_blend_func_extended", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_blend_func_extended) CONST_CAST(GLEW_ARB_blend_func_extended) = !_glewInit_GL_ARB_blend_func_extended(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_blend_func_extended */
#ifdef GL_ARB_cl_event
  CONST_CAST(GLEW_ARB_cl_event) = _glewSearchExtension("GL_ARB_cl_event", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_cl_event) CONST_CAST(GLEW_ARB_cl_event) = !_glewInit_GL_ARB_cl_event(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_cl_event */
#ifdef GL_ARB_color_buffer_float
  CONST_CAST(GLEW_ARB_color_buffer_float) = _glewSearchExtension("GL_ARB_color_buffer_float", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_color_buffer_float) CONST_CAST(GLEW_ARB_color_buffer_float) = !_glewInit_GL_ARB_color_buffer_float(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_color_buffer_float */
#ifdef GL_ARB_compatibility
  CONST_CAST(GLEW_ARB_compatibility) = _glewSearchExtension("GL_ARB_compatibility", extStart, extEnd);
#endif /* GL_ARB_compatibility */
#ifdef GL_ARB_compressed_texture_pixel_storage
  CONST_CAST(GLEW_ARB_compressed_texture_pixel_storage) = _glewSearchExtension("GL_ARB_compressed_texture_pixel_storage", extStart, extEnd);
#endif /* GL_ARB_compressed_texture_pixel_storage */
#ifdef GL_ARB_conservative_depth
  CONST_CAST(GLEW_ARB_conservative_depth) = _glewSearchExtension("GL_ARB_conservative_depth", extStart, extEnd);
#endif /* GL_ARB_conservative_depth */
#ifdef GL_ARB_copy_buffer
  CONST_CAST(GLEW_ARB_copy_buffer) = _glewSearchExtension("GL_ARB_copy_buffer", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_copy_buffer) CONST_CAST(GLEW_ARB_copy_buffer) = !_glewInit_GL_ARB_copy_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_copy_buffer */
#ifdef GL_ARB_debug_output
  CONST_CAST(GLEW_ARB_debug_output) = _glewSearchExtension("GL_ARB_debug_output", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_debug_output) CONST_CAST(GLEW_ARB_debug_output) = !_glewInit_GL_ARB_debug_output(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_debug_output */
#ifdef GL_ARB_depth_buffer_float
  CONST_CAST(GLEW_ARB_depth_buffer_float) = _glewSearchExtension("GL_ARB_depth_buffer_float", extStart, extEnd);
#endif /* GL_ARB_depth_buffer_float */
#ifdef GL_ARB_depth_clamp
  CONST_CAST(GLEW_ARB_depth_clamp) = _glewSearchExtension("GL_ARB_depth_clamp", extStart, extEnd);
#endif /* GL_ARB_depth_clamp */
#ifdef GL_ARB_depth_texture
  CONST_CAST(GLEW_ARB_depth_texture) = _glewSearchExtension("GL_ARB_depth_texture", extStart, extEnd);
#endif /* GL_ARB_depth_texture */
#ifdef GL_ARB_draw_buffers
  CONST_CAST(GLEW_ARB_draw_buffers) = _glewSearchExtension("GL_ARB_draw_buffers", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_draw_buffers) CONST_CAST(GLEW_ARB_draw_buffers) = !_glewInit_GL_ARB_draw_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_draw_buffers */
#ifdef GL_ARB_draw_buffers_blend
  CONST_CAST(GLEW_ARB_draw_buffers_blend) = _glewSearchExtension("GL_ARB_draw_buffers_blend", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_draw_buffers_blend) CONST_CAST(GLEW_ARB_draw_buffers_blend) = !_glewInit_GL_ARB_draw_buffers_blend(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_draw_buffers_blend */
#ifdef GL_ARB_draw_elements_base_vertex
  CONST_CAST(GLEW_ARB_draw_elements_base_vertex) = _glewSearchExtension("GL_ARB_draw_elements_base_vertex", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_draw_elements_base_vertex) CONST_CAST(GLEW_ARB_draw_elements_base_vertex) = !_glewInit_GL_ARB_draw_elements_base_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_draw_elements_base_vertex */
#ifdef GL_ARB_draw_indirect
  CONST_CAST(GLEW_ARB_draw_indirect) = _glewSearchExtension("GL_ARB_draw_indirect", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_draw_indirect) CONST_CAST(GLEW_ARB_draw_indirect) = !_glewInit_GL_ARB_draw_indirect(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_draw_indirect */
#ifdef GL_ARB_draw_instanced
  CONST_CAST(GLEW_ARB_draw_instanced) = _glewSearchExtension("GL_ARB_draw_instanced", extStart, extEnd);
#endif /* GL_ARB_draw_instanced */
#ifdef GL_ARB_explicit_attrib_location
  CONST_CAST(GLEW_ARB_explicit_attrib_location) = _glewSearchExtension("GL_ARB_explicit_attrib_location", extStart, extEnd);
#endif /* GL_ARB_explicit_attrib_location */
#ifdef GL_ARB_fragment_coord_conventions
  CONST_CAST(GLEW_ARB_fragment_coord_conventions) = _glewSearchExtension("GL_ARB_fragment_coord_conventions", extStart, extEnd);
#endif /* GL_ARB_fragment_coord_conventions */
#ifdef GL_ARB_fragment_program
  CONST_CAST(GLEW_ARB_fragment_program) = _glewSearchExtension("GL_ARB_fragment_program", extStart, extEnd);
#endif /* GL_ARB_fragment_program */
#ifdef GL_ARB_fragment_program_shadow
  CONST_CAST(GLEW_ARB_fragment_program_shadow) = _glewSearchExtension("GL_ARB_fragment_program_shadow", extStart, extEnd);
#endif /* GL_ARB_fragment_program_shadow */
#ifdef GL_ARB_fragment_shader
  CONST_CAST(GLEW_ARB_fragment_shader) = _glewSearchExtension("GL_ARB_fragment_shader", extStart, extEnd);
#endif /* GL_ARB_fragment_shader */
#ifdef GL_ARB_framebuffer_object
  CONST_CAST(GLEW_ARB_framebuffer_object) = _glewSearchExtension("GL_ARB_framebuffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_framebuffer_object) CONST_CAST(GLEW_ARB_framebuffer_object) = !_glewInit_GL_ARB_framebuffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_framebuffer_object */
#ifdef GL_ARB_framebuffer_sRGB
  CONST_CAST(GLEW_ARB_framebuffer_sRGB) = _glewSearchExtension("GL_ARB_framebuffer_sRGB", extStart, extEnd);
#endif /* GL_ARB_framebuffer_sRGB */
#ifdef GL_ARB_geometry_shader4
  CONST_CAST(GLEW_ARB_geometry_shader4) = _glewSearchExtension("GL_ARB_geometry_shader4", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_geometry_shader4) CONST_CAST(GLEW_ARB_geometry_shader4) = !_glewInit_GL_ARB_geometry_shader4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_geometry_shader4 */
#ifdef GL_ARB_get_program_binary
  CONST_CAST(GLEW_ARB_get_program_binary) = _glewSearchExtension("GL_ARB_get_program_binary", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_get_program_binary) CONST_CAST(GLEW_ARB_get_program_binary) = !_glewInit_GL_ARB_get_program_binary(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_get_program_binary */
#ifdef GL_ARB_gpu_shader5
  CONST_CAST(GLEW_ARB_gpu_shader5) = _glewSearchExtension("GL_ARB_gpu_shader5", extStart, extEnd);
#endif /* GL_ARB_gpu_shader5 */
#ifdef GL_ARB_gpu_shader_fp64
  CONST_CAST(GLEW_ARB_gpu_shader_fp64) = _glewSearchExtension("GL_ARB_gpu_shader_fp64", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_gpu_shader_fp64) CONST_CAST(GLEW_ARB_gpu_shader_fp64) = !_glewInit_GL_ARB_gpu_shader_fp64(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_gpu_shader_fp64 */
#ifdef GL_ARB_half_float_pixel
  CONST_CAST(GLEW_ARB_half_float_pixel) = _glewSearchExtension("GL_ARB_half_float_pixel", extStart, extEnd);
#endif /* GL_ARB_half_float_pixel */
#ifdef GL_ARB_half_float_vertex
  CONST_CAST(GLEW_ARB_half_float_vertex) = _glewSearchExtension("GL_ARB_half_float_vertex", extStart, extEnd);
#endif /* GL_ARB_half_float_vertex */
#ifdef GL_ARB_imaging
  CONST_CAST(GLEW_ARB_imaging) = _glewSearchExtension("GL_ARB_imaging", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_imaging) CONST_CAST(GLEW_ARB_imaging) = !_glewInit_GL_ARB_imaging(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_imaging */
#ifdef GL_ARB_instanced_arrays
  CONST_CAST(GLEW_ARB_instanced_arrays) = _glewSearchExtension("GL_ARB_instanced_arrays", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_instanced_arrays) CONST_CAST(GLEW_ARB_instanced_arrays) = !_glewInit_GL_ARB_instanced_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_instanced_arrays */
#ifdef GL_ARB_internalformat_query
  CONST_CAST(GLEW_ARB_internalformat_query) = _glewSearchExtension("GL_ARB_internalformat_query", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_internalformat_query) CONST_CAST(GLEW_ARB_internalformat_query) = !_glewInit_GL_ARB_internalformat_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_internalformat_query */
#ifdef GL_ARB_map_buffer_alignment
  CONST_CAST(GLEW_ARB_map_buffer_alignment) = _glewSearchExtension("GL_ARB_map_buffer_alignment", extStart, extEnd);
#endif /* GL_ARB_map_buffer_alignment */
#ifdef GL_ARB_map_buffer_range
  CONST_CAST(GLEW_ARB_map_buffer_range) = _glewSearchExtension("GL_ARB_map_buffer_range", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_map_buffer_range) CONST_CAST(GLEW_ARB_map_buffer_range) = !_glewInit_GL_ARB_map_buffer_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_map_buffer_range */
#ifdef GL_ARB_matrix_palette
  CONST_CAST(GLEW_ARB_matrix_palette) = _glewSearchExtension("GL_ARB_matrix_palette", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_matrix_palette) CONST_CAST(GLEW_ARB_matrix_palette) = !_glewInit_GL_ARB_matrix_palette(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_matrix_palette */
#ifdef GL_ARB_multisample
  CONST_CAST(GLEW_ARB_multisample) = _glewSearchExtension("GL_ARB_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_multisample) CONST_CAST(GLEW_ARB_multisample) = !_glewInit_GL_ARB_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_multisample */
#ifdef GL_ARB_multitexture
  CONST_CAST(GLEW_ARB_multitexture) = _glewSearchExtension("GL_ARB_multitexture", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_multitexture) CONST_CAST(GLEW_ARB_multitexture) = !_glewInit_GL_ARB_multitexture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_multitexture */
#ifdef GL_ARB_occlusion_query
  CONST_CAST(GLEW_ARB_occlusion_query) = _glewSearchExtension("GL_ARB_occlusion_query", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_occlusion_query) CONST_CAST(GLEW_ARB_occlusion_query) = !_glewInit_GL_ARB_occlusion_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_occlusion_query */
#ifdef GL_ARB_occlusion_query2
  CONST_CAST(GLEW_ARB_occlusion_query2) = _glewSearchExtension("GL_ARB_occlusion_query2", extStart, extEnd);
#endif /* GL_ARB_occlusion_query2 */
#ifdef GL_ARB_pixel_buffer_object
  CONST_CAST(GLEW_ARB_pixel_buffer_object) = _glewSearchExtension("GL_ARB_pixel_buffer_object", extStart, extEnd);
#endif /* GL_ARB_pixel_buffer_object */
#ifdef GL_ARB_point_parameters
  CONST_CAST(GLEW_ARB_point_parameters) = _glewSearchExtension("GL_ARB_point_parameters", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_point_parameters) CONST_CAST(GLEW_ARB_point_parameters) = !_glewInit_GL_ARB_point_parameters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_point_parameters */
#ifdef GL_ARB_point_sprite
  CONST_CAST(GLEW_ARB_point_sprite) = _glewSearchExtension("GL_ARB_point_sprite", extStart, extEnd);
#endif /* GL_ARB_point_sprite */
#ifdef GL_ARB_provoking_vertex
  CONST_CAST(GLEW_ARB_provoking_vertex) = _glewSearchExtension("GL_ARB_provoking_vertex", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_provoking_vertex) CONST_CAST(GLEW_ARB_provoking_vertex) = !_glewInit_GL_ARB_provoking_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_provoking_vertex */
#ifdef GL_ARB_robustness
  CONST_CAST(GLEW_ARB_robustness) = _glewSearchExtension("GL_ARB_robustness", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_robustness) CONST_CAST(GLEW_ARB_robustness) = !_glewInit_GL_ARB_robustness(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_robustness */
#ifdef GL_ARB_sample_shading
  CONST_CAST(GLEW_ARB_sample_shading) = _glewSearchExtension("GL_ARB_sample_shading", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_sample_shading) CONST_CAST(GLEW_ARB_sample_shading) = !_glewInit_GL_ARB_sample_shading(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_sample_shading */
#ifdef GL_ARB_sampler_objects
  CONST_CAST(GLEW_ARB_sampler_objects) = _glewSearchExtension("GL_ARB_sampler_objects", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_sampler_objects) CONST_CAST(GLEW_ARB_sampler_objects) = !_glewInit_GL_ARB_sampler_objects(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_sampler_objects */
#ifdef GL_ARB_seamless_cube_map
  CONST_CAST(GLEW_ARB_seamless_cube_map) = _glewSearchExtension("GL_ARB_seamless_cube_map", extStart, extEnd);
#endif /* GL_ARB_seamless_cube_map */
#ifdef GL_ARB_separate_shader_objects
  CONST_CAST(GLEW_ARB_separate_shader_objects) = _glewSearchExtension("GL_ARB_separate_shader_objects", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_separate_shader_objects) CONST_CAST(GLEW_ARB_separate_shader_objects) = !_glewInit_GL_ARB_separate_shader_objects(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_separate_shader_objects */
#ifdef GL_ARB_shader_atomic_counters
  CONST_CAST(GLEW_ARB_shader_atomic_counters) = _glewSearchExtension("GL_ARB_shader_atomic_counters", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_shader_atomic_counters) CONST_CAST(GLEW_ARB_shader_atomic_counters) = !_glewInit_GL_ARB_shader_atomic_counters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_shader_atomic_counters */
#ifdef GL_ARB_shader_bit_encoding
  CONST_CAST(GLEW_ARB_shader_bit_encoding) = _glewSearchExtension("GL_ARB_shader_bit_encoding", extStart, extEnd);
#endif /* GL_ARB_shader_bit_encoding */
#ifdef GL_ARB_shader_image_load_store
  CONST_CAST(GLEW_ARB_shader_image_load_store) = _glewSearchExtension("GL_ARB_shader_image_load_store", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_shader_image_load_store) CONST_CAST(GLEW_ARB_shader_image_load_store) = !_glewInit_GL_ARB_shader_image_load_store(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_shader_image_load_store */
#ifdef GL_ARB_shader_objects
  CONST_CAST(GLEW_ARB_shader_objects) = _glewSearchExtension("GL_ARB_shader_objects", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_shader_objects) CONST_CAST(GLEW_ARB_shader_objects) = !_glewInit_GL_ARB_shader_objects(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_shader_objects */
#ifdef GL_ARB_shader_precision
  CONST_CAST(GLEW_ARB_shader_precision) = _glewSearchExtension("GL_ARB_shader_precision", extStart, extEnd);
#endif /* GL_ARB_shader_precision */
#ifdef GL_ARB_shader_stencil_export
  CONST_CAST(GLEW_ARB_shader_stencil_export) = _glewSearchExtension("GL_ARB_shader_stencil_export", extStart, extEnd);
#endif /* GL_ARB_shader_stencil_export */
#ifdef GL_ARB_shader_subroutine
  CONST_CAST(GLEW_ARB_shader_subroutine) = _glewSearchExtension("GL_ARB_shader_subroutine", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_shader_subroutine) CONST_CAST(GLEW_ARB_shader_subroutine) = !_glewInit_GL_ARB_shader_subroutine(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_shader_subroutine */
#ifdef GL_ARB_shader_texture_lod
  CONST_CAST(GLEW_ARB_shader_texture_lod) = _glewSearchExtension("GL_ARB_shader_texture_lod", extStart, extEnd);
#endif /* GL_ARB_shader_texture_lod */
#ifdef GL_ARB_shading_language_100
  CONST_CAST(GLEW_ARB_shading_language_100) = _glewSearchExtension("GL_ARB_shading_language_100", extStart, extEnd);
#endif /* GL_ARB_shading_language_100 */
#ifdef GL_ARB_shading_language_420pack
  CONST_CAST(GLEW_ARB_shading_language_420pack) = _glewSearchExtension("GL_ARB_shading_language_420pack", extStart, extEnd);
#endif /* GL_ARB_shading_language_420pack */
#ifdef GL_ARB_shading_language_include
  CONST_CAST(GLEW_ARB_shading_language_include) = _glewSearchExtension("GL_ARB_shading_language_include", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_shading_language_include) CONST_CAST(GLEW_ARB_shading_language_include) = !_glewInit_GL_ARB_shading_language_include(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_shading_language_include */
#ifdef GL_ARB_shading_language_packing
  CONST_CAST(GLEW_ARB_shading_language_packing) = _glewSearchExtension("GL_ARB_shading_language_packing", extStart, extEnd);
#endif /* GL_ARB_shading_language_packing */
#ifdef GL_ARB_shadow
  CONST_CAST(GLEW_ARB_shadow) = _glewSearchExtension("GL_ARB_shadow", extStart, extEnd);
#endif /* GL_ARB_shadow */
#ifdef GL_ARB_shadow_ambient
  CONST_CAST(GLEW_ARB_shadow_ambient) = _glewSearchExtension("GL_ARB_shadow_ambient", extStart, extEnd);
#endif /* GL_ARB_shadow_ambient */
#ifdef GL_ARB_sync
  CONST_CAST(GLEW_ARB_sync) = _glewSearchExtension("GL_ARB_sync", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_sync) CONST_CAST(GLEW_ARB_sync) = !_glewInit_GL_ARB_sync(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_sync */
#ifdef GL_ARB_tessellation_shader
  CONST_CAST(GLEW_ARB_tessellation_shader) = _glewSearchExtension("GL_ARB_tessellation_shader", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_tessellation_shader) CONST_CAST(GLEW_ARB_tessellation_shader) = !_glewInit_GL_ARB_tessellation_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_tessellation_shader */
#ifdef GL_ARB_texture_border_clamp
  CONST_CAST(GLEW_ARB_texture_border_clamp) = _glewSearchExtension("GL_ARB_texture_border_clamp", extStart, extEnd);
#endif /* GL_ARB_texture_border_clamp */
#ifdef GL_ARB_texture_buffer_object
  CONST_CAST(GLEW_ARB_texture_buffer_object) = _glewSearchExtension("GL_ARB_texture_buffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_texture_buffer_object) CONST_CAST(GLEW_ARB_texture_buffer_object) = !_glewInit_GL_ARB_texture_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_texture_buffer_object */
#ifdef GL_ARB_texture_buffer_object_rgb32
  CONST_CAST(GLEW_ARB_texture_buffer_object_rgb32) = _glewSearchExtension("GL_ARB_texture_buffer_object_rgb32", extStart, extEnd);
#endif /* GL_ARB_texture_buffer_object_rgb32 */
#ifdef GL_ARB_texture_compression
  CONST_CAST(GLEW_ARB_texture_compression) = _glewSearchExtension("GL_ARB_texture_compression", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_texture_compression) CONST_CAST(GLEW_ARB_texture_compression) = !_glewInit_GL_ARB_texture_compression(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_texture_compression */
#ifdef GL_ARB_texture_compression_bptc
  CONST_CAST(GLEW_ARB_texture_compression_bptc) = _glewSearchExtension("GL_ARB_texture_compression_bptc", extStart, extEnd);
#endif /* GL_ARB_texture_compression_bptc */
#ifdef GL_ARB_texture_compression_rgtc
  CONST_CAST(GLEW_ARB_texture_compression_rgtc) = _glewSearchExtension("GL_ARB_texture_compression_rgtc", extStart, extEnd);
#endif /* GL_ARB_texture_compression_rgtc */
#ifdef GL_ARB_texture_cube_map
  CONST_CAST(GLEW_ARB_texture_cube_map) = _glewSearchExtension("GL_ARB_texture_cube_map", extStart, extEnd);
#endif /* GL_ARB_texture_cube_map */
#ifdef GL_ARB_texture_cube_map_array
  CONST_CAST(GLEW_ARB_texture_cube_map_array) = _glewSearchExtension("GL_ARB_texture_cube_map_array", extStart, extEnd);
#endif /* GL_ARB_texture_cube_map_array */
#ifdef GL_ARB_texture_env_add
  CONST_CAST(GLEW_ARB_texture_env_add) = _glewSearchExtension("GL_ARB_texture_env_add", extStart, extEnd);
#endif /* GL_ARB_texture_env_add */
#ifdef GL_ARB_texture_env_combine
  CONST_CAST(GLEW_ARB_texture_env_combine) = _glewSearchExtension("GL_ARB_texture_env_combine", extStart, extEnd);
#endif /* GL_ARB_texture_env_combine */
#ifdef GL_ARB_texture_env_crossbar
  CONST_CAST(GLEW_ARB_texture_env_crossbar) = _glewSearchExtension("GL_ARB_texture_env_crossbar", extStart, extEnd);
#endif /* GL_ARB_texture_env_crossbar */
#ifdef GL_ARB_texture_env_dot3
  CONST_CAST(GLEW_ARB_texture_env_dot3) = _glewSearchExtension("GL_ARB_texture_env_dot3", extStart, extEnd);
#endif /* GL_ARB_texture_env_dot3 */
#ifdef GL_ARB_texture_float
  CONST_CAST(GLEW_ARB_texture_float) = _glewSearchExtension("GL_ARB_texture_float", extStart, extEnd);
#endif /* GL_ARB_texture_float */
#ifdef GL_ARB_texture_gather
  CONST_CAST(GLEW_ARB_texture_gather) = _glewSearchExtension("GL_ARB_texture_gather", extStart, extEnd);
#endif /* GL_ARB_texture_gather */
#ifdef GL_ARB_texture_mirrored_repeat
  CONST_CAST(GLEW_ARB_texture_mirrored_repeat) = _glewSearchExtension("GL_ARB_texture_mirrored_repeat", extStart, extEnd);
#endif /* GL_ARB_texture_mirrored_repeat */
#ifdef GL_ARB_texture_multisample
  CONST_CAST(GLEW_ARB_texture_multisample) = _glewSearchExtension("GL_ARB_texture_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_texture_multisample) CONST_CAST(GLEW_ARB_texture_multisample) = !_glewInit_GL_ARB_texture_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_texture_multisample */
#ifdef GL_ARB_texture_non_power_of_two
  CONST_CAST(GLEW_ARB_texture_non_power_of_two) = _glewSearchExtension("GL_ARB_texture_non_power_of_two", extStart, extEnd);
#endif /* GL_ARB_texture_non_power_of_two */
#ifdef GL_ARB_texture_query_lod
  CONST_CAST(GLEW_ARB_texture_query_lod) = _glewSearchExtension("GL_ARB_texture_query_lod", extStart, extEnd);
#endif /* GL_ARB_texture_query_lod */
#ifdef GL_ARB_texture_rectangle
  CONST_CAST(GLEW_ARB_texture_rectangle) = _glewSearchExtension("GL_ARB_texture_rectangle", extStart, extEnd);
#endif /* GL_ARB_texture_rectangle */
#ifdef GL_ARB_texture_rg
  CONST_CAST(GLEW_ARB_texture_rg) = _glewSearchExtension("GL_ARB_texture_rg", extStart, extEnd);
#endif /* GL_ARB_texture_rg */
#ifdef GL_ARB_texture_rgb10_a2ui
  CONST_CAST(GLEW_ARB_texture_rgb10_a2ui) = _glewSearchExtension("GL_ARB_texture_rgb10_a2ui", extStart, extEnd);
#endif /* GL_ARB_texture_rgb10_a2ui */
#ifdef GL_ARB_texture_storage
  CONST_CAST(GLEW_ARB_texture_storage) = _glewSearchExtension("GL_ARB_texture_storage", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_texture_storage) CONST_CAST(GLEW_ARB_texture_storage) = !_glewInit_GL_ARB_texture_storage(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_texture_storage */
#ifdef GL_ARB_texture_swizzle
  CONST_CAST(GLEW_ARB_texture_swizzle) = _glewSearchExtension("GL_ARB_texture_swizzle", extStart, extEnd);
#endif /* GL_ARB_texture_swizzle */
#ifdef GL_ARB_timer_query
  CONST_CAST(GLEW_ARB_timer_query) = _glewSearchExtension("GL_ARB_timer_query", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_timer_query) CONST_CAST(GLEW_ARB_timer_query) = !_glewInit_GL_ARB_timer_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_timer_query */
#ifdef GL_ARB_transform_feedback2
  CONST_CAST(GLEW_ARB_transform_feedback2) = _glewSearchExtension("GL_ARB_transform_feedback2", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_transform_feedback2) CONST_CAST(GLEW_ARB_transform_feedback2) = !_glewInit_GL_ARB_transform_feedback2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_transform_feedback2 */
#ifdef GL_ARB_transform_feedback3
  CONST_CAST(GLEW_ARB_transform_feedback3) = _glewSearchExtension("GL_ARB_transform_feedback3", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_transform_feedback3) CONST_CAST(GLEW_ARB_transform_feedback3) = !_glewInit_GL_ARB_transform_feedback3(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_transform_feedback3 */
#ifdef GL_ARB_transform_feedback_instanced
  CONST_CAST(GLEW_ARB_transform_feedback_instanced) = _glewSearchExtension("GL_ARB_transform_feedback_instanced", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_transform_feedback_instanced) CONST_CAST(GLEW_ARB_transform_feedback_instanced) = !_glewInit_GL_ARB_transform_feedback_instanced(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_transform_feedback_instanced */
#ifdef GL_ARB_transpose_matrix
  CONST_CAST(GLEW_ARB_transpose_matrix) = _glewSearchExtension("GL_ARB_transpose_matrix", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_transpose_matrix) CONST_CAST(GLEW_ARB_transpose_matrix) = !_glewInit_GL_ARB_transpose_matrix(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_transpose_matrix */
#ifdef GL_ARB_uniform_buffer_object
  CONST_CAST(GLEW_ARB_uniform_buffer_object) = _glewSearchExtension("GL_ARB_uniform_buffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_uniform_buffer_object) CONST_CAST(GLEW_ARB_uniform_buffer_object) = !_glewInit_GL_ARB_uniform_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_uniform_buffer_object */
#ifdef GL_ARB_vertex_array_bgra
  CONST_CAST(GLEW_ARB_vertex_array_bgra) = _glewSearchExtension("GL_ARB_vertex_array_bgra", extStart, extEnd);
#endif /* GL_ARB_vertex_array_bgra */
#ifdef GL_ARB_vertex_array_object
  CONST_CAST(GLEW_ARB_vertex_array_object) = _glewSearchExtension("GL_ARB_vertex_array_object", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_array_object) CONST_CAST(GLEW_ARB_vertex_array_object) = !_glewInit_GL_ARB_vertex_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_array_object */
#ifdef GL_ARB_vertex_attrib_64bit
  CONST_CAST(GLEW_ARB_vertex_attrib_64bit) = _glewSearchExtension("GL_ARB_vertex_attrib_64bit", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_attrib_64bit) CONST_CAST(GLEW_ARB_vertex_attrib_64bit) = !_glewInit_GL_ARB_vertex_attrib_64bit(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_attrib_64bit */
#ifdef GL_ARB_vertex_blend
  CONST_CAST(GLEW_ARB_vertex_blend) = _glewSearchExtension("GL_ARB_vertex_blend", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_blend) CONST_CAST(GLEW_ARB_vertex_blend) = !_glewInit_GL_ARB_vertex_blend(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_blend */
#ifdef GL_ARB_vertex_buffer_object
  CONST_CAST(GLEW_ARB_vertex_buffer_object) = _glewSearchExtension("GL_ARB_vertex_buffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_buffer_object) CONST_CAST(GLEW_ARB_vertex_buffer_object) = !_glewInit_GL_ARB_vertex_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_buffer_object */
#ifdef GL_ARB_vertex_program
  CONST_CAST(GLEW_ARB_vertex_program) = _glewSearchExtension("GL_ARB_vertex_program", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_program) CONST_CAST(GLEW_ARB_vertex_program) = !_glewInit_GL_ARB_vertex_program(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_program */
#ifdef GL_ARB_vertex_shader
  CONST_CAST(GLEW_ARB_vertex_shader) = _glewSearchExtension("GL_ARB_vertex_shader", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_shader) CONST_CAST(GLEW_ARB_vertex_shader) = !_glewInit_GL_ARB_vertex_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_shader */
#ifdef GL_ARB_vertex_type_2_10_10_10_rev
  CONST_CAST(GLEW_ARB_vertex_type_2_10_10_10_rev) = _glewSearchExtension("GL_ARB_vertex_type_2_10_10_10_rev", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_vertex_type_2_10_10_10_rev) CONST_CAST(GLEW_ARB_vertex_type_2_10_10_10_rev) = !_glewInit_GL_ARB_vertex_type_2_10_10_10_rev(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_vertex_type_2_10_10_10_rev */
#ifdef GL_ARB_viewport_array
  CONST_CAST(GLEW_ARB_viewport_array) = _glewSearchExtension("GL_ARB_viewport_array", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_viewport_array) CONST_CAST(GLEW_ARB_viewport_array) = !_glewInit_GL_ARB_viewport_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_viewport_array */
#ifdef GL_ARB_window_pos
  CONST_CAST(GLEW_ARB_window_pos) = _glewSearchExtension("GL_ARB_window_pos", extStart, extEnd);
  if (glewExperimental || GLEW_ARB_window_pos) CONST_CAST(GLEW_ARB_window_pos) = !_glewInit_GL_ARB_window_pos(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ARB_window_pos */
#ifdef GL_ATIX_point_sprites
  CONST_CAST(GLEW_ATIX_point_sprites) = _glewSearchExtension("GL_ATIX_point_sprites", extStart, extEnd);
#endif /* GL_ATIX_point_sprites */
#ifdef GL_ATIX_texture_env_combine3
  CONST_CAST(GLEW_ATIX_texture_env_combine3) = _glewSearchExtension("GL_ATIX_texture_env_combine3", extStart, extEnd);
#endif /* GL_ATIX_texture_env_combine3 */
#ifdef GL_ATIX_texture_env_route
  CONST_CAST(GLEW_ATIX_texture_env_route) = _glewSearchExtension("GL_ATIX_texture_env_route", extStart, extEnd);
#endif /* GL_ATIX_texture_env_route */
#ifdef GL_ATIX_vertex_shader_output_point_size
  CONST_CAST(GLEW_ATIX_vertex_shader_output_point_size) = _glewSearchExtension("GL_ATIX_vertex_shader_output_point_size", extStart, extEnd);
#endif /* GL_ATIX_vertex_shader_output_point_size */
#ifdef GL_ATI_draw_buffers
  CONST_CAST(GLEW_ATI_draw_buffers) = _glewSearchExtension("GL_ATI_draw_buffers", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_draw_buffers) CONST_CAST(GLEW_ATI_draw_buffers) = !_glewInit_GL_ATI_draw_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_draw_buffers */
#ifdef GL_ATI_element_array
  CONST_CAST(GLEW_ATI_element_array) = _glewSearchExtension("GL_ATI_element_array", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_element_array) CONST_CAST(GLEW_ATI_element_array) = !_glewInit_GL_ATI_element_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_element_array */
#ifdef GL_ATI_envmap_bumpmap
  CONST_CAST(GLEW_ATI_envmap_bumpmap) = _glewSearchExtension("GL_ATI_envmap_bumpmap", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_envmap_bumpmap) CONST_CAST(GLEW_ATI_envmap_bumpmap) = !_glewInit_GL_ATI_envmap_bumpmap(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_envmap_bumpmap */
#ifdef GL_ATI_fragment_shader
  CONST_CAST(GLEW_ATI_fragment_shader) = _glewSearchExtension("GL_ATI_fragment_shader", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_fragment_shader) CONST_CAST(GLEW_ATI_fragment_shader) = !_glewInit_GL_ATI_fragment_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_fragment_shader */
#ifdef GL_ATI_map_object_buffer
  CONST_CAST(GLEW_ATI_map_object_buffer) = _glewSearchExtension("GL_ATI_map_object_buffer", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_map_object_buffer) CONST_CAST(GLEW_ATI_map_object_buffer) = !_glewInit_GL_ATI_map_object_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_map_object_buffer */
#ifdef GL_ATI_meminfo
  CONST_CAST(GLEW_ATI_meminfo) = _glewSearchExtension("GL_ATI_meminfo", extStart, extEnd);
#endif /* GL_ATI_meminfo */
#ifdef GL_ATI_pn_triangles
  CONST_CAST(GLEW_ATI_pn_triangles) = _glewSearchExtension("GL_ATI_pn_triangles", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_pn_triangles) CONST_CAST(GLEW_ATI_pn_triangles) = !_glewInit_GL_ATI_pn_triangles(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_pn_triangles */
#ifdef GL_ATI_separate_stencil
  CONST_CAST(GLEW_ATI_separate_stencil) = _glewSearchExtension("GL_ATI_separate_stencil", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_separate_stencil) CONST_CAST(GLEW_ATI_separate_stencil) = !_glewInit_GL_ATI_separate_stencil(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_separate_stencil */
#ifdef GL_ATI_shader_texture_lod
  CONST_CAST(GLEW_ATI_shader_texture_lod) = _glewSearchExtension("GL_ATI_shader_texture_lod", extStart, extEnd);
#endif /* GL_ATI_shader_texture_lod */
#ifdef GL_ATI_text_fragment_shader
  CONST_CAST(GLEW_ATI_text_fragment_shader) = _glewSearchExtension("GL_ATI_text_fragment_shader", extStart, extEnd);
#endif /* GL_ATI_text_fragment_shader */
#ifdef GL_ATI_texture_compression_3dc
  CONST_CAST(GLEW_ATI_texture_compression_3dc) = _glewSearchExtension("GL_ATI_texture_compression_3dc", extStart, extEnd);
#endif /* GL_ATI_texture_compression_3dc */
#ifdef GL_ATI_texture_env_combine3
  CONST_CAST(GLEW_ATI_texture_env_combine3) = _glewSearchExtension("GL_ATI_texture_env_combine3", extStart, extEnd);
#endif /* GL_ATI_texture_env_combine3 */
#ifdef GL_ATI_texture_float
  CONST_CAST(GLEW_ATI_texture_float) = _glewSearchExtension("GL_ATI_texture_float", extStart, extEnd);
#endif /* GL_ATI_texture_float */
#ifdef GL_ATI_texture_mirror_once
  CONST_CAST(GLEW_ATI_texture_mirror_once) = _glewSearchExtension("GL_ATI_texture_mirror_once", extStart, extEnd);
#endif /* GL_ATI_texture_mirror_once */
#ifdef GL_ATI_vertex_array_object
  CONST_CAST(GLEW_ATI_vertex_array_object) = _glewSearchExtension("GL_ATI_vertex_array_object", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_vertex_array_object) CONST_CAST(GLEW_ATI_vertex_array_object) = !_glewInit_GL_ATI_vertex_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_vertex_array_object */
#ifdef GL_ATI_vertex_attrib_array_object
  CONST_CAST(GLEW_ATI_vertex_attrib_array_object) = _glewSearchExtension("GL_ATI_vertex_attrib_array_object", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_vertex_attrib_array_object) CONST_CAST(GLEW_ATI_vertex_attrib_array_object) = !_glewInit_GL_ATI_vertex_attrib_array_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_vertex_attrib_array_object */
#ifdef GL_ATI_vertex_streams
  CONST_CAST(GLEW_ATI_vertex_streams) = _glewSearchExtension("GL_ATI_vertex_streams", extStart, extEnd);
  if (glewExperimental || GLEW_ATI_vertex_streams) CONST_CAST(GLEW_ATI_vertex_streams) = !_glewInit_GL_ATI_vertex_streams(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_ATI_vertex_streams */
#ifdef GL_EXT_422_pixels
  CONST_CAST(GLEW_EXT_422_pixels) = _glewSearchExtension("GL_EXT_422_pixels", extStart, extEnd);
#endif /* GL_EXT_422_pixels */
#ifdef GL_EXT_Cg_shader
  CONST_CAST(GLEW_EXT_Cg_shader) = _glewSearchExtension("GL_EXT_Cg_shader", extStart, extEnd);
#endif /* GL_EXT_Cg_shader */
#ifdef GL_EXT_abgr
  CONST_CAST(GLEW_EXT_abgr) = _glewSearchExtension("GL_EXT_abgr", extStart, extEnd);
#endif /* GL_EXT_abgr */
#ifdef GL_EXT_bgra
  CONST_CAST(GLEW_EXT_bgra) = _glewSearchExtension("GL_EXT_bgra", extStart, extEnd);
#endif /* GL_EXT_bgra */
#ifdef GL_EXT_bindable_uniform
  CONST_CAST(GLEW_EXT_bindable_uniform) = _glewSearchExtension("GL_EXT_bindable_uniform", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_bindable_uniform) CONST_CAST(GLEW_EXT_bindable_uniform) = !_glewInit_GL_EXT_bindable_uniform(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_bindable_uniform */
#ifdef GL_EXT_blend_color
  CONST_CAST(GLEW_EXT_blend_color) = _glewSearchExtension("GL_EXT_blend_color", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_blend_color) CONST_CAST(GLEW_EXT_blend_color) = !_glewInit_GL_EXT_blend_color(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_color */
#ifdef GL_EXT_blend_equation_separate
  CONST_CAST(GLEW_EXT_blend_equation_separate) = _glewSearchExtension("GL_EXT_blend_equation_separate", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_blend_equation_separate) CONST_CAST(GLEW_EXT_blend_equation_separate) = !_glewInit_GL_EXT_blend_equation_separate(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_equation_separate */
#ifdef GL_EXT_blend_func_separate
  CONST_CAST(GLEW_EXT_blend_func_separate) = _glewSearchExtension("GL_EXT_blend_func_separate", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_blend_func_separate) CONST_CAST(GLEW_EXT_blend_func_separate) = !_glewInit_GL_EXT_blend_func_separate(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_func_separate */
#ifdef GL_EXT_blend_logic_op
  CONST_CAST(GLEW_EXT_blend_logic_op) = _glewSearchExtension("GL_EXT_blend_logic_op", extStart, extEnd);
#endif /* GL_EXT_blend_logic_op */
#ifdef GL_EXT_blend_minmax
  CONST_CAST(GLEW_EXT_blend_minmax) = _glewSearchExtension("GL_EXT_blend_minmax", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_blend_minmax) CONST_CAST(GLEW_EXT_blend_minmax) = !_glewInit_GL_EXT_blend_minmax(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_blend_minmax */
#ifdef GL_EXT_blend_subtract
  CONST_CAST(GLEW_EXT_blend_subtract) = _glewSearchExtension("GL_EXT_blend_subtract", extStart, extEnd);
#endif /* GL_EXT_blend_subtract */
#ifdef GL_EXT_clip_volume_hint
  CONST_CAST(GLEW_EXT_clip_volume_hint) = _glewSearchExtension("GL_EXT_clip_volume_hint", extStart, extEnd);
#endif /* GL_EXT_clip_volume_hint */
#ifdef GL_EXT_cmyka
  CONST_CAST(GLEW_EXT_cmyka) = _glewSearchExtension("GL_EXT_cmyka", extStart, extEnd);
#endif /* GL_EXT_cmyka */
#ifdef GL_EXT_color_subtable
  CONST_CAST(GLEW_EXT_color_subtable) = _glewSearchExtension("GL_EXT_color_subtable", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_color_subtable) CONST_CAST(GLEW_EXT_color_subtable) = !_glewInit_GL_EXT_color_subtable(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_color_subtable */
#ifdef GL_EXT_compiled_vertex_array
  CONST_CAST(GLEW_EXT_compiled_vertex_array) = _glewSearchExtension("GL_EXT_compiled_vertex_array", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_compiled_vertex_array) CONST_CAST(GLEW_EXT_compiled_vertex_array) = !_glewInit_GL_EXT_compiled_vertex_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_compiled_vertex_array */
#ifdef GL_EXT_convolution
  CONST_CAST(GLEW_EXT_convolution) = _glewSearchExtension("GL_EXT_convolution", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_convolution) CONST_CAST(GLEW_EXT_convolution) = !_glewInit_GL_EXT_convolution(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_convolution */
#ifdef GL_EXT_coordinate_frame
  CONST_CAST(GLEW_EXT_coordinate_frame) = _glewSearchExtension("GL_EXT_coordinate_frame", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_coordinate_frame) CONST_CAST(GLEW_EXT_coordinate_frame) = !_glewInit_GL_EXT_coordinate_frame(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_coordinate_frame */
#ifdef GL_EXT_copy_texture
  CONST_CAST(GLEW_EXT_copy_texture) = _glewSearchExtension("GL_EXT_copy_texture", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_copy_texture) CONST_CAST(GLEW_EXT_copy_texture) = !_glewInit_GL_EXT_copy_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_copy_texture */
#ifdef GL_EXT_cull_vertex
  CONST_CAST(GLEW_EXT_cull_vertex) = _glewSearchExtension("GL_EXT_cull_vertex", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_cull_vertex) CONST_CAST(GLEW_EXT_cull_vertex) = !_glewInit_GL_EXT_cull_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_cull_vertex */
#ifdef GL_EXT_depth_bounds_test
  CONST_CAST(GLEW_EXT_depth_bounds_test) = _glewSearchExtension("GL_EXT_depth_bounds_test", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_depth_bounds_test) CONST_CAST(GLEW_EXT_depth_bounds_test) = !_glewInit_GL_EXT_depth_bounds_test(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_depth_bounds_test */
#ifdef GL_EXT_direct_state_access
  CONST_CAST(GLEW_EXT_direct_state_access) = _glewSearchExtension("GL_EXT_direct_state_access", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_direct_state_access) CONST_CAST(GLEW_EXT_direct_state_access) = !_glewInit_GL_EXT_direct_state_access(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_direct_state_access */
#ifdef GL_EXT_draw_buffers2
  CONST_CAST(GLEW_EXT_draw_buffers2) = _glewSearchExtension("GL_EXT_draw_buffers2", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_draw_buffers2) CONST_CAST(GLEW_EXT_draw_buffers2) = !_glewInit_GL_EXT_draw_buffers2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_draw_buffers2 */
#ifdef GL_EXT_draw_instanced
  CONST_CAST(GLEW_EXT_draw_instanced) = _glewSearchExtension("GL_EXT_draw_instanced", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_draw_instanced) CONST_CAST(GLEW_EXT_draw_instanced) = !_glewInit_GL_EXT_draw_instanced(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_draw_instanced */
#ifdef GL_EXT_draw_range_elements
  CONST_CAST(GLEW_EXT_draw_range_elements) = _glewSearchExtension("GL_EXT_draw_range_elements", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_draw_range_elements) CONST_CAST(GLEW_EXT_draw_range_elements) = !_glewInit_GL_EXT_draw_range_elements(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_draw_range_elements */
#ifdef GL_EXT_fog_coord
  CONST_CAST(GLEW_EXT_fog_coord) = _glewSearchExtension("GL_EXT_fog_coord", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_fog_coord) CONST_CAST(GLEW_EXT_fog_coord) = !_glewInit_GL_EXT_fog_coord(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_fog_coord */
#ifdef GL_EXT_fragment_lighting
  CONST_CAST(GLEW_EXT_fragment_lighting) = _glewSearchExtension("GL_EXT_fragment_lighting", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_fragment_lighting) CONST_CAST(GLEW_EXT_fragment_lighting) = !_glewInit_GL_EXT_fragment_lighting(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_fragment_lighting */
#ifdef GL_EXT_framebuffer_blit
  CONST_CAST(GLEW_EXT_framebuffer_blit) = _glewSearchExtension("GL_EXT_framebuffer_blit", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_framebuffer_blit) CONST_CAST(GLEW_EXT_framebuffer_blit) = !_glewInit_GL_EXT_framebuffer_blit(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_framebuffer_blit */
#ifdef GL_EXT_framebuffer_multisample
  CONST_CAST(GLEW_EXT_framebuffer_multisample) = _glewSearchExtension("GL_EXT_framebuffer_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_framebuffer_multisample) CONST_CAST(GLEW_EXT_framebuffer_multisample) = !_glewInit_GL_EXT_framebuffer_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_framebuffer_multisample */
#ifdef GL_EXT_framebuffer_multisample_blit_scaled
  CONST_CAST(GLEW_EXT_framebuffer_multisample_blit_scaled) = _glewSearchExtension("GL_EXT_framebuffer_multisample_blit_scaled", extStart, extEnd);
#endif /* GL_EXT_framebuffer_multisample_blit_scaled */
#ifdef GL_EXT_framebuffer_object
  CONST_CAST(GLEW_EXT_framebuffer_object) = _glewSearchExtension("GL_EXT_framebuffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_framebuffer_object) CONST_CAST(GLEW_EXT_framebuffer_object) = !_glewInit_GL_EXT_framebuffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_framebuffer_object */
#ifdef GL_EXT_framebuffer_sRGB
  CONST_CAST(GLEW_EXT_framebuffer_sRGB) = _glewSearchExtension("GL_EXT_framebuffer_sRGB", extStart, extEnd);
#endif /* GL_EXT_framebuffer_sRGB */
#ifdef GL_EXT_geometry_shader4
  CONST_CAST(GLEW_EXT_geometry_shader4) = _glewSearchExtension("GL_EXT_geometry_shader4", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_geometry_shader4) CONST_CAST(GLEW_EXT_geometry_shader4) = !_glewInit_GL_EXT_geometry_shader4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_geometry_shader4 */
#ifdef GL_EXT_gpu_program_parameters
  CONST_CAST(GLEW_EXT_gpu_program_parameters) = _glewSearchExtension("GL_EXT_gpu_program_parameters", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_gpu_program_parameters) CONST_CAST(GLEW_EXT_gpu_program_parameters) = !_glewInit_GL_EXT_gpu_program_parameters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_gpu_program_parameters */
#ifdef GL_EXT_gpu_shader4
  CONST_CAST(GLEW_EXT_gpu_shader4) = _glewSearchExtension("GL_EXT_gpu_shader4", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_gpu_shader4) CONST_CAST(GLEW_EXT_gpu_shader4) = !_glewInit_GL_EXT_gpu_shader4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_gpu_shader4 */
#ifdef GL_EXT_histogram
  CONST_CAST(GLEW_EXT_histogram) = _glewSearchExtension("GL_EXT_histogram", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_histogram) CONST_CAST(GLEW_EXT_histogram) = !_glewInit_GL_EXT_histogram(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_histogram */
#ifdef GL_EXT_index_array_formats
  CONST_CAST(GLEW_EXT_index_array_formats) = _glewSearchExtension("GL_EXT_index_array_formats", extStart, extEnd);
#endif /* GL_EXT_index_array_formats */
#ifdef GL_EXT_index_func
  CONST_CAST(GLEW_EXT_index_func) = _glewSearchExtension("GL_EXT_index_func", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_index_func) CONST_CAST(GLEW_EXT_index_func) = !_glewInit_GL_EXT_index_func(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_index_func */
#ifdef GL_EXT_index_material
  CONST_CAST(GLEW_EXT_index_material) = _glewSearchExtension("GL_EXT_index_material", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_index_material) CONST_CAST(GLEW_EXT_index_material) = !_glewInit_GL_EXT_index_material(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_index_material */
#ifdef GL_EXT_index_texture
  CONST_CAST(GLEW_EXT_index_texture) = _glewSearchExtension("GL_EXT_index_texture", extStart, extEnd);
#endif /* GL_EXT_index_texture */
#ifdef GL_EXT_light_texture
  CONST_CAST(GLEW_EXT_light_texture) = _glewSearchExtension("GL_EXT_light_texture", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_light_texture) CONST_CAST(GLEW_EXT_light_texture) = !_glewInit_GL_EXT_light_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_light_texture */
#ifdef GL_EXT_misc_attribute
  CONST_CAST(GLEW_EXT_misc_attribute) = _glewSearchExtension("GL_EXT_misc_attribute", extStart, extEnd);
#endif /* GL_EXT_misc_attribute */
#ifdef GL_EXT_multi_draw_arrays
  CONST_CAST(GLEW_EXT_multi_draw_arrays) = _glewSearchExtension("GL_EXT_multi_draw_arrays", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_multi_draw_arrays) CONST_CAST(GLEW_EXT_multi_draw_arrays) = !_glewInit_GL_EXT_multi_draw_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_multi_draw_arrays */
#ifdef GL_EXT_multisample
  CONST_CAST(GLEW_EXT_multisample) = _glewSearchExtension("GL_EXT_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_multisample) CONST_CAST(GLEW_EXT_multisample) = !_glewInit_GL_EXT_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_multisample */
#ifdef GL_EXT_packed_depth_stencil
  CONST_CAST(GLEW_EXT_packed_depth_stencil) = _glewSearchExtension("GL_EXT_packed_depth_stencil", extStart, extEnd);
#endif /* GL_EXT_packed_depth_stencil */
#ifdef GL_EXT_packed_float
  CONST_CAST(GLEW_EXT_packed_float) = _glewSearchExtension("GL_EXT_packed_float", extStart, extEnd);
#endif /* GL_EXT_packed_float */
#ifdef GL_EXT_packed_pixels
  CONST_CAST(GLEW_EXT_packed_pixels) = _glewSearchExtension("GL_EXT_packed_pixels", extStart, extEnd);
#endif /* GL_EXT_packed_pixels */
#ifdef GL_EXT_paletted_texture
  CONST_CAST(GLEW_EXT_paletted_texture) = _glewSearchExtension("GL_EXT_paletted_texture", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_paletted_texture) CONST_CAST(GLEW_EXT_paletted_texture) = !_glewInit_GL_EXT_paletted_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_paletted_texture */
#ifdef GL_EXT_pixel_buffer_object
  CONST_CAST(GLEW_EXT_pixel_buffer_object) = _glewSearchExtension("GL_EXT_pixel_buffer_object", extStart, extEnd);
#endif /* GL_EXT_pixel_buffer_object */
#ifdef GL_EXT_pixel_transform
  CONST_CAST(GLEW_EXT_pixel_transform) = _glewSearchExtension("GL_EXT_pixel_transform", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_pixel_transform) CONST_CAST(GLEW_EXT_pixel_transform) = !_glewInit_GL_EXT_pixel_transform(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_pixel_transform */
#ifdef GL_EXT_pixel_transform_color_table
  CONST_CAST(GLEW_EXT_pixel_transform_color_table) = _glewSearchExtension("GL_EXT_pixel_transform_color_table", extStart, extEnd);
#endif /* GL_EXT_pixel_transform_color_table */
#ifdef GL_EXT_point_parameters
  CONST_CAST(GLEW_EXT_point_parameters) = _glewSearchExtension("GL_EXT_point_parameters", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_point_parameters) CONST_CAST(GLEW_EXT_point_parameters) = !_glewInit_GL_EXT_point_parameters(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_point_parameters */
#ifdef GL_EXT_polygon_offset
  CONST_CAST(GLEW_EXT_polygon_offset) = _glewSearchExtension("GL_EXT_polygon_offset", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_polygon_offset) CONST_CAST(GLEW_EXT_polygon_offset) = !_glewInit_GL_EXT_polygon_offset(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_polygon_offset */
#ifdef GL_EXT_provoking_vertex
  CONST_CAST(GLEW_EXT_provoking_vertex) = _glewSearchExtension("GL_EXT_provoking_vertex", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_provoking_vertex) CONST_CAST(GLEW_EXT_provoking_vertex) = !_glewInit_GL_EXT_provoking_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_provoking_vertex */
#ifdef GL_EXT_rescale_normal
  CONST_CAST(GLEW_EXT_rescale_normal) = _glewSearchExtension("GL_EXT_rescale_normal", extStart, extEnd);
#endif /* GL_EXT_rescale_normal */
#ifdef GL_EXT_scene_marker
  CONST_CAST(GLEW_EXT_scene_marker) = _glewSearchExtension("GL_EXT_scene_marker", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_scene_marker) CONST_CAST(GLEW_EXT_scene_marker) = !_glewInit_GL_EXT_scene_marker(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_scene_marker */
#ifdef GL_EXT_secondary_color
  CONST_CAST(GLEW_EXT_secondary_color) = _glewSearchExtension("GL_EXT_secondary_color", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_secondary_color) CONST_CAST(GLEW_EXT_secondary_color) = !_glewInit_GL_EXT_secondary_color(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_secondary_color */
#ifdef GL_EXT_separate_shader_objects
  CONST_CAST(GLEW_EXT_separate_shader_objects) = _glewSearchExtension("GL_EXT_separate_shader_objects", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_separate_shader_objects) CONST_CAST(GLEW_EXT_separate_shader_objects) = !_glewInit_GL_EXT_separate_shader_objects(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_separate_shader_objects */
#ifdef GL_EXT_separate_specular_color
  CONST_CAST(GLEW_EXT_separate_specular_color) = _glewSearchExtension("GL_EXT_separate_specular_color", extStart, extEnd);
#endif /* GL_EXT_separate_specular_color */
#ifdef GL_EXT_shader_image_load_store
  CONST_CAST(GLEW_EXT_shader_image_load_store) = _glewSearchExtension("GL_EXT_shader_image_load_store", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_shader_image_load_store) CONST_CAST(GLEW_EXT_shader_image_load_store) = !_glewInit_GL_EXT_shader_image_load_store(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_shader_image_load_store */
#ifdef GL_EXT_shadow_funcs
  CONST_CAST(GLEW_EXT_shadow_funcs) = _glewSearchExtension("GL_EXT_shadow_funcs", extStart, extEnd);
#endif /* GL_EXT_shadow_funcs */
#ifdef GL_EXT_shared_texture_palette
  CONST_CAST(GLEW_EXT_shared_texture_palette) = _glewSearchExtension("GL_EXT_shared_texture_palette", extStart, extEnd);
#endif /* GL_EXT_shared_texture_palette */
#ifdef GL_EXT_stencil_clear_tag
  CONST_CAST(GLEW_EXT_stencil_clear_tag) = _glewSearchExtension("GL_EXT_stencil_clear_tag", extStart, extEnd);
#endif /* GL_EXT_stencil_clear_tag */
#ifdef GL_EXT_stencil_two_side
  CONST_CAST(GLEW_EXT_stencil_two_side) = _glewSearchExtension("GL_EXT_stencil_two_side", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_stencil_two_side) CONST_CAST(GLEW_EXT_stencil_two_side) = !_glewInit_GL_EXT_stencil_two_side(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_stencil_two_side */
#ifdef GL_EXT_stencil_wrap
  CONST_CAST(GLEW_EXT_stencil_wrap) = _glewSearchExtension("GL_EXT_stencil_wrap", extStart, extEnd);
#endif /* GL_EXT_stencil_wrap */
#ifdef GL_EXT_subtexture
  CONST_CAST(GLEW_EXT_subtexture) = _glewSearchExtension("GL_EXT_subtexture", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_subtexture) CONST_CAST(GLEW_EXT_subtexture) = !_glewInit_GL_EXT_subtexture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_subtexture */
#ifdef GL_EXT_texture
  CONST_CAST(GLEW_EXT_texture) = _glewSearchExtension("GL_EXT_texture", extStart, extEnd);
#endif /* GL_EXT_texture */
#ifdef GL_EXT_texture3D
  CONST_CAST(GLEW_EXT_texture3D) = _glewSearchExtension("GL_EXT_texture3D", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_texture3D) CONST_CAST(GLEW_EXT_texture3D) = !_glewInit_GL_EXT_texture3D(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture3D */
#ifdef GL_EXT_texture_array
  CONST_CAST(GLEW_EXT_texture_array) = _glewSearchExtension("GL_EXT_texture_array", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_texture_array) CONST_CAST(GLEW_EXT_texture_array) = !_glewInit_GL_EXT_texture_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_array */
#ifdef GL_EXT_texture_buffer_object
  CONST_CAST(GLEW_EXT_texture_buffer_object) = _glewSearchExtension("GL_EXT_texture_buffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_texture_buffer_object) CONST_CAST(GLEW_EXT_texture_buffer_object) = !_glewInit_GL_EXT_texture_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_buffer_object */
#ifdef GL_EXT_texture_compression_dxt1
  CONST_CAST(GLEW_EXT_texture_compression_dxt1) = _glewSearchExtension("GL_EXT_texture_compression_dxt1", extStart, extEnd);
#endif /* GL_EXT_texture_compression_dxt1 */
#ifdef GL_EXT_texture_compression_latc
  CONST_CAST(GLEW_EXT_texture_compression_latc) = _glewSearchExtension("GL_EXT_texture_compression_latc", extStart, extEnd);
#endif /* GL_EXT_texture_compression_latc */
#ifdef GL_EXT_texture_compression_rgtc
  CONST_CAST(GLEW_EXT_texture_compression_rgtc) = _glewSearchExtension("GL_EXT_texture_compression_rgtc", extStart, extEnd);
#endif /* GL_EXT_texture_compression_rgtc */
#ifdef GL_EXT_texture_compression_s3tc
  CONST_CAST(GLEW_EXT_texture_compression_s3tc) = _glewSearchExtension("GL_EXT_texture_compression_s3tc", extStart, extEnd);
#endif /* GL_EXT_texture_compression_s3tc */
#ifdef GL_EXT_texture_cube_map
  CONST_CAST(GLEW_EXT_texture_cube_map) = _glewSearchExtension("GL_EXT_texture_cube_map", extStart, extEnd);
#endif /* GL_EXT_texture_cube_map */
#ifdef GL_EXT_texture_edge_clamp
  CONST_CAST(GLEW_EXT_texture_edge_clamp) = _glewSearchExtension("GL_EXT_texture_edge_clamp", extStart, extEnd);
#endif /* GL_EXT_texture_edge_clamp */
#ifdef GL_EXT_texture_env
  CONST_CAST(GLEW_EXT_texture_env) = _glewSearchExtension("GL_EXT_texture_env", extStart, extEnd);
#endif /* GL_EXT_texture_env */
#ifdef GL_EXT_texture_env_add
  CONST_CAST(GLEW_EXT_texture_env_add) = _glewSearchExtension("GL_EXT_texture_env_add", extStart, extEnd);
#endif /* GL_EXT_texture_env_add */
#ifdef GL_EXT_texture_env_combine
  CONST_CAST(GLEW_EXT_texture_env_combine) = _glewSearchExtension("GL_EXT_texture_env_combine", extStart, extEnd);
#endif /* GL_EXT_texture_env_combine */
#ifdef GL_EXT_texture_env_dot3
  CONST_CAST(GLEW_EXT_texture_env_dot3) = _glewSearchExtension("GL_EXT_texture_env_dot3", extStart, extEnd);
#endif /* GL_EXT_texture_env_dot3 */
#ifdef GL_EXT_texture_filter_anisotropic
  CONST_CAST(GLEW_EXT_texture_filter_anisotropic) = _glewSearchExtension("GL_EXT_texture_filter_anisotropic", extStart, extEnd);
#endif /* GL_EXT_texture_filter_anisotropic */
#ifdef GL_EXT_texture_integer
  CONST_CAST(GLEW_EXT_texture_integer) = _glewSearchExtension("GL_EXT_texture_integer", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_texture_integer) CONST_CAST(GLEW_EXT_texture_integer) = !_glewInit_GL_EXT_texture_integer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_integer */
#ifdef GL_EXT_texture_lod_bias
  CONST_CAST(GLEW_EXT_texture_lod_bias) = _glewSearchExtension("GL_EXT_texture_lod_bias", extStart, extEnd);
#endif /* GL_EXT_texture_lod_bias */
#ifdef GL_EXT_texture_mirror_clamp
  CONST_CAST(GLEW_EXT_texture_mirror_clamp) = _glewSearchExtension("GL_EXT_texture_mirror_clamp", extStart, extEnd);
#endif /* GL_EXT_texture_mirror_clamp */
#ifdef GL_EXT_texture_object
  CONST_CAST(GLEW_EXT_texture_object) = _glewSearchExtension("GL_EXT_texture_object", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_texture_object) CONST_CAST(GLEW_EXT_texture_object) = !_glewInit_GL_EXT_texture_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_object */
#ifdef GL_EXT_texture_perturb_normal
  CONST_CAST(GLEW_EXT_texture_perturb_normal) = _glewSearchExtension("GL_EXT_texture_perturb_normal", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_texture_perturb_normal) CONST_CAST(GLEW_EXT_texture_perturb_normal) = !_glewInit_GL_EXT_texture_perturb_normal(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_texture_perturb_normal */
#ifdef GL_EXT_texture_rectangle
  CONST_CAST(GLEW_EXT_texture_rectangle) = _glewSearchExtension("GL_EXT_texture_rectangle", extStart, extEnd);
#endif /* GL_EXT_texture_rectangle */
#ifdef GL_EXT_texture_sRGB
  CONST_CAST(GLEW_EXT_texture_sRGB) = _glewSearchExtension("GL_EXT_texture_sRGB", extStart, extEnd);
#endif /* GL_EXT_texture_sRGB */
#ifdef GL_EXT_texture_sRGB_decode
  CONST_CAST(GLEW_EXT_texture_sRGB_decode) = _glewSearchExtension("GL_EXT_texture_sRGB_decode", extStart, extEnd);
#endif /* GL_EXT_texture_sRGB_decode */
#ifdef GL_EXT_texture_shared_exponent
  CONST_CAST(GLEW_EXT_texture_shared_exponent) = _glewSearchExtension("GL_EXT_texture_shared_exponent", extStart, extEnd);
#endif /* GL_EXT_texture_shared_exponent */
#ifdef GL_EXT_texture_snorm
  CONST_CAST(GLEW_EXT_texture_snorm) = _glewSearchExtension("GL_EXT_texture_snorm", extStart, extEnd);
#endif /* GL_EXT_texture_snorm */
#ifdef GL_EXT_texture_swizzle
  CONST_CAST(GLEW_EXT_texture_swizzle) = _glewSearchExtension("GL_EXT_texture_swizzle", extStart, extEnd);
#endif /* GL_EXT_texture_swizzle */
#ifdef GL_EXT_timer_query
  CONST_CAST(GLEW_EXT_timer_query) = _glewSearchExtension("GL_EXT_timer_query", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_timer_query) CONST_CAST(GLEW_EXT_timer_query) = !_glewInit_GL_EXT_timer_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_timer_query */
#ifdef GL_EXT_transform_feedback
  CONST_CAST(GLEW_EXT_transform_feedback) = _glewSearchExtension("GL_EXT_transform_feedback", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_transform_feedback) CONST_CAST(GLEW_EXT_transform_feedback) = !_glewInit_GL_EXT_transform_feedback(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_transform_feedback */
#ifdef GL_EXT_vertex_array
  CONST_CAST(GLEW_EXT_vertex_array) = _glewSearchExtension("GL_EXT_vertex_array", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_vertex_array) CONST_CAST(GLEW_EXT_vertex_array) = !_glewInit_GL_EXT_vertex_array(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_array */
#ifdef GL_EXT_vertex_array_bgra
  CONST_CAST(GLEW_EXT_vertex_array_bgra) = _glewSearchExtension("GL_EXT_vertex_array_bgra", extStart, extEnd);
#endif /* GL_EXT_vertex_array_bgra */
#ifdef GL_EXT_vertex_attrib_64bit
  CONST_CAST(GLEW_EXT_vertex_attrib_64bit) = _glewSearchExtension("GL_EXT_vertex_attrib_64bit", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_vertex_attrib_64bit) CONST_CAST(GLEW_EXT_vertex_attrib_64bit) = !_glewInit_GL_EXT_vertex_attrib_64bit(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_attrib_64bit */
#ifdef GL_EXT_vertex_shader
  CONST_CAST(GLEW_EXT_vertex_shader) = _glewSearchExtension("GL_EXT_vertex_shader", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_vertex_shader) CONST_CAST(GLEW_EXT_vertex_shader) = !_glewInit_GL_EXT_vertex_shader(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_shader */
#ifdef GL_EXT_vertex_weighting
  CONST_CAST(GLEW_EXT_vertex_weighting) = _glewSearchExtension("GL_EXT_vertex_weighting", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_vertex_weighting) CONST_CAST(GLEW_EXT_vertex_weighting) = !_glewInit_GL_EXT_vertex_weighting(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_vertex_weighting */
#ifdef GL_EXT_x11_sync_object
  CONST_CAST(GLEW_EXT_x11_sync_object) = _glewSearchExtension("GL_EXT_x11_sync_object", extStart, extEnd);
  if (glewExperimental || GLEW_EXT_x11_sync_object) CONST_CAST(GLEW_EXT_x11_sync_object) = !_glewInit_GL_EXT_x11_sync_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_EXT_x11_sync_object */
#ifdef GL_GREMEDY_frame_terminator
  CONST_CAST(GLEW_GREMEDY_frame_terminator) = _glewSearchExtension("GL_GREMEDY_frame_terminator", extStart, extEnd);
  if (glewExperimental || GLEW_GREMEDY_frame_terminator) CONST_CAST(GLEW_GREMEDY_frame_terminator) = !_glewInit_GL_GREMEDY_frame_terminator(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_GREMEDY_frame_terminator */
#ifdef GL_GREMEDY_string_marker
  CONST_CAST(GLEW_GREMEDY_string_marker) = _glewSearchExtension("GL_GREMEDY_string_marker", extStart, extEnd);
  if (glewExperimental || GLEW_GREMEDY_string_marker) CONST_CAST(GLEW_GREMEDY_string_marker) = !_glewInit_GL_GREMEDY_string_marker(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_GREMEDY_string_marker */
#ifdef GL_HP_convolution_border_modes
  CONST_CAST(GLEW_HP_convolution_border_modes) = _glewSearchExtension("GL_HP_convolution_border_modes", extStart, extEnd);
#endif /* GL_HP_convolution_border_modes */
#ifdef GL_HP_image_transform
  CONST_CAST(GLEW_HP_image_transform) = _glewSearchExtension("GL_HP_image_transform", extStart, extEnd);
  if (glewExperimental || GLEW_HP_image_transform) CONST_CAST(GLEW_HP_image_transform) = !_glewInit_GL_HP_image_transform(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_HP_image_transform */
#ifdef GL_HP_occlusion_test
  CONST_CAST(GLEW_HP_occlusion_test) = _glewSearchExtension("GL_HP_occlusion_test", extStart, extEnd);
#endif /* GL_HP_occlusion_test */
#ifdef GL_HP_texture_lighting
  CONST_CAST(GLEW_HP_texture_lighting) = _glewSearchExtension("GL_HP_texture_lighting", extStart, extEnd);
#endif /* GL_HP_texture_lighting */
#ifdef GL_IBM_cull_vertex
  CONST_CAST(GLEW_IBM_cull_vertex) = _glewSearchExtension("GL_IBM_cull_vertex", extStart, extEnd);
#endif /* GL_IBM_cull_vertex */
#ifdef GL_IBM_multimode_draw_arrays
  CONST_CAST(GLEW_IBM_multimode_draw_arrays) = _glewSearchExtension("GL_IBM_multimode_draw_arrays", extStart, extEnd);
  if (glewExperimental || GLEW_IBM_multimode_draw_arrays) CONST_CAST(GLEW_IBM_multimode_draw_arrays) = !_glewInit_GL_IBM_multimode_draw_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_IBM_multimode_draw_arrays */
#ifdef GL_IBM_rasterpos_clip
  CONST_CAST(GLEW_IBM_rasterpos_clip) = _glewSearchExtension("GL_IBM_rasterpos_clip", extStart, extEnd);
#endif /* GL_IBM_rasterpos_clip */
#ifdef GL_IBM_static_data
  CONST_CAST(GLEW_IBM_static_data) = _glewSearchExtension("GL_IBM_static_data", extStart, extEnd);
#endif /* GL_IBM_static_data */
#ifdef GL_IBM_texture_mirrored_repeat
  CONST_CAST(GLEW_IBM_texture_mirrored_repeat) = _glewSearchExtension("GL_IBM_texture_mirrored_repeat", extStart, extEnd);
#endif /* GL_IBM_texture_mirrored_repeat */
#ifdef GL_IBM_vertex_array_lists
  CONST_CAST(GLEW_IBM_vertex_array_lists) = _glewSearchExtension("GL_IBM_vertex_array_lists", extStart, extEnd);
  if (glewExperimental || GLEW_IBM_vertex_array_lists) CONST_CAST(GLEW_IBM_vertex_array_lists) = !_glewInit_GL_IBM_vertex_array_lists(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_IBM_vertex_array_lists */
#ifdef GL_INGR_color_clamp
  CONST_CAST(GLEW_INGR_color_clamp) = _glewSearchExtension("GL_INGR_color_clamp", extStart, extEnd);
#endif /* GL_INGR_color_clamp */
#ifdef GL_INGR_interlace_read
  CONST_CAST(GLEW_INGR_interlace_read) = _glewSearchExtension("GL_INGR_interlace_read", extStart, extEnd);
#endif /* GL_INGR_interlace_read */
#ifdef GL_INTEL_parallel_arrays
  CONST_CAST(GLEW_INTEL_parallel_arrays) = _glewSearchExtension("GL_INTEL_parallel_arrays", extStart, extEnd);
  if (glewExperimental || GLEW_INTEL_parallel_arrays) CONST_CAST(GLEW_INTEL_parallel_arrays) = !_glewInit_GL_INTEL_parallel_arrays(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_INTEL_parallel_arrays */
#ifdef GL_INTEL_texture_scissor
  CONST_CAST(GLEW_INTEL_texture_scissor) = _glewSearchExtension("GL_INTEL_texture_scissor", extStart, extEnd);
  if (glewExperimental || GLEW_INTEL_texture_scissor) CONST_CAST(GLEW_INTEL_texture_scissor) = !_glewInit_GL_INTEL_texture_scissor(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_INTEL_texture_scissor */
#ifdef GL_KTX_buffer_region
  CONST_CAST(GLEW_KTX_buffer_region) = _glewSearchExtension("GL_KTX_buffer_region", extStart, extEnd);
  if (glewExperimental || GLEW_KTX_buffer_region) CONST_CAST(GLEW_KTX_buffer_region) = !_glewInit_GL_KTX_buffer_region(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_KTX_buffer_region */
#ifdef GL_MESAX_texture_stack
  CONST_CAST(GLEW_MESAX_texture_stack) = _glewSearchExtension("GL_MESAX_texture_stack", extStart, extEnd);
#endif /* GL_MESAX_texture_stack */
#ifdef GL_MESA_pack_invert
  CONST_CAST(GLEW_MESA_pack_invert) = _glewSearchExtension("GL_MESA_pack_invert", extStart, extEnd);
#endif /* GL_MESA_pack_invert */
#ifdef GL_MESA_resize_buffers
  CONST_CAST(GLEW_MESA_resize_buffers) = _glewSearchExtension("GL_MESA_resize_buffers", extStart, extEnd);
  if (glewExperimental || GLEW_MESA_resize_buffers) CONST_CAST(GLEW_MESA_resize_buffers) = !_glewInit_GL_MESA_resize_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_MESA_resize_buffers */
#ifdef GL_MESA_window_pos
  CONST_CAST(GLEW_MESA_window_pos) = _glewSearchExtension("GL_MESA_window_pos", extStart, extEnd);
  if (glewExperimental || GLEW_MESA_window_pos) CONST_CAST(GLEW_MESA_window_pos) = !_glewInit_GL_MESA_window_pos(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_MESA_window_pos */
#ifdef GL_MESA_ycbcr_texture
  CONST_CAST(GLEW_MESA_ycbcr_texture) = _glewSearchExtension("GL_MESA_ycbcr_texture", extStart, extEnd);
#endif /* GL_MESA_ycbcr_texture */
#ifdef GL_NVX_gpu_memory_info
  CONST_CAST(GLEW_NVX_gpu_memory_info) = _glewSearchExtension("GL_NVX_gpu_memory_info", extStart, extEnd);
#endif /* GL_NVX_gpu_memory_info */
#ifdef GL_NV_blend_square
  CONST_CAST(GLEW_NV_blend_square) = _glewSearchExtension("GL_NV_blend_square", extStart, extEnd);
#endif /* GL_NV_blend_square */
#ifdef GL_NV_conditional_render
  CONST_CAST(GLEW_NV_conditional_render) = _glewSearchExtension("GL_NV_conditional_render", extStart, extEnd);
  if (glewExperimental || GLEW_NV_conditional_render) CONST_CAST(GLEW_NV_conditional_render) = !_glewInit_GL_NV_conditional_render(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_conditional_render */
#ifdef GL_NV_copy_depth_to_color
  CONST_CAST(GLEW_NV_copy_depth_to_color) = _glewSearchExtension("GL_NV_copy_depth_to_color", extStart, extEnd);
#endif /* GL_NV_copy_depth_to_color */
#ifdef GL_NV_copy_image
  CONST_CAST(GLEW_NV_copy_image) = _glewSearchExtension("GL_NV_copy_image", extStart, extEnd);
  if (glewExperimental || GLEW_NV_copy_image) CONST_CAST(GLEW_NV_copy_image) = !_glewInit_GL_NV_copy_image(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_copy_image */
#ifdef GL_NV_depth_buffer_float
  CONST_CAST(GLEW_NV_depth_buffer_float) = _glewSearchExtension("GL_NV_depth_buffer_float", extStart, extEnd);
  if (glewExperimental || GLEW_NV_depth_buffer_float) CONST_CAST(GLEW_NV_depth_buffer_float) = !_glewInit_GL_NV_depth_buffer_float(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_depth_buffer_float */
#ifdef GL_NV_depth_clamp
  CONST_CAST(GLEW_NV_depth_clamp) = _glewSearchExtension("GL_NV_depth_clamp", extStart, extEnd);
#endif /* GL_NV_depth_clamp */
#ifdef GL_NV_depth_range_unclamped
  CONST_CAST(GLEW_NV_depth_range_unclamped) = _glewSearchExtension("GL_NV_depth_range_unclamped", extStart, extEnd);
#endif /* GL_NV_depth_range_unclamped */
#ifdef GL_NV_evaluators
  CONST_CAST(GLEW_NV_evaluators) = _glewSearchExtension("GL_NV_evaluators", extStart, extEnd);
  if (glewExperimental || GLEW_NV_evaluators) CONST_CAST(GLEW_NV_evaluators) = !_glewInit_GL_NV_evaluators(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_evaluators */
#ifdef GL_NV_explicit_multisample
  CONST_CAST(GLEW_NV_explicit_multisample) = _glewSearchExtension("GL_NV_explicit_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_NV_explicit_multisample) CONST_CAST(GLEW_NV_explicit_multisample) = !_glewInit_GL_NV_explicit_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_explicit_multisample */
#ifdef GL_NV_fence
  CONST_CAST(GLEW_NV_fence) = _glewSearchExtension("GL_NV_fence", extStart, extEnd);
  if (glewExperimental || GLEW_NV_fence) CONST_CAST(GLEW_NV_fence) = !_glewInit_GL_NV_fence(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_fence */
#ifdef GL_NV_float_buffer
  CONST_CAST(GLEW_NV_float_buffer) = _glewSearchExtension("GL_NV_float_buffer", extStart, extEnd);
#endif /* GL_NV_float_buffer */
#ifdef GL_NV_fog_distance
  CONST_CAST(GLEW_NV_fog_distance) = _glewSearchExtension("GL_NV_fog_distance", extStart, extEnd);
#endif /* GL_NV_fog_distance */
#ifdef GL_NV_fragment_program
  CONST_CAST(GLEW_NV_fragment_program) = _glewSearchExtension("GL_NV_fragment_program", extStart, extEnd);
  if (glewExperimental || GLEW_NV_fragment_program) CONST_CAST(GLEW_NV_fragment_program) = !_glewInit_GL_NV_fragment_program(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_fragment_program */
#ifdef GL_NV_fragment_program2
  CONST_CAST(GLEW_NV_fragment_program2) = _glewSearchExtension("GL_NV_fragment_program2", extStart, extEnd);
#endif /* GL_NV_fragment_program2 */
#ifdef GL_NV_fragment_program4
  CONST_CAST(GLEW_NV_fragment_program4) = _glewSearchExtension("GL_NV_gpu_program4", extStart, extEnd);
#endif /* GL_NV_fragment_program4 */
#ifdef GL_NV_fragment_program_option
  CONST_CAST(GLEW_NV_fragment_program_option) = _glewSearchExtension("GL_NV_fragment_program_option", extStart, extEnd);
#endif /* GL_NV_fragment_program_option */
#ifdef GL_NV_framebuffer_multisample_coverage
  CONST_CAST(GLEW_NV_framebuffer_multisample_coverage) = _glewSearchExtension("GL_NV_framebuffer_multisample_coverage", extStart, extEnd);
  if (glewExperimental || GLEW_NV_framebuffer_multisample_coverage) CONST_CAST(GLEW_NV_framebuffer_multisample_coverage) = !_glewInit_GL_NV_framebuffer_multisample_coverage(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_framebuffer_multisample_coverage */
#ifdef GL_NV_geometry_program4
  CONST_CAST(GLEW_NV_geometry_program4) = _glewSearchExtension("GL_NV_gpu_program4", extStart, extEnd);
  if (glewExperimental || GLEW_NV_geometry_program4) CONST_CAST(GLEW_NV_geometry_program4) = !_glewInit_GL_NV_geometry_program4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_geometry_program4 */
#ifdef GL_NV_geometry_shader4
  CONST_CAST(GLEW_NV_geometry_shader4) = _glewSearchExtension("GL_NV_geometry_shader4", extStart, extEnd);
#endif /* GL_NV_geometry_shader4 */
#ifdef GL_NV_gpu_program4
  CONST_CAST(GLEW_NV_gpu_program4) = _glewSearchExtension("GL_NV_gpu_program4", extStart, extEnd);
  if (glewExperimental || GLEW_NV_gpu_program4) CONST_CAST(GLEW_NV_gpu_program4) = !_glewInit_GL_NV_gpu_program4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_gpu_program4 */
#ifdef GL_NV_gpu_program5
  CONST_CAST(GLEW_NV_gpu_program5) = _glewSearchExtension("GL_NV_gpu_program5", extStart, extEnd);
#endif /* GL_NV_gpu_program5 */
#ifdef GL_NV_gpu_program_fp64
  CONST_CAST(GLEW_NV_gpu_program_fp64) = _glewSearchExtension("GL_NV_gpu_program_fp64", extStart, extEnd);
#endif /* GL_NV_gpu_program_fp64 */
#ifdef GL_NV_gpu_shader5
  CONST_CAST(GLEW_NV_gpu_shader5) = _glewSearchExtension("GL_NV_gpu_shader5", extStart, extEnd);
  if (glewExperimental || GLEW_NV_gpu_shader5) CONST_CAST(GLEW_NV_gpu_shader5) = !_glewInit_GL_NV_gpu_shader5(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_gpu_shader5 */
#ifdef GL_NV_half_float
  CONST_CAST(GLEW_NV_half_float) = _glewSearchExtension("GL_NV_half_float", extStart, extEnd);
  if (glewExperimental || GLEW_NV_half_float) CONST_CAST(GLEW_NV_half_float) = !_glewInit_GL_NV_half_float(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_half_float */
#ifdef GL_NV_light_max_exponent
  CONST_CAST(GLEW_NV_light_max_exponent) = _glewSearchExtension("GL_NV_light_max_exponent", extStart, extEnd);
#endif /* GL_NV_light_max_exponent */
#ifdef GL_NV_multisample_coverage
  CONST_CAST(GLEW_NV_multisample_coverage) = _glewSearchExtension("GL_NV_multisample_coverage", extStart, extEnd);
#endif /* GL_NV_multisample_coverage */
#ifdef GL_NV_multisample_filter_hint
  CONST_CAST(GLEW_NV_multisample_filter_hint) = _glewSearchExtension("GL_NV_multisample_filter_hint", extStart, extEnd);
#endif /* GL_NV_multisample_filter_hint */
#ifdef GL_NV_occlusion_query
  CONST_CAST(GLEW_NV_occlusion_query) = _glewSearchExtension("GL_NV_occlusion_query", extStart, extEnd);
  if (glewExperimental || GLEW_NV_occlusion_query) CONST_CAST(GLEW_NV_occlusion_query) = !_glewInit_GL_NV_occlusion_query(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_occlusion_query */
#ifdef GL_NV_packed_depth_stencil
  CONST_CAST(GLEW_NV_packed_depth_stencil) = _glewSearchExtension("GL_NV_packed_depth_stencil", extStart, extEnd);
#endif /* GL_NV_packed_depth_stencil */
#ifdef GL_NV_parameter_buffer_object
  CONST_CAST(GLEW_NV_parameter_buffer_object) = _glewSearchExtension("GL_NV_parameter_buffer_object", extStart, extEnd);
  if (glewExperimental || GLEW_NV_parameter_buffer_object) CONST_CAST(GLEW_NV_parameter_buffer_object) = !_glewInit_GL_NV_parameter_buffer_object(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_parameter_buffer_object */
#ifdef GL_NV_parameter_buffer_object2
  CONST_CAST(GLEW_NV_parameter_buffer_object2) = _glewSearchExtension("GL_NV_parameter_buffer_object2", extStart, extEnd);
#endif /* GL_NV_parameter_buffer_object2 */
#ifdef GL_NV_path_rendering
  CONST_CAST(GLEW_NV_path_rendering) = _glewSearchExtension("GL_NV_path_rendering", extStart, extEnd);
  if (glewExperimental || GLEW_NV_path_rendering) CONST_CAST(GLEW_NV_path_rendering) = !_glewInit_GL_NV_path_rendering(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_path_rendering */
#ifdef GL_NV_pixel_data_range
  CONST_CAST(GLEW_NV_pixel_data_range) = _glewSearchExtension("GL_NV_pixel_data_range", extStart, extEnd);
  if (glewExperimental || GLEW_NV_pixel_data_range) CONST_CAST(GLEW_NV_pixel_data_range) = !_glewInit_GL_NV_pixel_data_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_pixel_data_range */
#ifdef GL_NV_point_sprite
  CONST_CAST(GLEW_NV_point_sprite) = _glewSearchExtension("GL_NV_point_sprite", extStart, extEnd);
  if (glewExperimental || GLEW_NV_point_sprite) CONST_CAST(GLEW_NV_point_sprite) = !_glewInit_GL_NV_point_sprite(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_point_sprite */
#ifdef GL_NV_present_video
  CONST_CAST(GLEW_NV_present_video) = _glewSearchExtension("GL_NV_present_video", extStart, extEnd);
  if (glewExperimental || GLEW_NV_present_video) CONST_CAST(GLEW_NV_present_video) = !_glewInit_GL_NV_present_video(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_present_video */
#ifdef GL_NV_primitive_restart
  CONST_CAST(GLEW_NV_primitive_restart) = _glewSearchExtension("GL_NV_primitive_restart", extStart, extEnd);
  if (glewExperimental || GLEW_NV_primitive_restart) CONST_CAST(GLEW_NV_primitive_restart) = !_glewInit_GL_NV_primitive_restart(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_primitive_restart */
#ifdef GL_NV_register_combiners
  CONST_CAST(GLEW_NV_register_combiners) = _glewSearchExtension("GL_NV_register_combiners", extStart, extEnd);
  if (glewExperimental || GLEW_NV_register_combiners) CONST_CAST(GLEW_NV_register_combiners) = !_glewInit_GL_NV_register_combiners(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_register_combiners */
#ifdef GL_NV_register_combiners2
  CONST_CAST(GLEW_NV_register_combiners2) = _glewSearchExtension("GL_NV_register_combiners2", extStart, extEnd);
  if (glewExperimental || GLEW_NV_register_combiners2) CONST_CAST(GLEW_NV_register_combiners2) = !_glewInit_GL_NV_register_combiners2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_register_combiners2 */
#ifdef GL_NV_shader_buffer_load
  CONST_CAST(GLEW_NV_shader_buffer_load) = _glewSearchExtension("GL_NV_shader_buffer_load", extStart, extEnd);
  if (glewExperimental || GLEW_NV_shader_buffer_load) CONST_CAST(GLEW_NV_shader_buffer_load) = !_glewInit_GL_NV_shader_buffer_load(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_shader_buffer_load */
#ifdef GL_NV_tessellation_program5
  CONST_CAST(GLEW_NV_tessellation_program5) = _glewSearchExtension("GL_NV_gpu_program5", extStart, extEnd);
#endif /* GL_NV_tessellation_program5 */
#ifdef GL_NV_texgen_emboss
  CONST_CAST(GLEW_NV_texgen_emboss) = _glewSearchExtension("GL_NV_texgen_emboss", extStart, extEnd);
#endif /* GL_NV_texgen_emboss */
#ifdef GL_NV_texgen_reflection
  CONST_CAST(GLEW_NV_texgen_reflection) = _glewSearchExtension("GL_NV_texgen_reflection", extStart, extEnd);
#endif /* GL_NV_texgen_reflection */
#ifdef GL_NV_texture_barrier
  CONST_CAST(GLEW_NV_texture_barrier) = _glewSearchExtension("GL_NV_texture_barrier", extStart, extEnd);
  if (glewExperimental || GLEW_NV_texture_barrier) CONST_CAST(GLEW_NV_texture_barrier) = !_glewInit_GL_NV_texture_barrier(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_texture_barrier */
#ifdef GL_NV_texture_compression_vtc
  CONST_CAST(GLEW_NV_texture_compression_vtc) = _glewSearchExtension("GL_NV_texture_compression_vtc", extStart, extEnd);
#endif /* GL_NV_texture_compression_vtc */
#ifdef GL_NV_texture_env_combine4
  CONST_CAST(GLEW_NV_texture_env_combine4) = _glewSearchExtension("GL_NV_texture_env_combine4", extStart, extEnd);
#endif /* GL_NV_texture_env_combine4 */
#ifdef GL_NV_texture_expand_normal
  CONST_CAST(GLEW_NV_texture_expand_normal) = _glewSearchExtension("GL_NV_texture_expand_normal", extStart, extEnd);
#endif /* GL_NV_texture_expand_normal */
#ifdef GL_NV_texture_multisample
  CONST_CAST(GLEW_NV_texture_multisample) = _glewSearchExtension("GL_NV_texture_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_NV_texture_multisample) CONST_CAST(GLEW_NV_texture_multisample) = !_glewInit_GL_NV_texture_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_texture_multisample */
#ifdef GL_NV_texture_rectangle
  CONST_CAST(GLEW_NV_texture_rectangle) = _glewSearchExtension("GL_NV_texture_rectangle", extStart, extEnd);
#endif /* GL_NV_texture_rectangle */
#ifdef GL_NV_texture_shader
  CONST_CAST(GLEW_NV_texture_shader) = _glewSearchExtension("GL_NV_texture_shader", extStart, extEnd);
#endif /* GL_NV_texture_shader */
#ifdef GL_NV_texture_shader2
  CONST_CAST(GLEW_NV_texture_shader2) = _glewSearchExtension("GL_NV_texture_shader2", extStart, extEnd);
#endif /* GL_NV_texture_shader2 */
#ifdef GL_NV_texture_shader3
  CONST_CAST(GLEW_NV_texture_shader3) = _glewSearchExtension("GL_NV_texture_shader3", extStart, extEnd);
#endif /* GL_NV_texture_shader3 */
#ifdef GL_NV_transform_feedback
  CONST_CAST(GLEW_NV_transform_feedback) = _glewSearchExtension("GL_NV_transform_feedback", extStart, extEnd);
  if (glewExperimental || GLEW_NV_transform_feedback) CONST_CAST(GLEW_NV_transform_feedback) = !_glewInit_GL_NV_transform_feedback(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_transform_feedback */
#ifdef GL_NV_transform_feedback2
  CONST_CAST(GLEW_NV_transform_feedback2) = _glewSearchExtension("GL_NV_transform_feedback2", extStart, extEnd);
  if (glewExperimental || GLEW_NV_transform_feedback2) CONST_CAST(GLEW_NV_transform_feedback2) = !_glewInit_GL_NV_transform_feedback2(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_transform_feedback2 */
#ifdef GL_NV_vdpau_interop
  CONST_CAST(GLEW_NV_vdpau_interop) = _glewSearchExtension("GL_NV_vdpau_interop", extStart, extEnd);
  if (glewExperimental || GLEW_NV_vdpau_interop) CONST_CAST(GLEW_NV_vdpau_interop) = !_glewInit_GL_NV_vdpau_interop(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vdpau_interop */
#ifdef GL_NV_vertex_array_range
  CONST_CAST(GLEW_NV_vertex_array_range) = _glewSearchExtension("GL_NV_vertex_array_range", extStart, extEnd);
  if (glewExperimental || GLEW_NV_vertex_array_range) CONST_CAST(GLEW_NV_vertex_array_range) = !_glewInit_GL_NV_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vertex_array_range */
#ifdef GL_NV_vertex_array_range2
  CONST_CAST(GLEW_NV_vertex_array_range2) = _glewSearchExtension("GL_NV_vertex_array_range2", extStart, extEnd);
#endif /* GL_NV_vertex_array_range2 */
#ifdef GL_NV_vertex_attrib_integer_64bit
  CONST_CAST(GLEW_NV_vertex_attrib_integer_64bit) = _glewSearchExtension("GL_NV_vertex_attrib_integer_64bit", extStart, extEnd);
  if (glewExperimental || GLEW_NV_vertex_attrib_integer_64bit) CONST_CAST(GLEW_NV_vertex_attrib_integer_64bit) = !_glewInit_GL_NV_vertex_attrib_integer_64bit(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vertex_attrib_integer_64bit */
#ifdef GL_NV_vertex_buffer_unified_memory
  CONST_CAST(GLEW_NV_vertex_buffer_unified_memory) = _glewSearchExtension("GL_NV_vertex_buffer_unified_memory", extStart, extEnd);
  if (glewExperimental || GLEW_NV_vertex_buffer_unified_memory) CONST_CAST(GLEW_NV_vertex_buffer_unified_memory) = !_glewInit_GL_NV_vertex_buffer_unified_memory(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vertex_buffer_unified_memory */
#ifdef GL_NV_vertex_program
  CONST_CAST(GLEW_NV_vertex_program) = _glewSearchExtension("GL_NV_vertex_program", extStart, extEnd);
  if (glewExperimental || GLEW_NV_vertex_program) CONST_CAST(GLEW_NV_vertex_program) = !_glewInit_GL_NV_vertex_program(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_vertex_program */
#ifdef GL_NV_vertex_program1_1
  CONST_CAST(GLEW_NV_vertex_program1_1) = _glewSearchExtension("GL_NV_vertex_program1_1", extStart, extEnd);
#endif /* GL_NV_vertex_program1_1 */
#ifdef GL_NV_vertex_program2
  CONST_CAST(GLEW_NV_vertex_program2) = _glewSearchExtension("GL_NV_vertex_program2", extStart, extEnd);
#endif /* GL_NV_vertex_program2 */
#ifdef GL_NV_vertex_program2_option
  CONST_CAST(GLEW_NV_vertex_program2_option) = _glewSearchExtension("GL_NV_vertex_program2_option", extStart, extEnd);
#endif /* GL_NV_vertex_program2_option */
#ifdef GL_NV_vertex_program3
  CONST_CAST(GLEW_NV_vertex_program3) = _glewSearchExtension("GL_NV_vertex_program3", extStart, extEnd);
#endif /* GL_NV_vertex_program3 */
#ifdef GL_NV_vertex_program4
  CONST_CAST(GLEW_NV_vertex_program4) = _glewSearchExtension("GL_NV_gpu_program4", extStart, extEnd);
#endif /* GL_NV_vertex_program4 */
#ifdef GL_NV_video_capture
  CONST_CAST(GLEW_NV_video_capture) = _glewSearchExtension("GL_NV_video_capture", extStart, extEnd);
  if (glewExperimental || GLEW_NV_video_capture) CONST_CAST(GLEW_NV_video_capture) = !_glewInit_GL_NV_video_capture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_NV_video_capture */
#ifdef GL_OES_byte_coordinates
  CONST_CAST(GLEW_OES_byte_coordinates) = _glewSearchExtension("GL_OES_byte_coordinates", extStart, extEnd);
#endif /* GL_OES_byte_coordinates */
#ifdef GL_OES_compressed_paletted_texture
  CONST_CAST(GLEW_OES_compressed_paletted_texture) = _glewSearchExtension("GL_OES_compressed_paletted_texture", extStart, extEnd);
#endif /* GL_OES_compressed_paletted_texture */
#ifdef GL_OES_read_format
  CONST_CAST(GLEW_OES_read_format) = _glewSearchExtension("GL_OES_read_format", extStart, extEnd);
#endif /* GL_OES_read_format */
#ifdef GL_OES_single_precision
  CONST_CAST(GLEW_OES_single_precision) = _glewSearchExtension("GL_OES_single_precision", extStart, extEnd);
  if (glewExperimental || GLEW_OES_single_precision) CONST_CAST(GLEW_OES_single_precision) = !_glewInit_GL_OES_single_precision(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_OES_single_precision */
#ifdef GL_OML_interlace
  CONST_CAST(GLEW_OML_interlace) = _glewSearchExtension("GL_OML_interlace", extStart, extEnd);
#endif /* GL_OML_interlace */
#ifdef GL_OML_resample
  CONST_CAST(GLEW_OML_resample) = _glewSearchExtension("GL_OML_resample", extStart, extEnd);
#endif /* GL_OML_resample */
#ifdef GL_OML_subsample
  CONST_CAST(GLEW_OML_subsample) = _glewSearchExtension("GL_OML_subsample", extStart, extEnd);
#endif /* GL_OML_subsample */
#ifdef GL_PGI_misc_hints
  CONST_CAST(GLEW_PGI_misc_hints) = _glewSearchExtension("GL_PGI_misc_hints", extStart, extEnd);
#endif /* GL_PGI_misc_hints */
#ifdef GL_PGI_vertex_hints
  CONST_CAST(GLEW_PGI_vertex_hints) = _glewSearchExtension("GL_PGI_vertex_hints", extStart, extEnd);
#endif /* GL_PGI_vertex_hints */
#ifdef GL_REND_screen_coordinates
  CONST_CAST(GLEW_REND_screen_coordinates) = _glewSearchExtension("GL_REND_screen_coordinates", extStart, extEnd);
#endif /* GL_REND_screen_coordinates */
#ifdef GL_S3_s3tc
  CONST_CAST(GLEW_S3_s3tc) = _glewSearchExtension("GL_S3_s3tc", extStart, extEnd);
#endif /* GL_S3_s3tc */
#ifdef GL_SGIS_color_range
  CONST_CAST(GLEW_SGIS_color_range) = _glewSearchExtension("GL_SGIS_color_range", extStart, extEnd);
#endif /* GL_SGIS_color_range */
#ifdef GL_SGIS_detail_texture
  CONST_CAST(GLEW_SGIS_detail_texture) = _glewSearchExtension("GL_SGIS_detail_texture", extStart, extEnd);
  if (glewExperimental || GLEW_SGIS_detail_texture) CONST_CAST(GLEW_SGIS_detail_texture) = !_glewInit_GL_SGIS_detail_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_detail_texture */
#ifdef GL_SGIS_fog_function
  CONST_CAST(GLEW_SGIS_fog_function) = _glewSearchExtension("GL_SGIS_fog_function", extStart, extEnd);
  if (glewExperimental || GLEW_SGIS_fog_function) CONST_CAST(GLEW_SGIS_fog_function) = !_glewInit_GL_SGIS_fog_function(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_fog_function */
#ifdef GL_SGIS_generate_mipmap
  CONST_CAST(GLEW_SGIS_generate_mipmap) = _glewSearchExtension("GL_SGIS_generate_mipmap", extStart, extEnd);
#endif /* GL_SGIS_generate_mipmap */
#ifdef GL_SGIS_multisample
  CONST_CAST(GLEW_SGIS_multisample) = _glewSearchExtension("GL_SGIS_multisample", extStart, extEnd);
  if (glewExperimental || GLEW_SGIS_multisample) CONST_CAST(GLEW_SGIS_multisample) = !_glewInit_GL_SGIS_multisample(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_multisample */
#ifdef GL_SGIS_pixel_texture
  CONST_CAST(GLEW_SGIS_pixel_texture) = _glewSearchExtension("GL_SGIS_pixel_texture", extStart, extEnd);
#endif /* GL_SGIS_pixel_texture */
#ifdef GL_SGIS_point_line_texgen
  CONST_CAST(GLEW_SGIS_point_line_texgen) = _glewSearchExtension("GL_SGIS_point_line_texgen", extStart, extEnd);
#endif /* GL_SGIS_point_line_texgen */
#ifdef GL_SGIS_sharpen_texture
  CONST_CAST(GLEW_SGIS_sharpen_texture) = _glewSearchExtension("GL_SGIS_sharpen_texture", extStart, extEnd);
  if (glewExperimental || GLEW_SGIS_sharpen_texture) CONST_CAST(GLEW_SGIS_sharpen_texture) = !_glewInit_GL_SGIS_sharpen_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_sharpen_texture */
#ifdef GL_SGIS_texture4D
  CONST_CAST(GLEW_SGIS_texture4D) = _glewSearchExtension("GL_SGIS_texture4D", extStart, extEnd);
  if (glewExperimental || GLEW_SGIS_texture4D) CONST_CAST(GLEW_SGIS_texture4D) = !_glewInit_GL_SGIS_texture4D(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_texture4D */
#ifdef GL_SGIS_texture_border_clamp
  CONST_CAST(GLEW_SGIS_texture_border_clamp) = _glewSearchExtension("GL_SGIS_texture_border_clamp", extStart, extEnd);
#endif /* GL_SGIS_texture_border_clamp */
#ifdef GL_SGIS_texture_edge_clamp
  CONST_CAST(GLEW_SGIS_texture_edge_clamp) = _glewSearchExtension("GL_SGIS_texture_edge_clamp", extStart, extEnd);
#endif /* GL_SGIS_texture_edge_clamp */
#ifdef GL_SGIS_texture_filter4
  CONST_CAST(GLEW_SGIS_texture_filter4) = _glewSearchExtension("GL_SGIS_texture_filter4", extStart, extEnd);
  if (glewExperimental || GLEW_SGIS_texture_filter4) CONST_CAST(GLEW_SGIS_texture_filter4) = !_glewInit_GL_SGIS_texture_filter4(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIS_texture_filter4 */
#ifdef GL_SGIS_texture_lod
  CONST_CAST(GLEW_SGIS_texture_lod) = _glewSearchExtension("GL_SGIS_texture_lod", extStart, extEnd);
#endif /* GL_SGIS_texture_lod */
#ifdef GL_SGIS_texture_select
  CONST_CAST(GLEW_SGIS_texture_select) = _glewSearchExtension("GL_SGIS_texture_select", extStart, extEnd);
#endif /* GL_SGIS_texture_select */
#ifdef GL_SGIX_async
  CONST_CAST(GLEW_SGIX_async) = _glewSearchExtension("GL_SGIX_async", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_async) CONST_CAST(GLEW_SGIX_async) = !_glewInit_GL_SGIX_async(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_async */
#ifdef GL_SGIX_async_histogram
  CONST_CAST(GLEW_SGIX_async_histogram) = _glewSearchExtension("GL_SGIX_async_histogram", extStart, extEnd);
#endif /* GL_SGIX_async_histogram */
#ifdef GL_SGIX_async_pixel
  CONST_CAST(GLEW_SGIX_async_pixel) = _glewSearchExtension("GL_SGIX_async_pixel", extStart, extEnd);
#endif /* GL_SGIX_async_pixel */
#ifdef GL_SGIX_blend_alpha_minmax
  CONST_CAST(GLEW_SGIX_blend_alpha_minmax) = _glewSearchExtension("GL_SGIX_blend_alpha_minmax", extStart, extEnd);
#endif /* GL_SGIX_blend_alpha_minmax */
#ifdef GL_SGIX_clipmap
  CONST_CAST(GLEW_SGIX_clipmap) = _glewSearchExtension("GL_SGIX_clipmap", extStart, extEnd);
#endif /* GL_SGIX_clipmap */
#ifdef GL_SGIX_convolution_accuracy
  CONST_CAST(GLEW_SGIX_convolution_accuracy) = _glewSearchExtension("GL_SGIX_convolution_accuracy", extStart, extEnd);
#endif /* GL_SGIX_convolution_accuracy */
#ifdef GL_SGIX_depth_texture
  CONST_CAST(GLEW_SGIX_depth_texture) = _glewSearchExtension("GL_SGIX_depth_texture", extStart, extEnd);
#endif /* GL_SGIX_depth_texture */
#ifdef GL_SGIX_flush_raster
  CONST_CAST(GLEW_SGIX_flush_raster) = _glewSearchExtension("GL_SGIX_flush_raster", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_flush_raster) CONST_CAST(GLEW_SGIX_flush_raster) = !_glewInit_GL_SGIX_flush_raster(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_flush_raster */
#ifdef GL_SGIX_fog_offset
  CONST_CAST(GLEW_SGIX_fog_offset) = _glewSearchExtension("GL_SGIX_fog_offset", extStart, extEnd);
#endif /* GL_SGIX_fog_offset */
#ifdef GL_SGIX_fog_texture
  CONST_CAST(GLEW_SGIX_fog_texture) = _glewSearchExtension("GL_SGIX_fog_texture", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_fog_texture) CONST_CAST(GLEW_SGIX_fog_texture) = !_glewInit_GL_SGIX_fog_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_fog_texture */
#ifdef GL_SGIX_fragment_specular_lighting
  CONST_CAST(GLEW_SGIX_fragment_specular_lighting) = _glewSearchExtension("GL_SGIX_fragment_specular_lighting", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_fragment_specular_lighting) CONST_CAST(GLEW_SGIX_fragment_specular_lighting) = !_glewInit_GL_SGIX_fragment_specular_lighting(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_fragment_specular_lighting */
#ifdef GL_SGIX_framezoom
  CONST_CAST(GLEW_SGIX_framezoom) = _glewSearchExtension("GL_SGIX_framezoom", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_framezoom) CONST_CAST(GLEW_SGIX_framezoom) = !_glewInit_GL_SGIX_framezoom(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_framezoom */
#ifdef GL_SGIX_interlace
  CONST_CAST(GLEW_SGIX_interlace) = _glewSearchExtension("GL_SGIX_interlace", extStart, extEnd);
#endif /* GL_SGIX_interlace */
#ifdef GL_SGIX_ir_instrument1
  CONST_CAST(GLEW_SGIX_ir_instrument1) = _glewSearchExtension("GL_SGIX_ir_instrument1", extStart, extEnd);
#endif /* GL_SGIX_ir_instrument1 */
#ifdef GL_SGIX_list_priority
  CONST_CAST(GLEW_SGIX_list_priority) = _glewSearchExtension("GL_SGIX_list_priority", extStart, extEnd);
#endif /* GL_SGIX_list_priority */
#ifdef GL_SGIX_pixel_texture
  CONST_CAST(GLEW_SGIX_pixel_texture) = _glewSearchExtension("GL_SGIX_pixel_texture", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_pixel_texture) CONST_CAST(GLEW_SGIX_pixel_texture) = !_glewInit_GL_SGIX_pixel_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_pixel_texture */
#ifdef GL_SGIX_pixel_texture_bits
  CONST_CAST(GLEW_SGIX_pixel_texture_bits) = _glewSearchExtension("GL_SGIX_pixel_texture_bits", extStart, extEnd);
#endif /* GL_SGIX_pixel_texture_bits */
#ifdef GL_SGIX_reference_plane
  CONST_CAST(GLEW_SGIX_reference_plane) = _glewSearchExtension("GL_SGIX_reference_plane", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_reference_plane) CONST_CAST(GLEW_SGIX_reference_plane) = !_glewInit_GL_SGIX_reference_plane(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_reference_plane */
#ifdef GL_SGIX_resample
  CONST_CAST(GLEW_SGIX_resample) = _glewSearchExtension("GL_SGIX_resample", extStart, extEnd);
#endif /* GL_SGIX_resample */
#ifdef GL_SGIX_shadow
  CONST_CAST(GLEW_SGIX_shadow) = _glewSearchExtension("GL_SGIX_shadow", extStart, extEnd);
#endif /* GL_SGIX_shadow */
#ifdef GL_SGIX_shadow_ambient
  CONST_CAST(GLEW_SGIX_shadow_ambient) = _glewSearchExtension("GL_SGIX_shadow_ambient", extStart, extEnd);
#endif /* GL_SGIX_shadow_ambient */
#ifdef GL_SGIX_sprite
  CONST_CAST(GLEW_SGIX_sprite) = _glewSearchExtension("GL_SGIX_sprite", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_sprite) CONST_CAST(GLEW_SGIX_sprite) = !_glewInit_GL_SGIX_sprite(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_sprite */
#ifdef GL_SGIX_tag_sample_buffer
  CONST_CAST(GLEW_SGIX_tag_sample_buffer) = _glewSearchExtension("GL_SGIX_tag_sample_buffer", extStart, extEnd);
  if (glewExperimental || GLEW_SGIX_tag_sample_buffer) CONST_CAST(GLEW_SGIX_tag_sample_buffer) = !_glewInit_GL_SGIX_tag_sample_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGIX_tag_sample_buffer */
#ifdef GL_SGIX_texture_add_env
  CONST_CAST(GLEW_SGIX_texture_add_env) = _glewSearchExtension("GL_SGIX_texture_add_env", extStart, extEnd);
#endif /* GL_SGIX_texture_add_env */
#ifdef GL_SGIX_texture_coordinate_clamp
  CONST_CAST(GLEW_SGIX_texture_coordinate_clamp) = _glewSearchExtension("GL_SGIX_texture_coordinate_clamp", extStart, extEnd);
#endif /* GL_SGIX_texture_coordinate_clamp */
#ifdef GL_SGIX_texture_lod_bias
  CONST_CAST(GLEW_SGIX_texture_lod_bias) = _glewSearchExtension("GL_SGIX_texture_lod_bias", extStart, extEnd);
#endif /* GL_SGIX_texture_lod_bias */
#ifdef GL_SGIX_texture_multi_buffer
  CONST_CAST(GLEW_SGIX_texture_multi_buffer) = _glewSearchExtension("GL_SGIX_texture_multi_buffer", extStart, extEnd);
#endif /* GL_SGIX_texture_multi_buffer */
#ifdef GL_SGIX_texture_range
  CONST_CAST(GLEW_SGIX_texture_range) = _glewSearchExtension("GL_SGIX_texture_range", extStart, extEnd);
#endif /* GL_SGIX_texture_range */
#ifdef GL_SGIX_texture_scale_bias
  CONST_CAST(GLEW_SGIX_texture_scale_bias) = _glewSearchExtension("GL_SGIX_texture_scale_bias", extStart, extEnd);
#endif /* GL_SGIX_texture_scale_bias */
#ifdef GL_SGIX_vertex_preclip
  CONST_CAST(GLEW_SGIX_vertex_preclip) = _glewSearchExtension("GL_SGIX_vertex_preclip", extStart, extEnd);
#endif /* GL_SGIX_vertex_preclip */
#ifdef GL_SGIX_vertex_preclip_hint
  CONST_CAST(GLEW_SGIX_vertex_preclip_hint) = _glewSearchExtension("GL_SGIX_vertex_preclip_hint", extStart, extEnd);
#endif /* GL_SGIX_vertex_preclip_hint */
#ifdef GL_SGIX_ycrcb
  CONST_CAST(GLEW_SGIX_ycrcb) = _glewSearchExtension("GL_SGIX_ycrcb", extStart, extEnd);
#endif /* GL_SGIX_ycrcb */
#ifdef GL_SGI_color_matrix
  CONST_CAST(GLEW_SGI_color_matrix) = _glewSearchExtension("GL_SGI_color_matrix", extStart, extEnd);
#endif /* GL_SGI_color_matrix */
#ifdef GL_SGI_color_table
  CONST_CAST(GLEW_SGI_color_table) = _glewSearchExtension("GL_SGI_color_table", extStart, extEnd);
  if (glewExperimental || GLEW_SGI_color_table) CONST_CAST(GLEW_SGI_color_table) = !_glewInit_GL_SGI_color_table(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SGI_color_table */
#ifdef GL_SGI_texture_color_table
  CONST_CAST(GLEW_SGI_texture_color_table) = _glewSearchExtension("GL_SGI_texture_color_table", extStart, extEnd);
#endif /* GL_SGI_texture_color_table */
#ifdef GL_SUNX_constant_data
  CONST_CAST(GLEW_SUNX_constant_data) = _glewSearchExtension("GL_SUNX_constant_data", extStart, extEnd);
  if (glewExperimental || GLEW_SUNX_constant_data) CONST_CAST(GLEW_SUNX_constant_data) = !_glewInit_GL_SUNX_constant_data(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUNX_constant_data */
#ifdef GL_SUN_convolution_border_modes
  CONST_CAST(GLEW_SUN_convolution_border_modes) = _glewSearchExtension("GL_SUN_convolution_border_modes", extStart, extEnd);
#endif /* GL_SUN_convolution_border_modes */
#ifdef GL_SUN_global_alpha
  CONST_CAST(GLEW_SUN_global_alpha) = _glewSearchExtension("GL_SUN_global_alpha", extStart, extEnd);
  if (glewExperimental || GLEW_SUN_global_alpha) CONST_CAST(GLEW_SUN_global_alpha) = !_glewInit_GL_SUN_global_alpha(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_global_alpha */
#ifdef GL_SUN_mesh_array
  CONST_CAST(GLEW_SUN_mesh_array) = _glewSearchExtension("GL_SUN_mesh_array", extStart, extEnd);
#endif /* GL_SUN_mesh_array */
#ifdef GL_SUN_read_video_pixels
  CONST_CAST(GLEW_SUN_read_video_pixels) = _glewSearchExtension("GL_SUN_read_video_pixels", extStart, extEnd);
  if (glewExperimental || GLEW_SUN_read_video_pixels) CONST_CAST(GLEW_SUN_read_video_pixels) = !_glewInit_GL_SUN_read_video_pixels(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_read_video_pixels */
#ifdef GL_SUN_slice_accum
  CONST_CAST(GLEW_SUN_slice_accum) = _glewSearchExtension("GL_SUN_slice_accum", extStart, extEnd);
#endif /* GL_SUN_slice_accum */
#ifdef GL_SUN_triangle_list
  CONST_CAST(GLEW_SUN_triangle_list) = _glewSearchExtension("GL_SUN_triangle_list", extStart, extEnd);
  if (glewExperimental || GLEW_SUN_triangle_list) CONST_CAST(GLEW_SUN_triangle_list) = !_glewInit_GL_SUN_triangle_list(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_triangle_list */
#ifdef GL_SUN_vertex
  CONST_CAST(GLEW_SUN_vertex) = _glewSearchExtension("GL_SUN_vertex", extStart, extEnd);
  if (glewExperimental || GLEW_SUN_vertex) CONST_CAST(GLEW_SUN_vertex) = !_glewInit_GL_SUN_vertex(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_SUN_vertex */
#ifdef GL_WIN_phong_shading
  CONST_CAST(GLEW_WIN_phong_shading) = _glewSearchExtension("GL_WIN_phong_shading", extStart, extEnd);
#endif /* GL_WIN_phong_shading */
#ifdef GL_WIN_specular_fog
  CONST_CAST(GLEW_WIN_specular_fog) = _glewSearchExtension("GL_WIN_specular_fog", extStart, extEnd);
#endif /* GL_WIN_specular_fog */
#ifdef GL_WIN_swap_hint
  CONST_CAST(GLEW_WIN_swap_hint) = _glewSearchExtension("GL_WIN_swap_hint", extStart, extEnd);
  if (glewExperimental || GLEW_WIN_swap_hint) CONST_CAST(GLEW_WIN_swap_hint) = !_glewInit_GL_WIN_swap_hint(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GL_WIN_swap_hint */

  return GLEW_OK;
}


#if defined(_WIN32)

#if !defined(GLEW_MX)

PFNWGLSETSTEREOEMITTERSTATE3DLPROC __wglewSetStereoEmitterState3DL = nullptr;

PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC __wglewBlitContextFramebufferAMD = nullptr;
PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC __wglewCreateAssociatedContextAMD = nullptr;
PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC __wglewCreateAssociatedContextAttribsAMD = nullptr;
PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC __wglewDeleteAssociatedContextAMD = nullptr;
PFNWGLGETCONTEXTGPUIDAMDPROC __wglewGetContextGPUIDAMD = nullptr;
PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC __wglewGetCurrentAssociatedContextAMD = nullptr;
PFNWGLGETGPUIDSAMDPROC __wglewGetGPUIDsAMD = nullptr;
PFNWGLGETGPUINFOAMDPROC __wglewGetGPUInfoAMD = nullptr;
PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC __wglewMakeAssociatedContextCurrentAMD = nullptr;

PFNWGLCREATEBUFFERREGIONARBPROC __wglewCreateBufferRegionARB = nullptr;
PFNWGLDELETEBUFFERREGIONARBPROC __wglewDeleteBufferRegionARB = nullptr;
PFNWGLRESTOREBUFFERREGIONARBPROC __wglewRestoreBufferRegionARB = nullptr;
PFNWGLSAVEBUFFERREGIONARBPROC __wglewSaveBufferRegionARB = nullptr;

PFNWGLCREATECONTEXTATTRIBSARBPROC __wglewCreateContextAttribsARB = nullptr;

PFNWGLGETEXTENSIONSSTRINGARBPROC __wglewGetExtensionsStringARB = nullptr;

PFNWGLGETCURRENTREADDCARBPROC __wglewGetCurrentReadDCARB = nullptr;
PFNWGLMAKECONTEXTCURRENTARBPROC __wglewMakeContextCurrentARB = nullptr;

PFNWGLCREATEPBUFFERARBPROC __wglewCreatePbufferARB = nullptr;
PFNWGLDESTROYPBUFFERARBPROC __wglewDestroyPbufferARB = nullptr;
PFNWGLGETPBUFFERDCARBPROC __wglewGetPbufferDCARB = nullptr;
PFNWGLQUERYPBUFFERARBPROC __wglewQueryPbufferARB = nullptr;
PFNWGLRELEASEPBUFFERDCARBPROC __wglewReleasePbufferDCARB = nullptr;

PFNWGLCHOOSEPIXELFORMATARBPROC __wglewChoosePixelFormatARB = nullptr;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC __wglewGetPixelFormatAttribfvARB = nullptr;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC __wglewGetPixelFormatAttribivARB = nullptr;

PFNWGLBINDTEXIMAGEARBPROC __wglewBindTexImageARB = nullptr;
PFNWGLRELEASETEXIMAGEARBPROC __wglewReleaseTexImageARB = nullptr;
PFNWGLSETPBUFFERATTRIBARBPROC __wglewSetPbufferAttribARB = nullptr;

PFNWGLBINDDISPLAYCOLORTABLEEXTPROC __wglewBindDisplayColorTableEXT = nullptr;
PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC __wglewCreateDisplayColorTableEXT = nullptr;
PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC __wglewDestroyDisplayColorTableEXT = nullptr;
PFNWGLLOADDISPLAYCOLORTABLEEXTPROC __wglewLoadDisplayColorTableEXT = nullptr;

PFNWGLGETEXTENSIONSSTRINGEXTPROC __wglewGetExtensionsStringEXT = nullptr;

PFNWGLGETCURRENTREADDCEXTPROC __wglewGetCurrentReadDCEXT = nullptr;
PFNWGLMAKECONTEXTCURRENTEXTPROC __wglewMakeContextCurrentEXT = nullptr;

PFNWGLCREATEPBUFFEREXTPROC __wglewCreatePbufferEXT = nullptr;
PFNWGLDESTROYPBUFFEREXTPROC __wglewDestroyPbufferEXT = nullptr;
PFNWGLGETPBUFFERDCEXTPROC __wglewGetPbufferDCEXT = nullptr;
PFNWGLQUERYPBUFFEREXTPROC __wglewQueryPbufferEXT = nullptr;
PFNWGLRELEASEPBUFFERDCEXTPROC __wglewReleasePbufferDCEXT = nullptr;

PFNWGLCHOOSEPIXELFORMATEXTPROC __wglewChoosePixelFormatEXT = nullptr;
PFNWGLGETPIXELFORMATATTRIBFVEXTPROC __wglewGetPixelFormatAttribfvEXT = nullptr;
PFNWGLGETPIXELFORMATATTRIBIVEXTPROC __wglewGetPixelFormatAttribivEXT = nullptr;

PFNWGLGETSWAPINTERVALEXTPROC __wglewGetSwapIntervalEXT = nullptr;
PFNWGLSWAPINTERVALEXTPROC __wglewSwapIntervalEXT = nullptr;

PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC __wglewGetDigitalVideoParametersI3D = nullptr;
PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC __wglewSetDigitalVideoParametersI3D = nullptr;

PFNWGLGETGAMMATABLEI3DPROC __wglewGetGammaTableI3D = nullptr;
PFNWGLGETGAMMATABLEPARAMETERSI3DPROC __wglewGetGammaTableParametersI3D = nullptr;
PFNWGLSETGAMMATABLEI3DPROC __wglewSetGammaTableI3D = nullptr;
PFNWGLSETGAMMATABLEPARAMETERSI3DPROC __wglewSetGammaTableParametersI3D = nullptr;

PFNWGLDISABLEGENLOCKI3DPROC __wglewDisableGenlockI3D = nullptr;
PFNWGLENABLEGENLOCKI3DPROC __wglewEnableGenlockI3D = nullptr;
PFNWGLGENLOCKSAMPLERATEI3DPROC __wglewGenlockSampleRateI3D = nullptr;
PFNWGLGENLOCKSOURCEDELAYI3DPROC __wglewGenlockSourceDelayI3D = nullptr;
PFNWGLGENLOCKSOURCEEDGEI3DPROC __wglewGenlockSourceEdgeI3D = nullptr;
PFNWGLGENLOCKSOURCEI3DPROC __wglewGenlockSourceI3D = nullptr;
PFNWGLGETGENLOCKSAMPLERATEI3DPROC __wglewGetGenlockSampleRateI3D = nullptr;
PFNWGLGETGENLOCKSOURCEDELAYI3DPROC __wglewGetGenlockSourceDelayI3D = nullptr;
PFNWGLGETGENLOCKSOURCEEDGEI3DPROC __wglewGetGenlockSourceEdgeI3D = nullptr;
PFNWGLGETGENLOCKSOURCEI3DPROC __wglewGetGenlockSourceI3D = nullptr;
PFNWGLISENABLEDGENLOCKI3DPROC __wglewIsEnabledGenlockI3D = nullptr;
PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC __wglewQueryGenlockMaxSourceDelayI3D = nullptr;

PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC __wglewAssociateImageBufferEventsI3D = nullptr;
PFNWGLCREATEIMAGEBUFFERI3DPROC __wglewCreateImageBufferI3D = nullptr;
PFNWGLDESTROYIMAGEBUFFERI3DPROC __wglewDestroyImageBufferI3D = nullptr;
PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC __wglewReleaseImageBufferEventsI3D = nullptr;

PFNWGLDISABLEFRAMELOCKI3DPROC __wglewDisableFrameLockI3D = nullptr;
PFNWGLENABLEFRAMELOCKI3DPROC __wglewEnableFrameLockI3D = nullptr;
PFNWGLISENABLEDFRAMELOCKI3DPROC __wglewIsEnabledFrameLockI3D = nullptr;
PFNWGLQUERYFRAMELOCKMASTERI3DPROC __wglewQueryFrameLockMasterI3D = nullptr;

PFNWGLBEGINFRAMETRACKINGI3DPROC __wglewBeginFrameTrackingI3D = nullptr;
PFNWGLENDFRAMETRACKINGI3DPROC __wglewEndFrameTrackingI3D = nullptr;
PFNWGLGETFRAMEUSAGEI3DPROC __wglewGetFrameUsageI3D = nullptr;
PFNWGLQUERYFRAMETRACKINGI3DPROC __wglewQueryFrameTrackingI3D = nullptr;

PFNWGLDXCLOSEDEVICENVPROC __wglewDXCloseDeviceNV = nullptr;
PFNWGLDXLOCKOBJECTSNVPROC __wglewDXLockObjectsNV = nullptr;
PFNWGLDXOBJECTACCESSNVPROC __wglewDXObjectAccessNV = nullptr;
PFNWGLDXOPENDEVICENVPROC __wglewDXOpenDeviceNV = nullptr;
PFNWGLDXREGISTEROBJECTNVPROC __wglewDXRegisterObjectNV = nullptr;
PFNWGLDXSETRESOURCESHAREHANDLENVPROC __wglewDXSetResourceShareHandleNV = nullptr;
PFNWGLDXUNLOCKOBJECTSNVPROC __wglewDXUnlockObjectsNV = nullptr;
PFNWGLDXUNREGISTEROBJECTNVPROC __wglewDXUnregisterObjectNV = nullptr;

PFNWGLCOPYIMAGESUBDATANVPROC __wglewCopyImageSubDataNV = nullptr;

PFNWGLCREATEAFFINITYDCNVPROC __wglewCreateAffinityDCNV = nullptr;
PFNWGLDELETEDCNVPROC __wglewDeleteDCNV = nullptr;
PFNWGLENUMGPUDEVICESNVPROC __wglewEnumGpuDevicesNV = nullptr;
PFNWGLENUMGPUSFROMAFFINITYDCNVPROC __wglewEnumGpusFromAffinityDCNV = nullptr;
PFNWGLENUMGPUSNVPROC __wglewEnumGpusNV = nullptr;

PFNWGLBINDVIDEODEVICENVPROC __wglewBindVideoDeviceNV = nullptr;
PFNWGLENUMERATEVIDEODEVICESNVPROC __wglewEnumerateVideoDevicesNV = nullptr;
PFNWGLQUERYCURRENTCONTEXTNVPROC __wglewQueryCurrentContextNV = nullptr;

PFNWGLBINDSWAPBARRIERNVPROC __wglewBindSwapBarrierNV = nullptr;
PFNWGLJOINSWAPGROUPNVPROC __wglewJoinSwapGroupNV = nullptr;
PFNWGLQUERYFRAMECOUNTNVPROC __wglewQueryFrameCountNV = nullptr;
PFNWGLQUERYMAXSWAPGROUPSNVPROC __wglewQueryMaxSwapGroupsNV = nullptr;
PFNWGLQUERYSWAPGROUPNVPROC __wglewQuerySwapGroupNV = nullptr;
PFNWGLRESETFRAMECOUNTNVPROC __wglewResetFrameCountNV = nullptr;

PFNWGLALLOCATEMEMORYNVPROC __wglewAllocateMemoryNV = nullptr;
PFNWGLFREEMEMORYNVPROC __wglewFreeMemoryNV = nullptr;

PFNWGLBINDVIDEOCAPTUREDEVICENVPROC __wglewBindVideoCaptureDeviceNV = nullptr;
PFNWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC __wglewEnumerateVideoCaptureDevicesNV = nullptr;
PFNWGLLOCKVIDEOCAPTUREDEVICENVPROC __wglewLockVideoCaptureDeviceNV = nullptr;
PFNWGLQUERYVIDEOCAPTUREDEVICENVPROC __wglewQueryVideoCaptureDeviceNV = nullptr;
PFNWGLRELEASEVIDEOCAPTUREDEVICENVPROC __wglewReleaseVideoCaptureDeviceNV = nullptr;

PFNWGLBINDVIDEOIMAGENVPROC __wglewBindVideoImageNV = nullptr;
PFNWGLGETVIDEODEVICENVPROC __wglewGetVideoDeviceNV = nullptr;
PFNWGLGETVIDEOINFONVPROC __wglewGetVideoInfoNV = nullptr;
PFNWGLRELEASEVIDEODEVICENVPROC __wglewReleaseVideoDeviceNV = nullptr;
PFNWGLRELEASEVIDEOIMAGENVPROC __wglewReleaseVideoImageNV = nullptr;
PFNWGLSENDPBUFFERTOVIDEONVPROC __wglewSendPbufferToVideoNV = nullptr;

PFNWGLGETMSCRATEOMLPROC __wglewGetMscRateOML = nullptr;
PFNWGLGETSYNCVALUESOMLPROC __wglewGetSyncValuesOML = nullptr;
PFNWGLSWAPBUFFERSMSCOMLPROC __wglewSwapBuffersMscOML = nullptr;
PFNWGLSWAPLAYERBUFFERSMSCOMLPROC __wglewSwapLayerBuffersMscOML = nullptr;
PFNWGLWAITFORMSCOMLPROC __wglewWaitForMscOML = nullptr;
PFNWGLWAITFORSBCOMLPROC __wglewWaitForSbcOML = nullptr;
GLboolean __WGLEW_3DFX_multisample = GL_FALSE;
GLboolean __WGLEW_3DL_stereo_control = GL_FALSE;
GLboolean __WGLEW_AMD_gpu_association = GL_FALSE;
GLboolean __WGLEW_ARB_buffer_region = GL_FALSE;
GLboolean __WGLEW_ARB_create_context = GL_FALSE;
GLboolean __WGLEW_ARB_create_context_profile = GL_FALSE;
GLboolean __WGLEW_ARB_create_context_robustness = GL_FALSE;
GLboolean __WGLEW_ARB_extensions_string = GL_FALSE;
GLboolean __WGLEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __WGLEW_ARB_make_current_read = GL_FALSE;
GLboolean __WGLEW_ARB_multisample = GL_FALSE;
GLboolean __WGLEW_ARB_pbuffer = GL_FALSE;
GLboolean __WGLEW_ARB_pixel_format = GL_FALSE;
GLboolean __WGLEW_ARB_pixel_format_float = GL_FALSE;
GLboolean __WGLEW_ARB_render_texture = GL_FALSE;
GLboolean __WGLEW_ATI_pixel_format_float = GL_FALSE;
GLboolean __WGLEW_ATI_render_texture_rectangle = GL_FALSE;
GLboolean __WGLEW_EXT_create_context_es2_profile = GL_FALSE;
GLboolean __WGLEW_EXT_depth_float = GL_FALSE;
GLboolean __WGLEW_EXT_display_color_table = GL_FALSE;
GLboolean __WGLEW_EXT_extensions_string = GL_FALSE;
GLboolean __WGLEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __WGLEW_EXT_make_current_read = GL_FALSE;
GLboolean __WGLEW_EXT_multisample = GL_FALSE;
GLboolean __WGLEW_EXT_pbuffer = GL_FALSE;
GLboolean __WGLEW_EXT_pixel_format = GL_FALSE;
GLboolean __WGLEW_EXT_pixel_format_packed_float = GL_FALSE;
GLboolean __WGLEW_EXT_swap_control = GL_FALSE;
GLboolean __WGLEW_I3D_digital_video_control = GL_FALSE;
GLboolean __WGLEW_I3D_gamma = GL_FALSE;
GLboolean __WGLEW_I3D_genlock = GL_FALSE;
GLboolean __WGLEW_I3D_image_buffer = GL_FALSE;
GLboolean __WGLEW_I3D_swap_frame_lock = GL_FALSE;
GLboolean __WGLEW_I3D_swap_frame_usage = GL_FALSE;
GLboolean __WGLEW_NV_DX_interop = GL_FALSE;
GLboolean __WGLEW_NV_copy_image = GL_FALSE;
GLboolean __WGLEW_NV_float_buffer = GL_FALSE;
GLboolean __WGLEW_NV_gpu_affinity = GL_FALSE;
GLboolean __WGLEW_NV_multisample_coverage = GL_FALSE;
GLboolean __WGLEW_NV_present_video = GL_FALSE;
GLboolean __WGLEW_NV_render_depth_texture = GL_FALSE;
GLboolean __WGLEW_NV_render_texture_rectangle = GL_FALSE;
GLboolean __WGLEW_NV_swap_group = GL_FALSE;
GLboolean __WGLEW_NV_vertex_array_range = GL_FALSE;
GLboolean __WGLEW_NV_video_capture = GL_FALSE;
GLboolean __WGLEW_NV_video_output = GL_FALSE;
GLboolean __WGLEW_OML_sync_control = GL_FALSE;

#endif /* !GLEW_MX */

#ifdef WGL_3DFX_multisample

#endif /* WGL_3DFX_multisample */

#ifdef WGL_3DL_stereo_control

static GLboolean _glewInit_WGL_3DL_stereo_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglSetStereoEmitterState3DL = (PFNWGLSETSTEREOEMITTERSTATE3DLPROC)glewGetProcAddress((const GLubyte*)"wglSetStereoEmitterState3DL")) == nullptr) || r;

  return r;
}

#endif /* WGL_3DL_stereo_control */

#ifdef WGL_AMD_gpu_association

static GLboolean _glewInit_WGL_AMD_gpu_association (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBlitContextFramebufferAMD = (PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC)glewGetProcAddress((const GLubyte*)"wglBlitContextFramebufferAMD")) == nullptr) || r;
  r = ((wglCreateAssociatedContextAMD = (PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC)glewGetProcAddress((const GLubyte*)"wglCreateAssociatedContextAMD")) == nullptr) || r;
  r = ((wglCreateAssociatedContextAttribsAMD = (PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC)glewGetProcAddress((const GLubyte*)"wglCreateAssociatedContextAttribsAMD")) == nullptr) || r;
  r = ((wglDeleteAssociatedContextAMD = (PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC)glewGetProcAddress((const GLubyte*)"wglDeleteAssociatedContextAMD")) == nullptr) || r;
  r = ((wglGetContextGPUIDAMD = (PFNWGLGETCONTEXTGPUIDAMDPROC)glewGetProcAddress((const GLubyte*)"wglGetContextGPUIDAMD")) == nullptr) || r;
  r = ((wglGetCurrentAssociatedContextAMD = (PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC)glewGetProcAddress((const GLubyte*)"wglGetCurrentAssociatedContextAMD")) == nullptr) || r;
  r = ((wglGetGPUIDsAMD = (PFNWGLGETGPUIDSAMDPROC)glewGetProcAddress((const GLubyte*)"wglGetGPUIDsAMD")) == nullptr) || r;
  r = ((wglGetGPUInfoAMD = (PFNWGLGETGPUINFOAMDPROC)glewGetProcAddress((const GLubyte*)"wglGetGPUInfoAMD")) == nullptr) || r;
  r = ((wglMakeAssociatedContextCurrentAMD = (PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC)glewGetProcAddress((const GLubyte*)"wglMakeAssociatedContextCurrentAMD")) == nullptr) || r;

  return r;
}

#endif /* WGL_AMD_gpu_association */

#ifdef WGL_ARB_buffer_region

static GLboolean _glewInit_WGL_ARB_buffer_region (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreateBufferRegionARB = (PFNWGLCREATEBUFFERREGIONARBPROC)glewGetProcAddress((const GLubyte*)"wglCreateBufferRegionARB")) == nullptr) || r;
  r = ((wglDeleteBufferRegionARB = (PFNWGLDELETEBUFFERREGIONARBPROC)glewGetProcAddress((const GLubyte*)"wglDeleteBufferRegionARB")) == nullptr) || r;
  r = ((wglRestoreBufferRegionARB = (PFNWGLRESTOREBUFFERREGIONARBPROC)glewGetProcAddress((const GLubyte*)"wglRestoreBufferRegionARB")) == nullptr) || r;
  r = ((wglSaveBufferRegionARB = (PFNWGLSAVEBUFFERREGIONARBPROC)glewGetProcAddress((const GLubyte*)"wglSaveBufferRegionARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_buffer_region */

#ifdef WGL_ARB_create_context

static GLboolean _glewInit_WGL_ARB_create_context (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)glewGetProcAddress((const GLubyte*)"wglCreateContextAttribsARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_create_context */

#ifdef WGL_ARB_create_context_profile

#endif /* WGL_ARB_create_context_profile */

#ifdef WGL_ARB_create_context_robustness

#endif /* WGL_ARB_create_context_robustness */

#ifdef WGL_ARB_extensions_string

static GLboolean _glewInit_WGL_ARB_extensions_string (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_extensions_string */

#ifdef WGL_ARB_framebuffer_sRGB

#endif /* WGL_ARB_framebuffer_sRGB */

#ifdef WGL_ARB_make_current_read

static GLboolean _glewInit_WGL_ARB_make_current_read (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetCurrentReadDCARB = (PFNWGLGETCURRENTREADDCARBPROC)glewGetProcAddress((const GLubyte*)"wglGetCurrentReadDCARB")) == nullptr) || r;
  r = ((wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)glewGetProcAddress((const GLubyte*)"wglMakeContextCurrentARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_make_current_read */

#ifdef WGL_ARB_multisample

#endif /* WGL_ARB_multisample */

#ifdef WGL_ARB_pbuffer

static GLboolean _glewInit_WGL_ARB_pbuffer (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"wglCreatePbufferARB")) == nullptr) || r;
  r = ((wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"wglDestroyPbufferARB")) == nullptr) || r;
  r = ((wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)glewGetProcAddress((const GLubyte*)"wglGetPbufferDCARB")) == nullptr) || r;
  r = ((wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)glewGetProcAddress((const GLubyte*)"wglQueryPbufferARB")) == nullptr) || r;
  r = ((wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)glewGetProcAddress((const GLubyte*)"wglReleasePbufferDCARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_pbuffer */

#ifdef WGL_ARB_pixel_format

static GLboolean _glewInit_WGL_ARB_pixel_format (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)glewGetProcAddress((const GLubyte*)"wglChoosePixelFormatARB")) == nullptr) || r;
  r = ((wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)glewGetProcAddress((const GLubyte*)"wglGetPixelFormatAttribfvARB")) == nullptr) || r;
  r = ((wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)glewGetProcAddress((const GLubyte*)"wglGetPixelFormatAttribivARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_pixel_format */

#ifdef WGL_ARB_pixel_format_float

#endif /* WGL_ARB_pixel_format_float */

#ifdef WGL_ARB_render_texture

static GLboolean _glewInit_WGL_ARB_render_texture (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"wglBindTexImageARB")) == nullptr) || r;
  r = ((wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"wglReleaseTexImageARB")) == nullptr) || r;
  r = ((wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)glewGetProcAddress((const GLubyte*)"wglSetPbufferAttribARB")) == nullptr) || r;

  return r;
}

#endif /* WGL_ARB_render_texture */

#ifdef WGL_ATI_pixel_format_float

#endif /* WGL_ATI_pixel_format_float */

#ifdef WGL_ATI_render_texture_rectangle

#endif /* WGL_ATI_render_texture_rectangle */

#ifdef WGL_EXT_create_context_es2_profile

#endif /* WGL_EXT_create_context_es2_profile */

#ifdef WGL_EXT_depth_float

#endif /* WGL_EXT_depth_float */

#ifdef WGL_EXT_display_color_table

static GLboolean _glewInit_WGL_EXT_display_color_table (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindDisplayColorTableEXT = (PFNWGLBINDDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"wglBindDisplayColorTableEXT")) == nullptr) || r;
  r = ((wglCreateDisplayColorTableEXT = (PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"wglCreateDisplayColorTableEXT")) == nullptr) || r;
  r = ((wglDestroyDisplayColorTableEXT = (PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"wglDestroyDisplayColorTableEXT")) == nullptr) || r;
  r = ((wglLoadDisplayColorTableEXT = (PFNWGLLOADDISPLAYCOLORTABLEEXTPROC)glewGetProcAddress((const GLubyte*)"wglLoadDisplayColorTableEXT")) == nullptr) || r;

  return r;
}

#endif /* WGL_EXT_display_color_table */

#ifdef WGL_EXT_extensions_string

static GLboolean _glewInit_WGL_EXT_extensions_string (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT")) == nullptr) || r;

  return r;
}

#endif /* WGL_EXT_extensions_string */

#ifdef WGL_EXT_framebuffer_sRGB

#endif /* WGL_EXT_framebuffer_sRGB */

#ifdef WGL_EXT_make_current_read

static GLboolean _glewInit_WGL_EXT_make_current_read (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetCurrentReadDCEXT = (PFNWGLGETCURRENTREADDCEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetCurrentReadDCEXT")) == nullptr) || r;
  r = ((wglMakeContextCurrentEXT = (PFNWGLMAKECONTEXTCURRENTEXTPROC)glewGetProcAddress((const GLubyte*)"wglMakeContextCurrentEXT")) == nullptr) || r;

  return r;
}

#endif /* WGL_EXT_make_current_read */

#ifdef WGL_EXT_multisample

#endif /* WGL_EXT_multisample */

#ifdef WGL_EXT_pbuffer

static GLboolean _glewInit_WGL_EXT_pbuffer (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreatePbufferEXT = (PFNWGLCREATEPBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"wglCreatePbufferEXT")) == nullptr) || r;
  r = ((wglDestroyPbufferEXT = (PFNWGLDESTROYPBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"wglDestroyPbufferEXT")) == nullptr) || r;
  r = ((wglGetPbufferDCEXT = (PFNWGLGETPBUFFERDCEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetPbufferDCEXT")) == nullptr) || r;
  r = ((wglQueryPbufferEXT = (PFNWGLQUERYPBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"wglQueryPbufferEXT")) == nullptr) || r;
  r = ((wglReleasePbufferDCEXT = (PFNWGLRELEASEPBUFFERDCEXTPROC)glewGetProcAddress((const GLubyte*)"wglReleasePbufferDCEXT")) == nullptr) || r;

  return r;
}

#endif /* WGL_EXT_pbuffer */

#ifdef WGL_EXT_pixel_format

static GLboolean _glewInit_WGL_EXT_pixel_format (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglChoosePixelFormatEXT = (PFNWGLCHOOSEPIXELFORMATEXTPROC)glewGetProcAddress((const GLubyte*)"wglChoosePixelFormatEXT")) == nullptr) || r;
  r = ((wglGetPixelFormatAttribfvEXT = (PFNWGLGETPIXELFORMATATTRIBFVEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetPixelFormatAttribfvEXT")) == nullptr) || r;
  r = ((wglGetPixelFormatAttribivEXT = (PFNWGLGETPIXELFORMATATTRIBIVEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetPixelFormatAttribivEXT")) == nullptr) || r;

  return r;
}

#endif /* WGL_EXT_pixel_format */

#ifdef WGL_EXT_pixel_format_packed_float

#endif /* WGL_EXT_pixel_format_packed_float */

#ifdef WGL_EXT_swap_control

static GLboolean _glewInit_WGL_EXT_swap_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetSwapIntervalEXT")) == nullptr) || r;
  r = ((wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)glewGetProcAddress((const GLubyte*)"wglSwapIntervalEXT")) == nullptr) || r;

  return r;
}

#endif /* WGL_EXT_swap_control */

#ifdef WGL_I3D_digital_video_control

static GLboolean _glewInit_WGL_I3D_digital_video_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetDigitalVideoParametersI3D = (PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetDigitalVideoParametersI3D")) == nullptr) || r;
  r = ((wglSetDigitalVideoParametersI3D = (PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC)glewGetProcAddress((const GLubyte*)"wglSetDigitalVideoParametersI3D")) == nullptr) || r;

  return r;
}

#endif /* WGL_I3D_digital_video_control */

#ifdef WGL_I3D_gamma

static GLboolean _glewInit_WGL_I3D_gamma (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetGammaTableI3D = (PFNWGLGETGAMMATABLEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetGammaTableI3D")) == nullptr) || r;
  r = ((wglGetGammaTableParametersI3D = (PFNWGLGETGAMMATABLEPARAMETERSI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetGammaTableParametersI3D")) == nullptr) || r;
  r = ((wglSetGammaTableI3D = (PFNWGLSETGAMMATABLEI3DPROC)glewGetProcAddress((const GLubyte*)"wglSetGammaTableI3D")) == nullptr) || r;
  r = ((wglSetGammaTableParametersI3D = (PFNWGLSETGAMMATABLEPARAMETERSI3DPROC)glewGetProcAddress((const GLubyte*)"wglSetGammaTableParametersI3D")) == nullptr) || r;

  return r;
}

#endif /* WGL_I3D_gamma */

#ifdef WGL_I3D_genlock

static GLboolean _glewInit_WGL_I3D_genlock (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglDisableGenlockI3D = (PFNWGLDISABLEGENLOCKI3DPROC)glewGetProcAddress((const GLubyte*)"wglDisableGenlockI3D")) == nullptr) || r;
  r = ((wglEnableGenlockI3D = (PFNWGLENABLEGENLOCKI3DPROC)glewGetProcAddress((const GLubyte*)"wglEnableGenlockI3D")) == nullptr) || r;
  r = ((wglGenlockSampleRateI3D = (PFNWGLGENLOCKSAMPLERATEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGenlockSampleRateI3D")) == nullptr) || r;
  r = ((wglGenlockSourceDelayI3D = (PFNWGLGENLOCKSOURCEDELAYI3DPROC)glewGetProcAddress((const GLubyte*)"wglGenlockSourceDelayI3D")) == nullptr) || r;
  r = ((wglGenlockSourceEdgeI3D = (PFNWGLGENLOCKSOURCEEDGEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGenlockSourceEdgeI3D")) == nullptr) || r;
  r = ((wglGenlockSourceI3D = (PFNWGLGENLOCKSOURCEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGenlockSourceI3D")) == nullptr) || r;
  r = ((wglGetGenlockSampleRateI3D = (PFNWGLGETGENLOCKSAMPLERATEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetGenlockSampleRateI3D")) == nullptr) || r;
  r = ((wglGetGenlockSourceDelayI3D = (PFNWGLGETGENLOCKSOURCEDELAYI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetGenlockSourceDelayI3D")) == nullptr) || r;
  r = ((wglGetGenlockSourceEdgeI3D = (PFNWGLGETGENLOCKSOURCEEDGEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetGenlockSourceEdgeI3D")) == nullptr) || r;
  r = ((wglGetGenlockSourceI3D = (PFNWGLGETGENLOCKSOURCEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetGenlockSourceI3D")) == nullptr) || r;
  r = ((wglIsEnabledGenlockI3D = (PFNWGLISENABLEDGENLOCKI3DPROC)glewGetProcAddress((const GLubyte*)"wglIsEnabledGenlockI3D")) == nullptr) || r;
  r = ((wglQueryGenlockMaxSourceDelayI3D = (PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC)glewGetProcAddress((const GLubyte*)"wglQueryGenlockMaxSourceDelayI3D")) == nullptr) || r;

  return r;
}

#endif /* WGL_I3D_genlock */

#ifdef WGL_I3D_image_buffer

static GLboolean _glewInit_WGL_I3D_image_buffer (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglAssociateImageBufferEventsI3D = (PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC)glewGetProcAddress((const GLubyte*)"wglAssociateImageBufferEventsI3D")) == nullptr) || r;
  r = ((wglCreateImageBufferI3D = (PFNWGLCREATEIMAGEBUFFERI3DPROC)glewGetProcAddress((const GLubyte*)"wglCreateImageBufferI3D")) == nullptr) || r;
  r = ((wglDestroyImageBufferI3D = (PFNWGLDESTROYIMAGEBUFFERI3DPROC)glewGetProcAddress((const GLubyte*)"wglDestroyImageBufferI3D")) == nullptr) || r;
  r = ((wglReleaseImageBufferEventsI3D = (PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC)glewGetProcAddress((const GLubyte*)"wglReleaseImageBufferEventsI3D")) == nullptr) || r;

  return r;
}

#endif /* WGL_I3D_image_buffer */

#ifdef WGL_I3D_swap_frame_lock

static GLboolean _glewInit_WGL_I3D_swap_frame_lock (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglDisableFrameLockI3D = (PFNWGLDISABLEFRAMELOCKI3DPROC)glewGetProcAddress((const GLubyte*)"wglDisableFrameLockI3D")) == nullptr) || r;
  r = ((wglEnableFrameLockI3D = (PFNWGLENABLEFRAMELOCKI3DPROC)glewGetProcAddress((const GLubyte*)"wglEnableFrameLockI3D")) == nullptr) || r;
  r = ((wglIsEnabledFrameLockI3D = (PFNWGLISENABLEDFRAMELOCKI3DPROC)glewGetProcAddress((const GLubyte*)"wglIsEnabledFrameLockI3D")) == nullptr) || r;
  r = ((wglQueryFrameLockMasterI3D = (PFNWGLQUERYFRAMELOCKMASTERI3DPROC)glewGetProcAddress((const GLubyte*)"wglQueryFrameLockMasterI3D")) == nullptr) || r;

  return r;
}

#endif /* WGL_I3D_swap_frame_lock */

#ifdef WGL_I3D_swap_frame_usage

static GLboolean _glewInit_WGL_I3D_swap_frame_usage (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBeginFrameTrackingI3D = (PFNWGLBEGINFRAMETRACKINGI3DPROC)glewGetProcAddress((const GLubyte*)"wglBeginFrameTrackingI3D")) == nullptr) || r;
  r = ((wglEndFrameTrackingI3D = (PFNWGLENDFRAMETRACKINGI3DPROC)glewGetProcAddress((const GLubyte*)"wglEndFrameTrackingI3D")) == nullptr) || r;
  r = ((wglGetFrameUsageI3D = (PFNWGLGETFRAMEUSAGEI3DPROC)glewGetProcAddress((const GLubyte*)"wglGetFrameUsageI3D")) == nullptr) || r;
  r = ((wglQueryFrameTrackingI3D = (PFNWGLQUERYFRAMETRACKINGI3DPROC)glewGetProcAddress((const GLubyte*)"wglQueryFrameTrackingI3D")) == nullptr) || r;

  return r;
}

#endif /* WGL_I3D_swap_frame_usage */

#ifdef WGL_NV_DX_interop

static GLboolean _glewInit_WGL_NV_DX_interop (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglDXCloseDeviceNV")) == nullptr) || r;
  r = ((wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)glewGetProcAddress((const GLubyte*)"wglDXLockObjectsNV")) == nullptr) || r;
  r = ((wglDXObjectAccessNV = (PFNWGLDXOBJECTACCESSNVPROC)glewGetProcAddress((const GLubyte*)"wglDXObjectAccessNV")) == nullptr) || r;
  r = ((wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglDXOpenDeviceNV")) == nullptr) || r;
  r = ((wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)glewGetProcAddress((const GLubyte*)"wglDXRegisterObjectNV")) == nullptr) || r;
  r = ((wglDXSetResourceShareHandleNV = (PFNWGLDXSETRESOURCESHAREHANDLENVPROC)glewGetProcAddress((const GLubyte*)"wglDXSetResourceShareHandleNV")) == nullptr) || r;
  r = ((wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)glewGetProcAddress((const GLubyte*)"wglDXUnlockObjectsNV")) == nullptr) || r;
  r = ((wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)glewGetProcAddress((const GLubyte*)"wglDXUnregisterObjectNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_DX_interop */

#ifdef WGL_NV_copy_image

static GLboolean _glewInit_WGL_NV_copy_image (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCopyImageSubDataNV = (PFNWGLCOPYIMAGESUBDATANVPROC)glewGetProcAddress((const GLubyte*)"wglCopyImageSubDataNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_copy_image */

#ifdef WGL_NV_float_buffer

#endif /* WGL_NV_float_buffer */

#ifdef WGL_NV_gpu_affinity

static GLboolean _glewInit_WGL_NV_gpu_affinity (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglCreateAffinityDCNV = (PFNWGLCREATEAFFINITYDCNVPROC)glewGetProcAddress((const GLubyte*)"wglCreateAffinityDCNV")) == nullptr) || r;
  r = ((wglDeleteDCNV = (PFNWGLDELETEDCNVPROC)glewGetProcAddress((const GLubyte*)"wglDeleteDCNV")) == nullptr) || r;
  r = ((wglEnumGpuDevicesNV = (PFNWGLENUMGPUDEVICESNVPROC)glewGetProcAddress((const GLubyte*)"wglEnumGpuDevicesNV")) == nullptr) || r;
  r = ((wglEnumGpusFromAffinityDCNV = (PFNWGLENUMGPUSFROMAFFINITYDCNVPROC)glewGetProcAddress((const GLubyte*)"wglEnumGpusFromAffinityDCNV")) == nullptr) || r;
  r = ((wglEnumGpusNV = (PFNWGLENUMGPUSNVPROC)glewGetProcAddress((const GLubyte*)"wglEnumGpusNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_gpu_affinity */

#ifdef WGL_NV_multisample_coverage

#endif /* WGL_NV_multisample_coverage */

#ifdef WGL_NV_present_video

static GLboolean _glewInit_WGL_NV_present_video (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindVideoDeviceNV = (PFNWGLBINDVIDEODEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglBindVideoDeviceNV")) == nullptr) || r;
  r = ((wglEnumerateVideoDevicesNV = (PFNWGLENUMERATEVIDEODEVICESNVPROC)glewGetProcAddress((const GLubyte*)"wglEnumerateVideoDevicesNV")) == nullptr) || r;
  r = ((wglQueryCurrentContextNV = (PFNWGLQUERYCURRENTCONTEXTNVPROC)glewGetProcAddress((const GLubyte*)"wglQueryCurrentContextNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_present_video */

#ifdef WGL_NV_render_depth_texture

#endif /* WGL_NV_render_depth_texture */

#ifdef WGL_NV_render_texture_rectangle

#endif /* WGL_NV_render_texture_rectangle */

#ifdef WGL_NV_swap_group

static GLboolean _glewInit_WGL_NV_swap_group (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindSwapBarrierNV = (PFNWGLBINDSWAPBARRIERNVPROC)glewGetProcAddress((const GLubyte*)"wglBindSwapBarrierNV")) == nullptr) || r;
  r = ((wglJoinSwapGroupNV = (PFNWGLJOINSWAPGROUPNVPROC)glewGetProcAddress((const GLubyte*)"wglJoinSwapGroupNV")) == nullptr) || r;
  r = ((wglQueryFrameCountNV = (PFNWGLQUERYFRAMECOUNTNVPROC)glewGetProcAddress((const GLubyte*)"wglQueryFrameCountNV")) == nullptr) || r;
  r = ((wglQueryMaxSwapGroupsNV = (PFNWGLQUERYMAXSWAPGROUPSNVPROC)glewGetProcAddress((const GLubyte*)"wglQueryMaxSwapGroupsNV")) == nullptr) || r;
  r = ((wglQuerySwapGroupNV = (PFNWGLQUERYSWAPGROUPNVPROC)glewGetProcAddress((const GLubyte*)"wglQuerySwapGroupNV")) == nullptr) || r;
  r = ((wglResetFrameCountNV = (PFNWGLRESETFRAMECOUNTNVPROC)glewGetProcAddress((const GLubyte*)"wglResetFrameCountNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_swap_group */

#ifdef WGL_NV_vertex_array_range

static GLboolean _glewInit_WGL_NV_vertex_array_range (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC)glewGetProcAddress((const GLubyte*)"wglAllocateMemoryNV")) == nullptr) || r;
  r = ((wglFreeMemoryNV = (PFNWGLFREEMEMORYNVPROC)glewGetProcAddress((const GLubyte*)"wglFreeMemoryNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_vertex_array_range */

#ifdef WGL_NV_video_capture

static GLboolean _glewInit_WGL_NV_video_capture (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindVideoCaptureDeviceNV = (PFNWGLBINDVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglBindVideoCaptureDeviceNV")) == nullptr) || r;
  r = ((wglEnumerateVideoCaptureDevicesNV = (PFNWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC)glewGetProcAddress((const GLubyte*)"wglEnumerateVideoCaptureDevicesNV")) == nullptr) || r;
  r = ((wglLockVideoCaptureDeviceNV = (PFNWGLLOCKVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglLockVideoCaptureDeviceNV")) == nullptr) || r;
  r = ((wglQueryVideoCaptureDeviceNV = (PFNWGLQUERYVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglQueryVideoCaptureDeviceNV")) == nullptr) || r;
  r = ((wglReleaseVideoCaptureDeviceNV = (PFNWGLRELEASEVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglReleaseVideoCaptureDeviceNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_video_capture */

#ifdef WGL_NV_video_output

static GLboolean _glewInit_WGL_NV_video_output (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglBindVideoImageNV = (PFNWGLBINDVIDEOIMAGENVPROC)glewGetProcAddress((const GLubyte*)"wglBindVideoImageNV")) == nullptr) || r;
  r = ((wglGetVideoDeviceNV = (PFNWGLGETVIDEODEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglGetVideoDeviceNV")) == nullptr) || r;
  r = ((wglGetVideoInfoNV = (PFNWGLGETVIDEOINFONVPROC)glewGetProcAddress((const GLubyte*)"wglGetVideoInfoNV")) == nullptr) || r;
  r = ((wglReleaseVideoDeviceNV = (PFNWGLRELEASEVIDEODEVICENVPROC)glewGetProcAddress((const GLubyte*)"wglReleaseVideoDeviceNV")) == nullptr) || r;
  r = ((wglReleaseVideoImageNV = (PFNWGLRELEASEVIDEOIMAGENVPROC)glewGetProcAddress((const GLubyte*)"wglReleaseVideoImageNV")) == nullptr) || r;
  r = ((wglSendPbufferToVideoNV = (PFNWGLSENDPBUFFERTOVIDEONVPROC)glewGetProcAddress((const GLubyte*)"wglSendPbufferToVideoNV")) == nullptr) || r;

  return r;
}

#endif /* WGL_NV_video_output */

#ifdef WGL_OML_sync_control

static GLboolean _glewInit_WGL_OML_sync_control (WGLEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetMscRateOML = (PFNWGLGETMSCRATEOMLPROC)glewGetProcAddress((const GLubyte*)"wglGetMscRateOML")) == nullptr) || r;
  r = ((wglGetSyncValuesOML = (PFNWGLGETSYNCVALUESOMLPROC)glewGetProcAddress((const GLubyte*)"wglGetSyncValuesOML")) == nullptr) || r;
  r = ((wglSwapBuffersMscOML = (PFNWGLSWAPBUFFERSMSCOMLPROC)glewGetProcAddress((const GLubyte*)"wglSwapBuffersMscOML")) == nullptr) || r;
  r = ((wglSwapLayerBuffersMscOML = (PFNWGLSWAPLAYERBUFFERSMSCOMLPROC)glewGetProcAddress((const GLubyte*)"wglSwapLayerBuffersMscOML")) == nullptr) || r;
  r = ((wglWaitForMscOML = (PFNWGLWAITFORMSCOMLPROC)glewGetProcAddress((const GLubyte*)"wglWaitForMscOML")) == nullptr) || r;
  r = ((wglWaitForSbcOML = (PFNWGLWAITFORSBCOMLPROC)glewGetProcAddress((const GLubyte*)"wglWaitForSbcOML")) == nullptr) || r;

  return r;
}

#endif /* WGL_OML_sync_control */

/* ------------------------------------------------------------------------- */

static PFNWGLGETEXTENSIONSSTRINGARBPROC _wglewGetExtensionsStringARB = nullptr;
static PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglewGetExtensionsStringEXT = nullptr;

GLboolean wglewGetExtension (const char* name)
{    
  const GLubyte* start;
  const GLubyte* end;
  if (_wglewGetExtensionsStringARB == nullptr)
    if (_wglewGetExtensionsStringEXT == nullptr)
      return GL_FALSE;
    else
      start = (const GLubyte*)_wglewGetExtensionsStringEXT();
  else
    start = (const GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
  if (start == 0)
    return GL_FALSE;
  end = start + _glewStrLen(start);
  return _glewSearchExtension(name, start, end);
}

GLenum wglewContextInit (WGLEW_CONTEXT_ARG_DEF_LIST)
{
  GLboolean crippled;
  const GLubyte* extStart;
  const GLubyte* extEnd;
  /* find wgl extension string query functions */
  _wglewGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB");
  _wglewGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT");
  /* query wgl extension string */
  if (_wglewGetExtensionsStringARB == nullptr)
    if (_wglewGetExtensionsStringEXT == nullptr)
      extStart = (const GLubyte*)"";
    else
      extStart = (const GLubyte*)_wglewGetExtensionsStringEXT();
  else
    extStart = (const GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
  extEnd = extStart + _glewStrLen(extStart);
  /* initialize extensions */
  crippled = _wglewGetExtensionsStringARB == nullptr && _wglewGetExtensionsStringEXT == nullptr;
#ifdef WGL_3DFX_multisample
  CONST_CAST(WGLEW_3DFX_multisample) = _glewSearchExtension("WGL_3DFX_multisample", extStart, extEnd);
#endif /* WGL_3DFX_multisample */
#ifdef WGL_3DL_stereo_control
  CONST_CAST(WGLEW_3DL_stereo_control) = _glewSearchExtension("WGL_3DL_stereo_control", extStart, extEnd);
  if (glewExperimental || WGLEW_3DL_stereo_control|| crippled) CONST_CAST(WGLEW_3DL_stereo_control)= !_glewInit_WGL_3DL_stereo_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_3DL_stereo_control */
#ifdef WGL_AMD_gpu_association
  CONST_CAST(WGLEW_AMD_gpu_association) = _glewSearchExtension("WGL_AMD_gpu_association", extStart, extEnd);
  if (glewExperimental || WGLEW_AMD_gpu_association|| crippled) CONST_CAST(WGLEW_AMD_gpu_association)= !_glewInit_WGL_AMD_gpu_association(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_AMD_gpu_association */
#ifdef WGL_ARB_buffer_region
  CONST_CAST(WGLEW_ARB_buffer_region) = _glewSearchExtension("WGL_ARB_buffer_region", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_buffer_region|| crippled) CONST_CAST(WGLEW_ARB_buffer_region)= !_glewInit_WGL_ARB_buffer_region(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_buffer_region */
#ifdef WGL_ARB_create_context
  CONST_CAST(WGLEW_ARB_create_context) = _glewSearchExtension("WGL_ARB_create_context", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_create_context|| crippled) CONST_CAST(WGLEW_ARB_create_context)= !_glewInit_WGL_ARB_create_context(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_create_context */
#ifdef WGL_ARB_create_context_profile
  CONST_CAST(WGLEW_ARB_create_context_profile) = _glewSearchExtension("WGL_ARB_create_context_profile", extStart, extEnd);
#endif /* WGL_ARB_create_context_profile */
#ifdef WGL_ARB_create_context_robustness
  CONST_CAST(WGLEW_ARB_create_context_robustness) = _glewSearchExtension("WGL_ARB_create_context_robustness", extStart, extEnd);
#endif /* WGL_ARB_create_context_robustness */
#ifdef WGL_ARB_extensions_string
  CONST_CAST(WGLEW_ARB_extensions_string) = _glewSearchExtension("WGL_ARB_extensions_string", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_extensions_string|| crippled) CONST_CAST(WGLEW_ARB_extensions_string)= !_glewInit_WGL_ARB_extensions_string(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_extensions_string */
#ifdef WGL_ARB_framebuffer_sRGB
  CONST_CAST(WGLEW_ARB_framebuffer_sRGB) = _glewSearchExtension("WGL_ARB_framebuffer_sRGB", extStart, extEnd);
#endif /* WGL_ARB_framebuffer_sRGB */
#ifdef WGL_ARB_make_current_read
  CONST_CAST(WGLEW_ARB_make_current_read) = _glewSearchExtension("WGL_ARB_make_current_read", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_make_current_read|| crippled) CONST_CAST(WGLEW_ARB_make_current_read)= !_glewInit_WGL_ARB_make_current_read(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_make_current_read */
#ifdef WGL_ARB_multisample
  CONST_CAST(WGLEW_ARB_multisample) = _glewSearchExtension("WGL_ARB_multisample", extStart, extEnd);
#endif /* WGL_ARB_multisample */
#ifdef WGL_ARB_pbuffer
  CONST_CAST(WGLEW_ARB_pbuffer) = _glewSearchExtension("WGL_ARB_pbuffer", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_pbuffer|| crippled) CONST_CAST(WGLEW_ARB_pbuffer)= !_glewInit_WGL_ARB_pbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_pbuffer */
#ifdef WGL_ARB_pixel_format
  CONST_CAST(WGLEW_ARB_pixel_format) = _glewSearchExtension("WGL_ARB_pixel_format", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_pixel_format|| crippled) CONST_CAST(WGLEW_ARB_pixel_format)= !_glewInit_WGL_ARB_pixel_format(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_pixel_format */
#ifdef WGL_ARB_pixel_format_float
  CONST_CAST(WGLEW_ARB_pixel_format_float) = _glewSearchExtension("WGL_ARB_pixel_format_float", extStart, extEnd);
#endif /* WGL_ARB_pixel_format_float */
#ifdef WGL_ARB_render_texture
  CONST_CAST(WGLEW_ARB_render_texture) = _glewSearchExtension("WGL_ARB_render_texture", extStart, extEnd);
  if (glewExperimental || WGLEW_ARB_render_texture|| crippled) CONST_CAST(WGLEW_ARB_render_texture)= !_glewInit_WGL_ARB_render_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_ARB_render_texture */
#ifdef WGL_ATI_pixel_format_float
  CONST_CAST(WGLEW_ATI_pixel_format_float) = _glewSearchExtension("WGL_ATI_pixel_format_float", extStart, extEnd);
#endif /* WGL_ATI_pixel_format_float */
#ifdef WGL_ATI_render_texture_rectangle
  CONST_CAST(WGLEW_ATI_render_texture_rectangle) = _glewSearchExtension("WGL_ATI_render_texture_rectangle", extStart, extEnd);
#endif /* WGL_ATI_render_texture_rectangle */
#ifdef WGL_EXT_create_context_es2_profile
  CONST_CAST(WGLEW_EXT_create_context_es2_profile) = _glewSearchExtension("WGL_EXT_create_context_es2_profile", extStart, extEnd);
#endif /* WGL_EXT_create_context_es2_profile */
#ifdef WGL_EXT_depth_float
  CONST_CAST(WGLEW_EXT_depth_float) = _glewSearchExtension("WGL_EXT_depth_float", extStart, extEnd);
#endif /* WGL_EXT_depth_float */
#ifdef WGL_EXT_display_color_table
  CONST_CAST(WGLEW_EXT_display_color_table) = _glewSearchExtension("WGL_EXT_display_color_table", extStart, extEnd);
  if (glewExperimental || WGLEW_EXT_display_color_table|| crippled) CONST_CAST(WGLEW_EXT_display_color_table)= !_glewInit_WGL_EXT_display_color_table(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_display_color_table */
#ifdef WGL_EXT_extensions_string
  CONST_CAST(WGLEW_EXT_extensions_string) = _glewSearchExtension("WGL_EXT_extensions_string", extStart, extEnd);
  if (glewExperimental || WGLEW_EXT_extensions_string|| crippled) CONST_CAST(WGLEW_EXT_extensions_string)= !_glewInit_WGL_EXT_extensions_string(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_extensions_string */
#ifdef WGL_EXT_framebuffer_sRGB
  CONST_CAST(WGLEW_EXT_framebuffer_sRGB) = _glewSearchExtension("WGL_EXT_framebuffer_sRGB", extStart, extEnd);
#endif /* WGL_EXT_framebuffer_sRGB */
#ifdef WGL_EXT_make_current_read
  CONST_CAST(WGLEW_EXT_make_current_read) = _glewSearchExtension("WGL_EXT_make_current_read", extStart, extEnd);
  if (glewExperimental || WGLEW_EXT_make_current_read|| crippled) CONST_CAST(WGLEW_EXT_make_current_read)= !_glewInit_WGL_EXT_make_current_read(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_make_current_read */
#ifdef WGL_EXT_multisample
  CONST_CAST(WGLEW_EXT_multisample) = _glewSearchExtension("WGL_EXT_multisample", extStart, extEnd);
#endif /* WGL_EXT_multisample */
#ifdef WGL_EXT_pbuffer
  CONST_CAST(WGLEW_EXT_pbuffer) = _glewSearchExtension("WGL_EXT_pbuffer", extStart, extEnd);
  if (glewExperimental || WGLEW_EXT_pbuffer|| crippled) CONST_CAST(WGLEW_EXT_pbuffer)= !_glewInit_WGL_EXT_pbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_pbuffer */
#ifdef WGL_EXT_pixel_format
  CONST_CAST(WGLEW_EXT_pixel_format) = _glewSearchExtension("WGL_EXT_pixel_format", extStart, extEnd);
  if (glewExperimental || WGLEW_EXT_pixel_format|| crippled) CONST_CAST(WGLEW_EXT_pixel_format)= !_glewInit_WGL_EXT_pixel_format(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_pixel_format */
#ifdef WGL_EXT_pixel_format_packed_float
  CONST_CAST(WGLEW_EXT_pixel_format_packed_float) = _glewSearchExtension("WGL_EXT_pixel_format_packed_float", extStart, extEnd);
#endif /* WGL_EXT_pixel_format_packed_float */
#ifdef WGL_EXT_swap_control
  CONST_CAST(WGLEW_EXT_swap_control) = _glewSearchExtension("WGL_EXT_swap_control", extStart, extEnd);
  if (glewExperimental || WGLEW_EXT_swap_control|| crippled) CONST_CAST(WGLEW_EXT_swap_control)= !_glewInit_WGL_EXT_swap_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_EXT_swap_control */
#ifdef WGL_I3D_digital_video_control
  CONST_CAST(WGLEW_I3D_digital_video_control) = _glewSearchExtension("WGL_I3D_digital_video_control", extStart, extEnd);
  if (glewExperimental || WGLEW_I3D_digital_video_control|| crippled) CONST_CAST(WGLEW_I3D_digital_video_control)= !_glewInit_WGL_I3D_digital_video_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_digital_video_control */
#ifdef WGL_I3D_gamma
  CONST_CAST(WGLEW_I3D_gamma) = _glewSearchExtension("WGL_I3D_gamma", extStart, extEnd);
  if (glewExperimental || WGLEW_I3D_gamma|| crippled) CONST_CAST(WGLEW_I3D_gamma)= !_glewInit_WGL_I3D_gamma(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_gamma */
#ifdef WGL_I3D_genlock
  CONST_CAST(WGLEW_I3D_genlock) = _glewSearchExtension("WGL_I3D_genlock", extStart, extEnd);
  if (glewExperimental || WGLEW_I3D_genlock|| crippled) CONST_CAST(WGLEW_I3D_genlock)= !_glewInit_WGL_I3D_genlock(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_genlock */
#ifdef WGL_I3D_image_buffer
  CONST_CAST(WGLEW_I3D_image_buffer) = _glewSearchExtension("WGL_I3D_image_buffer", extStart, extEnd);
  if (glewExperimental || WGLEW_I3D_image_buffer|| crippled) CONST_CAST(WGLEW_I3D_image_buffer)= !_glewInit_WGL_I3D_image_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_image_buffer */
#ifdef WGL_I3D_swap_frame_lock
  CONST_CAST(WGLEW_I3D_swap_frame_lock) = _glewSearchExtension("WGL_I3D_swap_frame_lock", extStart, extEnd);
  if (glewExperimental || WGLEW_I3D_swap_frame_lock|| crippled) CONST_CAST(WGLEW_I3D_swap_frame_lock)= !_glewInit_WGL_I3D_swap_frame_lock(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_swap_frame_lock */
#ifdef WGL_I3D_swap_frame_usage
  CONST_CAST(WGLEW_I3D_swap_frame_usage) = _glewSearchExtension("WGL_I3D_swap_frame_usage", extStart, extEnd);
  if (glewExperimental || WGLEW_I3D_swap_frame_usage|| crippled) CONST_CAST(WGLEW_I3D_swap_frame_usage)= !_glewInit_WGL_I3D_swap_frame_usage(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_I3D_swap_frame_usage */
#ifdef WGL_NV_DX_interop
  CONST_CAST(WGLEW_NV_DX_interop) = _glewSearchExtension("WGL_NV_DX_interop", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_DX_interop|| crippled) CONST_CAST(WGLEW_NV_DX_interop)= !_glewInit_WGL_NV_DX_interop(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_DX_interop */
#ifdef WGL_NV_copy_image
  CONST_CAST(WGLEW_NV_copy_image) = _glewSearchExtension("WGL_NV_copy_image", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_copy_image|| crippled) CONST_CAST(WGLEW_NV_copy_image)= !_glewInit_WGL_NV_copy_image(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_copy_image */
#ifdef WGL_NV_float_buffer
  CONST_CAST(WGLEW_NV_float_buffer) = _glewSearchExtension("WGL_NV_float_buffer", extStart, extEnd);
#endif /* WGL_NV_float_buffer */
#ifdef WGL_NV_gpu_affinity
  CONST_CAST(WGLEW_NV_gpu_affinity) = _glewSearchExtension("WGL_NV_gpu_affinity", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_gpu_affinity|| crippled) CONST_CAST(WGLEW_NV_gpu_affinity)= !_glewInit_WGL_NV_gpu_affinity(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_gpu_affinity */
#ifdef WGL_NV_multisample_coverage
  CONST_CAST(WGLEW_NV_multisample_coverage) = _glewSearchExtension("WGL_NV_multisample_coverage", extStart, extEnd);
#endif /* WGL_NV_multisample_coverage */
#ifdef WGL_NV_present_video
  CONST_CAST(WGLEW_NV_present_video) = _glewSearchExtension("WGL_NV_present_video", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_present_video|| crippled) CONST_CAST(WGLEW_NV_present_video)= !_glewInit_WGL_NV_present_video(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_present_video */
#ifdef WGL_NV_render_depth_texture
  CONST_CAST(WGLEW_NV_render_depth_texture) = _glewSearchExtension("WGL_NV_render_depth_texture", extStart, extEnd);
#endif /* WGL_NV_render_depth_texture */
#ifdef WGL_NV_render_texture_rectangle
  CONST_CAST(WGLEW_NV_render_texture_rectangle) = _glewSearchExtension("WGL_NV_render_texture_rectangle", extStart, extEnd);
#endif /* WGL_NV_render_texture_rectangle */
#ifdef WGL_NV_swap_group
  CONST_CAST(WGLEW_NV_swap_group) = _glewSearchExtension("WGL_NV_swap_group", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_swap_group|| crippled) CONST_CAST(WGLEW_NV_swap_group)= !_glewInit_WGL_NV_swap_group(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_swap_group */
#ifdef WGL_NV_vertex_array_range
  CONST_CAST(WGLEW_NV_vertex_array_range) = _glewSearchExtension("WGL_NV_vertex_array_range", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_vertex_array_range|| crippled) CONST_CAST(WGLEW_NV_vertex_array_range)= !_glewInit_WGL_NV_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_vertex_array_range */
#ifdef WGL_NV_video_capture
  CONST_CAST(WGLEW_NV_video_capture) = _glewSearchExtension("WGL_NV_video_capture", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_video_capture|| crippled) CONST_CAST(WGLEW_NV_video_capture)= !_glewInit_WGL_NV_video_capture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_video_capture */
#ifdef WGL_NV_video_output
  CONST_CAST(WGLEW_NV_video_output) = _glewSearchExtension("WGL_NV_video_output", extStart, extEnd);
  if (glewExperimental || WGLEW_NV_video_output|| crippled) CONST_CAST(WGLEW_NV_video_output)= !_glewInit_WGL_NV_video_output(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_NV_video_output */
#ifdef WGL_OML_sync_control
  CONST_CAST(WGLEW_OML_sync_control) = _glewSearchExtension("WGL_OML_sync_control", extStart, extEnd);
  if (glewExperimental || WGLEW_OML_sync_control|| crippled) CONST_CAST(WGLEW_OML_sync_control)= !_glewInit_WGL_OML_sync_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* WGL_OML_sync_control */

  return GLEW_OK;
}

#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)

PFNGLXGETCURRENTDISPLAYPROC __glewXGetCurrentDisplay = nullptr;

PFNGLXCHOOSEFBCONFIGPROC __glewXChooseFBConfig = nullptr;
PFNGLXCREATENEWCONTEXTPROC __glewXCreateNewContext = nullptr;
PFNGLXCREATEPBUFFERPROC __glewXCreatePbuffer = nullptr;
PFNGLXCREATEPIXMAPPROC __glewXCreatePixmap = nullptr;
PFNGLXCREATEWINDOWPROC __glewXCreateWindow = nullptr;
PFNGLXDESTROYPBUFFERPROC __glewXDestroyPbuffer = nullptr;
PFNGLXDESTROYPIXMAPPROC __glewXDestroyPixmap = nullptr;
PFNGLXDESTROYWINDOWPROC __glewXDestroyWindow = nullptr;
PFNGLXGETCURRENTREADDRAWABLEPROC __glewXGetCurrentReadDrawable = nullptr;
PFNGLXGETFBCONFIGATTRIBPROC __glewXGetFBConfigAttrib = nullptr;
PFNGLXGETFBCONFIGSPROC __glewXGetFBConfigs = nullptr;
PFNGLXGETSELECTEDEVENTPROC __glewXGetSelectedEvent = nullptr;
PFNGLXGETVISUALFROMFBCONFIGPROC __glewXGetVisualFromFBConfig = nullptr;
PFNGLXMAKECONTEXTCURRENTPROC __glewXMakeContextCurrent = nullptr;
PFNGLXQUERYCONTEXTPROC __glewXQueryContext = nullptr;
PFNGLXQUERYDRAWABLEPROC __glewXQueryDrawable = nullptr;
PFNGLXSELECTEVENTPROC __glewXSelectEvent = nullptr;

PFNGLXCREATECONTEXTATTRIBSARBPROC __glewXCreateContextAttribsARB = nullptr;

PFNGLXBINDTEXIMAGEATIPROC __glewXBindTexImageATI = nullptr;
PFNGLXDRAWABLEATTRIBATIPROC __glewXDrawableAttribATI = nullptr;
PFNGLXRELEASETEXIMAGEATIPROC __glewXReleaseTexImageATI = nullptr;

PFNGLXFREECONTEXTEXTPROC __glewXFreeContextEXT = nullptr;
PFNGLXGETCONTEXTIDEXTPROC __glewXGetContextIDEXT = nullptr;
PFNGLXIMPORTCONTEXTEXTPROC __glewXImportContextEXT = nullptr;
PFNGLXQUERYCONTEXTINFOEXTPROC __glewXQueryContextInfoEXT = nullptr;

PFNGLXSWAPINTERVALEXTPROC __glewXSwapIntervalEXT = nullptr;

PFNGLXBINDTEXIMAGEEXTPROC __glewXBindTexImageEXT = nullptr;
PFNGLXRELEASETEXIMAGEEXTPROC __glewXReleaseTexImageEXT = nullptr;

PFNGLXGETAGPOFFSETMESAPROC __glewXGetAGPOffsetMESA = nullptr;

PFNGLXCOPYSUBBUFFERMESAPROC __glewXCopySubBufferMESA = nullptr;

PFNGLXCREATEGLXPIXMAPMESAPROC __glewXCreateGLXPixmapMESA = nullptr;

PFNGLXRELEASEBUFFERSMESAPROC __glewXReleaseBuffersMESA = nullptr;

PFNGLXSET3DFXMODEMESAPROC __glewXSet3DfxModeMESA = nullptr;

PFNGLXGETSWAPINTERVALMESAPROC __glewXGetSwapIntervalMESA = nullptr;
PFNGLXSWAPINTERVALMESAPROC __glewXSwapIntervalMESA = nullptr;

PFNGLXCOPYIMAGESUBDATANVPROC __glewXCopyImageSubDataNV = nullptr;

PFNGLXBINDVIDEODEVICENVPROC __glewXBindVideoDeviceNV = nullptr;
PFNGLXENUMERATEVIDEODEVICESNVPROC __glewXEnumerateVideoDevicesNV = nullptr;

PFNGLXBINDSWAPBARRIERNVPROC __glewXBindSwapBarrierNV = nullptr;
PFNGLXJOINSWAPGROUPNVPROC __glewXJoinSwapGroupNV = nullptr;
PFNGLXQUERYFRAMECOUNTNVPROC __glewXQueryFrameCountNV = nullptr;
PFNGLXQUERYMAXSWAPGROUPSNVPROC __glewXQueryMaxSwapGroupsNV = nullptr;
PFNGLXQUERYSWAPGROUPNVPROC __glewXQuerySwapGroupNV = nullptr;
PFNGLXRESETFRAMECOUNTNVPROC __glewXResetFrameCountNV = nullptr;

PFNGLXALLOCATEMEMORYNVPROC __glewXAllocateMemoryNV = nullptr;
PFNGLXFREEMEMORYNVPROC __glewXFreeMemoryNV = nullptr;

PFNGLXBINDVIDEOCAPTUREDEVICENVPROC __glewXBindVideoCaptureDeviceNV = nullptr;
PFNGLXENUMERATEVIDEOCAPTUREDEVICESNVPROC __glewXEnumerateVideoCaptureDevicesNV = nullptr;
PFNGLXLOCKVIDEOCAPTUREDEVICENVPROC __glewXLockVideoCaptureDeviceNV = nullptr;
PFNGLXQUERYVIDEOCAPTUREDEVICENVPROC __glewXQueryVideoCaptureDeviceNV = nullptr;
PFNGLXRELEASEVIDEOCAPTUREDEVICENVPROC __glewXReleaseVideoCaptureDeviceNV = nullptr;

PFNGLXBINDVIDEOIMAGENVPROC __glewXBindVideoImageNV = nullptr;
PFNGLXGETVIDEODEVICENVPROC __glewXGetVideoDeviceNV = nullptr;
PFNGLXGETVIDEOINFONVPROC __glewXGetVideoInfoNV = nullptr;
PFNGLXRELEASEVIDEODEVICENVPROC __glewXReleaseVideoDeviceNV = nullptr;
PFNGLXRELEASEVIDEOIMAGENVPROC __glewXReleaseVideoImageNV = nullptr;
PFNGLXSENDPBUFFERTOVIDEONVPROC __glewXSendPbufferToVideoNV = nullptr;

PFNGLXGETMSCRATEOMLPROC __glewXGetMscRateOML = nullptr;
PFNGLXGETSYNCVALUESOMLPROC __glewXGetSyncValuesOML = nullptr;
PFNGLXSWAPBUFFERSMSCOMLPROC __glewXSwapBuffersMscOML = nullptr;
PFNGLXWAITFORMSCOMLPROC __glewXWaitForMscOML = nullptr;
PFNGLXWAITFORSBCOMLPROC __glewXWaitForSbcOML = nullptr;

PFNGLXCHOOSEFBCONFIGSGIXPROC __glewXChooseFBConfigSGIX = nullptr;
PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC __glewXCreateContextWithConfigSGIX = nullptr;
PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC __glewXCreateGLXPixmapWithConfigSGIX = nullptr;
PFNGLXGETFBCONFIGATTRIBSGIXPROC __glewXGetFBConfigAttribSGIX = nullptr;
PFNGLXGETFBCONFIGFROMVISUALSGIXPROC __glewXGetFBConfigFromVisualSGIX = nullptr;
PFNGLXGETVISUALFROMFBCONFIGSGIXPROC __glewXGetVisualFromFBConfigSGIX = nullptr;

PFNGLXBINDHYPERPIPESGIXPROC __glewXBindHyperpipeSGIX = nullptr;
PFNGLXDESTROYHYPERPIPECONFIGSGIXPROC __glewXDestroyHyperpipeConfigSGIX = nullptr;
PFNGLXHYPERPIPEATTRIBSGIXPROC __glewXHyperpipeAttribSGIX = nullptr;
PFNGLXHYPERPIPECONFIGSGIXPROC __glewXHyperpipeConfigSGIX = nullptr;
PFNGLXQUERYHYPERPIPEATTRIBSGIXPROC __glewXQueryHyperpipeAttribSGIX = nullptr;
PFNGLXQUERYHYPERPIPEBESTATTRIBSGIXPROC __glewXQueryHyperpipeBestAttribSGIX = nullptr;
PFNGLXQUERYHYPERPIPECONFIGSGIXPROC __glewXQueryHyperpipeConfigSGIX = nullptr;
PFNGLXQUERYHYPERPIPENETWORKSGIXPROC __glewXQueryHyperpipeNetworkSGIX = nullptr;

PFNGLXCREATEGLXPBUFFERSGIXPROC __glewXCreateGLXPbufferSGIX = nullptr;
PFNGLXDESTROYGLXPBUFFERSGIXPROC __glewXDestroyGLXPbufferSGIX = nullptr;
PFNGLXGETSELECTEDEVENTSGIXPROC __glewXGetSelectedEventSGIX = nullptr;
PFNGLXQUERYGLXPBUFFERSGIXPROC __glewXQueryGLXPbufferSGIX = nullptr;
PFNGLXSELECTEVENTSGIXPROC __glewXSelectEventSGIX = nullptr;

PFNGLXBINDSWAPBARRIERSGIXPROC __glewXBindSwapBarrierSGIX = nullptr;
PFNGLXQUERYMAXSWAPBARRIERSSGIXPROC __glewXQueryMaxSwapBarriersSGIX = nullptr;

PFNGLXJOINSWAPGROUPSGIXPROC __glewXJoinSwapGroupSGIX = nullptr;

PFNGLXBINDCHANNELTOWINDOWSGIXPROC __glewXBindChannelToWindowSGIX = nullptr;
PFNGLXCHANNELRECTSGIXPROC __glewXChannelRectSGIX = nullptr;
PFNGLXCHANNELRECTSYNCSGIXPROC __glewXChannelRectSyncSGIX = nullptr;
PFNGLXQUERYCHANNELDELTASSGIXPROC __glewXQueryChannelDeltasSGIX = nullptr;
PFNGLXQUERYCHANNELRECTSGIXPROC __glewXQueryChannelRectSGIX = nullptr;

PFNGLXCUSHIONSGIPROC __glewXCushionSGI = nullptr;

PFNGLXGETCURRENTREADDRAWABLESGIPROC __glewXGetCurrentReadDrawableSGI = nullptr;
PFNGLXMAKECURRENTREADSGIPROC __glewXMakeCurrentReadSGI = nullptr;

PFNGLXSWAPINTERVALSGIPROC __glewXSwapIntervalSGI = nullptr;

PFNGLXGETVIDEOSYNCSGIPROC __glewXGetVideoSyncSGI = nullptr;
PFNGLXWAITVIDEOSYNCSGIPROC __glewXWaitVideoSyncSGI = nullptr;

PFNGLXGETTRANSPARENTINDEXSUNPROC __glewXGetTransparentIndexSUN = nullptr;

PFNGLXGETVIDEORESIZESUNPROC __glewXGetVideoResizeSUN = nullptr;
PFNGLXVIDEORESIZESUNPROC __glewXVideoResizeSUN = nullptr;

#if !defined(GLEW_MX)

GLboolean __GLXEW_VERSION_1_0 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_1 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_2 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_3 = GL_FALSE;
GLboolean __GLXEW_VERSION_1_4 = GL_FALSE;
GLboolean __GLXEW_3DFX_multisample = GL_FALSE;
GLboolean __GLXEW_AMD_gpu_association = GL_FALSE;
GLboolean __GLXEW_ARB_create_context = GL_FALSE;
GLboolean __GLXEW_ARB_create_context_profile = GL_FALSE;
GLboolean __GLXEW_ARB_create_context_robustness = GL_FALSE;
GLboolean __GLXEW_ARB_fbconfig_float = GL_FALSE;
GLboolean __GLXEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __GLXEW_ARB_get_proc_address = GL_FALSE;
GLboolean __GLXEW_ARB_multisample = GL_FALSE;
GLboolean __GLXEW_ARB_vertex_buffer_object = GL_FALSE;
GLboolean __GLXEW_ATI_pixel_format_float = GL_FALSE;
GLboolean __GLXEW_ATI_render_texture = GL_FALSE;
GLboolean __GLXEW_EXT_create_context_es2_profile = GL_FALSE;
GLboolean __GLXEW_EXT_fbconfig_packed_float = GL_FALSE;
GLboolean __GLXEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __GLXEW_EXT_import_context = GL_FALSE;
GLboolean __GLXEW_EXT_scene_marker = GL_FALSE;
GLboolean __GLXEW_EXT_swap_control = GL_FALSE;
GLboolean __GLXEW_EXT_texture_from_pixmap = GL_FALSE;
GLboolean __GLXEW_EXT_visual_info = GL_FALSE;
GLboolean __GLXEW_EXT_visual_rating = GL_FALSE;
GLboolean __GLXEW_INTEL_swap_event = GL_FALSE;
GLboolean __GLXEW_MESA_agp_offset = GL_FALSE;
GLboolean __GLXEW_MESA_copy_sub_buffer = GL_FALSE;
GLboolean __GLXEW_MESA_pixmap_colormap = GL_FALSE;
GLboolean __GLXEW_MESA_release_buffers = GL_FALSE;
GLboolean __GLXEW_MESA_set_3dfx_mode = GL_FALSE;
GLboolean __GLXEW_MESA_swap_control = GL_FALSE;
GLboolean __GLXEW_NV_copy_image = GL_FALSE;
GLboolean __GLXEW_NV_float_buffer = GL_FALSE;
GLboolean __GLXEW_NV_multisample_coverage = GL_FALSE;
GLboolean __GLXEW_NV_present_video = GL_FALSE;
GLboolean __GLXEW_NV_swap_group = GL_FALSE;
GLboolean __GLXEW_NV_vertex_array_range = GL_FALSE;
GLboolean __GLXEW_NV_video_capture = GL_FALSE;
GLboolean __GLXEW_NV_video_output = GL_FALSE;
GLboolean __GLXEW_OML_swap_method = GL_FALSE;
GLboolean __GLXEW_OML_sync_control = GL_FALSE;
GLboolean __GLXEW_SGIS_blended_overlay = GL_FALSE;
GLboolean __GLXEW_SGIS_color_range = GL_FALSE;
GLboolean __GLXEW_SGIS_multisample = GL_FALSE;
GLboolean __GLXEW_SGIS_shared_multisample = GL_FALSE;
GLboolean __GLXEW_SGIX_fbconfig = GL_FALSE;
GLboolean __GLXEW_SGIX_hyperpipe = GL_FALSE;
GLboolean __GLXEW_SGIX_pbuffer = GL_FALSE;
GLboolean __GLXEW_SGIX_swap_barrier = GL_FALSE;
GLboolean __GLXEW_SGIX_swap_group = GL_FALSE;
GLboolean __GLXEW_SGIX_video_resize = GL_FALSE;
GLboolean __GLXEW_SGIX_visual_select_group = GL_FALSE;
GLboolean __GLXEW_SGI_cushion = GL_FALSE;
GLboolean __GLXEW_SGI_make_current_read = GL_FALSE;
GLboolean __GLXEW_SGI_swap_control = GL_FALSE;
GLboolean __GLXEW_SGI_video_sync = GL_FALSE;
GLboolean __GLXEW_SUN_get_transparent_index = GL_FALSE;
GLboolean __GLXEW_SUN_video_resize = GL_FALSE;

#endif /* !GLEW_MX */

#ifdef GLX_VERSION_1_2

static GLboolean _glewInit_GLX_VERSION_1_2 (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)glewGetProcAddress((const GLubyte*)"glXGetCurrentDisplay")) == nullptr) || r;

  return r;
}

#endif /* GLX_VERSION_1_2 */

#ifdef GLX_VERSION_1_3

static GLboolean _glewInit_GLX_VERSION_1_3 (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glewGetProcAddress((const GLubyte*)"glXChooseFBConfig")) == nullptr) || r;
  r = ((glXCreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)glewGetProcAddress((const GLubyte*)"glXCreateNewContext")) == nullptr) || r;
  r = ((glXCreatePbuffer = (PFNGLXCREATEPBUFFERPROC)glewGetProcAddress((const GLubyte*)"glXCreatePbuffer")) == nullptr) || r;
  r = ((glXCreatePixmap = (PFNGLXCREATEPIXMAPPROC)glewGetProcAddress((const GLubyte*)"glXCreatePixmap")) == nullptr) || r;
  r = ((glXCreateWindow = (PFNGLXCREATEWINDOWPROC)glewGetProcAddress((const GLubyte*)"glXCreateWindow")) == nullptr) || r;
  r = ((glXDestroyPbuffer = (PFNGLXDESTROYPBUFFERPROC)glewGetProcAddress((const GLubyte*)"glXDestroyPbuffer")) == nullptr) || r;
  r = ((glXDestroyPixmap = (PFNGLXDESTROYPIXMAPPROC)glewGetProcAddress((const GLubyte*)"glXDestroyPixmap")) == nullptr) || r;
  r = ((glXDestroyWindow = (PFNGLXDESTROYWINDOWPROC)glewGetProcAddress((const GLubyte*)"glXDestroyWindow")) == nullptr) || r;
  r = ((glXGetCurrentReadDrawable = (PFNGLXGETCURRENTREADDRAWABLEPROC)glewGetProcAddress((const GLubyte*)"glXGetCurrentReadDrawable")) == nullptr) || r;
  r = ((glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)glewGetProcAddress((const GLubyte*)"glXGetFBConfigAttrib")) == nullptr) || r;
  r = ((glXGetFBConfigs = (PFNGLXGETFBCONFIGSPROC)glewGetProcAddress((const GLubyte*)"glXGetFBConfigs")) == nullptr) || r;
  r = ((glXGetSelectedEvent = (PFNGLXGETSELECTEDEVENTPROC)glewGetProcAddress((const GLubyte*)"glXGetSelectedEvent")) == nullptr) || r;
  r = ((glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glewGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfig")) == nullptr) || r;
  r = ((glXMakeContextCurrent = (PFNGLXMAKECONTEXTCURRENTPROC)glewGetProcAddress((const GLubyte*)"glXMakeContextCurrent")) == nullptr) || r;
  r = ((glXQueryContext = (PFNGLXQUERYCONTEXTPROC)glewGetProcAddress((const GLubyte*)"glXQueryContext")) == nullptr) || r;
  r = ((glXQueryDrawable = (PFNGLXQUERYDRAWABLEPROC)glewGetProcAddress((const GLubyte*)"glXQueryDrawable")) == nullptr) || r;
  r = ((glXSelectEvent = (PFNGLXSELECTEVENTPROC)glewGetProcAddress((const GLubyte*)"glXSelectEvent")) == nullptr) || r;

  return r;
}

#endif /* GLX_VERSION_1_3 */

#ifdef GLX_VERSION_1_4

#endif /* GLX_VERSION_1_4 */

#ifdef GLX_3DFX_multisample

#endif /* GLX_3DFX_multisample */

#ifdef GLX_AMD_gpu_association

#endif /* GLX_AMD_gpu_association */

#ifdef GLX_ARB_create_context

static GLboolean _glewInit_GLX_ARB_create_context (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glewGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB")) == nullptr) || r;

  return r;
}

#endif /* GLX_ARB_create_context */

#ifdef GLX_ARB_create_context_profile

#endif /* GLX_ARB_create_context_profile */

#ifdef GLX_ARB_create_context_robustness

#endif /* GLX_ARB_create_context_robustness */

#ifdef GLX_ARB_fbconfig_float

#endif /* GLX_ARB_fbconfig_float */

#ifdef GLX_ARB_framebuffer_sRGB

#endif /* GLX_ARB_framebuffer_sRGB */

#ifdef GLX_ARB_get_proc_address

#endif /* GLX_ARB_get_proc_address */

#ifdef GLX_ARB_multisample

#endif /* GLX_ARB_multisample */

#ifdef GLX_ARB_vertex_buffer_object

#endif /* GLX_ARB_vertex_buffer_object */

#ifdef GLX_ATI_pixel_format_float

#endif /* GLX_ATI_pixel_format_float */

#ifdef GLX_ATI_render_texture

static GLboolean _glewInit_GLX_ATI_render_texture (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindTexImageATI = (PFNGLXBINDTEXIMAGEATIPROC)glewGetProcAddress((const GLubyte*)"glXBindTexImageATI")) == nullptr) || r;
  r = ((glXDrawableAttribATI = (PFNGLXDRAWABLEATTRIBATIPROC)glewGetProcAddress((const GLubyte*)"glXDrawableAttribATI")) == nullptr) || r;
  r = ((glXReleaseTexImageATI = (PFNGLXRELEASETEXIMAGEATIPROC)glewGetProcAddress((const GLubyte*)"glXReleaseTexImageATI")) == nullptr) || r;

  return r;
}

#endif /* GLX_ATI_render_texture */

#ifdef GLX_EXT_create_context_es2_profile

#endif /* GLX_EXT_create_context_es2_profile */

#ifdef GLX_EXT_fbconfig_packed_float

#endif /* GLX_EXT_fbconfig_packed_float */

#ifdef GLX_EXT_framebuffer_sRGB

#endif /* GLX_EXT_framebuffer_sRGB */

#ifdef GLX_EXT_import_context

static GLboolean _glewInit_GLX_EXT_import_context (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXFreeContextEXT = (PFNGLXFREECONTEXTEXTPROC)glewGetProcAddress((const GLubyte*)"glXFreeContextEXT")) == nullptr) || r;
  r = ((glXGetContextIDEXT = (PFNGLXGETCONTEXTIDEXTPROC)glewGetProcAddress((const GLubyte*)"glXGetContextIDEXT")) == nullptr) || r;
  r = ((glXImportContextEXT = (PFNGLXIMPORTCONTEXTEXTPROC)glewGetProcAddress((const GLubyte*)"glXImportContextEXT")) == nullptr) || r;
  r = ((glXQueryContextInfoEXT = (PFNGLXQUERYCONTEXTINFOEXTPROC)glewGetProcAddress((const GLubyte*)"glXQueryContextInfoEXT")) == nullptr) || r;

  return r;
}

#endif /* GLX_EXT_import_context */

#ifdef GLX_EXT_scene_marker

#endif /* GLX_EXT_scene_marker */

#ifdef GLX_EXT_swap_control

static GLboolean _glewInit_GLX_EXT_swap_control (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glewGetProcAddress((const GLubyte*)"glXSwapIntervalEXT")) == nullptr) || r;

  return r;
}

#endif /* GLX_EXT_swap_control */

#ifdef GLX_EXT_texture_from_pixmap

static GLboolean _glewInit_GLX_EXT_texture_from_pixmap (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glXBindTexImageEXT")) == nullptr) || r;
  r = ((glXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glXReleaseTexImageEXT")) == nullptr) || r;

  return r;
}

#endif /* GLX_EXT_texture_from_pixmap */

#ifdef GLX_EXT_visual_info

#endif /* GLX_EXT_visual_info */

#ifdef GLX_EXT_visual_rating

#endif /* GLX_EXT_visual_rating */

#ifdef GLX_INTEL_swap_event

#endif /* GLX_INTEL_swap_event */

#ifdef GLX_MESA_agp_offset

static GLboolean _glewInit_GLX_MESA_agp_offset (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetAGPOffsetMESA = (PFNGLXGETAGPOFFSETMESAPROC)glewGetProcAddress((const GLubyte*)"glXGetAGPOffsetMESA")) == nullptr) || r;

  return r;
}

#endif /* GLX_MESA_agp_offset */

#ifdef GLX_MESA_copy_sub_buffer

static GLboolean _glewInit_GLX_MESA_copy_sub_buffer (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCopySubBufferMESA = (PFNGLXCOPYSUBBUFFERMESAPROC)glewGetProcAddress((const GLubyte*)"glXCopySubBufferMESA")) == nullptr) || r;

  return r;
}

#endif /* GLX_MESA_copy_sub_buffer */

#ifdef GLX_MESA_pixmap_colormap

static GLboolean _glewInit_GLX_MESA_pixmap_colormap (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCreateGLXPixmapMESA = (PFNGLXCREATEGLXPIXMAPMESAPROC)glewGetProcAddress((const GLubyte*)"glXCreateGLXPixmapMESA")) == nullptr) || r;

  return r;
}

#endif /* GLX_MESA_pixmap_colormap */

#ifdef GLX_MESA_release_buffers

static GLboolean _glewInit_GLX_MESA_release_buffers (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXReleaseBuffersMESA = (PFNGLXRELEASEBUFFERSMESAPROC)glewGetProcAddress((const GLubyte*)"glXReleaseBuffersMESA")) == nullptr) || r;

  return r;
}

#endif /* GLX_MESA_release_buffers */

#ifdef GLX_MESA_set_3dfx_mode

static GLboolean _glewInit_GLX_MESA_set_3dfx_mode (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXSet3DfxModeMESA = (PFNGLXSET3DFXMODEMESAPROC)glewGetProcAddress((const GLubyte*)"glXSet3DfxModeMESA")) == nullptr) || r;

  return r;
}

#endif /* GLX_MESA_set_3dfx_mode */

#ifdef GLX_MESA_swap_control

static GLboolean _glewInit_GLX_MESA_swap_control (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetSwapIntervalMESA = (PFNGLXGETSWAPINTERVALMESAPROC)glewGetProcAddress((const GLubyte*)"glXGetSwapIntervalMESA")) == nullptr) || r;
  r = ((glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)glewGetProcAddress((const GLubyte*)"glXSwapIntervalMESA")) == nullptr) || r;

  return r;
}

#endif /* GLX_MESA_swap_control */

#ifdef GLX_NV_copy_image

static GLboolean _glewInit_GLX_NV_copy_image (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCopyImageSubDataNV = (PFNGLXCOPYIMAGESUBDATANVPROC)glewGetProcAddress((const GLubyte*)"glXCopyImageSubDataNV")) == nullptr) || r;

  return r;
}

#endif /* GLX_NV_copy_image */

#ifdef GLX_NV_float_buffer

#endif /* GLX_NV_float_buffer */

#ifdef GLX_NV_multisample_coverage

#endif /* GLX_NV_multisample_coverage */

#ifdef GLX_NV_present_video

static GLboolean _glewInit_GLX_NV_present_video (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindVideoDeviceNV = (PFNGLXBINDVIDEODEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXBindVideoDeviceNV")) == nullptr) || r;
  r = ((glXEnumerateVideoDevicesNV = (PFNGLXENUMERATEVIDEODEVICESNVPROC)glewGetProcAddress((const GLubyte*)"glXEnumerateVideoDevicesNV")) == nullptr) || r;

  return r;
}

#endif /* GLX_NV_present_video */

#ifdef GLX_NV_swap_group

static GLboolean _glewInit_GLX_NV_swap_group (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindSwapBarrierNV = (PFNGLXBINDSWAPBARRIERNVPROC)glewGetProcAddress((const GLubyte*)"glXBindSwapBarrierNV")) == nullptr) || r;
  r = ((glXJoinSwapGroupNV = (PFNGLXJOINSWAPGROUPNVPROC)glewGetProcAddress((const GLubyte*)"glXJoinSwapGroupNV")) == nullptr) || r;
  r = ((glXQueryFrameCountNV = (PFNGLXQUERYFRAMECOUNTNVPROC)glewGetProcAddress((const GLubyte*)"glXQueryFrameCountNV")) == nullptr) || r;
  r = ((glXQueryMaxSwapGroupsNV = (PFNGLXQUERYMAXSWAPGROUPSNVPROC)glewGetProcAddress((const GLubyte*)"glXQueryMaxSwapGroupsNV")) == nullptr) || r;
  r = ((glXQuerySwapGroupNV = (PFNGLXQUERYSWAPGROUPNVPROC)glewGetProcAddress((const GLubyte*)"glXQuerySwapGroupNV")) == nullptr) || r;
  r = ((glXResetFrameCountNV = (PFNGLXRESETFRAMECOUNTNVPROC)glewGetProcAddress((const GLubyte*)"glXResetFrameCountNV")) == nullptr) || r;

  return r;
}

#endif /* GLX_NV_swap_group */

#ifdef GLX_NV_vertex_array_range

static GLboolean _glewInit_GLX_NV_vertex_array_range (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXAllocateMemoryNV = (PFNGLXALLOCATEMEMORYNVPROC)glewGetProcAddress((const GLubyte*)"glXAllocateMemoryNV")) == nullptr) || r;
  r = ((glXFreeMemoryNV = (PFNGLXFREEMEMORYNVPROC)glewGetProcAddress((const GLubyte*)"glXFreeMemoryNV")) == nullptr) || r;

  return r;
}

#endif /* GLX_NV_vertex_array_range */

#ifdef GLX_NV_video_capture

static GLboolean _glewInit_GLX_NV_video_capture (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindVideoCaptureDeviceNV = (PFNGLXBINDVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXBindVideoCaptureDeviceNV")) == nullptr) || r;
  r = ((glXEnumerateVideoCaptureDevicesNV = (PFNGLXENUMERATEVIDEOCAPTUREDEVICESNVPROC)glewGetProcAddress((const GLubyte*)"glXEnumerateVideoCaptureDevicesNV")) == nullptr) || r;
  r = ((glXLockVideoCaptureDeviceNV = (PFNGLXLOCKVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXLockVideoCaptureDeviceNV")) == nullptr) || r;
  r = ((glXQueryVideoCaptureDeviceNV = (PFNGLXQUERYVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXQueryVideoCaptureDeviceNV")) == nullptr) || r;
  r = ((glXReleaseVideoCaptureDeviceNV = (PFNGLXRELEASEVIDEOCAPTUREDEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXReleaseVideoCaptureDeviceNV")) == nullptr) || r;

  return r;
}

#endif /* GLX_NV_video_capture */

#ifdef GLX_NV_video_output

static GLboolean _glewInit_GLX_NV_video_output (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindVideoImageNV = (PFNGLXBINDVIDEOIMAGENVPROC)glewGetProcAddress((const GLubyte*)"glXBindVideoImageNV")) == nullptr) || r;
  r = ((glXGetVideoDeviceNV = (PFNGLXGETVIDEODEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXGetVideoDeviceNV")) == nullptr) || r;
  r = ((glXGetVideoInfoNV = (PFNGLXGETVIDEOINFONVPROC)glewGetProcAddress((const GLubyte*)"glXGetVideoInfoNV")) == nullptr) || r;
  r = ((glXReleaseVideoDeviceNV = (PFNGLXRELEASEVIDEODEVICENVPROC)glewGetProcAddress((const GLubyte*)"glXReleaseVideoDeviceNV")) == nullptr) || r;
  r = ((glXReleaseVideoImageNV = (PFNGLXRELEASEVIDEOIMAGENVPROC)glewGetProcAddress((const GLubyte*)"glXReleaseVideoImageNV")) == nullptr) || r;
  r = ((glXSendPbufferToVideoNV = (PFNGLXSENDPBUFFERTOVIDEONVPROC)glewGetProcAddress((const GLubyte*)"glXSendPbufferToVideoNV")) == nullptr) || r;

  return r;
}

#endif /* GLX_NV_video_output */

#ifdef GLX_OML_swap_method

#endif /* GLX_OML_swap_method */

#ifdef GLX_OML_sync_control

static GLboolean _glewInit_GLX_OML_sync_control (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetMscRateOML = (PFNGLXGETMSCRATEOMLPROC)glewGetProcAddress((const GLubyte*)"glXGetMscRateOML")) == nullptr) || r;
  r = ((glXGetSyncValuesOML = (PFNGLXGETSYNCVALUESOMLPROC)glewGetProcAddress((const GLubyte*)"glXGetSyncValuesOML")) == nullptr) || r;
  r = ((glXSwapBuffersMscOML = (PFNGLXSWAPBUFFERSMSCOMLPROC)glewGetProcAddress((const GLubyte*)"glXSwapBuffersMscOML")) == nullptr) || r;
  r = ((glXWaitForMscOML = (PFNGLXWAITFORMSCOMLPROC)glewGetProcAddress((const GLubyte*)"glXWaitForMscOML")) == nullptr) || r;
  r = ((glXWaitForSbcOML = (PFNGLXWAITFORSBCOMLPROC)glewGetProcAddress((const GLubyte*)"glXWaitForSbcOML")) == nullptr) || r;

  return r;
}

#endif /* GLX_OML_sync_control */

#ifdef GLX_SGIS_blended_overlay

#endif /* GLX_SGIS_blended_overlay */

#ifdef GLX_SGIS_color_range

#endif /* GLX_SGIS_color_range */

#ifdef GLX_SGIS_multisample

#endif /* GLX_SGIS_multisample */

#ifdef GLX_SGIS_shared_multisample

#endif /* GLX_SGIS_shared_multisample */

#ifdef GLX_SGIX_fbconfig

static GLboolean _glewInit_GLX_SGIX_fbconfig (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXChooseFBConfigSGIX = (PFNGLXCHOOSEFBCONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXChooseFBConfigSGIX")) == nullptr) || r;
  r = ((glXCreateContextWithConfigSGIX = (PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXCreateContextWithConfigSGIX")) == nullptr) || r;
  r = ((glXCreateGLXPixmapWithConfigSGIX = (PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXCreateGLXPixmapWithConfigSGIX")) == nullptr) || r;
  r = ((glXGetFBConfigAttribSGIX = (PFNGLXGETFBCONFIGATTRIBSGIXPROC)glewGetProcAddress((const GLubyte*)"glXGetFBConfigAttribSGIX")) == nullptr) || r;
  r = ((glXGetFBConfigFromVisualSGIX = (PFNGLXGETFBCONFIGFROMVISUALSGIXPROC)glewGetProcAddress((const GLubyte*)"glXGetFBConfigFromVisualSGIX")) == nullptr) || r;
  r = ((glXGetVisualFromFBConfigSGIX = (PFNGLXGETVISUALFROMFBCONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfigSGIX")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGIX_fbconfig */

#ifdef GLX_SGIX_hyperpipe

static GLboolean _glewInit_GLX_SGIX_hyperpipe (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindHyperpipeSGIX = (PFNGLXBINDHYPERPIPESGIXPROC)glewGetProcAddress((const GLubyte*)"glXBindHyperpipeSGIX")) == nullptr) || r;
  r = ((glXDestroyHyperpipeConfigSGIX = (PFNGLXDESTROYHYPERPIPECONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXDestroyHyperpipeConfigSGIX")) == nullptr) || r;
  r = ((glXHyperpipeAttribSGIX = (PFNGLXHYPERPIPEATTRIBSGIXPROC)glewGetProcAddress((const GLubyte*)"glXHyperpipeAttribSGIX")) == nullptr) || r;
  r = ((glXHyperpipeConfigSGIX = (PFNGLXHYPERPIPECONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXHyperpipeConfigSGIX")) == nullptr) || r;
  r = ((glXQueryHyperpipeAttribSGIX = (PFNGLXQUERYHYPERPIPEATTRIBSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryHyperpipeAttribSGIX")) == nullptr) || r;
  r = ((glXQueryHyperpipeBestAttribSGIX = (PFNGLXQUERYHYPERPIPEBESTATTRIBSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryHyperpipeBestAttribSGIX")) == nullptr) || r;
  r = ((glXQueryHyperpipeConfigSGIX = (PFNGLXQUERYHYPERPIPECONFIGSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryHyperpipeConfigSGIX")) == nullptr) || r;
  r = ((glXQueryHyperpipeNetworkSGIX = (PFNGLXQUERYHYPERPIPENETWORKSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryHyperpipeNetworkSGIX")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGIX_hyperpipe */

#ifdef GLX_SGIX_pbuffer

static GLboolean _glewInit_GLX_SGIX_pbuffer (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCreateGLXPbufferSGIX = (PFNGLXCREATEGLXPBUFFERSGIXPROC)glewGetProcAddress((const GLubyte*)"glXCreateGLXPbufferSGIX")) == nullptr) || r;
  r = ((glXDestroyGLXPbufferSGIX = (PFNGLXDESTROYGLXPBUFFERSGIXPROC)glewGetProcAddress((const GLubyte*)"glXDestroyGLXPbufferSGIX")) == nullptr) || r;
  r = ((glXGetSelectedEventSGIX = (PFNGLXGETSELECTEDEVENTSGIXPROC)glewGetProcAddress((const GLubyte*)"glXGetSelectedEventSGIX")) == nullptr) || r;
  r = ((glXQueryGLXPbufferSGIX = (PFNGLXQUERYGLXPBUFFERSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryGLXPbufferSGIX")) == nullptr) || r;
  r = ((glXSelectEventSGIX = (PFNGLXSELECTEVENTSGIXPROC)glewGetProcAddress((const GLubyte*)"glXSelectEventSGIX")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGIX_pbuffer */

#ifdef GLX_SGIX_swap_barrier

static GLboolean _glewInit_GLX_SGIX_swap_barrier (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindSwapBarrierSGIX = (PFNGLXBINDSWAPBARRIERSGIXPROC)glewGetProcAddress((const GLubyte*)"glXBindSwapBarrierSGIX")) == nullptr) || r;
  r = ((glXQueryMaxSwapBarriersSGIX = (PFNGLXQUERYMAXSWAPBARRIERSSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryMaxSwapBarriersSGIX")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGIX_swap_barrier */

#ifdef GLX_SGIX_swap_group

static GLboolean _glewInit_GLX_SGIX_swap_group (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXJoinSwapGroupSGIX = (PFNGLXJOINSWAPGROUPSGIXPROC)glewGetProcAddress((const GLubyte*)"glXJoinSwapGroupSGIX")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGIX_swap_group */

#ifdef GLX_SGIX_video_resize

static GLboolean _glewInit_GLX_SGIX_video_resize (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXBindChannelToWindowSGIX = (PFNGLXBINDCHANNELTOWINDOWSGIXPROC)glewGetProcAddress((const GLubyte*)"glXBindChannelToWindowSGIX")) == nullptr) || r;
  r = ((glXChannelRectSGIX = (PFNGLXCHANNELRECTSGIXPROC)glewGetProcAddress((const GLubyte*)"glXChannelRectSGIX")) == nullptr) || r;
  r = ((glXChannelRectSyncSGIX = (PFNGLXCHANNELRECTSYNCSGIXPROC)glewGetProcAddress((const GLubyte*)"glXChannelRectSyncSGIX")) == nullptr) || r;
  r = ((glXQueryChannelDeltasSGIX = (PFNGLXQUERYCHANNELDELTASSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryChannelDeltasSGIX")) == nullptr) || r;
  r = ((glXQueryChannelRectSGIX = (PFNGLXQUERYCHANNELRECTSGIXPROC)glewGetProcAddress((const GLubyte*)"glXQueryChannelRectSGIX")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGIX_video_resize */

#ifdef GLX_SGIX_visual_select_group

#endif /* GLX_SGIX_visual_select_group */

#ifdef GLX_SGI_cushion

static GLboolean _glewInit_GLX_SGI_cushion (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXCushionSGI = (PFNGLXCUSHIONSGIPROC)glewGetProcAddress((const GLubyte*)"glXCushionSGI")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGI_cushion */

#ifdef GLX_SGI_make_current_read

static GLboolean _glewInit_GLX_SGI_make_current_read (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetCurrentReadDrawableSGI = (PFNGLXGETCURRENTREADDRAWABLESGIPROC)glewGetProcAddress((const GLubyte*)"glXGetCurrentReadDrawableSGI")) == nullptr) || r;
  r = ((glXMakeCurrentReadSGI = (PFNGLXMAKECURRENTREADSGIPROC)glewGetProcAddress((const GLubyte*)"glXMakeCurrentReadSGI")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGI_make_current_read */

#ifdef GLX_SGI_swap_control

static GLboolean _glewInit_GLX_SGI_swap_control (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)glewGetProcAddress((const GLubyte*)"glXSwapIntervalSGI")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGI_swap_control */

#ifdef GLX_SGI_video_sync

static GLboolean _glewInit_GLX_SGI_video_sync (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetVideoSyncSGI = (PFNGLXGETVIDEOSYNCSGIPROC)glewGetProcAddress((const GLubyte*)"glXGetVideoSyncSGI")) == nullptr) || r;
  r = ((glXWaitVideoSyncSGI = (PFNGLXWAITVIDEOSYNCSGIPROC)glewGetProcAddress((const GLubyte*)"glXWaitVideoSyncSGI")) == nullptr) || r;

  return r;
}

#endif /* GLX_SGI_video_sync */

#ifdef GLX_SUN_get_transparent_index

static GLboolean _glewInit_GLX_SUN_get_transparent_index (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetTransparentIndexSUN = (PFNGLXGETTRANSPARENTINDEXSUNPROC)glewGetProcAddress((const GLubyte*)"glXGetTransparentIndexSUN")) == nullptr) || r;

  return r;
}

#endif /* GLX_SUN_get_transparent_index */

#ifdef GLX_SUN_video_resize

static GLboolean _glewInit_GLX_SUN_video_resize (GLXEW_CONTEXT_ARG_DEF_INIT)
{
  GLboolean r = GL_FALSE;

  r = ((glXGetVideoResizeSUN = (PFNGLXGETVIDEORESIZESUNPROC)glewGetProcAddress((const GLubyte*)"glXGetVideoResizeSUN")) == nullptr) || r;
  r = ((glXVideoResizeSUN = (PFNGLXVIDEORESIZESUNPROC)glewGetProcAddress((const GLubyte*)"glXVideoResizeSUN")) == nullptr) || r;

  return r;
}

#endif /* GLX_SUN_video_resize */

/* ------------------------------------------------------------------------ */

GLboolean glxewGetExtension (const char* name)
{    
  const GLubyte* start;
  const GLubyte* end;

  if (glXGetCurrentDisplay == nullptr) return GL_FALSE;
  start = (const GLubyte*)glXGetClientString(glXGetCurrentDisplay(), GLX_EXTENSIONS);
  if (0 == start) return GL_FALSE;
  end = start + _glewStrLen(start);
  return _glewSearchExtension(name, start, end);
}

GLenum glxewContextInit (GLXEW_CONTEXT_ARG_DEF_LIST)
{
  int major, minor;
  const GLubyte* extStart;
  const GLubyte* extEnd;
  /* initialize core GLX 1.2 */
  if (_glewInit_GLX_VERSION_1_2(GLEW_CONTEXT_ARG_VAR_INIT)) return GLEW_ERROR_GLX_VERSION_11_ONLY;
  /* initialize flags */
  CONST_CAST(GLXEW_VERSION_1_0) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_1) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_2) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_3) = GL_TRUE;
  CONST_CAST(GLXEW_VERSION_1_4) = GL_TRUE;
  /* query GLX version */
  glXQueryVersion(glXGetCurrentDisplay(), &major, &minor);
  if (major == 1 && minor <= 3)
  {
    switch (minor)
    {
      case 3:
      CONST_CAST(GLXEW_VERSION_1_4) = GL_FALSE;
      break;
      case 2:
      CONST_CAST(GLXEW_VERSION_1_4) = GL_FALSE;
      CONST_CAST(GLXEW_VERSION_1_3) = GL_FALSE;
      break;
      default:
      return GLEW_ERROR_GLX_VERSION_11_ONLY;
      break;
    }
  }
  /* query GLX extension string */
  extStart = 0;
  if (glXGetCurrentDisplay != nullptr)
    extStart = (const GLubyte*)glXGetClientString(glXGetCurrentDisplay(), GLX_EXTENSIONS);
  if (extStart == 0)
    extStart = (const GLubyte *)"";
  extEnd = extStart + _glewStrLen(extStart);
  /* initialize extensions */
#ifdef GLX_VERSION_1_3
  if (glewExperimental || GLXEW_VERSION_1_3) CONST_CAST(GLXEW_VERSION_1_3) = !_glewInit_GLX_VERSION_1_3(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_VERSION_1_3 */
#ifdef GLX_3DFX_multisample
  CONST_CAST(GLXEW_3DFX_multisample) = _glewSearchExtension("GLX_3DFX_multisample", extStart, extEnd);
#endif /* GLX_3DFX_multisample */
#ifdef GLX_AMD_gpu_association
  CONST_CAST(GLXEW_AMD_gpu_association) = _glewSearchExtension("GLX_AMD_gpu_association", extStart, extEnd);
#endif /* GLX_AMD_gpu_association */
#ifdef GLX_ARB_create_context
  CONST_CAST(GLXEW_ARB_create_context) = _glewSearchExtension("GLX_ARB_create_context", extStart, extEnd);
  if (glewExperimental || GLXEW_ARB_create_context) CONST_CAST(GLXEW_ARB_create_context) = !_glewInit_GLX_ARB_create_context(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_ARB_create_context */
#ifdef GLX_ARB_create_context_profile
  CONST_CAST(GLXEW_ARB_create_context_profile) = _glewSearchExtension("GLX_ARB_create_context_profile", extStart, extEnd);
#endif /* GLX_ARB_create_context_profile */
#ifdef GLX_ARB_create_context_robustness
  CONST_CAST(GLXEW_ARB_create_context_robustness) = _glewSearchExtension("GLX_ARB_create_context_robustness", extStart, extEnd);
#endif /* GLX_ARB_create_context_robustness */
#ifdef GLX_ARB_fbconfig_float
  CONST_CAST(GLXEW_ARB_fbconfig_float) = _glewSearchExtension("GLX_ARB_fbconfig_float", extStart, extEnd);
#endif /* GLX_ARB_fbconfig_float */
#ifdef GLX_ARB_framebuffer_sRGB
  CONST_CAST(GLXEW_ARB_framebuffer_sRGB) = _glewSearchExtension("GLX_ARB_framebuffer_sRGB", extStart, extEnd);
#endif /* GLX_ARB_framebuffer_sRGB */
#ifdef GLX_ARB_get_proc_address
  CONST_CAST(GLXEW_ARB_get_proc_address) = _glewSearchExtension("GLX_ARB_get_proc_address", extStart, extEnd);
#endif /* GLX_ARB_get_proc_address */
#ifdef GLX_ARB_multisample
  CONST_CAST(GLXEW_ARB_multisample) = _glewSearchExtension("GLX_ARB_multisample", extStart, extEnd);
#endif /* GLX_ARB_multisample */
#ifdef GLX_ARB_vertex_buffer_object
  CONST_CAST(GLXEW_ARB_vertex_buffer_object) = _glewSearchExtension("GLX_ARB_vertex_buffer_object", extStart, extEnd);
#endif /* GLX_ARB_vertex_buffer_object */
#ifdef GLX_ATI_pixel_format_float
  CONST_CAST(GLXEW_ATI_pixel_format_float) = _glewSearchExtension("GLX_ATI_pixel_format_float", extStart, extEnd);
#endif /* GLX_ATI_pixel_format_float */
#ifdef GLX_ATI_render_texture
  CONST_CAST(GLXEW_ATI_render_texture) = _glewSearchExtension("GLX_ATI_render_texture", extStart, extEnd);
  if (glewExperimental || GLXEW_ATI_render_texture) CONST_CAST(GLXEW_ATI_render_texture) = !_glewInit_GLX_ATI_render_texture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_ATI_render_texture */
#ifdef GLX_EXT_create_context_es2_profile
  CONST_CAST(GLXEW_EXT_create_context_es2_profile) = _glewSearchExtension("GLX_EXT_create_context_es2_profile", extStart, extEnd);
#endif /* GLX_EXT_create_context_es2_profile */
#ifdef GLX_EXT_fbconfig_packed_float
  CONST_CAST(GLXEW_EXT_fbconfig_packed_float) = _glewSearchExtension("GLX_EXT_fbconfig_packed_float", extStart, extEnd);
#endif /* GLX_EXT_fbconfig_packed_float */
#ifdef GLX_EXT_framebuffer_sRGB
  CONST_CAST(GLXEW_EXT_framebuffer_sRGB) = _glewSearchExtension("GLX_EXT_framebuffer_sRGB", extStart, extEnd);
#endif /* GLX_EXT_framebuffer_sRGB */
#ifdef GLX_EXT_import_context
  CONST_CAST(GLXEW_EXT_import_context) = _glewSearchExtension("GLX_EXT_import_context", extStart, extEnd);
  if (glewExperimental || GLXEW_EXT_import_context) CONST_CAST(GLXEW_EXT_import_context) = !_glewInit_GLX_EXT_import_context(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_EXT_import_context */
#ifdef GLX_EXT_scene_marker
  CONST_CAST(GLXEW_EXT_scene_marker) = _glewSearchExtension("GLX_EXT_scene_marker", extStart, extEnd);
#endif /* GLX_EXT_scene_marker */
#ifdef GLX_EXT_swap_control
  CONST_CAST(GLXEW_EXT_swap_control) = _glewSearchExtension("GLX_EXT_swap_control", extStart, extEnd);
  if (glewExperimental || GLXEW_EXT_swap_control) CONST_CAST(GLXEW_EXT_swap_control) = !_glewInit_GLX_EXT_swap_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_EXT_swap_control */
#ifdef GLX_EXT_texture_from_pixmap
  CONST_CAST(GLXEW_EXT_texture_from_pixmap) = _glewSearchExtension("GLX_EXT_texture_from_pixmap", extStart, extEnd);
  if (glewExperimental || GLXEW_EXT_texture_from_pixmap) CONST_CAST(GLXEW_EXT_texture_from_pixmap) = !_glewInit_GLX_EXT_texture_from_pixmap(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_EXT_texture_from_pixmap */
#ifdef GLX_EXT_visual_info
  CONST_CAST(GLXEW_EXT_visual_info) = _glewSearchExtension("GLX_EXT_visual_info", extStart, extEnd);
#endif /* GLX_EXT_visual_info */
#ifdef GLX_EXT_visual_rating
  CONST_CAST(GLXEW_EXT_visual_rating) = _glewSearchExtension("GLX_EXT_visual_rating", extStart, extEnd);
#endif /* GLX_EXT_visual_rating */
#ifdef GLX_INTEL_swap_event
  CONST_CAST(GLXEW_INTEL_swap_event) = _glewSearchExtension("GLX_INTEL_swap_event", extStart, extEnd);
#endif /* GLX_INTEL_swap_event */
#ifdef GLX_MESA_agp_offset
  CONST_CAST(GLXEW_MESA_agp_offset) = _glewSearchExtension("GLX_MESA_agp_offset", extStart, extEnd);
  if (glewExperimental || GLXEW_MESA_agp_offset) CONST_CAST(GLXEW_MESA_agp_offset) = !_glewInit_GLX_MESA_agp_offset(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_agp_offset */
#ifdef GLX_MESA_copy_sub_buffer
  CONST_CAST(GLXEW_MESA_copy_sub_buffer) = _glewSearchExtension("GLX_MESA_copy_sub_buffer", extStart, extEnd);
  if (glewExperimental || GLXEW_MESA_copy_sub_buffer) CONST_CAST(GLXEW_MESA_copy_sub_buffer) = !_glewInit_GLX_MESA_copy_sub_buffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_copy_sub_buffer */
#ifdef GLX_MESA_pixmap_colormap
  CONST_CAST(GLXEW_MESA_pixmap_colormap) = _glewSearchExtension("GLX_MESA_pixmap_colormap", extStart, extEnd);
  if (glewExperimental || GLXEW_MESA_pixmap_colormap) CONST_CAST(GLXEW_MESA_pixmap_colormap) = !_glewInit_GLX_MESA_pixmap_colormap(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_pixmap_colormap */
#ifdef GLX_MESA_release_buffers
  CONST_CAST(GLXEW_MESA_release_buffers) = _glewSearchExtension("GLX_MESA_release_buffers", extStart, extEnd);
  if (glewExperimental || GLXEW_MESA_release_buffers) CONST_CAST(GLXEW_MESA_release_buffers) = !_glewInit_GLX_MESA_release_buffers(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_release_buffers */
#ifdef GLX_MESA_set_3dfx_mode
  CONST_CAST(GLXEW_MESA_set_3dfx_mode) = _glewSearchExtension("GLX_MESA_set_3dfx_mode", extStart, extEnd);
  if (glewExperimental || GLXEW_MESA_set_3dfx_mode) CONST_CAST(GLXEW_MESA_set_3dfx_mode) = !_glewInit_GLX_MESA_set_3dfx_mode(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_set_3dfx_mode */
#ifdef GLX_MESA_swap_control
  CONST_CAST(GLXEW_MESA_swap_control) = _glewSearchExtension("GLX_MESA_swap_control", extStart, extEnd);
  if (glewExperimental || GLXEW_MESA_swap_control) CONST_CAST(GLXEW_MESA_swap_control) = !_glewInit_GLX_MESA_swap_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_MESA_swap_control */
#ifdef GLX_NV_copy_image
  CONST_CAST(GLXEW_NV_copy_image) = _glewSearchExtension("GLX_NV_copy_image", extStart, extEnd);
  if (glewExperimental || GLXEW_NV_copy_image) CONST_CAST(GLXEW_NV_copy_image) = !_glewInit_GLX_NV_copy_image(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_copy_image */
#ifdef GLX_NV_float_buffer
  CONST_CAST(GLXEW_NV_float_buffer) = _glewSearchExtension("GLX_NV_float_buffer", extStart, extEnd);
#endif /* GLX_NV_float_buffer */
#ifdef GLX_NV_multisample_coverage
  CONST_CAST(GLXEW_NV_multisample_coverage) = _glewSearchExtension("GLX_NV_multisample_coverage", extStart, extEnd);
#endif /* GLX_NV_multisample_coverage */
#ifdef GLX_NV_present_video
  CONST_CAST(GLXEW_NV_present_video) = _glewSearchExtension("GLX_NV_present_video", extStart, extEnd);
  if (glewExperimental || GLXEW_NV_present_video) CONST_CAST(GLXEW_NV_present_video) = !_glewInit_GLX_NV_present_video(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_present_video */
#ifdef GLX_NV_swap_group
  CONST_CAST(GLXEW_NV_swap_group) = _glewSearchExtension("GLX_NV_swap_group", extStart, extEnd);
  if (glewExperimental || GLXEW_NV_swap_group) CONST_CAST(GLXEW_NV_swap_group) = !_glewInit_GLX_NV_swap_group(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_swap_group */
#ifdef GLX_NV_vertex_array_range
  CONST_CAST(GLXEW_NV_vertex_array_range) = _glewSearchExtension("GLX_NV_vertex_array_range", extStart, extEnd);
  if (glewExperimental || GLXEW_NV_vertex_array_range) CONST_CAST(GLXEW_NV_vertex_array_range) = !_glewInit_GLX_NV_vertex_array_range(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_vertex_array_range */
#ifdef GLX_NV_video_capture
  CONST_CAST(GLXEW_NV_video_capture) = _glewSearchExtension("GLX_NV_video_capture", extStart, extEnd);
  if (glewExperimental || GLXEW_NV_video_capture) CONST_CAST(GLXEW_NV_video_capture) = !_glewInit_GLX_NV_video_capture(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_video_capture */
#ifdef GLX_NV_video_output
  CONST_CAST(GLXEW_NV_video_output) = _glewSearchExtension("GLX_NV_video_output", extStart, extEnd);
  if (glewExperimental || GLXEW_NV_video_output) CONST_CAST(GLXEW_NV_video_output) = !_glewInit_GLX_NV_video_output(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_NV_video_output */
#ifdef GLX_OML_swap_method
  CONST_CAST(GLXEW_OML_swap_method) = _glewSearchExtension("GLX_OML_swap_method", extStart, extEnd);
#endif /* GLX_OML_swap_method */
#ifdef GLX_OML_sync_control
  CONST_CAST(GLXEW_OML_sync_control) = _glewSearchExtension("GLX_OML_sync_control", extStart, extEnd);
  if (glewExperimental || GLXEW_OML_sync_control) CONST_CAST(GLXEW_OML_sync_control) = !_glewInit_GLX_OML_sync_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_OML_sync_control */
#ifdef GLX_SGIS_blended_overlay
  CONST_CAST(GLXEW_SGIS_blended_overlay) = _glewSearchExtension("GLX_SGIS_blended_overlay", extStart, extEnd);
#endif /* GLX_SGIS_blended_overlay */
#ifdef GLX_SGIS_color_range
  CONST_CAST(GLXEW_SGIS_color_range) = _glewSearchExtension("GLX_SGIS_color_range", extStart, extEnd);
#endif /* GLX_SGIS_color_range */
#ifdef GLX_SGIS_multisample
  CONST_CAST(GLXEW_SGIS_multisample) = _glewSearchExtension("GLX_SGIS_multisample", extStart, extEnd);
#endif /* GLX_SGIS_multisample */
#ifdef GLX_SGIS_shared_multisample
  CONST_CAST(GLXEW_SGIS_shared_multisample) = _glewSearchExtension("GLX_SGIS_shared_multisample", extStart, extEnd);
#endif /* GLX_SGIS_shared_multisample */
#ifdef GLX_SGIX_fbconfig
  CONST_CAST(GLXEW_SGIX_fbconfig) = _glewSearchExtension("GLX_SGIX_fbconfig", extStart, extEnd);
  if (glewExperimental || GLXEW_SGIX_fbconfig) CONST_CAST(GLXEW_SGIX_fbconfig) = !_glewInit_GLX_SGIX_fbconfig(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_fbconfig */
#ifdef GLX_SGIX_hyperpipe
  CONST_CAST(GLXEW_SGIX_hyperpipe) = _glewSearchExtension("GLX_SGIX_hyperpipe", extStart, extEnd);
  if (glewExperimental || GLXEW_SGIX_hyperpipe) CONST_CAST(GLXEW_SGIX_hyperpipe) = !_glewInit_GLX_SGIX_hyperpipe(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_hyperpipe */
#ifdef GLX_SGIX_pbuffer
  CONST_CAST(GLXEW_SGIX_pbuffer) = _glewSearchExtension("GLX_SGIX_pbuffer", extStart, extEnd);
  if (glewExperimental || GLXEW_SGIX_pbuffer) CONST_CAST(GLXEW_SGIX_pbuffer) = !_glewInit_GLX_SGIX_pbuffer(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_pbuffer */
#ifdef GLX_SGIX_swap_barrier
  CONST_CAST(GLXEW_SGIX_swap_barrier) = _glewSearchExtension("GLX_SGIX_swap_barrier", extStart, extEnd);
  if (glewExperimental || GLXEW_SGIX_swap_barrier) CONST_CAST(GLXEW_SGIX_swap_barrier) = !_glewInit_GLX_SGIX_swap_barrier(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_swap_barrier */
#ifdef GLX_SGIX_swap_group
  CONST_CAST(GLXEW_SGIX_swap_group) = _glewSearchExtension("GLX_SGIX_swap_group", extStart, extEnd);
  if (glewExperimental || GLXEW_SGIX_swap_group) CONST_CAST(GLXEW_SGIX_swap_group) = !_glewInit_GLX_SGIX_swap_group(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_swap_group */
#ifdef GLX_SGIX_video_resize
  CONST_CAST(GLXEW_SGIX_video_resize) = _glewSearchExtension("GLX_SGIX_video_resize", extStart, extEnd);
  if (glewExperimental || GLXEW_SGIX_video_resize) CONST_CAST(GLXEW_SGIX_video_resize) = !_glewInit_GLX_SGIX_video_resize(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGIX_video_resize */
#ifdef GLX_SGIX_visual_select_group
  CONST_CAST(GLXEW_SGIX_visual_select_group) = _glewSearchExtension("GLX_SGIX_visual_select_group", extStart, extEnd);
#endif /* GLX_SGIX_visual_select_group */
#ifdef GLX_SGI_cushion
  CONST_CAST(GLXEW_SGI_cushion) = _glewSearchExtension("GLX_SGI_cushion", extStart, extEnd);
  if (glewExperimental || GLXEW_SGI_cushion) CONST_CAST(GLXEW_SGI_cushion) = !_glewInit_GLX_SGI_cushion(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_cushion */
#ifdef GLX_SGI_make_current_read
  CONST_CAST(GLXEW_SGI_make_current_read) = _glewSearchExtension("GLX_SGI_make_current_read", extStart, extEnd);
  if (glewExperimental || GLXEW_SGI_make_current_read) CONST_CAST(GLXEW_SGI_make_current_read) = !_glewInit_GLX_SGI_make_current_read(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_make_current_read */
#ifdef GLX_SGI_swap_control
  CONST_CAST(GLXEW_SGI_swap_control) = _glewSearchExtension("GLX_SGI_swap_control", extStart, extEnd);
  if (glewExperimental || GLXEW_SGI_swap_control) CONST_CAST(GLXEW_SGI_swap_control) = !_glewInit_GLX_SGI_swap_control(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_swap_control */
#ifdef GLX_SGI_video_sync
  CONST_CAST(GLXEW_SGI_video_sync) = _glewSearchExtension("GLX_SGI_video_sync", extStart, extEnd);
  if (glewExperimental || GLXEW_SGI_video_sync) CONST_CAST(GLXEW_SGI_video_sync) = !_glewInit_GLX_SGI_video_sync(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SGI_video_sync */
#ifdef GLX_SUN_get_transparent_index
  CONST_CAST(GLXEW_SUN_get_transparent_index) = _glewSearchExtension("GLX_SUN_get_transparent_index", extStart, extEnd);
  if (glewExperimental || GLXEW_SUN_get_transparent_index) CONST_CAST(GLXEW_SUN_get_transparent_index) = !_glewInit_GLX_SUN_get_transparent_index(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SUN_get_transparent_index */
#ifdef GLX_SUN_video_resize
  CONST_CAST(GLXEW_SUN_video_resize) = _glewSearchExtension("GLX_SUN_video_resize", extStart, extEnd);
  if (glewExperimental || GLXEW_SUN_video_resize) CONST_CAST(GLXEW_SUN_video_resize) = !_glewInit_GLX_SUN_video_resize(GLEW_CONTEXT_ARG_VAR_INIT);
#endif /* GLX_SUN_video_resize */

  return GLEW_OK;
}

#endif /* !__APPLE__ || GLEW_APPLE_GLX */

/* ------------------------------------------------------------------------ */

const GLubyte* glewGetErrorString (GLenum error)
{
  static const GLubyte* _glewErrorString[] =
  {
    (const GLubyte*)"No error",
    (const GLubyte*)"Missing GL version",
    (const GLubyte*)"GL 1.1 and up are not supported",
    (const GLubyte*)"GLX 1.2 and up are not supported",
    (const GLubyte*)"Unknown error"
  };
  const int max_error = sizeof(_glewErrorString)/sizeof(*_glewErrorString) - 1;
  return _glewErrorString[(int)error > max_error ? max_error : (int)error];
}

const GLubyte* glewGetString (GLenum name)
{
  static const GLubyte* _glewString[] =
  {
    (const GLubyte*)nullptr,
    (const GLubyte*)"1.7.0",
    (const GLubyte*)"1",
    (const GLubyte*)"7",
    (const GLubyte*)"0"
  };
  const int max_string = sizeof(_glewString)/sizeof(*_glewString) - 1;
  return _glewString[(int)name > max_string ? 0 : (int)name];
}

/* ------------------------------------------------------------------------ */

GLboolean glewExperimental = GL_FALSE;

#if !defined(GLEW_MX)

#if defined(_WIN32)
extern GLenum wglewContextInit (void);
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX) /* _UNIX */
extern GLenum glxewContextInit (void);
#endif /* _WIN32 */

GLenum glewInit ()
{
  GLenum r;
  if ( (r = glewContextInit()) ) return r;
#if defined(_WIN32)
  return wglewContextInit();
#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX) /* _UNIX */
  return glxewContextInit();
#else
  return r;
#endif /* _WIN32 */
}

#endif /* !GLEW_MX */
#ifdef GLEW_MX
GLboolean glewContextIsSupported (const GLEWContext* ctx, const char* name)
#else
GLboolean glewIsSupported (const char* name)
#endif
{
  GLubyte* pos = (GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"GL_", 3))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"VERSION_", 8))
      {
#ifdef GL_VERSION_1_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2", 3))
        {
          ret = GLEW_VERSION_1_2;
          continue;
        }
#endif
#ifdef GL_VERSION_1_2_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2_1", 5))
        {
          ret = GLEW_VERSION_1_2_1;
          continue;
        }
#endif
#ifdef GL_VERSION_1_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_3", 3))
        {
          ret = GLEW_VERSION_1_3;
          continue;
        }
#endif
#ifdef GL_VERSION_1_4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_4", 3))
        {
          ret = GLEW_VERSION_1_4;
          continue;
        }
#endif
#ifdef GL_VERSION_1_5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_5", 3))
        {
          ret = GLEW_VERSION_1_5;
          continue;
        }
#endif
#ifdef GL_VERSION_2_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"2_0", 3))
        {
          ret = GLEW_VERSION_2_0;
          continue;
        }
#endif
#ifdef GL_VERSION_2_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"2_1", 3))
        {
          ret = GLEW_VERSION_2_1;
          continue;
        }
#endif
#ifdef GL_VERSION_3_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_0", 3))
        {
          ret = GLEW_VERSION_3_0;
          continue;
        }
#endif
#ifdef GL_VERSION_3_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_1", 3))
        {
          ret = GLEW_VERSION_3_1;
          continue;
        }
#endif
#ifdef GL_VERSION_3_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_2", 3))
        {
          ret = GLEW_VERSION_3_2;
          continue;
        }
#endif
#ifdef GL_VERSION_3_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_3", 3))
        {
          ret = GLEW_VERSION_3_3;
          continue;
        }
#endif
#ifdef GL_VERSION_4_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_0", 3))
        {
          ret = GLEW_VERSION_4_0;
          continue;
        }
#endif
#ifdef GL_VERSION_4_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_1", 3))
        {
          ret = GLEW_VERSION_4_1;
          continue;
        }
#endif
#ifdef GL_VERSION_4_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_2", 3))
        {
          ret = GLEW_VERSION_4_2;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DFX_", 5))
      {
#ifdef GL_3DFX_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_3DFX_multisample;
          continue;
        }
#endif
#ifdef GL_3DFX_tbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tbuffer", 7))
        {
          ret = GLEW_3DFX_tbuffer;
          continue;
        }
#endif
#ifdef GL_3DFX_texture_compression_FXT1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_FXT1", 24))
        {
          ret = GLEW_3DFX_texture_compression_FXT1;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"AMD_", 4))
      {
#ifdef GL_AMD_blend_minmax_factor
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_minmax_factor", 19))
        {
          ret = GLEW_AMD_blend_minmax_factor;
          continue;
        }
#endif
#ifdef GL_AMD_conservative_depth
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conservative_depth", 18))
        {
          ret = GLEW_AMD_conservative_depth;
          continue;
        }
#endif
#ifdef GL_AMD_debug_output
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"debug_output", 12))
        {
          ret = GLEW_AMD_debug_output;
          continue;
        }
#endif
#ifdef GL_AMD_depth_clamp_separate
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_clamp_separate", 20))
        {
          ret = GLEW_AMD_depth_clamp_separate;
          continue;
        }
#endif
#ifdef GL_AMD_draw_buffers_blend
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers_blend", 18))
        {
          ret = GLEW_AMD_draw_buffers_blend;
          continue;
        }
#endif
#ifdef GL_AMD_multi_draw_indirect
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multi_draw_indirect", 19))
        {
          ret = GLEW_AMD_multi_draw_indirect;
          continue;
        }
#endif
#ifdef GL_AMD_name_gen_delete
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"name_gen_delete", 15))
        {
          ret = GLEW_AMD_name_gen_delete;
          continue;
        }
#endif
#ifdef GL_AMD_performance_monitor
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"performance_monitor", 19))
        {
          ret = GLEW_AMD_performance_monitor;
          continue;
        }
#endif
#ifdef GL_AMD_sample_positions
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sample_positions", 16))
        {
          ret = GLEW_AMD_sample_positions;
          continue;
        }
#endif
#ifdef GL_AMD_seamless_cubemap_per_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"seamless_cubemap_per_texture", 28))
        {
          ret = GLEW_AMD_seamless_cubemap_per_texture;
          continue;
        }
#endif
#ifdef GL_AMD_shader_stencil_export
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_stencil_export", 21))
        {
          ret = GLEW_AMD_shader_stencil_export;
          continue;
        }
#endif
#ifdef GL_AMD_texture_texture4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_texture4", 16))
        {
          ret = GLEW_AMD_texture_texture4;
          continue;
        }
#endif
#ifdef GL_AMD_transform_feedback3_lines_triangles
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback3_lines_triangles", 35))
        {
          ret = GLEW_AMD_transform_feedback3_lines_triangles;
          continue;
        }
#endif
#ifdef GL_AMD_vertex_shader_tessellator
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader_tessellator", 25))
        {
          ret = GLEW_AMD_vertex_shader_tessellator;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"APPLE_", 6))
      {
#ifdef GL_APPLE_aux_depth_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"aux_depth_stencil", 17))
        {
          ret = GLEW_APPLE_aux_depth_stencil;
          continue;
        }
#endif
#ifdef GL_APPLE_client_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"client_storage", 14))
        {
          ret = GLEW_APPLE_client_storage;
          continue;
        }
#endif
#ifdef GL_APPLE_element_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"element_array", 13))
        {
          ret = GLEW_APPLE_element_array;
          continue;
        }
#endif
#ifdef GL_APPLE_fence
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fence", 5))
        {
          ret = GLEW_APPLE_fence;
          continue;
        }
#endif
#ifdef GL_APPLE_float_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_pixels", 12))
        {
          ret = GLEW_APPLE_float_pixels;
          continue;
        }
#endif
#ifdef GL_APPLE_flush_buffer_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"flush_buffer_range", 18))
        {
          ret = GLEW_APPLE_flush_buffer_range;
          continue;
        }
#endif
#ifdef GL_APPLE_object_purgeable
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"object_purgeable", 16))
        {
          ret = GLEW_APPLE_object_purgeable;
          continue;
        }
#endif
#ifdef GL_APPLE_pixel_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer", 12))
        {
          ret = GLEW_APPLE_pixel_buffer;
          continue;
        }
#endif
#ifdef GL_APPLE_rgb_422
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"rgb_422", 7))
        {
          ret = GLEW_APPLE_rgb_422;
          continue;
        }
#endif
#ifdef GL_APPLE_row_bytes
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"row_bytes", 9))
        {
          ret = GLEW_APPLE_row_bytes;
          continue;
        }
#endif
#ifdef GL_APPLE_specular_vector
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"specular_vector", 15))
        {
          ret = GLEW_APPLE_specular_vector;
          continue;
        }
#endif
#ifdef GL_APPLE_texture_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_range", 13))
        {
          ret = GLEW_APPLE_texture_range;
          continue;
        }
#endif
#ifdef GL_APPLE_transform_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_hint", 14))
        {
          ret = GLEW_APPLE_transform_hint;
          continue;
        }
#endif
#ifdef GL_APPLE_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_APPLE_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_APPLE_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = GLEW_APPLE_vertex_array_range;
          continue;
        }
#endif
#ifdef GL_APPLE_vertex_program_evaluators
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program_evaluators", 25))
        {
          ret = GLEW_APPLE_vertex_program_evaluators;
          continue;
        }
#endif
#ifdef GL_APPLE_ycbcr_422
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ycbcr_422", 9))
        {
          ret = GLEW_APPLE_ycbcr_422;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef GL_ARB_ES2_compatibility
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ES2_compatibility", 17))
        {
          ret = GLEW_ARB_ES2_compatibility;
          continue;
        }
#endif
#ifdef GL_ARB_base_instance
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"base_instance", 13))
        {
          ret = GLEW_ARB_base_instance;
          continue;
        }
#endif
#ifdef GL_ARB_blend_func_extended
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_func_extended", 19))
        {
          ret = GLEW_ARB_blend_func_extended;
          continue;
        }
#endif
#ifdef GL_ARB_cl_event
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cl_event", 8))
        {
          ret = GLEW_ARB_cl_event;
          continue;
        }
#endif
#ifdef GL_ARB_color_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_buffer_float", 18))
        {
          ret = GLEW_ARB_color_buffer_float;
          continue;
        }
#endif
#ifdef GL_ARB_compatibility
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compatibility", 13))
        {
          ret = GLEW_ARB_compatibility;
          continue;
        }
#endif
#ifdef GL_ARB_compressed_texture_pixel_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compressed_texture_pixel_storage", 32))
        {
          ret = GLEW_ARB_compressed_texture_pixel_storage;
          continue;
        }
#endif
#ifdef GL_ARB_conservative_depth
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conservative_depth", 18))
        {
          ret = GLEW_ARB_conservative_depth;
          continue;
        }
#endif
#ifdef GL_ARB_copy_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_buffer", 11))
        {
          ret = GLEW_ARB_copy_buffer;
          continue;
        }
#endif
#ifdef GL_ARB_debug_output
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"debug_output", 12))
        {
          ret = GLEW_ARB_debug_output;
          continue;
        }
#endif
#ifdef GL_ARB_depth_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_buffer_float", 18))
        {
          ret = GLEW_ARB_depth_buffer_float;
          continue;
        }
#endif
#ifdef GL_ARB_depth_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_clamp", 11))
        {
          ret = GLEW_ARB_depth_clamp;
          continue;
        }
#endif
#ifdef GL_ARB_depth_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_texture", 13))
        {
          ret = GLEW_ARB_depth_texture;
          continue;
        }
#endif
#ifdef GL_ARB_draw_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers", 12))
        {
          ret = GLEW_ARB_draw_buffers;
          continue;
        }
#endif
#ifdef GL_ARB_draw_buffers_blend
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers_blend", 18))
        {
          ret = GLEW_ARB_draw_buffers_blend;
          continue;
        }
#endif
#ifdef GL_ARB_draw_elements_base_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_elements_base_vertex", 25))
        {
          ret = GLEW_ARB_draw_elements_base_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_draw_indirect
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_indirect", 13))
        {
          ret = GLEW_ARB_draw_indirect;
          continue;
        }
#endif
#ifdef GL_ARB_draw_instanced
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_instanced", 14))
        {
          ret = GLEW_ARB_draw_instanced;
          continue;
        }
#endif
#ifdef GL_ARB_explicit_attrib_location
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"explicit_attrib_location", 24))
        {
          ret = GLEW_ARB_explicit_attrib_location;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_coord_conventions
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_coord_conventions", 26))
        {
          ret = GLEW_ARB_fragment_coord_conventions;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program", 16))
        {
          ret = GLEW_ARB_fragment_program;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_program_shadow
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program_shadow", 23))
        {
          ret = GLEW_ARB_fragment_program_shadow;
          continue;
        }
#endif
#ifdef GL_ARB_fragment_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_shader", 15))
        {
          ret = GLEW_ARB_fragment_shader;
          continue;
        }
#endif
#ifdef GL_ARB_framebuffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_object", 18))
        {
          ret = GLEW_ARB_framebuffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GL_ARB_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_ARB_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_ARB_get_program_binary
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"get_program_binary", 18))
        {
          ret = GLEW_ARB_get_program_binary;
          continue;
        }
#endif
#ifdef GL_ARB_gpu_shader5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader5", 11))
        {
          ret = GLEW_ARB_gpu_shader5;
          continue;
        }
#endif
#ifdef GL_ARB_gpu_shader_fp64
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader_fp64", 15))
        {
          ret = GLEW_ARB_gpu_shader_fp64;
          continue;
        }
#endif
#ifdef GL_ARB_half_float_pixel
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float_pixel", 16))
        {
          ret = GLEW_ARB_half_float_pixel;
          continue;
        }
#endif
#ifdef GL_ARB_half_float_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float_vertex", 17))
        {
          ret = GLEW_ARB_half_float_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_imaging
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"imaging", 7))
        {
          ret = GLEW_ARB_imaging;
          continue;
        }
#endif
#ifdef GL_ARB_instanced_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"instanced_arrays", 16))
        {
          ret = GLEW_ARB_instanced_arrays;
          continue;
        }
#endif
#ifdef GL_ARB_internalformat_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"internalformat_query", 20))
        {
          ret = GLEW_ARB_internalformat_query;
          continue;
        }
#endif
#ifdef GL_ARB_map_buffer_alignment
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"map_buffer_alignment", 20))
        {
          ret = GLEW_ARB_map_buffer_alignment;
          continue;
        }
#endif
#ifdef GL_ARB_map_buffer_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"map_buffer_range", 16))
        {
          ret = GLEW_ARB_map_buffer_range;
          continue;
        }
#endif
#ifdef GL_ARB_matrix_palette
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"matrix_palette", 14))
        {
          ret = GLEW_ARB_matrix_palette;
          continue;
        }
#endif
#ifdef GL_ARB_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_ARB_multisample;
          continue;
        }
#endif
#ifdef GL_ARB_multitexture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multitexture", 12))
        {
          ret = GLEW_ARB_multitexture;
          continue;
        }
#endif
#ifdef GL_ARB_occlusion_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_query", 15))
        {
          ret = GLEW_ARB_occlusion_query;
          continue;
        }
#endif
#ifdef GL_ARB_occlusion_query2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_query2", 16))
        {
          ret = GLEW_ARB_occlusion_query2;
          continue;
        }
#endif
#ifdef GL_ARB_pixel_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer_object", 19))
        {
          ret = GLEW_ARB_pixel_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_point_parameters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_parameters", 16))
        {
          ret = GLEW_ARB_point_parameters;
          continue;
        }
#endif
#ifdef GL_ARB_point_sprite
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_sprite", 12))
        {
          ret = GLEW_ARB_point_sprite;
          continue;
        }
#endif
#ifdef GL_ARB_provoking_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"provoking_vertex", 16))
        {
          ret = GLEW_ARB_provoking_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_robustness
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"robustness", 10))
        {
          ret = GLEW_ARB_robustness;
          continue;
        }
#endif
#ifdef GL_ARB_sample_shading
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sample_shading", 14))
        {
          ret = GLEW_ARB_sample_shading;
          continue;
        }
#endif
#ifdef GL_ARB_sampler_objects
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sampler_objects", 15))
        {
          ret = GLEW_ARB_sampler_objects;
          continue;
        }
#endif
#ifdef GL_ARB_seamless_cube_map
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"seamless_cube_map", 17))
        {
          ret = GLEW_ARB_seamless_cube_map;
          continue;
        }
#endif
#ifdef GL_ARB_separate_shader_objects
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"separate_shader_objects", 23))
        {
          ret = GLEW_ARB_separate_shader_objects;
          continue;
        }
#endif
#ifdef GL_ARB_shader_atomic_counters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_atomic_counters", 22))
        {
          ret = GLEW_ARB_shader_atomic_counters;
          continue;
        }
#endif
#ifdef GL_ARB_shader_bit_encoding
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_bit_encoding", 19))
        {
          ret = GLEW_ARB_shader_bit_encoding;
          continue;
        }
#endif
#ifdef GL_ARB_shader_image_load_store
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_image_load_store", 23))
        {
          ret = GLEW_ARB_shader_image_load_store;
          continue;
        }
#endif
#ifdef GL_ARB_shader_objects
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_objects", 14))
        {
          ret = GLEW_ARB_shader_objects;
          continue;
        }
#endif
#ifdef GL_ARB_shader_precision
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_precision", 16))
        {
          ret = GLEW_ARB_shader_precision;
          continue;
        }
#endif
#ifdef GL_ARB_shader_stencil_export
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_stencil_export", 21))
        {
          ret = GLEW_ARB_shader_stencil_export;
          continue;
        }
#endif
#ifdef GL_ARB_shader_subroutine
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_subroutine", 17))
        {
          ret = GLEW_ARB_shader_subroutine;
          continue;
        }
#endif
#ifdef GL_ARB_shader_texture_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_texture_lod", 18))
        {
          ret = GLEW_ARB_shader_texture_lod;
          continue;
        }
#endif
#ifdef GL_ARB_shading_language_100
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shading_language_100", 20))
        {
          ret = GLEW_ARB_shading_language_100;
          continue;
        }
#endif
#ifdef GL_ARB_shading_language_420pack
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shading_language_420pack", 24))
        {
          ret = GLEW_ARB_shading_language_420pack;
          continue;
        }
#endif
#ifdef GL_ARB_shading_language_include
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shading_language_include", 24))
        {
          ret = GLEW_ARB_shading_language_include;
          continue;
        }
#endif
#ifdef GL_ARB_shading_language_packing
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shading_language_packing", 24))
        {
          ret = GLEW_ARB_shading_language_packing;
          continue;
        }
#endif
#ifdef GL_ARB_shadow
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow", 6))
        {
          ret = GLEW_ARB_shadow;
          continue;
        }
#endif
#ifdef GL_ARB_shadow_ambient
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow_ambient", 14))
        {
          ret = GLEW_ARB_shadow_ambient;
          continue;
        }
#endif
#ifdef GL_ARB_sync
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sync", 4))
        {
          ret = GLEW_ARB_sync;
          continue;
        }
#endif
#ifdef GL_ARB_tessellation_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tessellation_shader", 19))
        {
          ret = GLEW_ARB_tessellation_shader;
          continue;
        }
#endif
#ifdef GL_ARB_texture_border_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_border_clamp", 20))
        {
          ret = GLEW_ARB_texture_border_clamp;
          continue;
        }
#endif
#ifdef GL_ARB_texture_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_buffer_object", 21))
        {
          ret = GLEW_ARB_texture_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_texture_buffer_object_rgb32
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_buffer_object_rgb32", 27))
        {
          ret = GLEW_ARB_texture_buffer_object_rgb32;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression", 19))
        {
          ret = GLEW_ARB_texture_compression;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression_bptc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_bptc", 24))
        {
          ret = GLEW_ARB_texture_compression_bptc;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression_rgtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_rgtc", 24))
        {
          ret = GLEW_ARB_texture_compression_rgtc;
          continue;
        }
#endif
#ifdef GL_ARB_texture_cube_map
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_cube_map", 16))
        {
          ret = GLEW_ARB_texture_cube_map;
          continue;
        }
#endif
#ifdef GL_ARB_texture_cube_map_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_cube_map_array", 22))
        {
          ret = GLEW_ARB_texture_cube_map_array;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_add
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_add", 15))
        {
          ret = GLEW_ARB_texture_env_add;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_combine
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine", 19))
        {
          ret = GLEW_ARB_texture_env_combine;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_crossbar
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_crossbar", 20))
        {
          ret = GLEW_ARB_texture_env_crossbar;
          continue;
        }
#endif
#ifdef GL_ARB_texture_env_dot3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_dot3", 16))
        {
          ret = GLEW_ARB_texture_env_dot3;
          continue;
        }
#endif
#ifdef GL_ARB_texture_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_float", 13))
        {
          ret = GLEW_ARB_texture_float;
          continue;
        }
#endif
#ifdef GL_ARB_texture_gather
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_gather", 14))
        {
          ret = GLEW_ARB_texture_gather;
          continue;
        }
#endif
#ifdef GL_ARB_texture_mirrored_repeat
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirrored_repeat", 23))
        {
          ret = GLEW_ARB_texture_mirrored_repeat;
          continue;
        }
#endif
#ifdef GL_ARB_texture_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_multisample", 19))
        {
          ret = GLEW_ARB_texture_multisample;
          continue;
        }
#endif
#ifdef GL_ARB_texture_non_power_of_two
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_non_power_of_two", 24))
        {
          ret = GLEW_ARB_texture_non_power_of_two;
          continue;
        }
#endif
#ifdef GL_ARB_texture_query_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_query_lod", 17))
        {
          ret = GLEW_ARB_texture_query_lod;
          continue;
        }
#endif
#ifdef GL_ARB_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rectangle", 17))
        {
          ret = GLEW_ARB_texture_rectangle;
          continue;
        }
#endif
#ifdef GL_ARB_texture_rg
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rg", 10))
        {
          ret = GLEW_ARB_texture_rg;
          continue;
        }
#endif
#ifdef GL_ARB_texture_rgb10_a2ui
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rgb10_a2ui", 18))
        {
          ret = GLEW_ARB_texture_rgb10_a2ui;
          continue;
        }
#endif
#ifdef GL_ARB_texture_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_storage", 15))
        {
          ret = GLEW_ARB_texture_storage;
          continue;
        }
#endif
#ifdef GL_ARB_texture_swizzle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_swizzle", 15))
        {
          ret = GLEW_ARB_texture_swizzle;
          continue;
        }
#endif
#ifdef GL_ARB_timer_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"timer_query", 11))
        {
          ret = GLEW_ARB_timer_query;
          continue;
        }
#endif
#ifdef GL_ARB_transform_feedback2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback2", 19))
        {
          ret = GLEW_ARB_transform_feedback2;
          continue;
        }
#endif
#ifdef GL_ARB_transform_feedback3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback3", 19))
        {
          ret = GLEW_ARB_transform_feedback3;
          continue;
        }
#endif
#ifdef GL_ARB_transform_feedback_instanced
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback_instanced", 28))
        {
          ret = GLEW_ARB_transform_feedback_instanced;
          continue;
        }
#endif
#ifdef GL_ARB_transpose_matrix
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transpose_matrix", 16))
        {
          ret = GLEW_ARB_transpose_matrix;
          continue;
        }
#endif
#ifdef GL_ARB_uniform_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"uniform_buffer_object", 21))
        {
          ret = GLEW_ARB_uniform_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_array_bgra
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_bgra", 17))
        {
          ret = GLEW_ARB_vertex_array_bgra;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_ARB_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_attrib_64bit
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_attrib_64bit", 19))
        {
          ret = GLEW_ARB_vertex_attrib_64bit;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_blend
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_blend", 12))
        {
          ret = GLEW_ARB_vertex_blend;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_buffer_object", 20))
        {
          ret = GLEW_ARB_vertex_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program", 14))
        {
          ret = GLEW_ARB_vertex_program;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader", 13))
        {
          ret = GLEW_ARB_vertex_shader;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_type_2_10_10_10_rev
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_type_2_10_10_10_rev", 26))
        {
          ret = GLEW_ARB_vertex_type_2_10_10_10_rev;
          continue;
        }
#endif
#ifdef GL_ARB_viewport_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"viewport_array", 14))
        {
          ret = GLEW_ARB_viewport_array;
          continue;
        }
#endif
#ifdef GL_ARB_window_pos
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"window_pos", 10))
        {
          ret = GLEW_ARB_window_pos;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATIX_", 5))
      {
#ifdef GL_ATIX_point_sprites
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_sprites", 13))
        {
          ret = GLEW_ATIX_point_sprites;
          continue;
        }
#endif
#ifdef GL_ATIX_texture_env_combine3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine3", 20))
        {
          ret = GLEW_ATIX_texture_env_combine3;
          continue;
        }
#endif
#ifdef GL_ATIX_texture_env_route
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_route", 17))
        {
          ret = GLEW_ATIX_texture_env_route;
          continue;
        }
#endif
#ifdef GL_ATIX_vertex_shader_output_point_size
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader_output_point_size", 31))
        {
          ret = GLEW_ATIX_vertex_shader_output_point_size;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATI_", 4))
      {
#ifdef GL_ATI_draw_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers", 12))
        {
          ret = GLEW_ATI_draw_buffers;
          continue;
        }
#endif
#ifdef GL_ATI_element_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"element_array", 13))
        {
          ret = GLEW_ATI_element_array;
          continue;
        }
#endif
#ifdef GL_ATI_envmap_bumpmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"envmap_bumpmap", 14))
        {
          ret = GLEW_ATI_envmap_bumpmap;
          continue;
        }
#endif
#ifdef GL_ATI_fragment_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_shader", 15))
        {
          ret = GLEW_ATI_fragment_shader;
          continue;
        }
#endif
#ifdef GL_ATI_map_object_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"map_object_buffer", 17))
        {
          ret = GLEW_ATI_map_object_buffer;
          continue;
        }
#endif
#ifdef GL_ATI_meminfo
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"meminfo", 7))
        {
          ret = GLEW_ATI_meminfo;
          continue;
        }
#endif
#ifdef GL_ATI_pn_triangles
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pn_triangles", 12))
        {
          ret = GLEW_ATI_pn_triangles;
          continue;
        }
#endif
#ifdef GL_ATI_separate_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"separate_stencil", 16))
        {
          ret = GLEW_ATI_separate_stencil;
          continue;
        }
#endif
#ifdef GL_ATI_shader_texture_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_texture_lod", 18))
        {
          ret = GLEW_ATI_shader_texture_lod;
          continue;
        }
#endif
#ifdef GL_ATI_text_fragment_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"text_fragment_shader", 20))
        {
          ret = GLEW_ATI_text_fragment_shader;
          continue;
        }
#endif
#ifdef GL_ATI_texture_compression_3dc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_3dc", 23))
        {
          ret = GLEW_ATI_texture_compression_3dc;
          continue;
        }
#endif
#ifdef GL_ATI_texture_env_combine3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine3", 20))
        {
          ret = GLEW_ATI_texture_env_combine3;
          continue;
        }
#endif
#ifdef GL_ATI_texture_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_float", 13))
        {
          ret = GLEW_ATI_texture_float;
          continue;
        }
#endif
#ifdef GL_ATI_texture_mirror_once
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirror_once", 19))
        {
          ret = GLEW_ATI_texture_mirror_once;
          continue;
        }
#endif
#ifdef GL_ATI_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_ATI_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_ATI_vertex_attrib_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_attrib_array_object", 26))
        {
          ret = GLEW_ATI_vertex_attrib_array_object;
          continue;
        }
#endif
#ifdef GL_ATI_vertex_streams
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_streams", 14))
        {
          ret = GLEW_ATI_vertex_streams;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef GL_EXT_422_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"422_pixels", 10))
        {
          ret = GLEW_EXT_422_pixels;
          continue;
        }
#endif
#ifdef GL_EXT_Cg_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"Cg_shader", 9))
        {
          ret = GLEW_EXT_Cg_shader;
          continue;
        }
#endif
#ifdef GL_EXT_abgr
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"abgr", 4))
        {
          ret = GLEW_EXT_abgr;
          continue;
        }
#endif
#ifdef GL_EXT_bgra
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"bgra", 4))
        {
          ret = GLEW_EXT_bgra;
          continue;
        }
#endif
#ifdef GL_EXT_bindable_uniform
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"bindable_uniform", 16))
        {
          ret = GLEW_EXT_bindable_uniform;
          continue;
        }
#endif
#ifdef GL_EXT_blend_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_color", 11))
        {
          ret = GLEW_EXT_blend_color;
          continue;
        }
#endif
#ifdef GL_EXT_blend_equation_separate
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_equation_separate", 23))
        {
          ret = GLEW_EXT_blend_equation_separate;
          continue;
        }
#endif
#ifdef GL_EXT_blend_func_separate
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_func_separate", 19))
        {
          ret = GLEW_EXT_blend_func_separate;
          continue;
        }
#endif
#ifdef GL_EXT_blend_logic_op
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_logic_op", 14))
        {
          ret = GLEW_EXT_blend_logic_op;
          continue;
        }
#endif
#ifdef GL_EXT_blend_minmax
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_minmax", 12))
        {
          ret = GLEW_EXT_blend_minmax;
          continue;
        }
#endif
#ifdef GL_EXT_blend_subtract
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_subtract", 14))
        {
          ret = GLEW_EXT_blend_subtract;
          continue;
        }
#endif
#ifdef GL_EXT_clip_volume_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"clip_volume_hint", 16))
        {
          ret = GLEW_EXT_clip_volume_hint;
          continue;
        }
#endif
#ifdef GL_EXT_cmyka
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cmyka", 5))
        {
          ret = GLEW_EXT_cmyka;
          continue;
        }
#endif
#ifdef GL_EXT_color_subtable
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_subtable", 14))
        {
          ret = GLEW_EXT_color_subtable;
          continue;
        }
#endif
#ifdef GL_EXT_compiled_vertex_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compiled_vertex_array", 21))
        {
          ret = GLEW_EXT_compiled_vertex_array;
          continue;
        }
#endif
#ifdef GL_EXT_convolution
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution", 11))
        {
          ret = GLEW_EXT_convolution;
          continue;
        }
#endif
#ifdef GL_EXT_coordinate_frame
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"coordinate_frame", 16))
        {
          ret = GLEW_EXT_coordinate_frame;
          continue;
        }
#endif
#ifdef GL_EXT_copy_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_texture", 12))
        {
          ret = GLEW_EXT_copy_texture;
          continue;
        }
#endif
#ifdef GL_EXT_cull_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cull_vertex", 11))
        {
          ret = GLEW_EXT_cull_vertex;
          continue;
        }
#endif
#ifdef GL_EXT_depth_bounds_test
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_bounds_test", 17))
        {
          ret = GLEW_EXT_depth_bounds_test;
          continue;
        }
#endif
#ifdef GL_EXT_direct_state_access
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"direct_state_access", 19))
        {
          ret = GLEW_EXT_direct_state_access;
          continue;
        }
#endif
#ifdef GL_EXT_draw_buffers2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_buffers2", 13))
        {
          ret = GLEW_EXT_draw_buffers2;
          continue;
        }
#endif
#ifdef GL_EXT_draw_instanced
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_instanced", 14))
        {
          ret = GLEW_EXT_draw_instanced;
          continue;
        }
#endif
#ifdef GL_EXT_draw_range_elements
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_range_elements", 19))
        {
          ret = GLEW_EXT_draw_range_elements;
          continue;
        }
#endif
#ifdef GL_EXT_fog_coord
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_coord", 9))
        {
          ret = GLEW_EXT_fog_coord;
          continue;
        }
#endif
#ifdef GL_EXT_fragment_lighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_lighting", 17))
        {
          ret = GLEW_EXT_fragment_lighting;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_blit
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_blit", 16))
        {
          ret = GLEW_EXT_framebuffer_blit;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample", 23))
        {
          ret = GLEW_EXT_framebuffer_multisample;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_multisample_blit_scaled
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample_blit_scaled", 35))
        {
          ret = GLEW_EXT_framebuffer_multisample_blit_scaled;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_object", 18))
        {
          ret = GLEW_EXT_framebuffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GL_EXT_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_EXT_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_EXT_gpu_program_parameters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_program_parameters", 22))
        {
          ret = GLEW_EXT_gpu_program_parameters;
          continue;
        }
#endif
#ifdef GL_EXT_gpu_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader4", 11))
        {
          ret = GLEW_EXT_gpu_shader4;
          continue;
        }
#endif
#ifdef GL_EXT_histogram
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"histogram", 9))
        {
          ret = GLEW_EXT_histogram;
          continue;
        }
#endif
#ifdef GL_EXT_index_array_formats
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_array_formats", 19))
        {
          ret = GLEW_EXT_index_array_formats;
          continue;
        }
#endif
#ifdef GL_EXT_index_func
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_func", 10))
        {
          ret = GLEW_EXT_index_func;
          continue;
        }
#endif
#ifdef GL_EXT_index_material
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_material", 14))
        {
          ret = GLEW_EXT_index_material;
          continue;
        }
#endif
#ifdef GL_EXT_index_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"index_texture", 13))
        {
          ret = GLEW_EXT_index_texture;
          continue;
        }
#endif
#ifdef GL_EXT_light_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"light_texture", 13))
        {
          ret = GLEW_EXT_light_texture;
          continue;
        }
#endif
#ifdef GL_EXT_misc_attribute
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"misc_attribute", 14))
        {
          ret = GLEW_EXT_misc_attribute;
          continue;
        }
#endif
#ifdef GL_EXT_multi_draw_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multi_draw_arrays", 17))
        {
          ret = GLEW_EXT_multi_draw_arrays;
          continue;
        }
#endif
#ifdef GL_EXT_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_EXT_multisample;
          continue;
        }
#endif
#ifdef GL_EXT_packed_depth_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_depth_stencil", 20))
        {
          ret = GLEW_EXT_packed_depth_stencil;
          continue;
        }
#endif
#ifdef GL_EXT_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_float", 12))
        {
          ret = GLEW_EXT_packed_float;
          continue;
        }
#endif
#ifdef GL_EXT_packed_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_pixels", 13))
        {
          ret = GLEW_EXT_packed_pixels;
          continue;
        }
#endif
#ifdef GL_EXT_paletted_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"paletted_texture", 16))
        {
          ret = GLEW_EXT_paletted_texture;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer_object", 19))
        {
          ret = GLEW_EXT_pixel_buffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_transform
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_transform", 15))
        {
          ret = GLEW_EXT_pixel_transform;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_transform_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_transform_color_table", 27))
        {
          ret = GLEW_EXT_pixel_transform_color_table;
          continue;
        }
#endif
#ifdef GL_EXT_point_parameters
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_parameters", 16))
        {
          ret = GLEW_EXT_point_parameters;
          continue;
        }
#endif
#ifdef GL_EXT_polygon_offset
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"polygon_offset", 14))
        {
          ret = GLEW_EXT_polygon_offset;
          continue;
        }
#endif
#ifdef GL_EXT_provoking_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"provoking_vertex", 16))
        {
          ret = GLEW_EXT_provoking_vertex;
          continue;
        }
#endif
#ifdef GL_EXT_rescale_normal
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"rescale_normal", 14))
        {
          ret = GLEW_EXT_rescale_normal;
          continue;
        }
#endif
#ifdef GL_EXT_scene_marker
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"scene_marker", 12))
        {
          ret = GLEW_EXT_scene_marker;
          continue;
        }
#endif
#ifdef GL_EXT_secondary_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"secondary_color", 15))
        {
          ret = GLEW_EXT_secondary_color;
          continue;
        }
#endif
#ifdef GL_EXT_separate_shader_objects
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"separate_shader_objects", 23))
        {
          ret = GLEW_EXT_separate_shader_objects;
          continue;
        }
#endif
#ifdef GL_EXT_separate_specular_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"separate_specular_color", 23))
        {
          ret = GLEW_EXT_separate_specular_color;
          continue;
        }
#endif
#ifdef GL_EXT_shader_image_load_store
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_image_load_store", 23))
        {
          ret = GLEW_EXT_shader_image_load_store;
          continue;
        }
#endif
#ifdef GL_EXT_shadow_funcs
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow_funcs", 12))
        {
          ret = GLEW_EXT_shadow_funcs;
          continue;
        }
#endif
#ifdef GL_EXT_shared_texture_palette
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shared_texture_palette", 22))
        {
          ret = GLEW_EXT_shared_texture_palette;
          continue;
        }
#endif
#ifdef GL_EXT_stencil_clear_tag
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stencil_clear_tag", 17))
        {
          ret = GLEW_EXT_stencil_clear_tag;
          continue;
        }
#endif
#ifdef GL_EXT_stencil_two_side
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stencil_two_side", 16))
        {
          ret = GLEW_EXT_stencil_two_side;
          continue;
        }
#endif
#ifdef GL_EXT_stencil_wrap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stencil_wrap", 12))
        {
          ret = GLEW_EXT_stencil_wrap;
          continue;
        }
#endif
#ifdef GL_EXT_subtexture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"subtexture", 10))
        {
          ret = GLEW_EXT_subtexture;
          continue;
        }
#endif
#ifdef GL_EXT_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture", 7))
        {
          ret = GLEW_EXT_texture;
          continue;
        }
#endif
#ifdef GL_EXT_texture3D
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture3D", 9))
        {
          ret = GLEW_EXT_texture3D;
          continue;
        }
#endif
#ifdef GL_EXT_texture_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_array", 13))
        {
          ret = GLEW_EXT_texture_array;
          continue;
        }
#endif
#ifdef GL_EXT_texture_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_buffer_object", 21))
        {
          ret = GLEW_EXT_texture_buffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_dxt1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_dxt1", 24))
        {
          ret = GLEW_EXT_texture_compression_dxt1;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_latc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_latc", 24))
        {
          ret = GLEW_EXT_texture_compression_latc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_rgtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_rgtc", 24))
        {
          ret = GLEW_EXT_texture_compression_rgtc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_s3tc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_s3tc", 24))
        {
          ret = GLEW_EXT_texture_compression_s3tc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_cube_map
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_cube_map", 16))
        {
          ret = GLEW_EXT_texture_cube_map;
          continue;
        }
#endif
#ifdef GL_EXT_texture_edge_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_edge_clamp", 18))
        {
          ret = GLEW_EXT_texture_edge_clamp;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env", 11))
        {
          ret = GLEW_EXT_texture_env;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env_add
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_add", 15))
        {
          ret = GLEW_EXT_texture_env_add;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env_combine
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine", 19))
        {
          ret = GLEW_EXT_texture_env_combine;
          continue;
        }
#endif
#ifdef GL_EXT_texture_env_dot3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_dot3", 16))
        {
          ret = GLEW_EXT_texture_env_dot3;
          continue;
        }
#endif
#ifdef GL_EXT_texture_filter_anisotropic
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_filter_anisotropic", 26))
        {
          ret = GLEW_EXT_texture_filter_anisotropic;
          continue;
        }
#endif
#ifdef GL_EXT_texture_integer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_integer", 15))
        {
          ret = GLEW_EXT_texture_integer;
          continue;
        }
#endif
#ifdef GL_EXT_texture_lod_bias
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lod_bias", 16))
        {
          ret = GLEW_EXT_texture_lod_bias;
          continue;
        }
#endif
#ifdef GL_EXT_texture_mirror_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirror_clamp", 20))
        {
          ret = GLEW_EXT_texture_mirror_clamp;
          continue;
        }
#endif
#ifdef GL_EXT_texture_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_object", 14))
        {
          ret = GLEW_EXT_texture_object;
          continue;
        }
#endif
#ifdef GL_EXT_texture_perturb_normal
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_perturb_normal", 22))
        {
          ret = GLEW_EXT_texture_perturb_normal;
          continue;
        }
#endif
#ifdef GL_EXT_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rectangle", 17))
        {
          ret = GLEW_EXT_texture_rectangle;
          continue;
        }
#endif
#ifdef GL_EXT_texture_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_sRGB", 12))
        {
          ret = GLEW_EXT_texture_sRGB;
          continue;
        }
#endif
#ifdef GL_EXT_texture_sRGB_decode
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_sRGB_decode", 19))
        {
          ret = GLEW_EXT_texture_sRGB_decode;
          continue;
        }
#endif
#ifdef GL_EXT_texture_shared_exponent
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shared_exponent", 23))
        {
          ret = GLEW_EXT_texture_shared_exponent;
          continue;
        }
#endif
#ifdef GL_EXT_texture_snorm
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_snorm", 13))
        {
          ret = GLEW_EXT_texture_snorm;
          continue;
        }
#endif
#ifdef GL_EXT_texture_swizzle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_swizzle", 15))
        {
          ret = GLEW_EXT_texture_swizzle;
          continue;
        }
#endif
#ifdef GL_EXT_timer_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"timer_query", 11))
        {
          ret = GLEW_EXT_timer_query;
          continue;
        }
#endif
#ifdef GL_EXT_transform_feedback
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback", 18))
        {
          ret = GLEW_EXT_transform_feedback;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array", 12))
        {
          ret = GLEW_EXT_vertex_array;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_array_bgra
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_bgra", 17))
        {
          ret = GLEW_EXT_vertex_array_bgra;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_attrib_64bit
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_attrib_64bit", 19))
        {
          ret = GLEW_EXT_vertex_attrib_64bit;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_shader", 13))
        {
          ret = GLEW_EXT_vertex_shader;
          continue;
        }
#endif
#ifdef GL_EXT_vertex_weighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_weighting", 16))
        {
          ret = GLEW_EXT_vertex_weighting;
          continue;
        }
#endif
#ifdef GL_EXT_x11_sync_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"x11_sync_object", 15))
        {
          ret = GLEW_EXT_x11_sync_object;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"GREMEDY_", 8))
      {
#ifdef GL_GREMEDY_frame_terminator
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"frame_terminator", 16))
        {
          ret = GLEW_GREMEDY_frame_terminator;
          continue;
        }
#endif
#ifdef GL_GREMEDY_string_marker
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"string_marker", 13))
        {
          ret = GLEW_GREMEDY_string_marker;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"HP_", 3))
      {
#ifdef GL_HP_convolution_border_modes
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution_border_modes", 24))
        {
          ret = GLEW_HP_convolution_border_modes;
          continue;
        }
#endif
#ifdef GL_HP_image_transform
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"image_transform", 15))
        {
          ret = GLEW_HP_image_transform;
          continue;
        }
#endif
#ifdef GL_HP_occlusion_test
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_test", 14))
        {
          ret = GLEW_HP_occlusion_test;
          continue;
        }
#endif
#ifdef GL_HP_texture_lighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lighting", 16))
        {
          ret = GLEW_HP_texture_lighting;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"IBM_", 4))
      {
#ifdef GL_IBM_cull_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cull_vertex", 11))
        {
          ret = GLEW_IBM_cull_vertex;
          continue;
        }
#endif
#ifdef GL_IBM_multimode_draw_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multimode_draw_arrays", 21))
        {
          ret = GLEW_IBM_multimode_draw_arrays;
          continue;
        }
#endif
#ifdef GL_IBM_rasterpos_clip
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"rasterpos_clip", 14))
        {
          ret = GLEW_IBM_rasterpos_clip;
          continue;
        }
#endif
#ifdef GL_IBM_static_data
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"static_data", 11))
        {
          ret = GLEW_IBM_static_data;
          continue;
        }
#endif
#ifdef GL_IBM_texture_mirrored_repeat
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_mirrored_repeat", 23))
        {
          ret = GLEW_IBM_texture_mirrored_repeat;
          continue;
        }
#endif
#ifdef GL_IBM_vertex_array_lists
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_lists", 18))
        {
          ret = GLEW_IBM_vertex_array_lists;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"INGR_", 5))
      {
#ifdef GL_INGR_color_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_clamp", 11))
        {
          ret = GLEW_INGR_color_clamp;
          continue;
        }
#endif
#ifdef GL_INGR_interlace_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"interlace_read", 14))
        {
          ret = GLEW_INGR_interlace_read;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"INTEL_", 6))
      {
#ifdef GL_INTEL_parallel_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parallel_arrays", 15))
        {
          ret = GLEW_INTEL_parallel_arrays;
          continue;
        }
#endif
#ifdef GL_INTEL_texture_scissor
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_scissor", 15))
        {
          ret = GLEW_INTEL_texture_scissor;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"KTX_", 4))
      {
#ifdef GL_KTX_buffer_region
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"buffer_region", 13))
        {
          ret = GLEW_KTX_buffer_region;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESAX_", 6))
      {
#ifdef GL_MESAX_texture_stack
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_stack", 13))
        {
          ret = GLEW_MESAX_texture_stack;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESA_", 5))
      {
#ifdef GL_MESA_pack_invert
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pack_invert", 11))
        {
          ret = GLEW_MESA_pack_invert;
          continue;
        }
#endif
#ifdef GL_MESA_resize_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"resize_buffers", 14))
        {
          ret = GLEW_MESA_resize_buffers;
          continue;
        }
#endif
#ifdef GL_MESA_window_pos
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"window_pos", 10))
        {
          ret = GLEW_MESA_window_pos;
          continue;
        }
#endif
#ifdef GL_MESA_ycbcr_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ycbcr_texture", 13))
        {
          ret = GLEW_MESA_ycbcr_texture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NVX_", 4))
      {
#ifdef GL_NVX_gpu_memory_info
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_memory_info", 15))
        {
          ret = GLEW_NVX_gpu_memory_info;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef GL_NV_blend_square
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_square", 12))
        {
          ret = GLEW_NV_blend_square;
          continue;
        }
#endif
#ifdef GL_NV_conditional_render
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conditional_render", 18))
        {
          ret = GLEW_NV_conditional_render;
          continue;
        }
#endif
#ifdef GL_NV_copy_depth_to_color
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_depth_to_color", 19))
        {
          ret = GLEW_NV_copy_depth_to_color;
          continue;
        }
#endif
#ifdef GL_NV_copy_image
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_image", 10))
        {
          ret = GLEW_NV_copy_image;
          continue;
        }
#endif
#ifdef GL_NV_depth_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_buffer_float", 18))
        {
          ret = GLEW_NV_depth_buffer_float;
          continue;
        }
#endif
#ifdef GL_NV_depth_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_clamp", 11))
        {
          ret = GLEW_NV_depth_clamp;
          continue;
        }
#endif
#ifdef GL_NV_depth_range_unclamped
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_range_unclamped", 21))
        {
          ret = GLEW_NV_depth_range_unclamped;
          continue;
        }
#endif
#ifdef GL_NV_evaluators
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"evaluators", 10))
        {
          ret = GLEW_NV_evaluators;
          continue;
        }
#endif
#ifdef GL_NV_explicit_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"explicit_multisample", 20))
        {
          ret = GLEW_NV_explicit_multisample;
          continue;
        }
#endif
#ifdef GL_NV_fence
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fence", 5))
        {
          ret = GLEW_NV_fence;
          continue;
        }
#endif
#ifdef GL_NV_float_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_buffer", 12))
        {
          ret = GLEW_NV_float_buffer;
          continue;
        }
#endif
#ifdef GL_NV_fog_distance
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_distance", 12))
        {
          ret = GLEW_NV_fog_distance;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program", 16))
        {
          ret = GLEW_NV_fragment_program;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program2", 17))
        {
          ret = GLEW_NV_fragment_program2;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program4", 17))
        {
          ret = GLEW_NV_fragment_program4;
          continue;
        }
#endif
#ifdef GL_NV_fragment_program_option
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_program_option", 23))
        {
          ret = GLEW_NV_fragment_program_option;
          continue;
        }
#endif
#ifdef GL_NV_framebuffer_multisample_coverage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample_coverage", 32))
        {
          ret = GLEW_NV_framebuffer_multisample_coverage;
          continue;
        }
#endif
#ifdef GL_NV_geometry_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_program4", 17))
        {
          ret = GLEW_NV_geometry_program4;
          continue;
        }
#endif
#ifdef GL_NV_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_NV_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_NV_gpu_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_program4", 12))
        {
          ret = GLEW_NV_gpu_program4;
          continue;
        }
#endif
#ifdef GL_NV_gpu_program5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_program5", 12))
        {
          ret = GLEW_NV_gpu_program5;
          continue;
        }
#endif
#ifdef GL_NV_gpu_program_fp64
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_program_fp64", 16))
        {
          ret = GLEW_NV_gpu_program_fp64;
          continue;
        }
#endif
#ifdef GL_NV_gpu_shader5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader5", 11))
        {
          ret = GLEW_NV_gpu_shader5;
          continue;
        }
#endif
#ifdef GL_NV_half_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float", 10))
        {
          ret = GLEW_NV_half_float;
          continue;
        }
#endif
#ifdef GL_NV_light_max_exponent
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"light_max_exponent", 18))
        {
          ret = GLEW_NV_light_max_exponent;
          continue;
        }
#endif
#ifdef GL_NV_multisample_coverage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample_coverage", 20))
        {
          ret = GLEW_NV_multisample_coverage;
          continue;
        }
#endif
#ifdef GL_NV_multisample_filter_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample_filter_hint", 23))
        {
          ret = GLEW_NV_multisample_filter_hint;
          continue;
        }
#endif
#ifdef GL_NV_occlusion_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"occlusion_query", 15))
        {
          ret = GLEW_NV_occlusion_query;
          continue;
        }
#endif
#ifdef GL_NV_packed_depth_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_depth_stencil", 20))
        {
          ret = GLEW_NV_packed_depth_stencil;
          continue;
        }
#endif
#ifdef GL_NV_parameter_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parameter_buffer_object", 23))
        {
          ret = GLEW_NV_parameter_buffer_object;
          continue;
        }
#endif
#ifdef GL_NV_parameter_buffer_object2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parameter_buffer_object2", 24))
        {
          ret = GLEW_NV_parameter_buffer_object2;
          continue;
        }
#endif
#ifdef GL_NV_path_rendering
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"path_rendering", 14))
        {
          ret = GLEW_NV_path_rendering;
          continue;
        }
#endif
#ifdef GL_NV_pixel_data_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_data_range", 16))
        {
          ret = GLEW_NV_pixel_data_range;
          continue;
        }
#endif
#ifdef GL_NV_point_sprite
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_sprite", 12))
        {
          ret = GLEW_NV_point_sprite;
          continue;
        }
#endif
#ifdef GL_NV_present_video
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"present_video", 13))
        {
          ret = GLEW_NV_present_video;
          continue;
        }
#endif
#ifdef GL_NV_primitive_restart
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"primitive_restart", 17))
        {
          ret = GLEW_NV_primitive_restart;
          continue;
        }
#endif
#ifdef GL_NV_register_combiners
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"register_combiners", 18))
        {
          ret = GLEW_NV_register_combiners;
          continue;
        }
#endif
#ifdef GL_NV_register_combiners2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"register_combiners2", 19))
        {
          ret = GLEW_NV_register_combiners2;
          continue;
        }
#endif
#ifdef GL_NV_shader_buffer_load
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_buffer_load", 18))
        {
          ret = GLEW_NV_shader_buffer_load;
          continue;
        }
#endif
#ifdef GL_NV_tessellation_program5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tessellation_program5", 21))
        {
          ret = GLEW_NV_tessellation_program5;
          continue;
        }
#endif
#ifdef GL_NV_texgen_emboss
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texgen_emboss", 13))
        {
          ret = GLEW_NV_texgen_emboss;
          continue;
        }
#endif
#ifdef GL_NV_texgen_reflection
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texgen_reflection", 17))
        {
          ret = GLEW_NV_texgen_reflection;
          continue;
        }
#endif
#ifdef GL_NV_texture_barrier
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_barrier", 15))
        {
          ret = GLEW_NV_texture_barrier;
          continue;
        }
#endif
#ifdef GL_NV_texture_compression_vtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_vtc", 23))
        {
          ret = GLEW_NV_texture_compression_vtc;
          continue;
        }
#endif
#ifdef GL_NV_texture_env_combine4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_env_combine4", 20))
        {
          ret = GLEW_NV_texture_env_combine4;
          continue;
        }
#endif
#ifdef GL_NV_texture_expand_normal
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_expand_normal", 21))
        {
          ret = GLEW_NV_texture_expand_normal;
          continue;
        }
#endif
#ifdef GL_NV_texture_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_multisample", 19))
        {
          ret = GLEW_NV_texture_multisample;
          continue;
        }
#endif
#ifdef GL_NV_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rectangle", 17))
        {
          ret = GLEW_NV_texture_rectangle;
          continue;
        }
#endif
#ifdef GL_NV_texture_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shader", 14))
        {
          ret = GLEW_NV_texture_shader;
          continue;
        }
#endif
#ifdef GL_NV_texture_shader2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shader2", 15))
        {
          ret = GLEW_NV_texture_shader2;
          continue;
        }
#endif
#ifdef GL_NV_texture_shader3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_shader3", 15))
        {
          ret = GLEW_NV_texture_shader3;
          continue;
        }
#endif
#ifdef GL_NV_transform_feedback
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback", 18))
        {
          ret = GLEW_NV_transform_feedback;
          continue;
        }
#endif
#ifdef GL_NV_transform_feedback2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"transform_feedback2", 19))
        {
          ret = GLEW_NV_transform_feedback2;
          continue;
        }
#endif
#ifdef GL_NV_vdpau_interop
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vdpau_interop", 13))
        {
          ret = GLEW_NV_vdpau_interop;
          continue;
        }
#endif
#ifdef GL_NV_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = GLEW_NV_vertex_array_range;
          continue;
        }
#endif
#ifdef GL_NV_vertex_array_range2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range2", 19))
        {
          ret = GLEW_NV_vertex_array_range2;
          continue;
        }
#endif
#ifdef GL_NV_vertex_attrib_integer_64bit
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_attrib_integer_64bit", 27))
        {
          ret = GLEW_NV_vertex_attrib_integer_64bit;
          continue;
        }
#endif
#ifdef GL_NV_vertex_buffer_unified_memory
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_buffer_unified_memory", 28))
        {
          ret = GLEW_NV_vertex_buffer_unified_memory;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program", 14))
        {
          ret = GLEW_NV_vertex_program;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program1_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program1_1", 17))
        {
          ret = GLEW_NV_vertex_program1_1;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program2", 15))
        {
          ret = GLEW_NV_vertex_program2;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program2_option
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program2_option", 22))
        {
          ret = GLEW_NV_vertex_program2_option;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program3", 15))
        {
          ret = GLEW_NV_vertex_program3;
          continue;
        }
#endif
#ifdef GL_NV_vertex_program4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_program4", 15))
        {
          ret = GLEW_NV_vertex_program4;
          continue;
        }
#endif
#ifdef GL_NV_video_capture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_capture", 13))
        {
          ret = GLEW_NV_video_capture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OES_", 4))
      {
#ifdef GL_OES_byte_coordinates
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"byte_coordinates", 16))
        {
          ret = GLEW_OES_byte_coordinates;
          continue;
        }
#endif
#ifdef GL_OES_compressed_paletted_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compressed_paletted_texture", 27))
        {
          ret = GLEW_OES_compressed_paletted_texture;
          continue;
        }
#endif
#ifdef GL_OES_read_format
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"read_format", 11))
        {
          ret = GLEW_OES_read_format;
          continue;
        }
#endif
#ifdef GL_OES_single_precision
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"single_precision", 16))
        {
          ret = GLEW_OES_single_precision;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OML_", 4))
      {
#ifdef GL_OML_interlace
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"interlace", 9))
        {
          ret = GLEW_OML_interlace;
          continue;
        }
#endif
#ifdef GL_OML_resample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"resample", 8))
        {
          ret = GLEW_OML_resample;
          continue;
        }
#endif
#ifdef GL_OML_subsample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"subsample", 9))
        {
          ret = GLEW_OML_subsample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"PGI_", 4))
      {
#ifdef GL_PGI_misc_hints
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"misc_hints", 10))
        {
          ret = GLEW_PGI_misc_hints;
          continue;
        }
#endif
#ifdef GL_PGI_vertex_hints
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_hints", 12))
        {
          ret = GLEW_PGI_vertex_hints;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"REND_", 5))
      {
#ifdef GL_REND_screen_coordinates
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"screen_coordinates", 18))
        {
          ret = GLEW_REND_screen_coordinates;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"S3_", 3))
      {
#ifdef GL_S3_s3tc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"s3tc", 4))
        {
          ret = GLEW_S3_s3tc;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIS_", 5))
      {
#ifdef GL_SGIS_color_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_range", 11))
        {
          ret = GLEW_SGIS_color_range;
          continue;
        }
#endif
#ifdef GL_SGIS_detail_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"detail_texture", 14))
        {
          ret = GLEW_SGIS_detail_texture;
          continue;
        }
#endif
#ifdef GL_SGIS_fog_function
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_function", 12))
        {
          ret = GLEW_SGIS_fog_function;
          continue;
        }
#endif
#ifdef GL_SGIS_generate_mipmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"generate_mipmap", 15))
        {
          ret = GLEW_SGIS_generate_mipmap;
          continue;
        }
#endif
#ifdef GL_SGIS_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLEW_SGIS_multisample;
          continue;
        }
#endif
#ifdef GL_SGIS_pixel_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_texture", 13))
        {
          ret = GLEW_SGIS_pixel_texture;
          continue;
        }
#endif
#ifdef GL_SGIS_point_line_texgen
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"point_line_texgen", 17))
        {
          ret = GLEW_SGIS_point_line_texgen;
          continue;
        }
#endif
#ifdef GL_SGIS_sharpen_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sharpen_texture", 15))
        {
          ret = GLEW_SGIS_sharpen_texture;
          continue;
        }
#endif
#ifdef GL_SGIS_texture4D
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture4D", 9))
        {
          ret = GLEW_SGIS_texture4D;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_border_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_border_clamp", 20))
        {
          ret = GLEW_SGIS_texture_border_clamp;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_edge_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_edge_clamp", 18))
        {
          ret = GLEW_SGIS_texture_edge_clamp;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_filter4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_filter4", 15))
        {
          ret = GLEW_SGIS_texture_filter4;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lod", 11))
        {
          ret = GLEW_SGIS_texture_lod;
          continue;
        }
#endif
#ifdef GL_SGIS_texture_select
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_select", 14))
        {
          ret = GLEW_SGIS_texture_select;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIX_", 5))
      {
#ifdef GL_SGIX_async
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"async", 5))
        {
          ret = GLEW_SGIX_async;
          continue;
        }
#endif
#ifdef GL_SGIX_async_histogram
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"async_histogram", 15))
        {
          ret = GLEW_SGIX_async_histogram;
          continue;
        }
#endif
#ifdef GL_SGIX_async_pixel
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"async_pixel", 11))
        {
          ret = GLEW_SGIX_async_pixel;
          continue;
        }
#endif
#ifdef GL_SGIX_blend_alpha_minmax
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blend_alpha_minmax", 18))
        {
          ret = GLEW_SGIX_blend_alpha_minmax;
          continue;
        }
#endif
#ifdef GL_SGIX_clipmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"clipmap", 7))
        {
          ret = GLEW_SGIX_clipmap;
          continue;
        }
#endif
#ifdef GL_SGIX_convolution_accuracy
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution_accuracy", 20))
        {
          ret = GLEW_SGIX_convolution_accuracy;
          continue;
        }
#endif
#ifdef GL_SGIX_depth_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_texture", 13))
        {
          ret = GLEW_SGIX_depth_texture;
          continue;
        }
#endif
#ifdef GL_SGIX_flush_raster
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"flush_raster", 12))
        {
          ret = GLEW_SGIX_flush_raster;
          continue;
        }
#endif
#ifdef GL_SGIX_fog_offset
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_offset", 10))
        {
          ret = GLEW_SGIX_fog_offset;
          continue;
        }
#endif
#ifdef GL_SGIX_fog_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fog_texture", 11))
        {
          ret = GLEW_SGIX_fog_texture;
          continue;
        }
#endif
#ifdef GL_SGIX_fragment_specular_lighting
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fragment_specular_lighting", 26))
        {
          ret = GLEW_SGIX_fragment_specular_lighting;
          continue;
        }
#endif
#ifdef GL_SGIX_framezoom
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framezoom", 9))
        {
          ret = GLEW_SGIX_framezoom;
          continue;
        }
#endif
#ifdef GL_SGIX_interlace
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"interlace", 9))
        {
          ret = GLEW_SGIX_interlace;
          continue;
        }
#endif
#ifdef GL_SGIX_ir_instrument1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ir_instrument1", 14))
        {
          ret = GLEW_SGIX_ir_instrument1;
          continue;
        }
#endif
#ifdef GL_SGIX_list_priority
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"list_priority", 13))
        {
          ret = GLEW_SGIX_list_priority;
          continue;
        }
#endif
#ifdef GL_SGIX_pixel_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_texture", 13))
        {
          ret = GLEW_SGIX_pixel_texture;
          continue;
        }
#endif
#ifdef GL_SGIX_pixel_texture_bits
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_texture_bits", 18))
        {
          ret = GLEW_SGIX_pixel_texture_bits;
          continue;
        }
#endif
#ifdef GL_SGIX_reference_plane
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"reference_plane", 15))
        {
          ret = GLEW_SGIX_reference_plane;
          continue;
        }
#endif
#ifdef GL_SGIX_resample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"resample", 8))
        {
          ret = GLEW_SGIX_resample;
          continue;
        }
#endif
#ifdef GL_SGIX_shadow
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow", 6))
        {
          ret = GLEW_SGIX_shadow;
          continue;
        }
#endif
#ifdef GL_SGIX_shadow_ambient
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shadow_ambient", 14))
        {
          ret = GLEW_SGIX_shadow_ambient;
          continue;
        }
#endif
#ifdef GL_SGIX_sprite
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sprite", 6))
        {
          ret = GLEW_SGIX_sprite;
          continue;
        }
#endif
#ifdef GL_SGIX_tag_sample_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tag_sample_buffer", 17))
        {
          ret = GLEW_SGIX_tag_sample_buffer;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_add_env
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_add_env", 15))
        {
          ret = GLEW_SGIX_texture_add_env;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_coordinate_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_coordinate_clamp", 24))
        {
          ret = GLEW_SGIX_texture_coordinate_clamp;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_lod_bias
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_lod_bias", 16))
        {
          ret = GLEW_SGIX_texture_lod_bias;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_multi_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_multi_buffer", 20))
        {
          ret = GLEW_SGIX_texture_multi_buffer;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_range", 13))
        {
          ret = GLEW_SGIX_texture_range;
          continue;
        }
#endif
#ifdef GL_SGIX_texture_scale_bias
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_scale_bias", 18))
        {
          ret = GLEW_SGIX_texture_scale_bias;
          continue;
        }
#endif
#ifdef GL_SGIX_vertex_preclip
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_preclip", 14))
        {
          ret = GLEW_SGIX_vertex_preclip;
          continue;
        }
#endif
#ifdef GL_SGIX_vertex_preclip_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_preclip_hint", 19))
        {
          ret = GLEW_SGIX_vertex_preclip_hint;
          continue;
        }
#endif
#ifdef GL_SGIX_ycrcb
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"ycrcb", 5))
        {
          ret = GLEW_SGIX_ycrcb;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGI_", 4))
      {
#ifdef GL_SGI_color_matrix
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_matrix", 12))
        {
          ret = GLEW_SGI_color_matrix;
          continue;
        }
#endif
#ifdef GL_SGI_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_table", 11))
        {
          ret = GLEW_SGI_color_table;
          continue;
        }
#endif
#ifdef GL_SGI_texture_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_color_table", 19))
        {
          ret = GLEW_SGI_texture_color_table;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SUNX_", 5))
      {
#ifdef GL_SUNX_constant_data
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"constant_data", 13))
        {
          ret = GLEW_SUNX_constant_data;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SUN_", 4))
      {
#ifdef GL_SUN_convolution_border_modes
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"convolution_border_modes", 24))
        {
          ret = GLEW_SUN_convolution_border_modes;
          continue;
        }
#endif
#ifdef GL_SUN_global_alpha
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"global_alpha", 12))
        {
          ret = GLEW_SUN_global_alpha;
          continue;
        }
#endif
#ifdef GL_SUN_mesh_array
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"mesh_array", 10))
        {
          ret = GLEW_SUN_mesh_array;
          continue;
        }
#endif
#ifdef GL_SUN_read_video_pixels
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"read_video_pixels", 17))
        {
          ret = GLEW_SUN_read_video_pixels;
          continue;
        }
#endif
#ifdef GL_SUN_slice_accum
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"slice_accum", 11))
        {
          ret = GLEW_SUN_slice_accum;
          continue;
        }
#endif
#ifdef GL_SUN_triangle_list
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"triangle_list", 13))
        {
          ret = GLEW_SUN_triangle_list;
          continue;
        }
#endif
#ifdef GL_SUN_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex", 6))
        {
          ret = GLEW_SUN_vertex;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"WIN_", 4))
      {
#ifdef GL_WIN_phong_shading
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"phong_shading", 13))
        {
          ret = GLEW_WIN_phong_shading;
          continue;
        }
#endif
#ifdef GL_WIN_specular_fog
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"specular_fog", 12))
        {
          ret = GLEW_WIN_specular_fog;
          continue;
        }
#endif
#ifdef GL_WIN_swap_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_hint", 9))
        {
          ret = GLEW_WIN_swap_hint;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#if defined(_WIN32)

#if defined(GLEW_MX)
GLboolean wglewContextIsSupported (const WGLEWContext* ctx, const char* name)
#else
GLboolean wglewIsSupported (const char* name)
#endif
{
  GLubyte* pos = (GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"WGL_", 4))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DFX_", 5))
      {
#ifdef WGL_3DFX_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = WGLEW_3DFX_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DL_", 4))
      {
#ifdef WGL_3DL_stereo_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"stereo_control", 14))
        {
          ret = WGLEW_3DL_stereo_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"AMD_", 4))
      {
#ifdef WGL_AMD_gpu_association
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_association", 15))
        {
          ret = WGLEW_AMD_gpu_association;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef WGL_ARB_buffer_region
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"buffer_region", 13))
        {
          ret = WGLEW_ARB_buffer_region;
          continue;
        }
#endif
#ifdef WGL_ARB_create_context
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context", 14))
        {
          ret = WGLEW_ARB_create_context;
          continue;
        }
#endif
#ifdef WGL_ARB_create_context_profile
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context_profile", 22))
        {
          ret = WGLEW_ARB_create_context_profile;
          continue;
        }
#endif
#ifdef WGL_ARB_create_context_robustness
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context_robustness", 25))
        {
          ret = WGLEW_ARB_create_context_robustness;
          continue;
        }
#endif
#ifdef WGL_ARB_extensions_string
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"extensions_string", 17))
        {
          ret = WGLEW_ARB_extensions_string;
          continue;
        }
#endif
#ifdef WGL_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = WGLEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef WGL_ARB_make_current_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"make_current_read", 17))
        {
          ret = WGLEW_ARB_make_current_read;
          continue;
        }
#endif
#ifdef WGL_ARB_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = WGLEW_ARB_multisample;
          continue;
        }
#endif
#ifdef WGL_ARB_pbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pbuffer", 7))
        {
          ret = WGLEW_ARB_pbuffer;
          continue;
        }
#endif
#ifdef WGL_ARB_pixel_format
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format", 12))
        {
          ret = WGLEW_ARB_pixel_format;
          continue;
        }
#endif
#ifdef WGL_ARB_pixel_format_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_float", 18))
        {
          ret = WGLEW_ARB_pixel_format_float;
          continue;
        }
#endif
#ifdef WGL_ARB_render_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture", 14))
        {
          ret = WGLEW_ARB_render_texture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATI_", 4))
      {
#ifdef WGL_ATI_pixel_format_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_float", 18))
        {
          ret = WGLEW_ATI_pixel_format_float;
          continue;
        }
#endif
#ifdef WGL_ATI_render_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture_rectangle", 24))
        {
          ret = WGLEW_ATI_render_texture_rectangle;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef WGL_EXT_create_context_es2_profile
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context_es2_profile", 26))
        {
          ret = WGLEW_EXT_create_context_es2_profile;
          continue;
        }
#endif
#ifdef WGL_EXT_depth_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_float", 11))
        {
          ret = WGLEW_EXT_depth_float;
          continue;
        }
#endif
#ifdef WGL_EXT_display_color_table
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"display_color_table", 19))
        {
          ret = WGLEW_EXT_display_color_table;
          continue;
        }
#endif
#ifdef WGL_EXT_extensions_string
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"extensions_string", 17))
        {
          ret = WGLEW_EXT_extensions_string;
          continue;
        }
#endif
#ifdef WGL_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = WGLEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef WGL_EXT_make_current_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"make_current_read", 17))
        {
          ret = WGLEW_EXT_make_current_read;
          continue;
        }
#endif
#ifdef WGL_EXT_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = WGLEW_EXT_multisample;
          continue;
        }
#endif
#ifdef WGL_EXT_pbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pbuffer", 7))
        {
          ret = WGLEW_EXT_pbuffer;
          continue;
        }
#endif
#ifdef WGL_EXT_pixel_format
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format", 12))
        {
          ret = WGLEW_EXT_pixel_format;
          continue;
        }
#endif
#ifdef WGL_EXT_pixel_format_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_packed_float", 25))
        {
          ret = WGLEW_EXT_pixel_format_packed_float;
          continue;
        }
#endif
#ifdef WGL_EXT_swap_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_control", 12))
        {
          ret = WGLEW_EXT_swap_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"I3D_", 4))
      {
#ifdef WGL_I3D_digital_video_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"digital_video_control", 21))
        {
          ret = WGLEW_I3D_digital_video_control;
          continue;
        }
#endif
#ifdef WGL_I3D_gamma
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gamma", 5))
        {
          ret = WGLEW_I3D_gamma;
          continue;
        }
#endif
#ifdef WGL_I3D_genlock
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"genlock", 7))
        {
          ret = WGLEW_I3D_genlock;
          continue;
        }
#endif
#ifdef WGL_I3D_image_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"image_buffer", 12))
        {
          ret = WGLEW_I3D_image_buffer;
          continue;
        }
#endif
#ifdef WGL_I3D_swap_frame_lock
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_frame_lock", 15))
        {
          ret = WGLEW_I3D_swap_frame_lock;
          continue;
        }
#endif
#ifdef WGL_I3D_swap_frame_usage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_frame_usage", 16))
        {
          ret = WGLEW_I3D_swap_frame_usage;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef WGL_NV_DX_interop
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"DX_interop", 10))
        {
          ret = WGLEW_NV_DX_interop;
          continue;
        }
#endif
#ifdef WGL_NV_copy_image
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_image", 10))
        {
          ret = WGLEW_NV_copy_image;
          continue;
        }
#endif
#ifdef WGL_NV_float_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_buffer", 12))
        {
          ret = WGLEW_NV_float_buffer;
          continue;
        }
#endif
#ifdef WGL_NV_gpu_affinity
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_affinity", 12))
        {
          ret = WGLEW_NV_gpu_affinity;
          continue;
        }
#endif
#ifdef WGL_NV_multisample_coverage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample_coverage", 20))
        {
          ret = WGLEW_NV_multisample_coverage;
          continue;
        }
#endif
#ifdef WGL_NV_present_video
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"present_video", 13))
        {
          ret = WGLEW_NV_present_video;
          continue;
        }
#endif
#ifdef WGL_NV_render_depth_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_depth_texture", 20))
        {
          ret = WGLEW_NV_render_depth_texture;
          continue;
        }
#endif
#ifdef WGL_NV_render_texture_rectangle
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture_rectangle", 24))
        {
          ret = WGLEW_NV_render_texture_rectangle;
          continue;
        }
#endif
#ifdef WGL_NV_swap_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_group", 10))
        {
          ret = WGLEW_NV_swap_group;
          continue;
        }
#endif
#ifdef WGL_NV_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = WGLEW_NV_vertex_array_range;
          continue;
        }
#endif
#ifdef WGL_NV_video_capture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_capture", 13))
        {
          ret = WGLEW_NV_video_capture;
          continue;
        }
#endif
#ifdef WGL_NV_video_output
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_output", 12))
        {
          ret = WGLEW_NV_video_output;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OML_", 4))
      {
#ifdef WGL_OML_sync_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sync_control", 12))
        {
          ret = WGLEW_OML_sync_control;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX)

#if defined(GLEW_MX)
GLboolean glxewContextIsSupported (const GLXEWContext* ctx, const char* name)
#else
GLboolean glxewIsSupported (const char* name)
#endif
{
  GLubyte* pos = (GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if(_glewStrSame1(&pos, &len, (const GLubyte*)"GLX_", 4))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"VERSION_", 8))
      {
#ifdef GLX_VERSION_1_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2", 3))
        {
          ret = GLXEW_VERSION_1_2;
          continue;
        }
#endif
#ifdef GLX_VERSION_1_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_3", 3))
        {
          ret = GLXEW_VERSION_1_3;
          continue;
        }
#endif
#ifdef GLX_VERSION_1_4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_4", 3))
        {
          ret = GLXEW_VERSION_1_4;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"3DFX_", 5))
      {
#ifdef GLX_3DFX_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLXEW_3DFX_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"AMD_", 4))
      {
#ifdef GLX_AMD_gpu_association
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_association", 15))
        {
          ret = GLXEW_AMD_gpu_association;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef GLX_ARB_create_context
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context", 14))
        {
          ret = GLXEW_ARB_create_context;
          continue;
        }
#endif
#ifdef GLX_ARB_create_context_profile
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context_profile", 22))
        {
          ret = GLXEW_ARB_create_context_profile;
          continue;
        }
#endif
#ifdef GLX_ARB_create_context_robustness
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context_robustness", 25))
        {
          ret = GLXEW_ARB_create_context_robustness;
          continue;
        }
#endif
#ifdef GLX_ARB_fbconfig_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fbconfig_float", 14))
        {
          ret = GLXEW_ARB_fbconfig_float;
          continue;
        }
#endif
#ifdef GLX_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLXEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GLX_ARB_get_proc_address
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"get_proc_address", 16))
        {
          ret = GLXEW_ARB_get_proc_address;
          continue;
        }
#endif
#ifdef GLX_ARB_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLXEW_ARB_multisample;
          continue;
        }
#endif
#ifdef GLX_ARB_vertex_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_buffer_object", 20))
        {
          ret = GLXEW_ARB_vertex_buffer_object;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ATI_", 4))
      {
#ifdef GLX_ATI_pixel_format_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_format_float", 18))
        {
          ret = GLXEW_ATI_pixel_format_float;
          continue;
        }
#endif
#ifdef GLX_ATI_render_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"render_texture", 14))
        {
          ret = GLXEW_ATI_render_texture;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef GLX_EXT_create_context_es2_profile
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"create_context_es2_profile", 26))
        {
          ret = GLXEW_EXT_create_context_es2_profile;
          continue;
        }
#endif
#ifdef GLX_EXT_fbconfig_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fbconfig_packed_float", 21))
        {
          ret = GLXEW_EXT_fbconfig_packed_float;
          continue;
        }
#endif
#ifdef GLX_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLXEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GLX_EXT_import_context
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"import_context", 14))
        {
          ret = GLXEW_EXT_import_context;
          continue;
        }
#endif
#ifdef GLX_EXT_scene_marker
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"scene_marker", 12))
        {
          ret = GLXEW_EXT_scene_marker;
          continue;
        }
#endif
#ifdef GLX_EXT_swap_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_control", 12))
        {
          ret = GLXEW_EXT_swap_control;
          continue;
        }
#endif
#ifdef GLX_EXT_texture_from_pixmap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_from_pixmap", 19))
        {
          ret = GLXEW_EXT_texture_from_pixmap;
          continue;
        }
#endif
#ifdef GLX_EXT_visual_info
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"visual_info", 11))
        {
          ret = GLXEW_EXT_visual_info;
          continue;
        }
#endif
#ifdef GLX_EXT_visual_rating
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"visual_rating", 13))
        {
          ret = GLXEW_EXT_visual_rating;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"INTEL_", 6))
      {
#ifdef GLX_INTEL_swap_event
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_event", 10))
        {
          ret = GLXEW_INTEL_swap_event;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESA_", 5))
      {
#ifdef GLX_MESA_agp_offset
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"agp_offset", 10))
        {
          ret = GLXEW_MESA_agp_offset;
          continue;
        }
#endif
#ifdef GLX_MESA_copy_sub_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_sub_buffer", 15))
        {
          ret = GLXEW_MESA_copy_sub_buffer;
          continue;
        }
#endif
#ifdef GLX_MESA_pixmap_colormap
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixmap_colormap", 15))
        {
          ret = GLXEW_MESA_pixmap_colormap;
          continue;
        }
#endif
#ifdef GLX_MESA_release_buffers
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"release_buffers", 15))
        {
          ret = GLXEW_MESA_release_buffers;
          continue;
        }
#endif
#ifdef GLX_MESA_set_3dfx_mode
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"set_3dfx_mode", 13))
        {
          ret = GLXEW_MESA_set_3dfx_mode;
          continue;
        }
#endif
#ifdef GLX_MESA_swap_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_control", 12))
        {
          ret = GLXEW_MESA_swap_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef GLX_NV_copy_image
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_image", 10))
        {
          ret = GLXEW_NV_copy_image;
          continue;
        }
#endif
#ifdef GLX_NV_float_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"float_buffer", 12))
        {
          ret = GLXEW_NV_float_buffer;
          continue;
        }
#endif
#ifdef GLX_NV_multisample_coverage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample_coverage", 20))
        {
          ret = GLXEW_NV_multisample_coverage;
          continue;
        }
#endif
#ifdef GLX_NV_present_video
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"present_video", 13))
        {
          ret = GLXEW_NV_present_video;
          continue;
        }
#endif
#ifdef GLX_NV_swap_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_group", 10))
        {
          ret = GLXEW_NV_swap_group;
          continue;
        }
#endif
#ifdef GLX_NV_vertex_array_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_range", 18))
        {
          ret = GLXEW_NV_vertex_array_range;
          continue;
        }
#endif
#ifdef GLX_NV_video_capture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_capture", 13))
        {
          ret = GLXEW_NV_video_capture;
          continue;
        }
#endif
#ifdef GLX_NV_video_output
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_output", 12))
        {
          ret = GLXEW_NV_video_output;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"OML_", 4))
      {
#ifdef GLX_OML_swap_method
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_method", 11))
        {
          ret = GLXEW_OML_swap_method;
          continue;
        }
#endif
#ifdef GLX_OML_sync_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sync_control", 12))
        {
          ret = GLXEW_OML_sync_control;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIS_", 5))
      {
#ifdef GLX_SGIS_blended_overlay
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"blended_overlay", 15))
        {
          ret = GLXEW_SGIS_blended_overlay;
          continue;
        }
#endif
#ifdef GLX_SGIS_color_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"color_range", 11))
        {
          ret = GLXEW_SGIS_color_range;
          continue;
        }
#endif
#ifdef GLX_SGIS_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample", 11))
        {
          ret = GLXEW_SGIS_multisample;
          continue;
        }
#endif
#ifdef GLX_SGIS_shared_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shared_multisample", 18))
        {
          ret = GLXEW_SGIS_shared_multisample;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGIX_", 5))
      {
#ifdef GLX_SGIX_fbconfig
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"fbconfig", 8))
        {
          ret = GLXEW_SGIX_fbconfig;
          continue;
        }
#endif
#ifdef GLX_SGIX_hyperpipe
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"hyperpipe", 9))
        {
          ret = GLXEW_SGIX_hyperpipe;
          continue;
        }
#endif
#ifdef GLX_SGIX_pbuffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pbuffer", 7))
        {
          ret = GLXEW_SGIX_pbuffer;
          continue;
        }
#endif
#ifdef GLX_SGIX_swap_barrier
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_barrier", 12))
        {
          ret = GLXEW_SGIX_swap_barrier;
          continue;
        }
#endif
#ifdef GLX_SGIX_swap_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_group", 10))
        {
          ret = GLXEW_SGIX_swap_group;
          continue;
        }
#endif
#ifdef GLX_SGIX_video_resize
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_resize", 12))
        {
          ret = GLXEW_SGIX_video_resize;
          continue;
        }
#endif
#ifdef GLX_SGIX_visual_select_group
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"visual_select_group", 19))
        {
          ret = GLXEW_SGIX_visual_select_group;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SGI_", 4))
      {
#ifdef GLX_SGI_cushion
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"cushion", 7))
        {
          ret = GLXEW_SGI_cushion;
          continue;
        }
#endif
#ifdef GLX_SGI_make_current_read
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"make_current_read", 17))
        {
          ret = GLXEW_SGI_make_current_read;
          continue;
        }
#endif
#ifdef GLX_SGI_swap_control
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"swap_control", 12))
        {
          ret = GLXEW_SGI_swap_control;
          continue;
        }
#endif
#ifdef GLX_SGI_video_sync
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_sync", 10))
        {
          ret = GLXEW_SGI_video_sync;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"SUN_", 4))
      {
#ifdef GLX_SUN_get_transparent_index
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"get_transparent_index", 21))
        {
          ret = GLXEW_SUN_get_transparent_index;
          continue;
        }
#endif
#ifdef GLX_SUN_video_resize
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"video_resize", 12))
        {
          ret = GLXEW_SUN_video_resize;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#endif /* _WIN32 */
