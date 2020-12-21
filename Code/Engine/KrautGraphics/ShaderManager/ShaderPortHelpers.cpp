#include "Main.h"
#include <KrautFoundation/Utility/LuaWrapper.h>
#include <deque>
#include "ShaderPort.h"
#include <KrautGraphics/RenderAPI/Declarations.h>



namespace AE_NS_GRAPHICS
{
	aeString getTextureTypeName (AE_TEXTURE_TYPES type)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			switch (type)
			{
			case AE_TEX_2D:
				return ("uniform sampler2D");
			case AE_TEX_3D:
				return ("uniform sampler3D");
			case AE_TEX_CUBE:
				return ("uniform samplerCube");
			}
		}
#endif

		return (" ### unknown texture type ### ");
	}


	aeString getVariableReplacement (AE_VAR_TYPES var)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			switch (var)
			{
			case AE_VERTEX_POSITION:
				return ("vec3");
			case AE_FLOAT:
				return ("float");
			case AE_FLOAT2:
				return ("vec2");
			case AE_FLOAT3:
				return ("vec3");
			case AE_FLOAT4:
				return ("vec4");

			case AE_INT:
				return ("int");
			case AE_INT2:
				return ("ivec2");
			case AE_INT3:
				return ("ivec3");
			case AE_INT4:
				return ("ivec4");

			case AE_FLOAT2X2:
				return ("mat2");
			case AE_FLOAT3X3:
				return ("mat3");
			case AE_FLOAT4X4:
				return ("mat4");
			case AE_INSTANCE_ID:
				return ("uint");
			}
		}
#endif

		return (" ### unknown variable type ### ");
	}

	bool FindKeyword (const aeString& sCode, const aeString& sKeyword, aeString& sPrev, aeString& sPost)
	{
		aeInt32 iPos = aeStringFunctions::FindFirstStringPos (sCode.c_str (), sKeyword.c_str ());

		if (iPos < 0)
			return (false);

		aeUInt32 uiLen = sKeyword.length ();

		sPrev = aeString (sCode.c_str (), iPos);
		sPost = aeString (&(sCode.c_str ()[iPos + uiLen]));

		return (true);
	}


	bool FindKeywordAndArrayIndex (const aeString& sCode, const aeString& sKeyword, aeString& sPrev, aeString& sPost, aeString& sArrayIndex)
	{
		sArrayIndex = "";
		aeInt32 iPos = aeStringFunctions::FindFirstStringPos (sCode.c_str (), sKeyword.c_str ());

		if (iPos < 0)
			return (false);

		aeUInt32 uiLen = sKeyword.length ();

		sPrev = aeString (sCode.c_str (), iPos);
		sPost = aeString (&(sCode.c_str ()[iPos + uiLen]));

		if (sPost[0] == '[')
		{
			int iPos = 1;
			int iBrackets = 1;
			while (iBrackets > 0)
			{
				if (sPost[iPos] == '[')
					++iBrackets;
				if (sPost[iPos] == ']')
					--iBrackets;
				++iPos;
			}
			//sArrayIndex.assign (sPost.c_str (), iPos);
      sArrayIndex = &sPost.c_str ()[iPos];

			aeString sTemp = sPost;
			sPost = &(sTemp.c_str ()[iPos]);
		}

		return (true);
	}

	aeString ReplaceKeyword (const aeString& sCode, const aeString& sKeyword, const aeString& sReplacement)
	{
		aeString sResult = sCode;
		aeString sPrev, sPost;

		while (FindKeyword (sResult, sKeyword, sPrev, sPost))
		{
			sResult = sPrev + sReplacement + sPost;
		}
		
		return (sResult);
	}

	aeString ReplaceKeywordAndArrayIndex (const aeString& sCode, const aeString& sKeyword, const aeString& sReplacement)
	{
		aeString sResult = sCode;
		aeString sPrev, sPost, sArray;

		while (FindKeywordAndArrayIndex (sResult, sKeyword, sPrev, sPost, sArray))
		{
			sResult = sPrev + "input" + sArray + sReplacement + sPost;
		}
		
		return (sResult);
	}

	aeString ReplaceKeyword2 (const aeString& sCode, const aeString& sKeyword, const aeString& sReplacement)
	{
		aeString sResult = sCode;

		sResult = ReplaceKeyword (sResult, sKeyword + " ", sReplacement + " ");
		sResult = ReplaceKeyword (sResult, sKeyword + "(", sReplacement + "(");
		sResult = ReplaceKeyword (sResult, sKeyword + "\t", sReplacement + "\t");

		return (sResult);
	}

	bool FindPortFunction (const aeString& sCode, const aeString& sFunction, aeString& sPrev, aeString& sPost, aeString& sResult, bool bEatSemicolon)
	{
		aeInt32 iPos = aeStringFunctions::FindFirstStringPos (sCode.c_str (), sFunction.c_str ());

		if (iPos < 0)
			return (false);

		aeInt32 iBrackets = 1;

		sPrev = aeString (sCode.c_str (), iPos);

		while (sCode[iPos] != '(')
			++iPos;

		sResult = "";

		while (iBrackets > 0)
		{
			++iPos;

			sResult += sCode[iPos];

			if (sCode[iPos] == '(')
				++iBrackets;
			if (sCode[iPos] == ')')
				--iBrackets;
		}

		if ((bEatSemicolon) && (sCode[iPos+1] == ';'))
			++iPos;
	
		sPost = aeString (&(sCode.c_str ()[iPos + 1]));

		return (true);
	}
}

