// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_DYNAMICTREE_DYNAMICOCTREE_H
#define AE_GRAPHICS_DYNAMICTREE_DYNAMICOCTREE_H

#include "Declarations.h"
#include <KrautFoundation/Math/IncludeAll.h>
//#include <KrautUtility/Culling/Main.h>
#include <map>

namespace AE_NS_FOUNDATION
{

	class aeDynamicOctree
	{
	public:
		AE_FOUNDATION_DLL aeDynamicOctree ();

		AE_FOUNDATION_DLL void CreateTree (const aeVec3& vCenter, const aeVec3& vHalfExtents, float fMinNodeSize);

		//! Adds and object at position vCenter with bounding-box dimensions vHalfExtents to the tree. If the object is outside the tree and bOnlyIfInside is true, nothing will be inserted.
		AE_FOUNDATION_DLL bool InsertObject (const aeVec3& vCenter, const aeVec3& vHalfExtents, aeInt32 iObjectType, aeInt32 iObjectInstance, bool bOnlyIfInside = false);
		//! Adds and object at position vCenter with bounding-box dimensions vHalfExtents to the tree. If the object is outside the tree and bOnlyIfInside is true, nothing will be inserted.
		AE_FOUNDATION_DLL bool InsertObject (const aeVec3& vCenter, const aeVec3& vHalfExtents, aeInt32 iObjectType, aeInt32 iObjectInstance, bool bOnlyIfInside, aeDynamicTreeObject& out_Object);

		//! Calls the Callback for every object that is inside the Viewfrustum. pPassThrough is passed to the Callback for custom purposes.
		//AE_FOUNDATION_DLL void FindVisibleObjects (const aeViewfrustum& Viewfrustum, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough) const;

		//! Returns all objects that are in the same cell as the given point.
		AE_FOUNDATION_DLL void FindObjectsInRange (const aeVec3& vPoint, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough) const;

    //! Returns all objects that are in the same area of the given sphere.
    AE_FOUNDATION_DLL void FindObjectsInRange (const aeVec3& vPoint, float fRadius, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough) const;

		//! Removes the given Object. This is an O(1) operation.
		AE_FOUNDATION_DLL void RemoveObject (aeDynamicTreeObject obj);

		//! Removes the Instance of the given Type from the tree. SLOW: O(n)
		AE_FOUNDATION_DLL void RemoveObject (aeInt32 iObjectType, aeInt32 iObjectInstance);

		//! Removes ALL Instances of a given Type from the tree. O(n) time complexity.
		AE_FOUNDATION_DLL void RemoveObjectsOfType (aeInt32 iObjectType);

		//! Clears the tree from all objects.
		AE_FOUNDATION_DLL void RemoveAllObjects (void) {m_NodeMap.clear ();}

		//! Returns the trees BoundingBox.
		AE_FOUNDATION_DLL void getAABB (float& out_fMinX, float& out_fMaxX, float& out_fMinY, float& out_fMaxY, float& out_fMinZ, float& out_fMaxZ) const;

	private:
		bool InsertObject (const aeVec3& vCenter, const aeVec3& vHalfExtents, const aeObjectData& Obj, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeDynamicTreeObject& out_Object);

		//void FindVisibleObjects (const aeViewfrustum& Viewfrustum, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeUInt32 uiNextNodeID) const;

		bool FindObjectsInRange (const aeVec3& vPoint, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeUInt32 uiNextNodeID) const;

    bool FindObjectsInRange (const aeVec3& vPoint, float fRadius, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeUInt32 uiNextNodeID) const;

		aeUInt32 m_uiMaxTreeDepth;
		aeUInt32 m_uiAddIDTopLevel;

		//! The square bounding Box (to prevent long thin nodes) 
		float m_fMinX, m_fMaxX, m_fMinY, m_fMaxY, m_fMinZ, m_fMaxZ;
		//! The actual bounding box (to discard objects that are outside the world)
		float m_fRealMinX, m_fRealMaxX, m_fRealMinY, m_fRealMaxY, m_fRealMinZ, m_fRealMaxZ;

		
		std::multimap<aeUInt32, aeObjectData> m_NodeMap;
	};
}

#pragma once

#endif


