#include "../LuaWrapper.h"
#include "../../Logging/Log.h"


namespace AE_NS_FOUNDATION
{
  void* aeLuaWrapper::GetFunctionLightUserData (void)
  {
    lua_pushvalue (mp_State, LUA_ENVIRONINDEX);
    lua_pushnumber (mp_State, 0);
    lua_gettable (mp_State, -2);

    void* p = (void*) lua_topointer (mp_State, -1);

    lua_pop (mp_State, 2);

    return (p);
  }

  void aeLuaWrapper::RegisterCFunction (const char* szFunctionName, lua_CFunction pFunction, void* pLightUserData) const
  {
    lua_pushcfunction (mp_State, pFunction);
    lua_setglobal (mp_State, szFunctionName);


    if (pLightUserData != nullptr)
    {
      lua_pushstring (mp_State, szFunctionName);
      lua_gettable (mp_State, LUA_GLOBALSINDEX);

      lua_newtable (mp_State);
      lua_pushnumber (mp_State, 0);
      lua_pushlightuserdata (mp_State, pLightUserData);
      lua_settable (mp_State, -3);

      lua_setfenv (mp_State, -2);

      lua_pop (mp_State, 1);
    }
  }

  bool aeLuaWrapper::PrepareFunctionCall (const char* szFunctionName, bool bCheckFunctionExistence)
  {
    AE_CHECK_DEV (m_States.m_iLuaReturnValues == 0, "aeLuaWrapper::PrepareFunctionCall: You didn't discard the return-values of the previous script call. %d Return-values were expected.", m_States.m_iLuaReturnValues);

    m_States.m_iParametersPushed = 0;

    lua_pushstring (mp_State, szFunctionName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    if ((bCheckFunctionExistence) && (lua_isfunction (mp_State, -1) == 0))
    {
      lua_pop (mp_State, 1);
      return (false);
    }

    return (true);
  }

  void aeLuaWrapper::CallPreparedFunction (aeUInt32 iExpectedReturnValues)
  {
    m_States.m_iLuaReturnValues = iExpectedReturnValues;

    // save the current states on a cheap stack
    const aeScriptStates StackedStates = m_States;
    m_States = aeScriptStates ();

    if (lua_pcall (mp_State, StackedStates.m_iParametersPushed, iExpectedReturnValues, 0) != 0)
    {
      // restore the states to their previous values
      m_States = StackedStates;

      m_States.m_iLuaReturnValues = 0;

      aeLog::Error ("Scriptfunction Call: %s", lua_tostring (mp_State, -1));
      lua_pop (mp_State, 1);  /* pop error message from the stack */
    }
    else
    {
      if ((m_States.m_iLuaReturnValues != 0) || (m_States.m_iOpenTables != 0))
        aeLog::FatalError ("After aeLuaWrapper::CallPreparedFunction: Return values: %d, Open Tables: %d", m_States.m_iLuaReturnValues, m_States.m_iOpenTables);

      m_States = StackedStates;
    }
  }

  void aeLuaWrapper::DiscardReturnValues (void)
  {
    if (m_States.m_iLuaReturnValues == 0)
      return;

    lua_pop (mp_State, m_States.m_iLuaReturnValues);
    m_States.m_iLuaReturnValues = 0;
  }

  bool aeLuaWrapper::IsReturnValueInt (aeUInt32 iReturnValue) const
  {
    return (lua_type (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1) == LUA_TNUMBER);
  }

  bool aeLuaWrapper::IsReturnValueBool (aeUInt32 iReturnValue) const
  {
    return (lua_type (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1) == LUA_TBOOLEAN);
  }

  bool aeLuaWrapper::IsReturnValueFloat (aeUInt32 iReturnValue) const
  {
    return (lua_type (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1) == LUA_TNUMBER);
  }

  bool aeLuaWrapper::IsReturnValueTable (aeUInt32 iReturnValue) const
  {
    return (lua_type (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1) == LUA_TTABLE);
  }

  bool aeLuaWrapper::IsReturnValueString (aeUInt32 iReturnValue) const
  {
    return (lua_type (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1) == LUA_TSTRING);
  }

  aeInt32 aeLuaWrapper::GetIntReturnValue (aeUInt32 iReturnValue) const
  {
    return ((int) (lua_tointeger (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1)));
  }

  bool aeLuaWrapper::GetBoolReturnValue (aeUInt32 iReturnValue) const
  {
    return (lua_toboolean (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1) != 0);
  }

  float aeLuaWrapper::GetFloatReturnValue (aeUInt32 iReturnValue) const
  {
    return ((float) (lua_tonumber (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1)));
  }

  const char* aeLuaWrapper::GetStringReturnValue (aeUInt32 iReturnValue) const
  {
    return (lua_tostring (mp_State, -m_States.m_iLuaReturnValues + (iReturnValue + s_ParamOffset) - 1));
  }
}





