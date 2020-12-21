// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Condition.h"
#include "ShaderResource.h"



namespace AE_NS_GRAPHICS
{
	struct aeShaderBuilderVarValue
	{
		aeString m_sCurrentValue;
		aeString m_sDefaultValue;
	};

	//! \todo DLL errors
	static aeMap<aeStaticString<32>, aeShaderBuilderVarValue> s_ShaderVariables;
	static bool m_bConditionsChanged = true;

	void LogShaderSource (const char* szHeader, const aeString& sSource, aeInt32 iErrorLine = -1);

	aeSubCondition::aeSubCondition (const char* szText) : m_sTextAtBeginning (szText)
	{
	}

	aeSubCondition::~aeSubCondition ()
	{
	}

	aeCondition::aeCondition ()
	{
		m_bPureIF = false;
	}

	aeCondition::~aeCondition ()
	{
		UnloadCondition ();
	}

	const aeString& aeCondition::getShaderVariable (const aeStaticString<32>& sVar) 
	{
#ifdef AE_COMPILE_FOR_DEVELOPMENT
    aeMap<aeStaticString<32>, aeShaderBuilderVarValue>::iterator it = s_ShaderVariables.find (sVar);

		if (it == s_ShaderVariables.end ())
		{
			aeLog::Error ("The Shader-Builder Variable \"%s\" is not defined but used inside a shader.", sVar.c_str ());
			return (s_ShaderVariables[sVar].m_sCurrentValue);
		}

		return (it.value().m_sCurrentValue);
#endif

		return (s_ShaderVariables[sVar].m_sCurrentValue);
	}

	bool aeCondition::HaveConditionsChanged (void) 
	{
		bool b = m_bConditionsChanged; 
		m_bConditionsChanged = false; 
		return (b);
	}

	void aeCondition::UnloadCondition (void)
	{
		for (aeUInt32 i = 0; i < m_ChildConditions.size (); ++i)
    {
			delete (m_ChildConditions[i]);
    }

		m_ChildConditions.clear ();

		m_sTextAtEnd = "";
	}

	void aeCondition::setShaderBuilderVarsToDefault ()
	{
    aeMap<aeStaticString<32>, aeShaderBuilderVarValue>::iterator it = s_ShaderVariables.begin ();
		const aeMap<aeStaticString<32>, aeShaderBuilderVarValue>::iterator itend = s_ShaderVariables.end ();

		for (; it != itend; ++it)
		{
			if (it.value().m_sCurrentValue != it.value().m_sDefaultValue)
			{
        it.value().m_sCurrentValue = it.value().m_sDefaultValue;
				m_bConditionsChanged = true;
			}
		}
	}

	void aeCondition::setConditionVariable (const char* szVariable, const char* szValue, bool bSetAsDefaultValue) 
	{
		aeStaticString<32> sVar = szVariable;
    sVar.ToUpperCase ();
		aeString sVal = szValue;

    aeMap<aeStaticString<32>, aeShaderBuilderVarValue>::iterator it = s_ShaderVariables.find (sVar);
		if (it == s_ShaderVariables.end ())
		{
			s_ShaderVariables[sVar].m_sCurrentValue = sVal;
			s_ShaderVariables[sVar].m_sDefaultValue = sVal;
			m_bConditionsChanged = true;
		}
		else
		if (bSetAsDefaultValue)
		{
			it.value().m_sDefaultValue = sVal;

			if (it.value().m_sCurrentValue != sVal)
			{
        it.value().m_sCurrentValue = sVal;
				m_bConditionsChanged = true;
			}
		}
		else
		if (it.value().m_sCurrentValue != sVal)
		{
      it.value().m_sCurrentValue = sVal;
			m_bConditionsChanged = true;
		}
	}

	aeString getVariableUsed (const char* szSource, int& out_iAddPos)
	{
		int iBracketOpen = aeStringFunctions::FindFirstStringPos (szSource, "(");
		int iBracketClose = aeStringFunctions::FindFirstStringPos (szSource, ")");

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

    aeString sRet (&szSource[iBracketOpen], iBracketClose - iBracketOpen + 1);
    sRet.ToUpperCase ();
		return sRet;
	}

