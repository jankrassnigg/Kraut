#ifndef AE_FOUNDATION_UTILITY_LUAWRAPPER_INL
#define AE_FOUNDATION_UTILITY_LUAWRAPPER_INL

namespace AE_NS_FOUNDATION
{
  AE_INLINE aeInt32 aeLuaWrapper::ReturnToScript (void) const 
  {
    return (m_States.m_iParametersPushed);
  }

  AE_INLINE aeUInt32 aeLuaWrapper::GetNumberOfFunctionParameters (void) const
  {
    return ((int) lua_gettop (mp_State));
  }

  AE_INLINE bool aeLuaWrapper::IsParameterBool (aeUInt32 iParameter) const
  {
    return (lua_type (mp_State, iParameter + s_ParamOffset) == LUA_TBOOLEAN);
  }

  AE_INLINE bool aeLuaWrapper::IsParameterFloat (aeUInt32 iParameter) const
  {
    return (lua_type (mp_State, iParameter + s_ParamOffset) == LUA_TNUMBER);
  }

  AE_INLINE bool aeLuaWrapper::IsParameterInt (aeUInt32 iParameter) const
  {
    return (lua_type (mp_State, iParameter + s_ParamOffset) == LUA_TNUMBER);
  }

  AE_INLINE bool aeLuaWrapper::IsParameterString (aeUInt32 iParameter) const
  {
    return (lua_type (mp_State, iParameter + s_ParamOffset) == LUA_TSTRING);
  }

  AE_INLINE bool aeLuaWrapper::IsParameterNil (aeUInt32 iParameter) const
  {
    return (lua_type (mp_State, iParameter + s_ParamOffset) == LUA_TNIL);
  }

  AE_INLINE bool aeLuaWrapper::IsParameterTable (aeUInt32 iParameter) const
  {
    return (lua_type (mp_State, iParameter + s_ParamOffset) == LUA_TTABLE);
  }

  AE_INLINE void aeLuaWrapper::PushParameter (aeInt32 iParameter)
  {
    lua_pushinteger (mp_State, iParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushParameter (bool bParameter)
  {
    lua_pushboolean (mp_State, bParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushParameter (float fParameter)
  {
    lua_pushnumber (mp_State, fParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushParameter (const char* szParameter)
  {
    lua_pushstring (mp_State, szParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushParameter (const char* szParameter, aeUInt32 length)
  {
    lua_pushlstring (mp_State, szParameter, length);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushParameter (const aeBasicString& sParameter)
  {
    lua_pushlstring (mp_State, sParameter.c_str (), sParameter.length ());
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushParameterNil (void)
  {
    lua_pushnil (mp_State);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValue (aeInt32 iParameter)
  {
    lua_pushinteger (mp_State, iParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValue (bool bParameter)
  {
    lua_pushboolean (mp_State, bParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValue (float fParameter)
  {
    lua_pushnumber (mp_State, fParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValue (const char* szParameter)
  {
    lua_pushstring (mp_State, szParameter);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValue (const char* szParameter, aeUInt32 length)
  {
    lua_pushlstring (mp_State, szParameter, length);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValue (const aeBasicString& sParameter)
  {
    lua_pushlstring (mp_State, sParameter.c_str (), sParameter.length ());
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::PushReturnValueNil (void)
  {
    lua_pushnil (mp_State);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE void aeLuaWrapper::SetVariableNil (const char* szName) const
  {
    lua_pushnil (mp_State);
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::SetVariable (const char* szName, aeInt32 iValue) const
  {
    lua_pushinteger (mp_State, iValue);
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::SetVariable (const char* szName, float fValue) const
  {
    lua_pushnumber (mp_State, fValue);
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::SetVariable (const char* szName, bool bValue) const
  {
    lua_pushboolean (mp_State, bValue);
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::SetVariable (const char* szName, const char* szValue) const
  {
    lua_pushstring (mp_State, szValue);
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::SetVariable (const char* szName, const char* szValue, aeUInt32 len) const
  {
    lua_pushlstring (mp_State, szValue, len);
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::SetVariable (const char* szName, const aeBasicString& sValue) const
  {
    lua_pushlstring (mp_State, sValue.c_str (), sValue.length ());
    lua_setfield (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2, szName);
  }

  AE_INLINE void aeLuaWrapper::PushTable (const char* szTableName, bool bGlobalTable)
  {
    lua_pushstring (mp_State, szTableName);
    lua_gettable (mp_State, ((bGlobalTable) || (m_States.m_iOpenTables == 0)) ? LUA_GLOBALSINDEX : -2);
    m_States.m_iParametersPushed++;
  }

  AE_INLINE int aeLuaWrapper::GetIntParameter (aeUInt32 iParameter) const 
  {
    return ((int) (lua_tointeger (mp_State, iParameter + s_ParamOffset)));
  }

  AE_INLINE bool aeLuaWrapper::GetBoolParameter (aeUInt32 iParameter) const
  {
    return (lua_toboolean (mp_State, iParameter + s_ParamOffset) != 0);
  }

  AE_INLINE float aeLuaWrapper::GetFloatParameter (aeUInt32 iParameter) const
  {
    return ((float) (lua_tonumber (mp_State, iParameter + s_ParamOffset)));
  }

  AE_INLINE const char* aeLuaWrapper::GetStringParameter (aeUInt32 iParameter) const 
  {
    return (lua_tostring (mp_State, iParameter + s_ParamOffset));
  }

}

#endif

