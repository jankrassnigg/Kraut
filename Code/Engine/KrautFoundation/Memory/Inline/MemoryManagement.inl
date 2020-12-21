#ifndef AE_FOUNDATION_MEMORY_MEMORYMANAGEMENT_INL
#define AE_FOUNDATION_MEMORY_MEMORYMANAGEMENT_INL

#include "../Defines.h"
#include "../Memory/Memory.h"
#include "../Memory/MemoryManagement.h"
#include "../Basics/Checks.h"
#include "../Math/Math.h"

// Make sure new/delete are not already defined, otherwise the placement new functions might break
#ifdef new
  #error "new is already #define'd"
#endif
#ifdef delete
  #error "delete is already #define'd"
#endif

namespace AE_NS_FOUNDATION
{
  AE_INLINE aeMemoryManagement::aeAllocationStats::aeAllocationStats (void)
  {
    m_uiSumMemoryAllocated = 0;
    m_uiSumMemoryDeallocated = 0;
    m_uiCurMemoryAllocated = 0;
    m_uiSumMemoryAllocations = 0;
    m_uiSumMemoryDeallocations = 0;
    m_uiCurMemoryAllocations = 0;
  }

  #ifdef AE_COMPILE_FOR_DEBUG
    AE_INLINE void* aeMemoryManagement::Allocate (aeUInt32 uiBytes, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szType, bool bIgnoreDebugLayer, bool bArray)
    {
      uiBytes += 4;

      void* ptr = malloc (uiBytes);
      AE_CHECK_DEV (ptr != NULL, "Memory allocation of %d bytes failed.", uiBytes - 4);

      if ((!bIgnoreDebugLayer) && (s_bMemoryDebuggingEnabled))
        Allocate_Debug (ptr, uiBytes, szSourceFile, uiLine, szFunction, szType, bArray);

      // store somewhere how large this block of data is
      aeUInt32* pData = (aeUInt32*) ptr;
      *pData = uiBytes;

      if (s_bMemoryDebuggingEnabled)
        *pData |= (aeUInt32) (1 << 31);

      ++pData;
      ptr = (void*) pData;

      m_AllocStats.m_uiSumMemoryAllocated += uiBytes;
      m_AllocStats.m_uiCurMemoryAllocated += uiBytes;
      m_AllocStats.m_uiSumMemoryAllocations++;
      m_AllocStats.m_uiCurMemoryAllocations++;
      m_AllocStats.m_uiMaxMemoryAllocated = aeMath::Max (m_AllocStats.m_uiMaxMemoryAllocated, m_AllocStats.m_uiCurMemoryAllocated);
      m_AllocStats.m_uiMaxMemoryAllocations = aeMath::Max (m_AllocStats.m_uiMaxMemoryAllocations, m_AllocStats.m_uiCurMemoryAllocations);

      return (ptr);
    }

    AE_INLINE void aeMemoryManagement::Deallocate (void* pData, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, bool bIgnoreDebugLayer, bool bArray)
    {
      if (pData == NULL)
        return;

      aeUInt32* pPointer = (aeUInt32*) pData;
      --pPointer;
      aeUInt32 uiBytes = (*pPointer) & (~(aeUInt32) (1 << 31));
      pData = (void*) pPointer;

      const bool bDebugging = ((*pPointer) & ((aeUInt32) (1 << 31))) != 0;
      
      if ((s_bMemoryDebuggingEnabled) && (!bIgnoreDebugLayer) && (bDebugging))
        Deallocate_Debug (pData, szSourceFile, uiLine, szFunction, bArray);

      free (pData);

      m_AllocStats.m_uiSumMemoryDeallocated += uiBytes;
      m_AllocStats.m_uiCurMemoryAllocated -= uiBytes;
      m_AllocStats.m_uiSumMemoryDeallocations++;
      m_AllocStats.m_uiCurMemoryAllocations--;
    }

  #else
    AE_INLINE void* aeMemoryManagement::Allocate (aeUInt32 uiBytes)
    {
      uiBytes += 4;

      void* ptr = malloc (uiBytes);
      AE_CHECK_DEV (ptr != NULL, "Memory allocation of %d bytes failed.", uiBytes - 4);

      // store somewhere how large this block of data is
      aeUInt32* pData = (aeUInt32*) ptr;
      *pData = uiBytes;
      ++pData;
      ptr = (void*) pData;

      m_AllocStats.m_uiSumMemoryAllocated += uiBytes;
      m_AllocStats.m_uiCurMemoryAllocated += uiBytes;
      m_AllocStats.m_uiSumMemoryAllocations++;
      m_AllocStats.m_uiCurMemoryAllocations++;

      return (ptr);
    }

