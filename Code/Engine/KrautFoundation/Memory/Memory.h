#ifndef AE_FOUNDATION_MEMORY_MEMORY_H
#define AE_FOUNDATION_MEMORY_MEMORY_H

#include "../Defines.h"

namespace AE_NS_FOUNDATION
{
  //! Provides static functions for working with raw memory.
  struct AE_FOUNDATION_DLL aeMemory
  {
    template<class T>
    static T* GetByteOffsetAddress (void* pBaseAddress, aeInt32 iOffset);

    //! Copies uiBytes bytes from pSource to pDest
    static void Copy (const void* pSource, void* pDest, aeUInt32 uiBytes);

    //! Compares uiBytes of bytes from pSource1 and pSource1. Returns true, if all are equal.
    static bool Compare (const void* pSource1, const void* pSource2, aeUInt32 uiBytes);

    //! Compares uiBytes of bytes from pSource1 and pSource1. Returns 0 if both are equal. Negative values if the first buffer is 'smaller', positive values otherwise.
    static aeInt32 CompareForSorting (const void* pSource1, const void* pSource2, aeUInt32 uiBytes);

    //! Sets the data at pDest to all zero.
    static void FillWithZeros (void* pDest, aeUInt32 uiBytes);

    //! Returns a pointer to a temporary buffer of at least the desired size.
    static void* AccquireTempBuffer (aeUInt32 uiSize);
    //! Flags the temp buffer as released, such that it can be reused by other code.
    static void  ReleaseTempBuffer (void* pBuffer);

    //! Logs the number and size of the used temp-buffers.
    static void LogTempBufferStats (void);

    //! Frees all memory from temp-buffers.
    static void ClearTempBuffers (void);
  };
}

#include "Inline/Memory.inl"

#endif

