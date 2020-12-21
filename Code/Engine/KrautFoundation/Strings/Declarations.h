#ifndef AE_FOUNDATION_STRINGS_DECLARATIONS_H
#define AE_FOUNDATION_STRINGS_DECLARATIONS_H

#include "../Defines.h"

namespace AE_NS_FOUNDATION
{

  // *** BasicString.h ***

    class aeBasicString;

  // *** HybridString.h ***

    template<aeUInt32 SIZE>
    class aeHybridString;

  // *** LocaleFixer.h ***

    struct aeLocaleFixer;

  // *** PathFunctions.h ***

    typedef aeHybridString<256> aeFilePath;
    typedef aeHybridString<16> aeFileExtension;
    typedef aeHybridString<64> aeFileName;

    struct aePathFunctions;

  // *** StaticString.h ***

    template<aeUInt16 SIZE>
    class aeStaticString;

  // *** String.h ***

    class aeString;

  // *** StringFunctions.h ***

    struct aeStringFunctions;

}

#endif

