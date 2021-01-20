#include "../CriticalSection.h"

//#ifdef AE_PLATFORM_WINDOWS

#include <windows.h>

namespace AE_NS_FOUNDATION
{
  struct aeCriticalSectionImpl
  {
    bool m_bEntered;
    CRITICAL_SECTION m_CS;
  };

  aeCriticalSection::aeCriticalSection(void)
  {
    m_Impl = new aeCriticalSectionImpl;

    InitializeCriticalSection(&m_Impl->m_CS);
  }

  aeCriticalSection::~aeCriticalSection()
  {
    DeleteCriticalSection(&m_Impl->m_CS);

    delete m_Impl;
  }

  void aeCriticalSection::Enter(void)
  {
    EnterCriticalSection(&m_Impl->m_CS);
  }

  void aeCriticalSection::Leave(void)
  {
    LeaveCriticalSection(&m_Impl->m_CS);
  }

  bool aeCriticalSection::TryEnter(void)
  {
    return (TryEnterCriticalSection(&m_Impl->m_CS) == TRUE);
  }

} // namespace AE_NS_FOUNDATION

//#endif
