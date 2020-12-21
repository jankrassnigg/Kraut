// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "ShaderResource.h"
#include <KrautFoundation/FileSystem/FileIn.h>


namespace AE_NS_GRAPHICS
{
	aeStaticString<128> aeShaderResource::s_CurrentlyLoadedShader;	

	void LogShaderSource (const char* szHeader, const aeString& sSource, aeInt32 iErrorLine = -1);

  AE_RESOURCE_IMPLEMENTATION (aeShaderResource, "");

	aeString getIncludePath (const char* szSource, int& out_iAddPos)
	{
		int iBracketOpen = aeStringFunctions::FindFirstStringPos (szSource, "<");
		int iBracketClose = aeStringFunctions::FindFirstStringPos (szSource, ">");

		AE_CHECK_ALWAYS (iBracketOpen >= 0, "Shader \"%s\": An ill-formed Shader-Condition is used. Expected opening round Brackets before a Shader-Variable (after an $if/$ifnot, $elseif/$elseifnot or $val).", aeShaderResource::getCurrentlyLoadedShader ().c_str ());
		AE_CHECK_ALWAYS (iBracketClose > iBracketOpen, "Shader \"%s\": An ill-formed Shader-Condition is used. Expected closing round Brackets after a Shader-Variable (after an $if/$ifnot, $elseif/$elseifnot or $val).", aeShaderResource::getCurrentlyLoadedShader ().c_str ());

		out_iAddPos = iBracketClose + 1;

		++iBracketOpen;
		--iBracketClose;

		while (szSource[iBracketOpen] == ' ')
			++iBracketOpen;
		while (szSource[iBracketClose] == ' ')
			--iBracketClose;

		AE_CHECK_ALWAYS (iBracketClose > iBracketOpen, "Shader \"%s\": An ill-formed Shader-Condition is used. Expected SOME form of Shader-Variable between two round brackets (after an $if/$ifnot, $elseif/$elseifnot or $val).", aeShaderResource::getCurrentlyLoadedShader ().c_str ());

		return (aeString (&szSource[iBracketOpen], iBracketClose - iBracketOpen + 1));
	}

	aeString ReplaceIncludes (aeString sCode)
	{
		int iPos = -1;
		int iCounter = 0;

		do
		{
			++iCounter;

			if (iCounter > 100)
			{
				aeLog::Error ("ShaderManager: There seems to be an $include-loop in the currently loaded shader.");
			}

			iPos = aeStringFunctions::FindFirstStringPos (sCode.c_str (), "$include");
			if (iPos != -1)
			{
				const int iStartPos = iPos;
				int iAddPos;

				aeString sPath = getIncludePath (&(sCode.c_str ()[iStartPos]), iAddPos);

				const int iEndPos = iStartPos + iAddPos;

				aeFileIn File;

        File.Open (sPath.c_str (), 32);

				aeString sSource;
				char szTemp[256] = "";
				while (!File.IsEndOfStream ())
				{
					int iRead = File.Read (szTemp, 255);
					szTemp[iRead] = '\0';
					sSource += szTemp;
				}

				aeString sCodeAnte = sCode;
				sCodeAnte.resize (iStartPos);

				aeString sCodePost = &(sCode.c_str ()[iEndPos]);

				sCode = sCodeAnte + sSource + sCodePost;
			}
		}
		while (iPos != -1);

		return (sCode);
	}

	int TempMin (int start, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int end)
	{
		if ((a1 > start) && (a1 < end))
			end = a1;

		if ((a2 > start) && (a2 < end))
			end = a2;

		if ((a3 > start) && (a3 < end))
			end = a3;

		if ((a4 > start) && (a4 < end))
			end = a4;

		if ((a5 > start) && (a5 < end))
			end = a5;

		if ((a6 > start) && (a6 < end))
			end = a6;

		if ((a7 > start) && (a7 < end))
			end = a7;

		if ((a8 > start) && (a8 < end))
			end = a8;

		if ((a9 > start) && (a9 < end))
			end = a9;

		return (end);
	}

