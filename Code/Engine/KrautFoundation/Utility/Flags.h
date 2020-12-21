#ifndef AE_FOUNDATION_UTILITY_FLAGS_H
#define AE_FOUNDATION_UTILITY_FLAGS_H

#include "../Defines.h"
//#include "../Streams/Streams.h"

namespace AE_NS_FOUNDATION
{
  //! A helper class to manage bit-flags.
  /*! The class takes a template as the base-type to store the bits.
      For example aeUInt8, aeUInt16, aeUInt32.\n
      Predefined types for this are:\n
      aeFlags8, aeFlags16, aeFlags32\n
  */
  template<class BASETYPE>
  class aeFlags
  {
  public:
    //! Initializes all bits to zero.
    aeFlags (void)                        : m_Data (0) {}
    //! Initializes the data with the given value.
    aeFlags (BASETYPE init)               : m_Data (init) {}
    //! Copies the bit values from the other flags.
    aeFlags (const aeFlags<BASETYPE>& cc) : m_Data (cc.m_Data) {}

    //! Resets all flags to some value. Default is zero, but can be exactly defined.
    void SetFlags (BASETYPE uiFlags = 0)
    {
      m_Data = uiFlags;
    }

    //! Sets all the given flags.
    void RaiseFlags (BASETYPE uiFlags)
    {
      m_Data |= uiFlags;
    }

    //! All the given flags will be set to zero.
    void ClearFlags (BASETYPE uiFlags)
    {
      m_Data &= ~uiFlags;
    }

    //! Sets or unsets the given flags, depending on bRaise.
    void RaiseOrClearFlags (BASETYPE uiFlags, bool bRaise)
    {
      if (bRaise)
        RaiseFlags (uiFlags);
      else
        ClearFlags (uiFlags);
    }

    //! Checks if any of the given flags are set.
    bool IsAnyFlagSet (BASETYPE uiFlags) const
    {
      return ((m_Data & uiFlags) != 0);
    }

    //! Checks if all of the given flags are set.
    bool AreAllFlagsSet (BASETYPE uiFlags) const
    {
      return ((m_Data & uiFlags) == uiFlags);
    }

    //! Returns the pure flag data.
    BASETYPE GetData (void) const
    {
      return (m_Data);
    }

  private:
    BASETYPE m_Data;
  };

  // Export these instanciations of the template from the DLL
  #ifdef AE_COMPILE_ENGINE_AS_DLL
    #pragma warning (push)
    #pragma warning (disable : 4231)

    AE_EXPIMP_TEMPLATE_CLASS aeFlags<aeUInt8>;
    AE_EXPIMP_TEMPLATE_CLASS aeFlags<aeUInt16>;
    AE_EXPIMP_TEMPLATE_CLASS aeFlags<aeUInt32>;

    #pragma warning (pop)
  #endif

  // Predefined Types:
  typedef aeFlags<aeUInt8> aeFlags8;
  typedef aeFlags<aeUInt16> aeFlags16;
  typedef aeFlags<aeUInt32> aeFlags32;
}

#endif
