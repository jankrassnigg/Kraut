#include "../../FileSystem/FileIn.h"
#include "../../Logging/Log.h"
#include "../LuaWrapper.h"


namespace AE_NS_FOUNDATION
{
  aeLuaWrapper::aeLuaWrapper (void)
  {
    m_bReleaseOnExit = true;
    mp_State = NULL;

    Reset ();
  }

  aeLuaWrapper::aeLuaWrapper (lua_State* s)
  {
    mp_State = s;
    m_bReleaseOnExit = false;
  }

  aeLuaWrapper::~aeLuaWrapper ()
  {
    if (m_bReleaseOnExit)
      lua_close (mp_State);
  }

  void aeLuaWrapper::Reset (void)
  {
    if (m_bReleaseOnExit)
    {
      if (mp_State)
        lua_close (mp_State);

      mp_State = lua_newstate (lua_allocator, NULL);

      luaL_openlibs (mp_State);
      //luaopen_string (mp_State);
      //luaopen_math (mp_State);
      //luaopen_base (mp_State);
    }
  }

  bool aeLuaWrapper::ExecuteFile (const char* szFile) const
  {
    AE_CHECK_DEV (m_States.m_iLuaReturnValues == 0, "aeLuaWrapper::ExecuteFile: You didn't discard the return-values of the previous script call. %d Return-values were expected.", m_States.m_iLuaReturnValues);

    if (!aeFileSystem::ExistsFile (szFile))
      return (false);

    const int iKB = 64;
    const int iBufferSize = 1024*iKB+1;

    aeHybridString<iBufferSize> sBuffer;

    char szBuffer[iBufferSize] = "";
    aeFileIn File;
    File.Open(szFile);

    while (!File.IsEndOfStream ())
    {
      szBuffer[File.Read (szBuffer, iBufferSize-1)] = '\0';
      sBuffer += szBuffer;
    }

    return (ExecuteString (sBuffer));
  }

  bool aeLuaWrapper::ExecuteString (const aeBasicString& sString, aeString* pError) const
  {
    AE_CHECK_DEV (m_States.m_iLuaReturnValues == 0, "aeLuaWrapper::ExecuteString: You didn't discard the return-values of the previous script call. %d Return-values were expected.", m_States.m_iLuaReturnValues);

    int error = luaL_loadbuffer (mp_State, sString.c_str (), sString.length (), "chunk");
        
    if (error != 0)
    {
      if (pError)
        *pError = lua_tostring (mp_State, -1);

      aeLog::Error ("Cannot compile Lua Script: %s\nError: %s", sString.c_str (), lua_tostring (mp_State, -1));
      return (false);
    }

    error = lua_pcall (mp_State, 0, 0, 0);

    if (error != 0)
    {
      if (pError)
        *pError = lua_tostring (mp_State, -1);

      aeLog::Error ("Error executing Lua Script: %s\nError: %s", sString.c_str (), lua_tostring (mp_State, -1));
      return (false);
    }

    return (true);
  }

  void* aeLuaWrapper::lua_allocator (void* ud, void* ptr, size_t osize, size_t nsize)
  {
    //! \todo Create optimized allocator.

    if (nsize == 0)
    {
      delete[] (aeUInt8*) ptr;
      return (NULL);
    }

    unsigned char* ucPtr = new aeUInt8[nsize];

    if (ptr != NULL)
    {
      aeMemory::Copy (ptr, ucPtr, aeUInt32 (osize < nsize ? osize : nsize));

      delete[] (aeUInt8*) ptr;
    }

    return ((void*) ucPtr);
  }
}