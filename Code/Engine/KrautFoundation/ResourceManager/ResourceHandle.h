// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_RESOURCEHANDLE_H
#define AE_CORE_RESOURCEMANAGER_RESOURCEHANDLE_H

#include "Declarations.h"

namespace AE_NS_FOUNDATION
{
	/*!	The aeResourceHandle implements a reference-counting aeResource-reference. It automatically
		increases and decreases the ref-count of a aeResource, when it is created or destroyed.
		Use this class, instead of direct pointers to RESOURCEs. Everything else would be a bug,
		since resources, that have a ref-count of 0 (no RESOURCE_HANDLEs referencing them), will be
		deleted by the RESOURCE_MANAGER.\n
		Additionally RESOURCE_HANDLEs can be written to a file (savegame / level-file / ...). The
		RESOURCE_MANAGER takes care to store information how to recreate the resource, when the file
		is loaded.\n
		Later one might add functionality to allow RESOURCE_HANDLEs to reference not-yet loaded data,
		to load data in parallel in a separate thread. Although that is not implemented at the moment.
	*/
  class AE_FOUNDATION_DLL aeResourceHandle
	{
	public:
		AE_INLINE aeResourceHandle (void);
		AE_INLINE aeResourceHandle (const aeResourceHandle& cc);
		AE_INLINE explicit aeResourceHandle (aeResource* pResource);

		AE_INLINE ~aeResourceHandle ();

		AE_INLINE void operator= (const aeResourceHandle& cc);
		AE_INLINE void operator= (aeResource* cc);
		AE_INLINE bool operator== (const aeResourceHandle& cc) const;
		AE_INLINE bool operator!= (const aeResourceHandle& cc) const;
		
		//! Returns a pointer to the Resource, this Handle references. Casts it to the desired type.
		template<class RES>
		AE_INLINE const RES* GetResource (void) const;

		//! Returns a pointer to the Resource, this Handle references. Casts it to the desired type.
		template<class RES>
		AE_INLINE RES* GetResource (void);

		//! Checks, whether the aeResourceHandle references a valid aeResource.
		AE_INLINE bool IsValid (void) const;
		//! Makes the aeResourceHandle invalid, meaning the aeResource it references is not used any longer.
		AE_INLINE void Invalidate (void);

		//! Saves the Handle to disk.
		//AE_INLINE void Save (aeStreamOut& pFile) const;
		//! Loads the Handle from disk.
		//AE_INLINE void Load (aeStreamIn& pFile);

	private:
		friend class aeResourceManager;

		//! The aeResourceSaveID can be stored in a file and later be used to restore RESOURCE_HANDLEs.
		aeResourceSaveID GetResourceSaveID (void) const;

		//! The Pointer to the aeResource.
		aeResource* mp_Resource;
	};

}

#include "Inlines/ResourceHandle.inl"

#endif