	void aeCondition::SetVariable (const char* szVariable, bool bNegated, bool bReplaceOnly)
	{
		m_sVariable = szVariable;
		m_bNegate = bNegated;
		m_bReplace = bReplaceOnly;

		//if (s_ShaderVariables.find (m_sVariable) == s_ShaderVariables.end ())
		//	s_ShaderVariables[m_sVariable].m_sCurrentValue = "";
	}

	aeInt32 aeCondition::ParseForConditions (const char* szSource, aeSet<aeStaticString<32>>& out_VarsUsed)
	{
		bool bInsideBlock = false;
		int iPos = 0;
		int iAddPos = 0;

		while (szSource[iPos] != '\0')
		{
			if ((szSource[iPos] == '/') && (szSource[iPos+1] == '/'))
			{
				// keep the comment-text for better debugging
				while ((szSource[iPos] != '\0') && (szSource[iPos] != '\n'))
				{
					m_sTextAtEnd += szSource[iPos];
					++iPos;
				}

				m_sTextAtEnd += szSource[iPos];

				if (szSource[iPos] != '\0')
					++iPos;
				continue;
			}

			if ((szSource[iPos] == '/') && (szSource[iPos+1] == '*'))
			{
				m_sTextAtEnd += "/*";

				iPos += 2;

				// keep the comment-text for better debugging
				while ((szSource[iPos-1] != '*') || (szSource[iPos] != '/'))
				{
					m_sTextAtEnd += szSource[iPos];
					++iPos;

					if (szSource[iPos] == '\0')
						break;
				}

				m_sTextAtEnd += szSource[iPos];
				++iPos;
				continue;
			}

			if (szSource[iPos] == '$')
			{
				++iPos;

				if (aeStringFunctions::CompareEqual (&szSource[iPos], "ifnot", 5))
				{
					iPos += 5;
					aeString sVar = getVariableUsed (&szSource[iPos], iAddPos);
					iPos += iAddPos;

					out_VarsUsed.insert (sVar);

					bInsideBlock = true;

					m_ChildConditions.push_back (new aeSubCondition (m_sTextAtEnd.c_str ()));
					m_ChildConditions.back()->m_Condition.SetVariable (sVar.c_str (), true);
          m_ChildConditions.back()->m_Condition.m_bPureIF = true;

					iPos += m_ChildConditions.back()->m_Condition.ParseForConditions (&szSource[iPos], out_VarsUsed);
					m_sTextAtEnd = "";
				}
				else
				if (aeStringFunctions::CompareEqual (&szSource[iPos], "if", 2))
				{
					iPos += 2;
					aeString sVar = getVariableUsed (&szSource[iPos], iAddPos);
					iPos += iAddPos;

					out_VarsUsed.insert (sVar);

					bInsideBlock = true;

					m_ChildConditions.push_back (new aeSubCondition (m_sTextAtEnd.c_str ()));
					m_ChildConditions.back()->m_Condition.SetVariable (sVar.c_str (), false);
					m_ChildConditions.back()->m_Condition.m_bPureIF = true;

					iPos += m_ChildConditions.back()->m_Condition.ParseForConditions (&szSource[iPos], out_VarsUsed);
					m_sTextAtEnd = "";
				}
				else
				if (aeStringFunctions::CompareEqual (&szSource[iPos], "endif", 5))
				{
					if (bInsideBlock)
						bInsideBlock = false;
					else
					{
						iPos -= 1;
						break;
					}

					iPos += 5;
				}
				else
				if (aeStringFunctions::CompareEqual (&szSource[iPos], "elseifnot", 9))
				{
					if (bInsideBlock)
						bInsideBlock = false;
					else
					{
						iPos -= 1;
						break;
					}

					iPos += 9;
					aeString sVar = getVariableUsed (&szSource[iPos], iAddPos);
					iPos += iAddPos;

					out_VarsUsed.insert (sVar);

					bInsideBlock = true;

					m_ChildConditions.push_back (new aeSubCondition (m_sTextAtEnd.c_str ()));
          m_ChildConditions.back()->m_Condition.SetVariable (sVar.c_str (), true);

					iPos += m_ChildConditions.back()->m_Condition.ParseForConditions (&szSource[iPos], out_VarsUsed);
					m_sTextAtEnd = "";
				}
				else
				if (aeStringFunctions::CompareEqual (&szSource[iPos], "elseif", 6))
				{
					if (bInsideBlock)
						bInsideBlock = false;
					else
					{
						iPos -= 1;
						break;
					}

					iPos += 6;
					aeString sVar = getVariableUsed (&szSource[iPos], iAddPos);
					iPos += iAddPos;

					out_VarsUsed.insert (sVar);

					bInsideBlock = true;

					m_ChildConditions.push_back (new aeSubCondition (m_sTextAtEnd.c_str ()));
          m_ChildConditions.back()->m_Condition.SetVariable (sVar.c_str (), false);

					iPos += m_ChildConditions.back()->m_Condition.ParseForConditions (&szSource[iPos], out_VarsUsed);
					m_sTextAtEnd = "";
				}
				else
				if (aeStringFunctions::CompareEqual (&szSource[iPos], "else", 4))
				{
					if (bInsideBlock)
						bInsideBlock = false;
					else
					{
						iPos -= 1;
						break;
					}

					iPos += 4;

					bInsideBlock = true;

					m_ChildConditions.push_back (new aeSubCondition (m_sTextAtEnd.c_str ()));
          m_ChildConditions.back()->m_Condition.SetVariable ("BENDERISTHEGREATEST", false);

					iPos += m_ChildConditions.back()->m_Condition.ParseForConditions (&szSource[iPos], out_VarsUsed);
					m_sTextAtEnd = "";
				}
				else
				if (aeStringFunctions::CompareEqual (&szSource[iPos], "val", 3))
				{
					iPos += 3;
					aeString sVar = getVariableUsed (&szSource[iPos], iAddPos);
					iPos += iAddPos;

					out_VarsUsed.insert (sVar);

					// add a "condition" as place-holder, $val is no real condition, but implemented as one
					m_ChildConditions.push_back (new aeSubCondition (m_sTextAtEnd.c_str ()));
          m_ChildConditions.back()->m_Condition.SetVariable (sVar.c_str (), false, true);

					m_sTextAtEnd = "";
				}

				continue;
			}

			m_sTextAtEnd += szSource[iPos];
			++iPos;
		}

		return (iPos);
	}

	bool aeCondition::BuildShaderCode (aeString& out_sResult, bool bOnlyReplace)
	{
		if (m_bReplace)
		{
			out_sResult += s_ShaderVariables[m_sVariable].m_sCurrentValue;
			return (false);
		}

		 if (m_bPureIF)
			 bOnlyReplace = false;

		if (bOnlyReplace)
		{
			return (true);
		}

		// without this here, when using DLLs, the variable is not defined, don't know why
		//s_ShaderVariables["BENDERISTHEGREATEST"].m_sCurrentValue = "1";
		//s_ShaderVariables["BENDERISTHEGREATEST"].m_sDefaultValue = "1";

		bool bVar = (s_ShaderVariables[m_sVariable].m_sCurrentValue == "");
		if (bVar == m_bNegate)
		{
			bOnlyReplace = false;

			for (aeUInt32 i = 0; i < m_ChildConditions.size (); ++i)
			{
				out_sResult += m_ChildConditions[i]->m_sTextAtBeginning.c_str ();
				bOnlyReplace = (m_ChildConditions[i]->m_Condition.BuildShaderCode (out_sResult, bOnlyReplace));
			}

			out_sResult += m_sTextAtEnd.c_str ();
			return (true);
		}

		return (false);
	}

}


