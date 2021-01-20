#ifndef AE_FOUNDATION_UTILITY_LUAWRAPPER_H
#define AE_FOUNDATION_UTILITY_LUAWRAPPER_H

#include <KrautFoundation/Defines.h>
#include <KrautFoundation/Strings/BasicString.h>
#include <KrautGraphics/Base.h>

#include <KrautGraphics/_Externals/Lua/lauxlib.h>
#include <KrautGraphics/_Externals/Lua/lua.h>
#include <KrautGraphics/_Externals/Lua/lualib.h>

namespace AE_NS_FOUNDATION
{
  /*! This class encapsulates ONE Lua-Script. It makes it easier to interact with the script, to get data
    out of it (configuration files), to register C-functions to it and to call script-functions. It is possible
    to load more than one Lua-File into one Lua-Script, one can dynamically generate code and pass it as
    a string to the script. It ALSO allows to construct the aeLuaWrapper with a working lua_State-Pointer and thus
    only simplify interaction with an already existing Script (for example, when a C-Function is called in a Script,
    it passes its lua_State to that Function). 
    \note Lua starts counting at 1, not at 0. However aeLuaWrapper does NOT do this, but uses the C++ convention instead! 
    That means, when you query the first parameter or return-value passed to your function, you need to query for value 0, not for value 1.
  */
  class AE_GRAPHICS_DLL aeLuaWrapper
  {
  public:
    //! Generates a NEW Lua-Script, which is empty.
    aeLuaWrapper(void);
    //! Takes an EXISTING Lua-Script and allows to get easier access to it.
    aeLuaWrapper(lua_State* s);
    //! Destroyes the Lua-Script, if it was created, but leaves it intact, if this instance did not generate the Lua-Script.
    ~aeLuaWrapper();

    //! Clears the script to be empty.
    void Reset(void);

    //! Loads a file into the Script. Returns false, if the file contains errors or cannot be opened.
    bool ExecuteFile(const char* szFile) const;
    //! Executes a string containig Lua-Code. Returns true, if no errors occured. Returns false otherwise.
    bool ExecuteString(const aeBasicString& sString, aeString* pError = nullptr) const;

    //! Opens a Lua-Table inside the Script, with the given name. Needs to be in scope. Returns false, if it's not possible (the table does not exist...).
    bool OpenTable(const char* szTable);
    //! Opens the Table n, that was passed to a C-Function on its Parameter-Stack. Start counting at 0, not 1.
    bool OpenTable(aeUInt32 iFunctionParameter);

    //! Closes the table opened last.
    void CloseTable(void);
    //! Closes all open Tables.
    void CloseAllTables(void);

    //! Pushes an existing table onto Luas Stack. Either one that is in local scope, or a global table.
    void PushTable(const char* szTableName, bool bGlobalTable);

    //! Returns how many Parameters were passed to the called C-Function.
    aeUInt32 GetNumberOfFunctionParameters(void) const;

    //! Checks, whether the Variable with the given Name exists.
    bool IsVariableAvailable(const char* szVariable) const;
    //! Checks, whether the Function with the given Name exists.
    bool IsFunctionAvailable(const char* szFunction) const;

    //! Checks the nth Parameter passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsParameterInt(aeUInt32 iParameter) const;
    //! Checks the nth Parameter passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsParameterBool(aeUInt32 iParameter) const;
    //! Checks the nth Parameter passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsParameterFloat(aeUInt32 iParameter) const;
    //! Checks the nth Parameter passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsParameterTable(aeUInt32 iParameter) const;
    //! Checks the nth Parameter passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsParameterString(aeUInt32 iParameter) const;
    //! Checks the nth Parameter passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsParameterNil(aeUInt32 iParameter) const;

    //! Sets the Variable with the given name (in scope) to nil.
    void SetVariableNil(const char* szName) const;
    //! Sets the Variable with the given name (in scope) with the given value.
    void SetVariable(const char* szName, aeInt32 iValue) const;
    //! Sets the Variable with the given name (in scope) with the given value.
    void SetVariable(const char* szName, bool bValue) const;
    //! Sets the Variable with the given name (in scope) with the given value.
    void SetVariable(const char* szName, float fValue) const;
    //! Sets the Variable with the given name (in scope) with the given value.
    void SetVariable(const char* szName, const aeBasicString& szValue) const;
    //! Sets the Variable with the given name (in scope) with the given value.
    void SetVariable(const char* szName, const char* szValue) const;
    //! Sets the Variable with the given name (in scope) with the given value.
    void SetVariable(const char* szName, const char* szValue, aeUInt32 len) const;

    //! Returns the Value of the Variable with the given name, or the default-value, if it does not exist.
    int GetIntVariable(const char* szName, aeInt32 iDefault = 0) const;
    //! Returns the Value of the Variable with the given name, or the default-value, if it does not exist.
    bool GetBoolVariable(const char* szName, bool bDefault = false) const;
    //! Returns the Value of the Variable with the given name, or the default-value, if it does not exist.
    float GetFloatVariable(const char* szName, float fDefault = 0.0f) const;
    //! Returns the Value of the Variable with the given name, or the default-value, if it does not exist.
    const char* GetStringVariable(const char* szName, const char* szDefault = "") const;

