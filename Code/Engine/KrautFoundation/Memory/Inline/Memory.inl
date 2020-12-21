#ifndef AE_FOUNDATION_MEMORY_MEMORY_INL
#define AE_FOUNDATION_MEMORY_MEMORY_INL

#include <memory>

namespace AE_NS_FOUNDATION
{
  template<class T>
  AE_INLINE T* aeMemory::GetByteOffsetAddress (void* pBaseAddress, aeInt32 iOffset)
  {
    return ((T*) (((aeUInt8*) pBaseAddress) + iOffset));
  }

  AE_INLINE void aeMemory::Copy (const void* pSource, void* pDest, aeUInt32 uiBytes)
  {
    memcpy (pDest, pSource, uiBytes);
  }

  AE_INLINE bool aeMemory::Compare (const void* pSource1, const void* pSource2, aeUInt32 uiBytes)
  {
    return (memcmp (pSource1, pSource2, uiBytes) == 0);
  }

  AE_INLINE aeInt32 aeMemory::CompareForSorting (const void* pSource1, const void* pSource2, aeUInt32 uiBytes)
  {
    return memcmp (pSource1, pSource2, uiBytes);
  }

  AE_INLINE void aeMemory::FillWithZeros (void* pDest, aeUInt32 uiBytes)
  {
    memset (pDest, 0, uiBytes);
  }

}

#endif

