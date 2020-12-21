#include "../CVar.h"
#include "../../FileSystem/FileOut.h"
#include "../../Logging/Log.h"

namespace AE_NS_FOUNDATION
{
  AE_ENUMERABLE_CLASS_CODE (aeCVar);

  aeCVar::aeCVar (const char* szName, aeUInt32 flags, aeCVarType::Enum type, const char* szDescription)
    :	m_sName (szName), 
      m_sDescription (szDescription), 
      m_Flags (flags), 
      m_Type (type)
  {
  }

  aeCVar* aeCVar::GetCVar (const char* szName)
  {
    aeCVar* pCur = GetFirstInstance ();

    while (pCur)
    {
      if (aeStringFunctions::CompareEqual (pCur->GetName (), szName))
        return (pCur);

      pCur = GetNextInstance (pCur);
    }

    return (NULL);
  }

  static int LUAFUNC_ReadCVAR (lua_State* state)
  {
    aeLuaWrapper s (state);

    aeCVar* pCVar = aeCVar::GetCVar (s.GetStringParameter (0));

    if (pCVar == NULL)
    {
      s.PushReturnValueNil ();
      return (s.ReturnToScript ());
    }

    switch (pCVar->GetType ())
    {
    case aeCVarType::Int:
      {
        aeCVarInt* pVar = (aeCVarInt*) (pCVar);
        s.PushReturnValue (pVar->GetValue ());
      }
      break;
    case aeCVarType::Bool:
      {
        aeCVarBool* pVar = (aeCVarBool*) (pCVar);
        s.PushReturnValue (pVar->GetValue ());
      }
      break;
    case aeCVarType::Float:
      {
        aeCVarFloat* pVar = (aeCVarFloat*) (pCVar);
        s.PushReturnValue (pVar->GetValue ());
      }
      break;
    case aeCVarType::String:
      {
        aeCVarString* pVar = (aeCVarString*) (pCVar);
        s.PushReturnValue (pVar->GetValue ());
      }
      break;
    }

    return (s.ReturnToScript ());
  }


  static int LUAFUNC_WriteCVAR (lua_State* state)
  {
    aeLuaWrapper s (state);

    aeCVar* pCVar = aeCVar::GetCVar (s.GetStringParameter (0));

    if (pCVar == NULL)
    {
      s.PushReturnValue (false);
      return (s.ReturnToScript ());
    }

    s.PushReturnValue (true);

    switch (pCVar->GetType ())
    {
    case aeCVarType::Int:
      {
        aeCVarInt* pVar = (aeCVarInt*) (pCVar);
        *pVar = s.GetIntParameter (1);
      }
      break;
    case aeCVarType::Bool:
      {
        aeCVarBool* pVar = (aeCVarBool*) (pCVar);
        *pVar = s.GetBoolParameter (1);
      }
      break;
    case aeCVarType::Float:
      {
        aeCVarFloat* pVar = (aeCVarFloat*) (pCVar);
        *pVar = s.GetFloatParameter (1);
      }
      break;
    case aeCVarType::String:
      {
        aeCVarString* pVar = (aeCVarString*) (pCVar);
        *pVar = s.GetStringParameter (1);
      }
      break;
    }

    return (s.ReturnToScript ());
  }

  void aeCVar::AllowScriptAccessToCVars (aeLuaWrapper& Script)
  {
    Script.RegisterCFunction ("ReadCVar", LUAFUNC_ReadCVAR);
    Script.RegisterCFunction ("WriteCVar", LUAFUNC_WriteCVAR);

    const aeString sInit =
    "\
    function readcvar (t, key)\n\
      return (ReadCVar (key))\n\
    end\n\
    \n\
    function writecvar (t, key, value)\n\
      if not WriteCVar (key, value) then\n\
        rawset (t, key, value or false)\n\
      end\n\
    end\n\
    \n\
    setmetatable (_G, {\n\
      __newindex = writecvar,\n\
      __index = readcvar,\n\
      __metatable = \"Access Denied\",\n\
    })";

    if (!Script.ExecuteString (sInit.c_str ()))
      aeLog::Error ("aeCVar::AllowScriptAccessToCVars failed.");
  }