    AE_INLINE void aeMemoryManagement::Deallocate (void* pData)
    {
      if (pData == NULL)
        return;

      aeUInt32* pPointer = (aeUInt32*) pData;
      --pPointer;
      aeUInt32 uiBytes = *pPointer;
      pData = (void*) pPointer;

      m_AllocStats.m_uiSumMemoryDeallocated += uiBytes;
      m_AllocStats.m_uiCurMemoryAllocated -= uiBytes;
      m_AllocStats.m_uiSumMemoryDeallocations++;
      m_AllocStats.m_uiCurMemoryAllocations--;

      free (pData);
    }
  #endif

  #ifdef AE_COMPILE_FOR_DEBUG
    template<class TYPE>
    TYPE* aeMemoryManagement::Allocate (aeUInt32 uiElements, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szType, bool bIgnoreDebugLayer)
    {
      return ((TYPE*) Allocate (sizeof (TYPE) * uiElements, szSourceFile, uiLine, szFunction, szType, bIgnoreDebugLayer, false));
    }
  #else
    template<class TYPE>
    TYPE* aeMemoryManagement::Allocate (aeUInt32 uiElements)
    {
      return ((TYPE*) Allocate (sizeof (TYPE) * uiElements));
    }
  #endif

  template<class TYPE>
  void aeMemoryManagement::CopyConstruct (void* pInPlace, const void* pSource, aeUInt32 uiElements)
  {
    TYPE* pTypePtr = (TYPE*) pInPlace;
    const TYPE* pTypeCopyPtr = (const TYPE*) pSource;

    for (aeUInt32 i = 0; i < uiElements; ++i)
    {
      new (&pTypePtr[i]) TYPE (pTypeCopyPtr[i]);
    }
  }

  template<class TYPE>
  void aeMemoryManagement::CopyConstruct (void* pInPlace, const TYPE& init)
  {
    new (pInPlace) TYPE (init);
  }

  template<class TYPE>
  void aeMemoryManagement::Construct (void* pInPlace, aeUInt32 uiElements)
  {
    // just create one element in place
    if (uiElements <= 1)
    {
      new (pInPlace) TYPE;
      return;
    }

    #pragma warning (push)
    #pragma warning (disable : 4345)

    // or new up an array of elements
    aeUInt8* pData = (aeUInt8*) pInPlace;
    for (aeUInt32 ui = 0; ui < uiElements; ++ui)
    {
      new (pData) TYPE ();
      pData += sizeof (TYPE);
    }

    #pragma warning (pop)
  }

  template<class TYPE>
  void aeMemoryManagement::Construct (void* pInPlace, aeUInt32 uiElements, const TYPE& Init)
  {
    // just create one element in place
    if (uiElements <= 1)
    {
      new (pInPlace) TYPE (Init);
      return;
    }

    #pragma warning (push)
    #pragma warning (disable : 4345)

    // or new up an array of elements
    aeUInt8* pData = (aeUInt8*) pInPlace;
    for (aeUInt32 ui = 0; ui < uiElements; ++ui)
    {
      new (pData) TYPE (Init);
      pData += sizeof (TYPE);
    }

    #pragma warning (pop)
  }

  template<class TYPE>
  void aeMemoryManagement::Destruct (void* pInPlace, aeUInt32 uiElements)
  {
    if (!pInPlace)
      return;

    TYPE* pData = (TYPE*) pInPlace;

    // just destroy one element in place
    if (uiElements <= 1)
    {
      pData[0].~TYPE ();
      return;
    }

    // or destroy an array of elements
    for (aeUInt32 ui = 0; ui < uiElements; ++ui)
      pData[ui].~TYPE ();
  }

  // specialized versions for simple data types that do not need any construction / destruction
  template<> AE_INLINE void aeMemoryManagement::Construct<aeInt8>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeInt16>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeInt32>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeInt64>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeUInt8>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeUInt16>(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeUInt32>(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<aeUInt64>(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<float>	  (void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<double>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Construct<bool>		(void* pInPlace, aeUInt32 uiElements) {}

  template<> AE_INLINE void aeMemoryManagement::Destruct<aeInt8>		(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeInt16>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeInt32>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeInt64>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeUInt8>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeUInt16>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeUInt32>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<aeUInt64>	(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<float>		(void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<double>	  (void* pInPlace, aeUInt32 uiElements) {}
  template<> AE_INLINE void aeMemoryManagement::Destruct<bool>	    (void* pInPlace, aeUInt32 uiElements) {}

  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeInt8>	(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt8));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeInt16>	(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt16));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeInt32>	(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt32));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeInt64>	(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt64));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeUInt8>	(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt8));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeUInt16>(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt16));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeUInt32>(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt32));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<aeUInt64>(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (aeInt64));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<float>	  (void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (float));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<double>	(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (double));}
  template<> AE_INLINE void aeMemoryManagement::CopyConstruct<bool>		(void* pInPlace, const void* pSource, aeUInt32 uiElements) {aeMemory::Copy (pSource, pInPlace, uiElements * sizeof (bool));}

}

#endif

