#ifndef AE_FOUNDATION_CONFIGURATION_CVAR_INL
#define AE_FOUNDATION_CONFIGURATION_CVAR_INL

namespace AE_NS_FOUNDATION
{
  inline const char* aeCVar::GetName (void) const 
  { 
    return (m_sName.c_str ()); 
  }

  inline const char* aeCVar::GetDescription (void) const 
  { 
    return (m_sDescription.c_str ()); 
  }

  inline aeCVarType::Enum aeCVar::GetType (void) const 
  { 
    return (m_Type); 
  }

  inline const aeFlags32& aeCVar::GetFlags (void) const 
  {
    return (m_Flags); 
  }

  inline aeCVarFloat::aeCVarFloat (const char* szName, float value, aeUInt32 flags, const char* szDescription) 
    : aeCVar (szName, flags, aeCVarType::Float, szDescription)
  {
    m_Value[aeCVarValue::Default] = value;
    m_Value[aeCVarValue::Current] = value;
    m_Value[aeCVarValue::Stored] = value;
    m_Value[aeCVarValue::ToBeStored] = value;
  }

  inline aeCVarInt::aeCVarInt (const char* szName, int value, aeUInt32 flags, const char* szDescription) 
    : aeCVar (szName, flags, aeCVarType::Int, szDescription)
  {
    m_Value[aeCVarValue::Default] = value;
    m_Value[aeCVarValue::Current] = value;
    m_Value[aeCVarValue::Stored] = value;
    m_Value[aeCVarValue::ToBeStored] = value;
  }

  inline aeCVarBool::aeCVarBool (const char* szName, bool value, aeUInt32 flags, const char* szDescription) 
    : aeCVar (szName, flags, aeCVarType::Bool, szDescription)
  {
    m_Value[aeCVarValue::Default] = value;
    m_Value[aeCVarValue::Current] = value;
    m_Value[aeCVarValue::Stored] = value;
    m_Value[aeCVarValue::ToBeStored] = value;
  }

  inline aeCVarString::aeCVarString (const char* szName, const char* value, aeUInt32 flags, const char* szDescription) 
    : aeCVar (szName, flags, aeCVarType::String, szDescription)
  {
    m_Value[aeCVarValue::Default] = value;
    m_Value[aeCVarValue::Current] = value;
    m_Value[aeCVarValue::Stored] = value;
    m_Value[aeCVarValue::ToBeStored] = value;
  }


