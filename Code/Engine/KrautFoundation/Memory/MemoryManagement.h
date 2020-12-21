#ifndef AE_FOUNDATION_MEMORY_MEMORYMANAGEMENT_H
#define AE_FOUNDATION_MEMORY_MEMORYMANAGEMENT_H

#include "../Defines.h"
#include "../Utility/Flags.h"
#include "../Math/Math.h"
#include <memory>

namespace AE_NS_FOUNDATION
{
  struct AE_FOUNDATION_DLL aeMemoryData
  {
    const char* m_szSourceFile;
    const char* m_szFunction;
    unsigned short m_uiLine;
    aeFlags16 m_Flags;
    unsigned int m_uiBytes;
    const char* m_szType;
  };

  //! All Memory Management should go through this class.
  class AE_FOUNDATION_DLL aeMemoryManagement
  {
  public:
    static void EnableMemoryDebugging (bool b) { s_bMemoryDebuggingEnabled = b; }

    #ifdef AE_COMPILE_FOR_DEBUG
      //! [internal] Do not use directly, instead use Macro AE_MEMORY_ALLOCATE
      static void* Allocate (aeUInt32 uiBytes, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szType, bool bIgnoreDebugLayer, bool bArray);
      //! [internal] Do not use directly, instead use Macro AE_MEMORY_DEALLOCATE
      static void Deallocate (void* pData, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, bool bIgnoreDebugLayer, bool bArray);

      //! [internal] Do not use directly, instead use Macro AE_MEMORY_ALLOCATE_TYPE
      template<class TYPE>
      static TYPE* Allocate (aeUInt32 uiElements, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szType, bool bIgnoreDebugLayer);
    #else
      //! [internal] Do not use directly, instead use Macro AE_MEMORY_DEALLOCATE
      //! [internal] Do not use rectly, instead use Macro AE_MEMORY_ALLOCATE
      static void* Allocate (aeUInt32 uiBytes);
      static void Deallocate (void* pData);

      //! [internal] Do not use directly, instead use Macro AE_MEMORY_ALLOCATE_TYPE
      template<class TYPE>
      static TYPE* Allocate (aeUInt32 uiElements);

    #endif

    //! Constructs uiElements of TYPE at pInPlace by copy-constructing the elements from pSource.
    template<class TYPE>
    static void CopyConstruct (void* pInPlace, const void* pSource, aeUInt32 uiElements);

    //! Constructs one TYPE at address pInPlace via copy-construction of "init".
    template<class TYPE>
    static void CopyConstruct (void* pInPlace, const TYPE& init);

    //! Constructs uiElements elements of TYPE at pInPlace via default construction.
    template<class TYPE>
    static void Construct (void* pInPlace, aeUInt32 uiElements = 0);

    //! Constructs uiElements elements of TYPE at pInPlace via default construction.
    template<class TYPE>
    static void Construct (void* pInPlace, aeUInt32 uiElements, const TYPE& Init);

    //! Destructs uiElements elements of TYPE at pInPlace.
    template<class TYPE>
    static void Destruct (void* pInPlace, aeUInt32 uiElements = 0);

    static void PrintMemoryLeaks (void);

  public:

    //! Stores information about all memory allocations.
    struct AE_FOUNDATION_DLL aeAllocationStats
    {
      aeAllocationStats (void);

      aeUInt32 m_uiSumMemoryAllocated;			//!< How much memory was allocated over time (summed up). Never decreases.
      aeUInt32 m_uiSumMemoryDeallocated;		//!< How much memory was deallocated over time (summed up). Never decreases.
      aeUInt32 m_uiCurMemoryAllocated;			//!< How much memory is currently allocated.
      aeUInt32 m_uiMaxMemoryAllocated;			//!< The maximum amount of memory that was allocated at some time.
      aeUInt32 m_uiMaxMemoryAllocations;		//!< The maximum number of memory allocations that were done at some time.
      aeUInt32 m_uiSumMemoryAllocations;		//!< How many memory allocations were ever done. Never decreases.
      aeUInt32 m_uiSumMemoryDeallocations;	//!< How many memory deallocations were ever done. Never decreases.
      aeUInt32 m_uiCurMemoryAllocations;		//!< The number of current memory allocations.
    };

    //! Retrieves the current stats of the memory consumption.
    static aeAllocationStats GetAllocationStats (void) { return (m_AllocStats); }

  #ifdef AE_COMPILE_FOR_DEBUG

    public:

      //! Callback that sends the user one entry of the memory bookkeeping data (one allocation).
      typedef void (*GET_MEMORY_BOOKKEEPING_DATA_CB)(void* pPtr, const aeMemoryData&);

