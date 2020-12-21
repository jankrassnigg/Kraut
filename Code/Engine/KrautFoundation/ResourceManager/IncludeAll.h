// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_MAIN_H
#define AE_CORE_RESOURCEMANAGER_MAIN_H

#include "Declarations.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "ResourceHandle.h"
#include "TypedResourceHandle.h"

namespace AE_NS_FOUNDATION
{

	/*! \todo

	For Quicksaves:
	Allow resources to be stored in ANY file at ANY position -> when serializing the ResourceHandle, store the filename and fileposition as a tuple
	When loading, load filename and position, if not loaded already, open that file, jump to the position, read the embedded resource from there
	For performance, read the tuples, sort them by file and file-position, then load them in sequential order.
	Not trivial to implement.

	*/


}


#endif