  bool aeCVar::SaveCVars (const char* szSaveToFile)
  {
    aeFileOut File;
    if (!File.Open (szSaveToFile))
      return (false);

    aeCVar* pCVar = GetFirstInstance ();

    aeHybridString<1024> sTemp;

    while (pCVar)
    {

      if (pCVar->m_Flags.IsAnyFlagSet (aeCVarFlags::Save))
      {
        switch (pCVar->GetType ())
        {
        case aeCVarType::Int:
          {
            aeCVarInt* pVar = static_cast<aeCVarInt*> (pCVar);
            sTemp.Format ("%s = %i;\n", pVar->GetName (), pVar->m_Value[aeCVarValue::ToBeStored]);
          }
          break;

        case aeCVarType::Float:
          {
            aeCVarFloat* pVar = static_cast<aeCVarFloat*> (pCVar);
            sTemp.Format ("%s = %.5f;\n", pVar->GetName (), pVar->m_Value[aeCVarValue::ToBeStored]);
          }
          break;

        case aeCVarType::Bool:
          {
            aeCVarBool* pVar = static_cast<aeCVarBool*> (pCVar);
            sTemp.Format ("%s = %s;\n", pVar->GetName (), pVar->m_Value[aeCVarValue::ToBeStored] ? "true" : "false");
          }
          break;

        case aeCVarType::String:
          {
            aeCVarString* pVar = static_cast<aeCVarString*> (pCVar);
            sTemp.Format ("%s = \"%s\";\n", pVar->GetName (), pVar->m_Value[aeCVarValue::ToBeStored].c_str());
          }
          break;
        }

        File.Write (&sTemp[0], sizeof (char) * sTemp.length ());
      }

      pCVar = GetNextInstance (pCVar);
    }

    return (true);
  }

  bool aeCVar::LoadCVars (const char* szLoadFromFile)
  {
    aeLuaWrapper Script;
    if (!Script.ExecuteFile (szLoadFromFile))
      return (false);

    aeCVar* pCVar = GetFirstInstance ();

    while (pCVar)
    {
      switch (pCVar->GetType ())
      {
      case aeCVarType::Int:
        {
          aeCVarInt* pVar = static_cast<aeCVarInt*> (pCVar);
          pVar->m_Value[aeCVarValue::Stored] = Script.GetIntVariable (pVar->GetName (), *pVar);
          *pVar = Script.GetIntVariable (pVar->GetName (), *pVar);
        }
        break;

      case aeCVarType::Float:
        {
          aeCVarFloat* pVar = static_cast<aeCVarFloat*> (pCVar);
          pVar->m_Value[aeCVarValue::Stored] = Script.GetFloatVariable (pVar->GetName (), *pVar);
          *pVar = Script.GetFloatVariable (pVar->GetName (), *pVar);
        }
        break;

      case aeCVarType::Bool:
        {
          aeCVarBool* pVar = static_cast<aeCVarBool*> (pCVar);
          pVar->m_Value[aeCVarValue::Stored] = Script.GetBoolVariable (pVar->GetName (), *pVar);
          *pVar = Script.GetBoolVariable (pVar->GetName (), *pVar);
        }
        break;

      case aeCVarType::String:
        {
          aeCVarString* pVar = static_cast<aeCVarString*> (pCVar);
          pVar->m_Value[aeCVarValue::Stored] = Script.GetStringVariable (pVar->GetName (), *pVar);
          *pVar = Script.GetStringVariable (pVar->GetName (), *pVar);
        }
        break;
      }

      pCVar->SetToRestartValue ();

      pCVar = GetNextInstance (pCVar);
    }

    return (true);
  }
}



