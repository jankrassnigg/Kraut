#ifndef AE_GRAPHICS_SHADERMANAGER_SHADERPORT_H
#define AE_GRAPHICS_SHADERMANAGER_SHADERPORT_H

#include "Declarations.h"

namespace AE_NS_GRAPHICS
{
	enum AE_VAR_TYPES
	{
		AE_FLOAT,
		AE_FLOAT2,
		AE_FLOAT3,
		AE_FLOAT4,
		AE_FLOAT2X2,
		AE_FLOAT3X3,
		AE_FLOAT4X4,
		AE_INSTANCE_ID,
		AE_VERTEX_POSITION,
		AE_FRAGMENT_POSITION,
		AE_INT,
		AE_INT2,
		AE_INT3,
		AE_INT4,
	};

	enum AE_TEXTURE_TYPES
	{
		AE_TEX_2D,
		AE_TEX_3D,
		AE_TEX_CUBE,
	};

	enum AE_VARYING_MODIFIERS
	{
		AE_VM_DEFAULT,
		AE_VM_FLAT,
	};

	struct aeAttribute
	{
		AE_VAR_TYPES m_Type;
		aeString m_sSemantic;
	};

	struct aeVarying
	{
		AE_VAR_TYPES m_Type;
		aeString m_sSemantic;
		AE_VARYING_MODIFIERS m_Modifier;
	};

	struct aeTarget
	{
		AE_VAR_TYPES m_Type;
		aeString m_sSemantic;
		aeUInt8 m_uiRenderTarget;
	};

	struct aeTexture
	{
		AE_TEXTURE_TYPES m_Type;
		aeString m_sName;
	};

	void InitPortScript (void);
	aeString getTextureTypeName (AE_TEXTURE_TYPES type);
	aeString getVariableReplacement (AE_VAR_TYPES var);
	bool FindPortFunction (const aeString& sCode, const aeString& sFunction, aeString& sPrev, aeString& sPost, aeString& sResult, bool bEatSemicolon);
	bool FindKeyword (const aeString& sCode, const aeString& sKeyword, aeString& sPrev, aeString& sPost);
	aeString ReplaceKeyword (const aeString& sCode, const aeString& sKeyword, const aeString& sReplacement);
	aeString ReplaceKeyword2 (const aeString& sCode, const aeString& sKeyword, const aeString& sReplacement);
}

#endif