      //! Returns all the data about current memory allocations by sending it to the given callback.
      static void GetMemoryBookkeepingData (GET_MEMORY_BOOKKEEPING_DATA_CB Callback);

      //! [internal] Flags for memory allocations.
      enum aeMemAllocFlags_f
      {
        AE_MAF_NONE				= 0,
        AE_MAF_ARRAY			= 1 << 0,		//!< Memory represents an array (allocated using new[])
      };

  #endif

  private:
    #ifdef AE_COMPILE_FOR_DEBUG
      //! [internal] Stores information about an allocation.
      static void Allocate_Debug   (void* pData, aeUInt32 uiBytes, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szType, bool bArray);
      //! [internal] Removes information about an allocation, and checks that no errors occured. Might assert.
      static void Deallocate_Debug (void* pData, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, bool bArray);
    #endif

    static aeAllocationStats m_AllocStats;

    static bool s_bMemoryDebuggingEnabled;
  };

  #ifdef AE_COMPILE_FOR_DEBUG
    //! Macro that allocates "bytes" bytes of memory. Use this for raw memory allocations, instead of malloc.
    #define AE_MEMORY_ALLOCATE(bytes) aeMemoryManagement::Allocate (bytes, __FILE__, __LINE__, __FUNCTION__, "raw memory", false, false)
    //! [internal] Macro that allocates "elements" instances of "type". Used inside the Foundation-Lib. Not necessary to use, use C++ new instead.
    #define AE_MEMORY_ALLOCATE_TYPE(type, elements) aeMemoryManagement::Allocate<type> (elements, __FILE__, __LINE__, __FUNCTION__, #type, false)
    //! Macro to deallocate raw memory that was allocated using AE_MEMORY_ALLOCATE.
    #define AE_MEMORY_DEALLOCATE(ptr) aeMemoryManagement::Deallocate (ptr, __FILE__, __LINE__, __FUNCTION__, false, false)

    //! [internal] Do not use.
    #define AE_MEMORY_ALLOCATE_NODEBUG(bytes, bNoDebug) aeMemoryManagement::Allocate (bytes, __FILE__, __LINE__, __FUNCTION__, "raw memory", bNoDebug, false)
    //! [internal] Do not use.
    #define AE_MEMORY_ALLOCATE_TYPE_NODEBUG(type, elements, bNoDebug) aeMemoryManagement::Allocate<type> (elements, __FILE__, __LINE__, __FUNCTION__, #type, bNoDebug)
    //! [internal] Do not use.
    #define AE_MEMORY_DEALLOCATE_NODEBUG(ptr, bNoDebug) aeMemoryManagement::Deallocate (ptr, __FILE__, __LINE__, __FUNCTION__, bNoDebug, false)
  #else
    //! Macro that allocates "bytes" bytes of memory. Use this for raw memory allocations, instead of malloc.
    #define AE_MEMORY_ALLOCATE(bytes) aeMemoryManagement::Allocate (bytes)
    //! [internal] Macro that allocates "elements" instances of "type". Used inside the Foundation-Lib. Not necessary to use, use C++ new instead.
    #define AE_MEMORY_ALLOCATE_TYPE(type, elements) aeMemoryManagement::Allocate<type> (elements)
    //! Macro to deallocate raw memory that was allocated using AE_MEMORY_ALLOCATE.
    #define AE_MEMORY_DEALLOCATE(ptr) aeMemoryManagement::Deallocate (ptr)

    //! [internal] Do not use.
    #define AE_MEMORY_ALLOCATE_NODEBUG(bytes, bNoDebug) aeMemoryManagement::Allocate (bytes)
    //! [internal] Do not use.
    #define AE_MEMORY_ALLOCATE_TYPE_NODEBUG(type, elements, bNoDebug) aeMemoryManagement::Allocate<type> (elements)
    //! [internal] Do not use.
    #define AE_MEMORY_DEALLOCATE_NODEBUG(ptr, bNoDebug) aeMemoryManagement::Deallocate (ptr)

  #endif


  #ifdef AE_COMPILE_FOR_DEBUG
    //! [internal] Structure to pass some debug info to operator delete.
    struct aeDeleteData
    {
      const char* m_szFile;
      const char* m_szFunction;
      unsigned int m_uiLine;
    };

    //! [internal] Structure used to get some debugging info to operator delete.
    AE_FOUNDATION_DLL extern aeDeleteData g_DeleteData;
  #endif
}

#include "Inline/MemoryManagement.inl"

#endif

