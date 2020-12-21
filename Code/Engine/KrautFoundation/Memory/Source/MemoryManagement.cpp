#include "../../Containers/Map.h"
#include "../MemoryManagement.h"


namespace AE_NS_FOUNDATION
{
  aeMemoryManagement::aeAllocationStats aeMemoryManagement::m_AllocStats;
  bool aeMemoryManagement::s_bMemoryDebuggingEnabled = false;

  #ifdef AE_COMPILE_FOR_DEBUG
    
    typedef aeMap<void*, aeMemoryData, aeCompareLess<void*>, true> AE_MEMORY_BOOKKEEPER;
    AE_MEMORY_BOOKKEEPER s_MemoryBookkeepingMalloc;

    void aeMemoryManagement::GetMemoryBookkeepingData (GET_MEMORY_BOOKKEEPING_DATA_CB Callback)
    {
      AE_MEMORY_BOOKKEEPER::iterator itend = s_MemoryBookkeepingMalloc.end ();
      for (AE_MEMORY_BOOKKEEPER::iterator it = s_MemoryBookkeepingMalloc.begin (); it != itend; ++it)
      {
        Callback (it.key (), it.value ());
      }
    }

    void aeMemoryManagement::Allocate_Debug (void* pData, aeUInt32 uiBytes, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szType, bool bArray)
    {
      aeMemoryData md;
      md.m_szSourceFile = szSourceFile;
      md.m_szFunction = szFunction;
      md.m_uiLine = uiLine;
      md.m_uiBytes = uiBytes;
      md.m_szType = szType;

      md.m_Flags.RaiseOrClearFlags (AE_MAF_ARRAY, bArray);

      s_MemoryBookkeepingMalloc[pData] = md;
    }

    void aeMemoryManagement::Deallocate_Debug (void* pData, const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, bool bArray)
    {
      if (!pData)
        return;

      AE_MEMORY_BOOKKEEPER::iterator it = s_MemoryBookkeepingMalloc.find (pData);

      AE_CHECK_ALWAYS (it != s_MemoryBookkeepingMalloc.end (), "%s(%d) : error 2: In Function \"%s\" : The given address %p has not been allocated previously.\n", szSourceFile, uiLine, szFunction, pData);

      if (it != s_MemoryBookkeepingMalloc.end ())
      {
        if (!bArray)
        {
          AE_CHECK_ALWAYS (it.value ().m_Flags.IsAnyFlagSet (AE_MAF_ARRAY) == false, "%s(%d) : error 3: In Function \"%s\" : The given address %p was allocated using array-new but deallocated using non-array-delete.\n", szSourceFile, uiLine, szFunction, pData);
        }
        else
        {
          AE_CHECK_ALWAYS (it.value ().m_Flags.IsAnyFlagSet (AE_MAF_ARRAY) == true,"%s(%d) : error 4: In Function \"%s\" : The given address %p was allocated using non-array-new but deallocated using array-delete.\n", szSourceFile, uiLine, szFunction, pData);
        }

        s_MemoryBookkeepingMalloc.erase (it);
      }
    }

    void aeMemoryManagement::PrintMemoryLeaks (void)
    {
      AE_MEMORY_BOOKKEEPER::iterator it = s_MemoryBookkeepingMalloc.begin ();

      printf (" *** Memory Leaks *** \n");
      for (; it != s_MemoryBookkeepingMalloc.end (); ++it)
      {
        printf ("%s(%d) : error 1: Pointer %p (Type \"%s\"), allocated in Function \"%s\" was never deallocated.\n", it.value ().m_szSourceFile, it.value ().m_uiLine, it.key (), it.value ().m_szType, it.value ().m_szFunction);
      }

      printf (" *** Memory Leaks *** \n");

    }

  #else

    void aeMemoryManagement::PrintMemoryLeaks (void)
    {
    }

  #endif

}



// undefine new/delete
#ifdef new
  #undef new
#endif
#ifdef delete
  #undef delete
#endif


#ifdef AE_COMPILE_FOR_DEBUG

  namespace AE_NS_FOUNDATION
  {
    AE_FOUNDATION_DLL aeDeleteData g_DeleteData;
  }

#endif