    //! Returns the Value of the nth Parameter, or the default-value, if it does not exist. Start counting at 0, not 1.
    int GetIntParameter(aeUInt32 iParameter) const;
    //! Returns the Value of the nth Parameter, or the default-value, if it does not exist. Start counting at 0, not 1.
    bool GetBoolParameter(aeUInt32 iParameter) const;
    //! Returns the Value of the nth Parameter, or the default-value, if it does not exist. Start counting at 0, not 1.
    float GetFloatParameter(aeUInt32 iParameter) const;
    //! Returns the Value of the nth Parameter, or the default-value, if it does not exist. Start counting at 0, not 1.
    const char* GetStringParameter(aeUInt32 iParameter) const;

    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameter(aeInt32 iParam);
    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameter(bool bParam);
    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameter(float fParam);
    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameter(const char* szParam);
    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameter(const char* szParam, aeUInt32 length);
    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameter(const aeBasicString& sParam);
    //! Pushes a Parameter on the Stack to be passed to the next Function called
    void PushParameterNil(void);

    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValue(aeInt32 iParam);
    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValue(bool bParam);
    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValue(float fParam);
    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValue(const char* szParam);
    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValue(const char* szParam, aeUInt32 length);
    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValue(const aeBasicString& sParam);
    //! Pushes a value as a Return value for a called C-Function
    void PushReturnValueNil(void);

    //! Prepares a Function to be called. After that the Parameters can be pushed.
    bool PrepareFunctionCall(const char* szFunctionName, bool bCheckFunctionExistence = true);
    //! Calls the prepared Function with the previously pushed Parameters.
    void CallPreparedFunction(aeUInt32 iExpectedReturnValues = 0);
    //! Call this after you called a prepared lua-function, that returned some values. If zero values were returned, this function is optional.
    void DiscardReturnValues(void);

    //! Checks the nth return-value passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsReturnValueInt(aeUInt32 iReturnValue) const;
    //! Checks the nth return-value passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsReturnValueBool(aeUInt32 iReturnValue) const;
    //! Checks the nth return-value passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsReturnValueFloat(aeUInt32 iReturnValue) const;
    //! Checks the nth return-value passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsReturnValueTable(aeUInt32 iReturnValue) const;
    //! Checks the nth return-value passed to a C-Function for its type. Start counting at 0, not 1.
    bool IsReturnValueString(aeUInt32 iReturnValue) const;

    //! Returns the Value of the nth return-value. Start counting at 0, not 1.
    int GetIntReturnValue(aeUInt32 iReturnValue) const;
    //! Returns the Value of the nth return-value. Start counting at 0, not 1.
    bool GetBoolReturnValue(aeUInt32 iReturnValue) const;
    //! Returns the Value of the nth return-value. Start counting at 0, not 1.
    float GetFloatReturnValue(aeUInt32 iReturnValue) const;
    //! Returns the Value of the nth return-value. Start counting at 0, not 1.
    const char* GetStringReturnValue(aeUInt32 iReturnValue) const;


    //! Return the value of this Function in a called C-Function.
    aeInt32 ReturnToScript(void) const;

    //! Gets the Light-User-Data set for the currently called Function.
    void* GetFunctionLightUserData(void);

    //! Registers a C-Function to the Script under a certain Name. Sets the Lightuserdata for this particular Function.
    void RegisterCFunction(const char* szFunctionName, lua_CFunction pFunction, void* pLightUserData = nullptr) const;

    // USE CALLBACKS !
    //! Returns an array of all variables in the currently open table
    //void EnumVariables (vector<string>& sNames);
    //! Returns an array of all functions in the currently open table
    //void EnumFunctions (vector<string>& sNames);

  private:
    //! An Allocator for Lua. Optimizes (in theory) the allocations.
    static void* lua_allocator(void* ud, void* ptr, size_t osize, size_t nsize);

    //! The Lua-State for the Script.
    lua_State* mp_State;

    //! If this script created the Lua-State, it also releases it on exit.
    bool m_bReleaseOnExit;

    struct AE_GRAPHICS_DLL aeScriptStates
    {
      aeScriptStates(void)
        : m_iParametersPushed(0)
        , m_iOpenTables(0)
        , m_iLuaReturnValues(0)
      {
      }

      //! How many Parameters were pushed for the next function-call.
      aeInt32 m_iParametersPushed;

      //! How many Tables have been opened inside the Lua-Script.
      aeInt32 m_iOpenTables;

      //! How many values the called lua-function should return
      aeInt32 m_iLuaReturnValues;
    };

    aeScriptStates m_States;

    static const aeInt32 s_ParamOffset = 1; // should be one, to start counting at 0, instead of 1
  };
} // namespace AE_NS_FOUNDATION

// has to be included AFTER the declaration
#include "LuaWrapper/LuaWrapper.inl"

#endif
