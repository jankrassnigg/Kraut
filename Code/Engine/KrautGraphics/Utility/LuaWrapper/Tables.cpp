#include "../LuaWrapper.h"


namespace AE_NS_FOUNDATION
{
  bool aeLuaWrapper::OpenTable (const char* szName)
  {
    lua_pushstring (mp_State, szName);
    lua_gettable (mp_State, (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2);

    //failed, it's no table
    if (lua_istable (mp_State, -1) == 0)
    {
      lua_pop (mp_State, 1);
      return (false);
    }

    m_States.m_iOpenTables++;
    return (true);
  }

  bool aeLuaWrapper::OpenTable (aeUInt32 iFunctionParameter)
  {
    lua_pushvalue (mp_State, iFunctionParameter + s_ParamOffset);

    //failed, it's no table
    if (lua_istable (mp_State, -1) == 0)
    {
      lua_pop (mp_State, 1);
      return (false);
    }

    m_States.m_iOpenTables++;
    return (true);
  }

  void aeLuaWrapper::CloseTable (void)
  {
    if (m_States.m_iOpenTables == 0)
      return;

    DiscardReturnValues ();

    lua_pop (mp_State, 1);
    m_States.m_iOpenTables--;
  }

  void aeLuaWrapper::CloseAllTables (void)
  {
    DiscardReturnValues ();

    lua_pop (mp_State, m_States.m_iOpenTables);
    m_States.m_iOpenTables = 0;
  }

  //void aeLuaWrapper::EnumVariables (vector<string>& sNames)
  //{
  //	sNames.clear ();

  //	int t = (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2;
  //	size_t len = 0;

  //	/* table is in the stack at index 't' */
  //	lua_pushnil (mp_State);  /* first key */

  //	while (lua_next (mp_State, t) != 0) 
  //	{
  //		if (lua_type (mp_State, -2) == LUA_TSTRING)
  //		{
  //			int type = lua_type (mp_State, -1);

  //			if ((type == LUA_TSTRING) || (type == LUA_TNUMBER) || (type == LUA_TBOOLEAN))
  //				sNames.push_back (string (lua_tolstring (mp_State, -2, &len)));
  //		}

  //		/* removes 'value'; keeps 'key' for next iteration */
  //		lua_pop (mp_State, 1);
  //	}
  //}

  //void aeLuaWrapper::EnumFunctions (vector<string>& sNames)
  //{
  //	sNames.clear ();

  //	int t = (m_States.m_iOpenTables == 0) ? LUA_GLOBALSINDEX : -2;
  //	size_t len = 0;

  //	/* table is in the stack at index 't' */
  //	lua_pushnil (mp_State);  /* first key */

  //	while (lua_next (mp_State, t) != 0) 
  //	{
  //		if (lua_type (mp_State, -2) == LUA_TSTRING)
  //		{
  //			int type = lua_type (mp_State, -1);

  //			if (type == LUA_TFUNCTION)
  //				sNames.push_back (string (lua_tolstring (mp_State, -2, &len)));
  //		}

  //		/* removes 'value'; keeps 'key' for next iteration */
  //		lua_pop (mp_State, 1);
  //	}
  //}
}



