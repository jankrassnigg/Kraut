#include "../VariableRegistry.h"
#include "../../Containers/Map.h"
#include "../../Containers/HybridArray.h"
#include "../../Strings/HybridString.h"
#include "../../Strings/PathFunctions.h"



namespace AE_NS_FOUNDATION
{
  static aeMap<aeHybridString<64>, aeVariableRegistry::aeRegistryEntry> s_Registry;

  struct aePathRedirection
  {
    aeFilePath m_sSource;
    aeFilePath m_sTarget;
  };

  static aeDeque<aePathRedirection> s_Redirections;

  void aeVariableRegistry::SetPathRedirection (const char* szPathToRedirect, const char* szRedirectTo)
  {
    aePathRedirection pr;
    pr.m_sSource = szPathToRedirect;
    pr.m_sTarget = szRedirectTo;

    pr.m_sSource.ToLowerCase ();
    pr.m_sTarget.ToLowerCase ();

    for (aeUInt32 i = 0; i < s_Redirections.size (); ++i)
    {
      if (s_Redirections[i].m_sSource == pr.m_sSource)
      {
        s_Redirections[i].m_sTarget = pr.m_sTarget;
        return;
      }
    }

    s_Redirections.push_back (pr);
  }

  void RedirectPath (aeFilePath& inout_sPath)
  {
    for (aeUInt32 i = 0; i < s_Redirections.size (); ++i)
    {
      if (inout_sPath.CompareEqual (s_Redirections[i].m_sSource.c_str (), s_Redirections[i].m_sSource.length ()))
      {
        inout_sPath.ReplaceSubString (0, s_Redirections[i].m_sSource.length (), s_Redirections[i].m_sTarget.c_str ());
        return;
      }
    }
  }

  void aeVariableRegistry::RemoveVariable (const char* szVariablePathAndName)
  {
    aeFilePath sPath = szVariablePathAndName;
    sPath.ToLowerCase ();

    s_Registry.erase (sPath);
  }

  void aeVariableRegistry::ClearAllVariables (void)
  {
    s_Registry.clear ();
  }

  const aeMap<aeHybridString<64>, aeVariableRegistry::aeRegistryEntry>& aeVariableRegistry::GetAllVariables (void)
  {
    return s_Registry;
  }

  static void Store (const char* szVariablePathAndName, const void* pSourceData, aeUInt32 uiNumBytes, aeVariableRegistry::aeEntryType::Enum Type)
  {
    aeFilePath sPath = szVariablePathAndName;
    sPath.ToLowerCase ();

    AE_CHECK_DEV (!aePathFunctions::GetFileDirectory (sPath.c_str ()).empty (), "Global Variables (like '%s') are not allowed, please specify a category (like 'System/%s').", szVariablePathAndName, szVariablePathAndName);

    aeVariableRegistry::aeRegistryEntry& entry = s_Registry[szVariablePathAndName];
    entry.m_Type = Type;
    entry.m_Data.resize (uiNumBytes);
    aeMemory::Copy (pSourceData, &entry.m_Data[0], uiNumBytes);
  }

  static bool Retrieve (const char* szVariablePathAndName, void* pDestData, aeUInt32 uiNumBytes, aeVariableRegistry::aeEntryType::Enum Type)
  {
    aeFilePath sPath = szVariablePathAndName;
    sPath.ToLowerCase ();

    aeMap<aeHybridString<64>, aeVariableRegistry::aeRegistryEntry>::iterator it = s_Registry.find (sPath.c_str ());

    if (it == s_Registry.end ())
      return false;

    if (it.value ().m_Type != Type)
      return false;

    if (it.value ().m_Data.size () != uiNumBytes)
      return false;

    aeMemory::Copy (it.value ().m_Data.data (), pDestData, uiNumBytes);

    return true;
  }

  void aeVariableRegistry::StoreRaw (const char* szVariablePathAndName, const void* pSourceData, aeUInt32 uiNumBytes)
  {
    Store (szVariablePathAndName, pSourceData, uiNumBytes, aeEntryType::Raw);
  }

  bool aeVariableRegistry::RetrieveRaw (const char* szVariablePathAndName, void* pDestData, aeUInt32 uiNumBytes)
  {
    return Retrieve (szVariablePathAndName, pDestData, uiNumBytes, aeEntryType::Raw);
  }

  void aeVariableRegistry::StoreFloat (const char* szPathAndName, float var)
  {
    Store (szPathAndName, &var, sizeof (float), aeEntryType::Float);
  }

  bool aeVariableRegistry::RetrieveFloat (const char* szPathAndName, float& out_Result)
  {
    return Retrieve (szPathAndName, &out_Result, sizeof (float), aeEntryType::Float);
  }

  void aeVariableRegistry::StoreInt (const char* szPathAndName, aeInt32 var)
  {
    Store (szPathAndName, &var, sizeof (aeInt32), aeEntryType::Int);
  }

  bool aeVariableRegistry::RetrieveInt (const char* szPathAndName, aeInt32& out_Result)
  {
    return Retrieve (szPathAndName, &out_Result, sizeof (aeInt32), aeEntryType::Int);
  }

  void aeVariableRegistry::StoreBool (const char* szPathAndName, bool var)
  {
    Store (szPathAndName, &var, sizeof (bool), aeEntryType::Bool);
  }

  bool aeVariableRegistry::RetrieveBool (const char* szPathAndName, bool& out_Result)
  {
    return Retrieve (szPathAndName, &out_Result, sizeof (bool), aeEntryType::Bool);
  }

  void aeVariableRegistry::StoreString (const char* szPathAndName, const char* var)
  {
    // no need to store the terminating \0
    Store (szPathAndName, var, aeStringFunctions::GetLength (var), aeEntryType::String);
  }

  bool aeVariableRegistry::RetrieveString (const char* szPathAndName, aeString& out_Result)
  {
    aeFilePath sPath = szPathAndName;
    sPath.ToLowerCase ();

    aeMap<aeHybridString<64>, aeRegistryEntry>::iterator it = s_Registry.find (sPath.c_str ());

    if (it == s_Registry.end ())
      return false;

    if (it.value ().m_Type != aeEntryType::String)
      return false;

    const aeUInt32 uiNumBytes = it.value ().m_Data.size ();

    out_Result.resize (uiNumBytes);

    aeMemory::Copy (it.value ().m_Data.data (), &out_Result[0], uiNumBytes);

    return true;
  }
}

