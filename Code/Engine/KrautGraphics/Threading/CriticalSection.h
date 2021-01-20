#ifndef AE_FOUNDATION_THREADING_CRITICALSECTION_H
#define AE_FOUNDATION_THREADING_CRITICALSECTION_H

#include <KrautGraphics/Base.h>

namespace AE_NS_FOUNDATION
{
  struct aeCriticalSectionImpl;

  //! Encapsulates a critical-section object. Can be used to synchronize thread access to shared resources.
  class AE_GRAPHICS_DLL aeCriticalSection
  {
  public:
    //! Constructor, creates the internal CS object.
    aeCriticalSection();
    //! Destructor, deletes the internal CS object.
    ~aeCriticalSection();

    //! Enters the CS. Waits if necessary.
    void Enter(void);
    //! Leaves the CS.
    void Leave(void);

    //! Tries to enter the CS. If it is locked at the moment, it returns with 'false', otherwise the CS is entered and 'true' is returned.
    bool TryEnter(void);

  private:
    aeCriticalSectionImpl* m_Impl;
  };

  //! A RAII class to enter/leave a critical-section automatically and in an exception-safe way.
  class AE_GRAPHICS_DLL aeCriticalSectionLock
  {
  public:
    //! Enters the given critical section, waits if necessary.
    aeCriticalSectionLock(aeCriticalSection* cs)
    {
      m_pCS = cs;

      m_pCS->Enter();
    }

    //! Leaves the critical section.
    ~aeCriticalSectionLock()
    {
      m_pCS->Leave();
    }

  private:
    aeCriticalSection* m_pCS;
  };

// Helper macro to enter/leave a critical section
#define AE_CRITICALSECTION_BLOCK(cs) aeCriticalSectionLock __aeCSb_(&cs);
} // namespace AE_NS_FOUNDATION

#endif
