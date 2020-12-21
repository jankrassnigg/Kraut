// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_UTILITY_CULLING_DECLARATIONS_H
#define AE_UTILITY_CULLING_DECLARATIONS_H

// Include this as the first Header everywhere
#include "../Base.h"

namespace AE_NS_GRAPHICS
{
	enum AE_CULL_VOLUME_CHECK
	{
		AE_OBJECT_OUTSIDE_VOLUME,		//! means an object is ENTIRELY inside a volume (view-frustum)
		AE_OBJECT_INSIDE_VOLUME,		//! means an object is entirely outside a volume
		AE_OBJECT_INTERSECTS_VOLUME,	//! means an object is PARTIALLY inside/outside a volume
	};

	class aeViewfrustum;
	struct aeAABBComputer;

	typedef aeUInt32 aeVisibilityMarker;
}

#pragma once

#endif



