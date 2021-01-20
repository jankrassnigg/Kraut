// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_SHADERMANAGER_CONDITION_H
#define AE_GRAPHICS_SHADERMANAGER_CONDITION_H

#include "Declarations.h"
#include <vector>
#include <KrautGraphics/Containers/Set.h>

namespace AE_NS_GRAPHICS
{
	class aeCondition;

	struct aeSubCondition;

	class AE_GRAPHICS_DLL aeCondition
	{
	public:
		aeCondition ();
		~aeCondition ();

		void UnloadCondition (void);

		void SetVariable (const char* szVariable, bool bNegated, bool bReplaceOnly = false);
		aeInt32 ParseForConditions (const char* szSource, aeSet<aeStaticString<32>>& out_VarsUsed);

		static void setConditionVariable (const char* szVariable, const char* szValue, bool bSetAsDefaultValue);

		bool BuildShaderCode (aeString& out_sResult, bool bOnlyReplace);

		static const aeString& getShaderVariable (const aeStaticString<32>& sVar);

		static bool HaveConditionsChanged (void);

		static void setShaderBuilderVarsToDefault ();

	private:
		aeStaticString<32> m_sVariable;
		bool m_bNegate;
		bool m_bReplace;
		bool m_bPureIF;
		aeArray<aeSubCondition*> m_ChildConditions;

		aeString m_sTextAtEnd;
	};

	struct aeSubCondition
	{
		aeSubCondition (const char* szText);
		~aeSubCondition ();

		aeString m_sTextAtBeginning;
		aeCondition m_Condition;
	};

}

#endif


