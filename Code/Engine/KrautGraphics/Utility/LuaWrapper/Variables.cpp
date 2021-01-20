#include "../LuaWrapper.h"


namespace AE_NS_FOUNDATION
{
  bool aeLuaWrapper::IsVariableAvailable (const char* szName) const
  {
    lua_pushstring (mp_State, szName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    bool bRet = (lua_isnil (mp_State, -1) == 0);
    lua_pop (mp_State, 1);
    return (bRet);
  }

  bool aeLuaWrapper::IsFunctionAvailable (const char* szFunction) const
  {
    lua_pushstring (mp_State, szFunction);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    bool bRet = (lua_isfunction (mp_State, -1) != 0);
    lua_pop (mp_State, 1);
    return (bRet);
  }

  aeInt32 aeLuaWrapper::GetIntVariable (const char* szName, aeInt32 Default) const
  {
    lua_pushstring (mp_State, szName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    int ret = Default;

    if (lua_isnumber (mp_State, -1) != 0)
      ret = (int) lua_tonumber (mp_State, -1); //lua_tointeger (mp_State, -1);

    lua_pop (mp_State, 1);
    return (ret);
  }

  bool aeLuaWrapper::GetBoolVariable (const char* szName, bool Default) const
  {
    lua_pushstring (mp_State, szName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    bool ret = Default;

    if (lua_isboolean (mp_State, -1) != 0)
      ret = (lua_toboolean (mp_State, -1) != 0);

    lua_pop (mp_State, 1);
    return (ret);
  }

  float aeLuaWrapper::GetFloatVariable (const char* szName, float Default) const
  {
    lua_pushstring (mp_State, szName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    float ret = Default;

    if (lua_isnumber (mp_State, -1) != 0)
      ret = (float) lua_tonumber (mp_State, -1);

    lua_pop (mp_State, 1);
    return (ret);
  }

  const char* aeLuaWrapper::GetStringVariable (const char* szName, const char* Default) const
  {
    lua_pushstring (mp_State, szName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    const char* ret = Default;

//		if (lua_isstring (mp_State, -1) != 0)
//			ret = lua_tostring (mp_State, -1);
    if (lua_type (mp_State, -1) == LUA_TSTRING)	//don't want to convert numbers to strings, so be strict about it
      ret = (lua_tostring (mp_State, -1));

    lua_pop (mp_State, 1);
    return (ret);
  }
}


