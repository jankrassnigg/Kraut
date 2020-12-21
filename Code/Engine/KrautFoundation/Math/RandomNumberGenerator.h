#ifndef AE_FOUNDATION_MATH_RANDOMNUMBERGENERATOR_H
#define AE_FOUNDATION_MATH_RANDOMNUMBERGENERATOR_H

#include "Declarations.h"

namespace AE_NS_FOUNDATION
{
  //! A very simple random number generator, that works like the standard "rand" function.
  class AE_FOUNDATION_DLL aeRandomNumberGenerator
  {
  public:
    aeRandomNumberGenerator ()
    {
      m_uiSeedValue = 0;
    }
    //! Returns a new random number. Changes the current seed value.
    aeUInt32 GetRandomNumber (void)
    {
      m_uiSeedValue = 214013L * m_uiSeedValue + 2531011L;
      return ((m_uiSeedValue >> 16) & 0x7FFFF);
    }

    aeUInt32 m_uiSeedValue;
  };
}


#endif


