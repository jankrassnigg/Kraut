#ifndef AE_FOUNDATION_UTILITY_SINGLETONCLASS_H
#define AE_FOUNDATION_UTILITY_SINGLETONCLASS_H

#include "../Defines.h"
#include "../Basics/Checks.h"

namespace AE_NS_FOUNDATION
{
  // This started out as a nice clean templated class. However you cannot pass linkage information (dllimport / dllexport)
  // to templates, which is quite important to make singleton classes available from/to DLLs. So it was then converted
  // to an ugly macro, which does the same as the template: it creates a new class-type that has the static informaation
  // about the one instance. Another class now only has to derive from that class to become a singleton class.

  // Usage is as follows:
  //
  // If you have a class A that you want to be a singleton, add this to its header:
  //
  //   AE_SINGLETON_CLASS (linkage, A); // with linkage being AE_STATIC_LINKAGE or something else, see AE_FOUNDATION_DLL for an example
  //   class A : AE_MAKE_SINGLETON (A)
  //
  // Also add this somewhere in its source-file:
  //
  //   AE_SINGLETON_CLASS_CODE (A);
  //
  // That's it, now the class instance can be queried with 'GetInstance'.

  #define AE_SINGLETON_CLASS(linkage, EC) \
  \
  class EC; \
  \
  class linkage aeSingletonClass_##EC \
  { \
  public: \
    aeSingletonClass_##EC () \
    { \
      AE_CHECK_ALWAYS (s_pInstance == NULL, "Singleton of Class '%s' already exists.", #EC);\
      s_pInstance = this; \
    } \
    \
    virtual ~aeSingletonClass_##EC () \
    { \
      s_pInstance = NULL;\
    } \
    \
    static EC* GetInstance (void) \
    { \
      AE_CHECK_DEV (s_pInstance != NULL, "No Instance of class '%s' exists!", #EC); \
      return ((EC*) s_pInstance); \
    } \
  \
  private: \
    static aeSingletonClass_##EC* s_pInstance; \
  };  

  #define AE_MAKE_SINGLETON(EC) \
    public aeSingletonClass_##EC

  #define AE_SINGLETON_CLASS_CODE(EC) \
    aeSingletonClass_##EC* aeSingletonClass_##EC::s_pInstance = NULL;
}

#endif

