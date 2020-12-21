// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_DYNAMICTREE_DECLARATIONS_H
#define AE_GRAPHICS_DYNAMICTREE_DECLARATIONS_H

// for all global Defines
#include "../Defines.h"
#include <map>

namespace AE_NS_FOUNDATION
{
	struct aeObjectData
	{
		aeInt32 m_iObjectType;
		aeInt32 m_iObjectInstance;
	};

  typedef std::multimap<aeUInt32, aeObjectData>::iterator aeDynamicTreeObject;
  typedef std::multimap<aeUInt32, aeObjectData>::const_iterator aeDynamicTreeObjectConst;

	//! Callback type for object queries. Return "false" to abort a search (e.g. when the desired element has been found).
	typedef bool (*AE_VISIBLE_OBJ_CALLBACK) (void* pPassThrough, aeDynamicTreeObjectConst Object);

	class aeDynamicOctree;
	class aeDynamicQuadtree;

	const float GC_LOOSE_FACTOR = 1.1f;


}

#pragma once

#endif


