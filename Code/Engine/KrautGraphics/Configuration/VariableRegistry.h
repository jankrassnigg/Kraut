#ifndef AE_FOUNDATION_CONFIGURATION_VARIABLEREGISTRY_H
#define AE_FOUNDATION_CONFIGURATION_VARIABLEREGISTRY_H

#include <KrautFoundation/Containers/HybridArray.h>
#include <KrautFoundation/Containers/Map.h>
#include <KrautFoundation/Defines.h>
#include <KrautFoundation/Strings/String.h>
#include <KrautGraphics/Base.h>
#include <KrautGraphics/Strings/HybridString.h>

namespace AE_NS_FOUNDATION
{
  //! A central registry that can store any type of data under a given name and retrieve it again.
  /*! The idea of this registry is, that it allows to store and retrieve variables of any type.
      The code that stores variables and the one that retrieves them do not need to know of each other. That
      makes it especially useful to store special variables, like in windows the HWND or HINSTANCE, where they
      are created, and other code that might need those values, can query them, without the need to pass the
      variables somewhere directly from system to system.
      Since the variables are stored as simple memory dumps, any type can be stored, which makes it easy
      to encapsulate platform-specific code. This way code that might need some platform specific value can
      simply query it from the registry and does not need to have some platform-dependent interface just
      to configure its startup.
  */
  class AE_GRAPHICS_DLL aeVariableRegistry
  {
  public:
    static void RemoveVariable(const char* szVariablePathAndName);

    static void StoreRaw(const char* szVariablePathAndName, const void* pSourceData, aeUInt32 uiNumBytes);
    static bool RetrieveRaw(const char* szVariablePathAndName, void* pDestData, aeUInt32 uiNumBytes);

    static void StoreFloat(const char* szPathAndName, float var);
    static bool RetrieveFloat(const char* szPathAndName, float& out_Result);

    static void StoreInt(const char* szPathAndName, aeInt32 var);
    static bool RetrieveInt(const char* szPathAndName, aeInt32& out_Result);

    static void StoreBool(const char* szPathAndName, bool var);
    static bool RetrieveBool(const char* szPathAndName, bool& out_Result);

    static void StoreString(const char* szPathAndName, const char* var);
    static bool RetrieveString(const char* szPathAndName, aeString& out_Result);

    static void SetPathRedirection(const char* szPathToRedirect, const char* szRedirectTo);

  public:
    struct aeEntryType
    {
      enum Enum
      {
        Int,
        Float,
        Bool,
        String,
        Raw,
      };
    };

    struct aeRegistryEntry
    {
      aeEntryType::Enum m_Type;
      aeHybridArray<aeUInt8, 8> m_Data;
    };

    static void ClearAllVariables(void);
    static const aeMap<aeHybridString<64>, aeRegistryEntry>& GetAllVariables(void);
  };

} // namespace AE_NS_FOUNDATION

#endif