	void aeShaderResource::LoadResourceFromFile (const char* szFilename)
	{
		s_CurrentlyLoadedShader = szFilename;

		m_bHasVertexShader = false;
		m_bHasFragmentShader = false;
		m_bHasGeometryShader = false;


		aeFileIn File;
    File.Open (szFilename);

    AE_CHECK_DEV(File.IsStreamOpen(), "file does not exist");

		aeString sSource;
		char szTemp[256] = "";
		while (!File.IsEndOfStream ())
		{
			int iRead = File.Read (szTemp, 255);
			szTemp[iRead] = '\0';
			sSource += szTemp;
		}

		sSource = ReplaceIncludes (sSource);

		// LogShaderSource ("", sSource);
		
		
		aeString sSourceUpper = sSource.c_str ();
    sSourceUpper.ToUpperCase ();

		aeStaticString<32> sVSDelimiter ("[PORTABLE VERTEX SHADER]");
		aeStaticString<32> sFSDelimiter ("[PORTABLE FRAGMENT SHADER]");
		aeStaticString<32> sGSDelimiter ("[PORTABLE GEOMETRY SHADER]");
		aeStaticString<32> sVSDelimiterHLSL ("[HLSL VERTEX SHADER]");
		aeStaticString<32> sFSDelimiterHLSL ("[HLSL FRAGMENT SHADER]");
		aeStaticString<32> sGSDelimiterHLSL ("[HLSL GEOMETRY SHADER]");
		aeStaticString<32> sVSDelimiterGLSL ("[VERTEX SHADER]");
		aeStaticString<32> sFSDelimiterGLSL ("[FRAGMENT SHADER]");
		aeStaticString<32> sGSDelimiterGLSL ("[GEOMETRY SHADER]");


		int iEnd  = aeStringFunctions::GetLength (sSourceUpper.c_str ());
		int iVSp  = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sVSDelimiter.c_str ());
		int iFSp  = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sFSDelimiter.c_str ());
		int iGSp  = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sGSDelimiter.c_str ());
		int iVShl = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sVSDelimiterHLSL.c_str ());
		int iFShl = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sFSDelimiterHLSL.c_str ());
		int iGShl = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sGSDelimiterHLSL.c_str ());
		int iVSgl = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sVSDelimiterGLSL.c_str ());
		int iFSgl = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sFSDelimiterGLSL.c_str ());
		int iGSgl = aeStringFunctions::FindFirstStringPos (sSourceUpper.c_str (), sGSDelimiterGLSL.c_str ());

		// todo
		const int iVSpEnd  = TempMin (iVSp,  iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);
		const int iFSpEnd  = TempMin (iFSp,  iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);
		const int iGSpEnd  = TempMin (iGSp,  iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);

		const int iVShlEnd = TempMin (iVShl, iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);
		const int iFShlEnd = TempMin (iFShl, iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);
		const int iGShlEnd = TempMin (iGShl, iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);

		const int iVSglEnd = TempMin (iVSgl, iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);
		const int iFSglEnd = TempMin (iFSgl, iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);
		const int iGSglEnd = TempMin (iGSgl, iVSp, iFSp, iGSp, iVShl, iFShl, iGShl, iVSgl, iFSgl, iGSgl, iEnd);

		int iVS;
		int iFS;
		int iGS;
		int iVSend;
		int iFSend;
		int iGSend;

		if ((g_RenderAPI != AE_RA_OPENGL) &&
			(g_RenderAPI != AE_RA_D3D11))
			AE_CHECK_ALWAYS (false, "g_RenderAPI is not properly set: %d", g_RenderAPI);

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			//aeLog::Dev ("Using the OpenGL Shader.");

			m_bHasVertexShader	 = (iVSgl >= 0);
			m_bHasFragmentShader = (iFSgl >= 0);
			m_bHasGeometryShader = (iGSgl >= 0);
			iVS = iVSgl + sVSDelimiterGLSL.length ();
			iFS = iFSgl + sFSDelimiterGLSL.length ();
			iGS = iGSgl + sGSDelimiterGLSL.length ();
			iVSend = iVSglEnd;
			iFSend = iFSglEnd;
			iGSend = iGSglEnd;
		}
#endif

		if (!m_bHasVertexShader)
		{
			//aeLog::Dev ("Has no VS.");

			m_bHasVertexShader = (iVSp >= 0);
			iVS = iVSp + sVSDelimiter.length ();
			iVSend = iVSpEnd;
		}
		if (!m_bHasFragmentShader)
		{
			//aeLog::Dev ("Has no FS.");

			m_bHasFragmentShader = (iFSp >= 0);
			iFS = iFSp + sFSDelimiter.length ();
			iFSend = iFSpEnd;
		}
		if (!m_bHasGeometryShader)
		{
			//aeLog::Dev ("Has no FS.");

			m_bHasGeometryShader = (iGSp >= 0);
			iGS = iGSp + sGSDelimiter.length ();
			iGSend = iGSpEnd;
		}

		// copy vertex shader code
		if (m_bHasVertexShader)
		{
			//aeLog::Dev ("Has VS.");

			const int iVSLen = iVSend - iVS;

			aeString sVSCode (&sSource.c_str ()[iVS], iVSLen);
			//sVSCode.assign (sSource, iVS, iVSLen);
			m_VertexShader.setShaderCode (sVSCode.c_str ());

			//LogShaderSource ("", sVSCode);
		}

		// copy fragment shader code
		if (m_bHasFragmentShader)
		{
			//aeLog::Dev ("Has FS.");

			const int iFSLen = iFSend - iFS;

			aeString sFSCode (&sSource[iFS], iFSLen);
			//sFSCode.assign (sSource, iFS, iFSLen);
			m_FragmentShader.setShaderCode (sFSCode.c_str ());

			// LogShaderSource ("", sFSCode);
		}

		// copy geometry shader code
		if (m_bHasGeometryShader)
		{
			//aeLog::Dev ("Has FS.");

			const int iGSLen = iGSend - iGS;

			aeString sGSCode (&sSource[iGS], iGSLen);
			//sGSCode.assign (sSource, iGS, iGSLen);
			m_GeometryShader.setShaderCode (sGSCode.c_str ());

			// LogShaderSource ("", sGSCode);
		}

		//aeLog::Dev ("Done.");
	}

	void aeShaderResource::UnloadResource (void)
	{
		m_VertexShader.UnloadAllShaders ();
		m_FragmentShader.UnloadAllShaders ();
		m_GeometryShader.UnloadAllShaders ();
	}


	static aeShaderTypeData Empty;

	aeShaderTypeData* aeShaderResource::getVertexShaderID (void)
	{
		if (!m_bHasVertexShader)
			return (&Empty);

		return (m_VertexShader.getShaderID (AE_ST_VERTEX, aeResourceHandle (this)));
	}

	aeShaderTypeData* aeShaderResource::getFragmentShaderID (void)
	{
		if (!m_bHasFragmentShader)
			return (&Empty);

		return (m_FragmentShader.getShaderID (AE_ST_FRAGMENT, aeResourceHandle (this)));
	}

	aeShaderTypeData* aeShaderResource::getGeometryShaderID (void)
	{
		if (!m_bHasGeometryShader)
			return (&Empty);

		return (m_GeometryShader.getShaderID (AE_ST_GEOMETRY, aeResourceHandle (this)));
	}
}