  inline void aeCVarFloat::operator= (float value)
  {
    m_Value[aeCVarValue::ToBeStored] = value;
    aeCVarEvent e;

    m_Flags.ClearFlags (aeCVarFlags::HasTemporaryValue);

    if (!m_Flags.IsAnyFlagSet (aeCVarFlags::Restart))
    {
      m_Value[aeCVarValue::Current] = value;
      e.m_Type = aeCVarEventType::ValueChanged;
    }
    else
    {
      m_Flags.RaiseOrClearFlags (aeCVarFlags::HasRestartValue, (m_Value[aeCVarValue::ToBeStored] != m_Value[aeCVarValue::Current]));
      e.m_Type = aeCVarEventType::RestartValueChanged;
    }

    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarFloat::ResetToDefault (void)    
  { 
    *this = m_Value[aeCVarValue::Default]; 
  }

  inline void aeCVarFloat::ResetToStored (void)     
  { 
    *this = m_Value[aeCVarValue::Stored]; 
  }
  
  inline void aeCVarFloat::SetToRestartValue (void) 
  { 
    m_Value[aeCVarValue::Current] = m_Value[aeCVarValue::ToBeStored]; 
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue | aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarFloat::SetTemporaryValue (float value)
  {
    m_Value[aeCVarValue::Current] = value;
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue);
    m_Flags.RaiseFlags (aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }


  inline void aeCVarInt::operator= (aeInt32 value)
  {
    m_Value[aeCVarValue::ToBeStored] = value;
    aeCVarEvent e;

    m_Flags.ClearFlags (aeCVarFlags::HasTemporaryValue);

    if (!m_Flags.IsAnyFlagSet (aeCVarFlags::Restart))
    {
      m_Value[aeCVarValue::Current] = value;
      e.m_Type = aeCVarEventType::ValueChanged;
    }
    else
    {
      m_Flags.RaiseOrClearFlags (aeCVarFlags::HasRestartValue, (m_Value[aeCVarValue::ToBeStored] != m_Value[aeCVarValue::Current]));
      e.m_Type = aeCVarEventType::RestartValueChanged;
    }

    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarInt::ResetToDefault (void)    
  { 
    *this = m_Value[aeCVarValue::Default]; 
  }

  inline void aeCVarInt::ResetToStored (void)     
  { 
    *this = m_Value[aeCVarValue::Stored]; 
  }
  
  inline void aeCVarInt::SetToRestartValue (void) 
  { 
    m_Value[aeCVarValue::Current] = m_Value[aeCVarValue::ToBeStored]; 
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue | aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarInt::SetTemporaryValue (aeInt32 value)
  {
    m_Value[aeCVarValue::Current] = value;
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue);
    m_Flags.RaiseFlags (aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarBool::operator= (bool value)
  {
    m_Value[aeCVarValue::ToBeStored] = value;
    aeCVarEvent e;

    m_Flags.ClearFlags (aeCVarFlags::HasTemporaryValue);

    if (!m_Flags.IsAnyFlagSet (aeCVarFlags::Restart))
    {
      m_Value[aeCVarValue::Current] = value;
      e.m_Type = aeCVarEventType::ValueChanged;
    }
    else
    {
      m_Flags.RaiseOrClearFlags (aeCVarFlags::HasRestartValue, (m_Value[aeCVarValue::ToBeStored] != m_Value[aeCVarValue::Current]));
      e.m_Type = aeCVarEventType::RestartValueChanged;
    }

    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarBool::ResetToDefault (void)    
  { 
    *this = m_Value[aeCVarValue::Default]; 
  }

  inline void aeCVarBool::ResetToStored (void)     
  { 
    *this = m_Value[aeCVarValue::Stored]; 
  }
  
  inline void aeCVarBool::SetToRestartValue (void) 
  { 
    m_Value[aeCVarValue::Current] = m_Value[aeCVarValue::ToBeStored]; 
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue | aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarBool::SetTemporaryValue (bool value)
  {
    m_Value[aeCVarValue::Current] = value;
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue);
    m_Flags.RaiseFlags (aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarString::operator= (const aeBasicString& value)
  {
    m_Value[aeCVarValue::ToBeStored] = value;
    aeCVarEvent e;

    m_Flags.ClearFlags (aeCVarFlags::HasTemporaryValue);

    if (!m_Flags.IsAnyFlagSet (aeCVarFlags::Restart))
    {
      m_Value[aeCVarValue::Current] = value;
      e.m_Type = aeCVarEventType::ValueChanged;
    }
    else
    {
      m_Flags.RaiseOrClearFlags (aeCVarFlags::HasRestartValue, (m_Value[aeCVarValue::ToBeStored] != m_Value[aeCVarValue::Current]));
      e.m_Type = aeCVarEventType::RestartValueChanged;
    }

    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarString::ResetToDefault (void)    
  { 
    *this = m_Value[aeCVarValue::Default]; 
  }

  inline void aeCVarString::ResetToStored (void)     
  { 
    *this = m_Value[aeCVarValue::Stored]; 
  }
  
  inline void aeCVarString::SetToRestartValue (void) 
  { 
    m_Value[aeCVarValue::Current] = m_Value[aeCVarValue::ToBeStored]; 
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue | aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }

  inline void aeCVarString::SetTemporaryValue (const char* value)
  {
    m_Value[aeCVarValue::Current] = value;
    m_Flags.ClearFlags (aeCVarFlags::HasRestartValue);
    m_Flags.RaiseFlags (aeCVarFlags::HasTemporaryValue);

    aeCVarEvent e;
    e.m_Type = aeCVarEventType::ValueChanged;
    m_CVarEvent.RaiseEvent (&e);
  }
}

#endif


