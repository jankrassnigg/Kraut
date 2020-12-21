#ifndef AE_GRAPHICS_RENDERAPI_DECLARATIONSOGL_H
#define AE_GRAPHICS_RENDERAPI_DECLARATIONSOGL_H

// Include this as the first Header everywhere
#include "../Base.h"
#include <KrautFoundation/Math/Declarations.h>
#include <KrautFoundation/Basics/Checks.h>

#ifdef AE_RENDERAPI_OPENGL

#include "../Glew/glew.h"

namespace AE_NS_GRAPHICS
{
	inline GLenum mapIndexTypeOGL (AE_INDEX_TYPE t)
	{
		switch (t)
		{
		case AE_UNSIGNED_SHORT:
			return (GL_UNSIGNED_SHORT);
		case AE_UNSIGNED_INT:
			return (GL_UNSIGNED_INT);
		case AE_UNKNOWN:
		default:
			return (0);
		}
	}

	inline GLenum mapRenderModeOGL (AE_RENDER_MODE m)
	{
		switch (m)
		{
		case AE_RENDER_POINTS:
			return (GL_POINTS);
		case AE_RENDER_LINES:
			return (GL_LINES);
		case AE_RENDER_TRIANGLES:
			return (GL_TRIANGLES);
		case AE_RENDER_UNKNOWN:
		default:
			return (0);
		}
	}

	inline GLenum mapCompareFuncOGL (AE_COMPARE_FUNC m)
	{
		switch (m)
		{
		case AE_COMPARE_NEVER:
			return (GL_NEVER);
		case AE_COMPARE_LESS:
			return (GL_LESS);
		case AE_COMPARE_EQUAL:
			return (GL_EQUAL);
		case AE_COMPARE_LEQUAL:
			return (GL_LEQUAL);
		case AE_COMPARE_GREATER:
			return (GL_GREATER);
		case AE_COMPARE_NOTEQUAL:
			return (GL_NOTEQUAL);
		case AE_COMPARE_GEQUAL:
			return (GL_GEQUAL);
		case AE_COMPARE_ALWAYS:
			return (GL_ALWAYS);
		}

		AE_CHECK_DEV (false, "mapCompareFuncOGL: Wrong Control Path");
		return (GL_ALWAYS);
	}

	inline GLenum mapBlendFuncOGL (AE_BLEND_FUNC m)
	{
		switch (m)
		{
		case AE_FUNC_ADD:
			return (GL_FUNC_ADD);
		case AE_FUNC_SUBTRACT:
			return (GL_FUNC_SUBTRACT);
		case AE_FUNC_REV_SUBTRACT:
			return (GL_FUNC_REVERSE_SUBTRACT);
		case AE_FUNC_MIN:
			return (GL_MIN);
		case AE_FUNC_MAX:
			return (GL_MAX);
		}

		AE_CHECK_DEV (false, "mapBlendFuncOGL: Wrong Control Path");
		return (GL_FUNC_ADD);
	}

	inline GLenum mapBlendFactorOGL (AE_BLEND_FACTOR m)
	{
		switch (m)
		{
		case AE_FACTOR_ZERO:
			return (GL_ZERO);
		case AE_FACTOR_ONE:
			return (GL_ONE);
		case AE_FACTOR_SRC_COLOR:
			return (GL_SRC_COLOR);
		case AE_FACTOR_INV_SRC_COLOR:
			return (GL_ONE_MINUS_SRC_COLOR);
		case AE_FACTOR_SRC_ALPHA:
			return (GL_SRC_ALPHA);
		case AE_FACTOR_INV_SRC_ALPHA:
			return (GL_ONE_MINUS_SRC_ALPHA);
		case AE_FACTOR_DST_COLOR:
			return (GL_DST_COLOR);
		case AE_FACTOR_INV_DST_COLOR:
			return (GL_ONE_MINUS_DST_COLOR);
		case AE_FACTOR_DST_ALPHA:
			return (GL_DST_ALPHA);
		case AE_FACTOR_INV_DST_ALPHA:
			return (GL_ONE_MINUS_DST_ALPHA);
		case AE_FACTOR_BLEND_COLOR:
			return (GL_CONSTANT_COLOR);
		case AE_FACTOR_INV_BLEND_COLOR:
			return (GL_ONE_MINUS_CONSTANT_COLOR);
		}

		AE_CHECK_DEV (false, "mapBlendFactorOGL: Wrong Control Path");
		return (GL_ZERO);
	}

	inline GLenum mapStencilOpOGL (AE_STENCIL_OP m)
	{
		switch (m)
		{
		case AE_STENCIL_KEEP:
			return (GL_KEEP);
		case AE_STENCIL_ZERO:
			return (GL_ZERO);
		case AE_STENCIL_REPLACE:
			return (GL_REPLACE);
		case AE_STENCIL_INCR_SAT:
			return (GL_INCR);
		case AE_STENCIL_DECR_SAT:
			return (GL_DECR);
		case AE_STENCIL_INCR_WRAP:
			return (GL_INCR_WRAP);
		case AE_STENCIL_DECR_WRAP:
			return (GL_DECR_WRAP);
		case AE_STENCIL_INVERT:
			return (GL_INVERT);		
		}

		AE_CHECK_DEV (false, "mapStencilOpOGL: Wrong Control Path");
		return (GL_KEEP);
	}


}

#endif

#endif


