#ifndef AE_FOUNDATION_CONFIGURATION_CVAR_H
#define AE_FOUNDATION_CONFIGURATION_CVAR_H

#include "../Communication/Event.h"
#include "../Strings/HybridString.h"
#include "../Strings/StaticString.h"
#include "../Strings/String.h"
#include "../Utility/Flags.h"
#include "../Utility/Enum.h"
#include "../Utility/LuaWrapper.h"
#include "../Utility/EnumerableClass.h"

namespace AE_NS_FOUNDATION
{
  AE_ENUMERABLE_CLASS (AE_FOUNDATION_DLL, aeCVar);

  //! Enum that describes the type of a CVar.
  struct aeCVarType
  {
    enum Enum
    {
      None, 
      Int, 
      Bool, 
      String, 
      Float,
    };
  };

  //! Enum that describes the flags that a CVar can have.
  struct aeCVarFlags
  {
    enum Flags
    {
      None                  = 0,        //!< No Flag at all
      DevOnly               = 1 << 0,   //!< Only in Developer-Mode visible and writeble (else not listed in Console)
      Save                  = 1 << 1,   //!< Save the Value in a file to be able to load it
      Restart               = 1 << 2,   //!< Puts out a message, that the Engine needs to be restarted (such Vars are implicitly aeCVarFlags::Save)
      HasRestartValue       = 1 << 3,   //!< This flag will be set, when the CVar has the restart-flag and its original value has changed.
      HasTemporaryValue     = 1 << 4,   //!< This flag will be set, when the CVar is set to a "temporary" value.
    };
  };

  struct aeCVarValue
  {
    enum Enum
    {
      Default,
      Current,
      Stored,
      ToBeStored,
      ENUM_COUNT
    };
  };

    //! Enum to describe the type of events that can occur on a CVar.
  struct aeCVarEventType
  {
    enum Enum
    {
      ValueChanged,           //!< A CVars value has been changed.
      RestartValueChanged,    //!< A CVars value has been changed, but only after a "restart" will it be visible.
    };
  };

  //! Base-class for all CVar-types.
  class aeCVar : AE_MAKE_ENUMERABLE (aeCVar)
  {
  public:

    AE_FOUNDATION_DLL aeCVar (const char* szName, aeUInt32 flags, aeCVarType::Enum type, const char* szDescription);

    //! Returns the name of this CVar.
    AE_FOUNDATION_DLL const char* GetName (void) const;
    //! Returns the descriptive text of this CVar.
    AE_FOUNDATION_DLL const char* GetDescription (void) const;
    //! Returns the type of this CVar.
    AE_FOUNDATION_DLL aeCVarType::Enum GetType (void) const;
    //! Returns all the flags of this CVar.
    AE_FOUNDATION_DLL const aeFlags32& GetFlags (void) const;
  
    //! Returns the CVar with the given name, or NULL if that does not exist.
    AE_FOUNDATION_DLL static aeCVar* GetCVar (const char* szName);

    //! Registers functions in the given Lua-Script, such that it can directly read/write CVars.
    AE_FOUNDATION_DLL static void AllowScriptAccessToCVars (aeLuaWrapper& Script);

    //! Resets the CVar to its default value defined by the program.
    AE_FOUNDATION_DLL virtual void ResetToDefault (void) = 0;
    //! Resets the CVar to the value that is currently stored on file.
    AE_FOUNDATION_DLL virtual void ResetToStored (void) = 0;
    //! Sets the CVars current value to its restart value.
    AE_FOUNDATION_DLL virtual void SetToRestartValue (void) = 0;

    //! Writes all current values of the CVars to the given config-file.
    AE_FOUNDATION_DLL static bool SaveCVars (const char* szSaveToFile);
    //! Reads all current values of the CVars from the given config-file.
    AE_FOUNDATION_DLL static bool LoadCVars (const char* szLoadFromFile);

    //! The data sent through the aeEvent m_CVarEvent.
    struct aeCVarEvent
    {
      //! The events type.
      aeCVarEventType::Enum m_Type;
    };

    //! Event instance of each CVar. Interested clients can register here to be informed about the change of a cvar.
    aeEvent m_CVarEvent;

  protected:
    aeStaticString<32> m_sName;
    aeHybridString<64> m_sDescription;

    aeEnum<aeCVarType::Enum, aeUInt8> m_Type;
    aeFlags32 m_Flags;
  };

  //! A CVar that holds a float variable.
  class aeCVarFloat : public aeCVar
  {
  public:
    aeCVarFloat (const char* szName, float value, aeUInt32 flags, const char* szDescription);

    void operator= (float value);

    operator float () const { return (m_Value[aeCVarValue::Current]); }

    float GetValue (aeCVarValue::Enum val = aeCVarValue::Current) const { return (m_Value[val]); }

    //! Sets only the CURRENT value, but not the to-be-stored value.
    void SetTemporaryValue (float value);

    virtual void ResetToDefault (void);
    virtual void ResetToStored (void);
    virtual void SetToRestartValue (void);

  private:
    friend class aeCVar;
    
    float m_Value[aeCVarValue::ENUM_COUNT];
  };

  //! A CVar that holds an int variable.
  class aeCVarInt : public aeCVar
  {
  public:
    aeCVarInt (const char* szName, int value, aeUInt32 flags, const char* szDescription);

    void operator= (aeInt32 value);

    operator aeInt32 () const { return (m_Value[aeCVarValue::Current]); }

    aeInt32 GetValue (aeCVarValue::Enum val = aeCVarValue::Current) const { return (m_Value[val]); }

    //! Sets only the CURRENT value, but not the to-be-stored value.
    void SetTemporaryValue (aeInt32 value);

    virtual void ResetToDefault (void);
    virtual void ResetToStored (void);
    virtual void SetToRestartValue (void);

  private:
    friend class aeCVar;

    aeInt32 m_Value[aeCVarValue::ENUM_COUNT];
  };

  //! A CVar that holds a bool variable.
  class aeCVarBool : public aeCVar
  {
  public:
    aeCVarBool (const char* szName, bool value, aeUInt32 flags, const char* szDescription);

    void operator= (bool value);

    operator bool () const { return (m_Value[aeCVarValue::Current]); }

    bool GetValue (aeCVarValue::Enum val = aeCVarValue::Current) const { return (m_Value[val]); }

    //! Sets only the CURRENT value, but not the to-be-stored value.
    void SetTemporaryValue (bool value);

    virtual void ResetToDefault (void);
    virtual void ResetToStored (void);
    virtual void SetToRestartValue (void);

  private:
    friend class aeCVar;

    bool m_Value[aeCVarValue::ENUM_COUNT];
  };

  //! A CVar that holds a string variable.
  class aeCVarString : public aeCVar
  {
  public:
    aeCVarString (const char* szName, const char* value, aeUInt32 flags, const char* szDescription);

    void operator= (const aeBasicString& value);

    operator const char* () const { return (m_Value[aeCVarValue::Current].c_str ()); }

    const char* GetValue (aeCVarValue::Enum val = aeCVarValue::Current) const { return (m_Value[val].c_str ()); }

    //! Sets only the CURRENT value, but not the to-be-stored value.
    void SetTemporaryValue (const char* value);

    virtual void ResetToDefault (void);
    virtual void ResetToStored (void);
    virtual void SetToRestartValue (void);

  private:
    friend class aeCVar;

    aeString m_Value[aeCVarValue::ENUM_COUNT];
  };
  
}

#include "Inline/CVar.inl"

#endif


