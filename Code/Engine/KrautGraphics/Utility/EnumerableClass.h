#ifndef AE_FOUNDATION_UTILITY_ENUMERABLECLASS_H
#define AE_FOUNDATION_UTILITY_ENUMERABLECLASS_H

#include <KrautFoundation/Defines.h>

namespace AE_NS_FOUNDATION
{
  // This started out as a nice clean templated class. However you cannot pass linkage information (dllimport / dllexport)
  // to templates, which is quite important to make enumerable classes available from/to DLLs. So it was then converted
  // to an ugly macro, which does the same as the template: it creates a new class-type that has the static informaation
  // about all instances that were created from that class. Another class now only has to derive from that class and will
  // then gain the ability to count and enumerate its instances.

  // Usage is as follows:
  //
  // If you have a class A that you want to be enumerable, add this to its header:
  //
  //   AE_ENUMERABLE_CLASS (linkage, A); // with linkage being AE_STATIC_LINKAGE or something else, see AE_FOUNDATION_DLL for an example
  //   class A : AE_MAKE_ENUMERABLE (A)
  //
  // Also add this somewhere in its source-file:
  //
  //   AE_ENUMERABLE_CLASS_CODE (A);
  //
  // That's it, now the class instances can be enumerated with 'GetFirstInstance' and 'GetNextInstance'

#define AE_ENUMERABLE_CLASS(linkage, EC)                             \
                                                                     \
  class EC;                                                          \
                                                                     \
  class linkage aeEnumerableClass_##EC                               \
  {                                                                  \
  public:                                                            \
    aeEnumerableClass_##EC()                                         \
    {                                                                \
      if (s_pFirstInstance == nullptr)                               \
        s_pFirstInstance = this;                                     \
      else                                                           \
        s_pLastInstance->m_pNextInstance = this;                     \
                                                                     \
      s_pLastInstance = this;                                        \
      m_pNextInstance = nullptr;                                     \
      ++s_uiInstances;                                               \
    }                                                                \
                                                                     \
    virtual ~aeEnumerableClass_##EC()                                \
    {                                                                \
      --s_uiInstances;                                               \
      aeEnumerableClass_##EC* pPrev = nullptr;                       \
      aeEnumerableClass_##EC* pCur = s_pFirstInstance;               \
                                                                     \
      while (pCur)                                                   \
      {                                                              \
        if (pCur == this)                                            \
        {                                                            \
          if (pPrev == nullptr)                                      \
            s_pFirstInstance = m_pNextInstance;                      \
          else                                                       \
            pPrev->m_pNextInstance = m_pNextInstance;                \
                                                                     \
          if (s_pLastInstance == this)                               \
            s_pLastInstance = pPrev;                                 \
                                                                     \
          break;                                                     \
        }                                                            \
                                                                     \
        pPrev = pCur;                                                \
        pCur = pCur->m_pNextInstance;                                \
      }                                                              \
    }                                                                \
                                                                     \
    static EC* GetFirstInstance(void)                                \
    {                                                                \
      return ((EC*)s_pFirstInstance);                                \
    }                                                                \
                                                                     \
    static EC* GetNextInstance(EC* pCur)                             \
    {                                                                \
      aeEnumerableClass_##EC* pThis = (aeEnumerableClass_##EC*)pCur; \
      return ((EC*)(pThis->m_pNextInstance));                        \
    }                                                                \
                                                                     \
    static aeUInt32 GetInstanceCount(void)                           \
    {                                                                \
      return (s_uiInstances);                                        \
    }                                                                \
                                                                     \
  private:                                                           \
    static aeUInt32 s_uiInstances;                                   \
    static aeEnumerableClass_##EC* s_pFirstInstance;                 \
    static aeEnumerableClass_##EC* s_pLastInstance;                  \
    aeEnumerableClass_##EC* m_pNextInstance;                         \
  };

#define AE_MAKE_ENUMERABLE(EC) \
public                         \
  aeEnumerableClass_##EC

#define AE_ENUMERABLE_CLASS_CODE(EC)                                          \
  aeEnumerableClass_##EC* aeEnumerableClass_##EC::s_pFirstInstance = nullptr; \
  aeEnumerableClass_##EC* aeEnumerableClass_##EC::s_pLastInstance = nullptr;  \
  aeUInt32 aeEnumerableClass_##EC::s_uiInstances = 0;
} // namespace AE_NS_FOUNDATION

#endif
