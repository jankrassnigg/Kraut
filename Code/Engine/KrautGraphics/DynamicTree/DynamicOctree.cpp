// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "DynamicOctree.h"
#include <KrautFoundation/Math/Vec3.h>
#include <KrautGraphics/Logging/Log.h>

namespace AE_NS_FOUNDATION
{
  aeDynamicOctree::aeDynamicOctree()
    : m_uiMaxTreeDepth(0)
    , m_uiAddIDTopLevel(0)
  {
  }

  void aeDynamicOctree::CreateTree(const aeVec3& vCenter, const aeVec3& vHalfExtents, float fMinNodeSize)
  {
    m_NodeMap.clear();

    // the real bounding box might be long and thing -> bad node-size
    // but still it can be used to reject inserting objects that are entirely outside the world
    m_fRealMinX = vCenter.x - vHalfExtents.x;
    m_fRealMaxX = vCenter.x + vHalfExtents.x;
    m_fRealMinY = vCenter.y - vHalfExtents.y;
    m_fRealMaxY = vCenter.y + vHalfExtents.y;
    m_fRealMinZ = vCenter.z - vHalfExtents.z;
    m_fRealMaxZ = vCenter.z + vHalfExtents.z;

    // the bounding box should be square, so use the maximum of the x, y and z extents
    float fMax = aeMath::Max(vHalfExtents.x, aeMath::Max(vHalfExtents.y, vHalfExtents.z));

    m_fMinX = vCenter.x - fMax;
    m_fMaxX = vCenter.x + fMax;
    m_fMinY = vCenter.y - fMax;
    m_fMaxY = vCenter.y + fMax;
    m_fMinZ = vCenter.z - fMax;
    m_fMaxZ = vCenter.z + fMax;

    float fLength = fMax * 2.0f;

    m_uiMaxTreeDepth = 0;
    while (fLength > fMinNodeSize)
    {
      ++m_uiMaxTreeDepth;
      fLength = (fLength / 2.0f) * GC_LOOSE_FACTOR;
    }

    m_uiAddIDTopLevel = 0;
    for (aeUInt32 i = 0; i < m_uiMaxTreeDepth; ++i)
      m_uiAddIDTopLevel += aeMath::Pow(8, i);


    aeLog::Log("Dynamic Octree: Cube [%.2f]³, Depth: %d", fMax * 2.0f, m_uiMaxTreeDepth);
  }

  void aeDynamicOctree::getAABB(float& out_fMinX, float& out_fMaxX, float& out_fMinY, float& out_fMaxY, float& out_fMinZ, float& out_fMaxZ) const
  {
    out_fMinX = m_fMinX;
    out_fMaxX = m_fMaxX;
    out_fMinY = m_fMinY;
    out_fMaxY = m_fMaxY;
    out_fMinZ = m_fMinZ;
    out_fMaxZ = m_fMaxZ;
  }


  bool aeDynamicOctree::InsertObject(const aeVec3& vCenter, const aeVec3& vHalfExtents, aeInt32 iObjectType, aeInt32 iObjectInstance, bool bOnlyIfInside)
  {
    aeDynamicTreeObject temp;
    return (InsertObject(vCenter, vHalfExtents, iObjectType, iObjectInstance, bOnlyIfInside, temp));
  }

  /*! The object lies at vCenter and has vHalfExtents as its bounding box.
  If bOnlyIfInside is false, the object is ALWAYS inserted, even if it is outside the tree.
  \note In such a case it is inserted at the root-node and thus ALWAYS returned in range/viewfrustum queries.

  If bOnlyIfInside is true, the object is discarded, if it is not inside the actual bounding box of the tree.

  The min and max Y value of the trees bounding box is updated, if the object lies above/below previously inserted objects.

  \return Returns true, if the object was inserted, false otherwise.
  */
  bool aeDynamicOctree::InsertObject(const aeVec3& vCenter, const aeVec3& vHalfExtents, aeInt32 iObjectType, aeInt32 iObjectInstance, bool bOnlyIfInside, aeDynamicTreeObject& out_Object)
  {
    out_Object = m_NodeMap.end();

    if (bOnlyIfInside)
    {
      if (vCenter.x + vHalfExtents.x < m_fRealMinX)
        return (false);
      if (vCenter.x - vHalfExtents.x > m_fRealMaxX)
        return (false);

      if (vCenter.y + vHalfExtents.y < m_fRealMinY)
        return (false);
      if (vCenter.y - vHalfExtents.y > m_fRealMaxY)
        return (false);

      if (vCenter.z + vHalfExtents.z < m_fRealMinZ)
        return (false);
      if (vCenter.z - vHalfExtents.z > m_fRealMaxZ)
        return (false);
    }

    aeObjectData oData;
    oData.m_iObjectType = iObjectType;
    oData.m_iObjectInstance = iObjectInstance;

    // insert the object into the best child
    if (!InsertObject(vCenter, vHalfExtents, oData, m_fMinX, m_fMaxX, m_fMinY, m_fMaxY, m_fMinZ, m_fMaxZ, 0, m_uiAddIDTopLevel, aeMath::Pow(8, m_uiMaxTreeDepth - 1), out_Object))
    {
      if (!bOnlyIfInside)
      {
        // outside the tree, but needs to be inserted -> insert at root-node
        aeLog::Dev("Object outside Root-Node, inserted at Root-Node.");

        out_Object = m_NodeMap.insert(std::pair<aeUInt32, aeObjectData>(0, oData));
        return (true);
      }

      return (false);
    }

    return (true);
  }

  bool aeDynamicOctree::InsertObject(const aeVec3& vCenter, const aeVec3& vHalfExtents, const aeObjectData& Obj, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeDynamicTreeObject& out_Object)
  {
    if (vCenter.x - vHalfExtents.x < minx)
      return (false);
    if (vCenter.x + vHalfExtents.x > maxx)
      return (false);
    if (vCenter.y - vHalfExtents.y < miny)
      return (false);
    if (vCenter.y + vHalfExtents.y > maxy)
      return (false);
    if (vCenter.z - vHalfExtents.z < minz)
      return (false);
    if (vCenter.z + vHalfExtents.z > maxz)
      return (false);

    if (uiAddID > 0)
    {
      const float lx = ((maxx - minx) * 0.5f) * GC_LOOSE_FACTOR;
      const float ly = ((maxy - miny) * 0.5f) * GC_LOOSE_FACTOR;
      const float lz = ((maxz - minz) * 0.5f) * GC_LOOSE_FACTOR;

      const aeUInt32 uiNodeIDBase = uiNodeID + 1;
      const aeUInt32 uiAddIDChild = uiAddID - uiSubAddID;
      const aeUInt32 uiSubAddIDChild = uiSubAddID >> 3;

      if (InsertObject(vCenter, vHalfExtents, Obj, minx, minx + lx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 0, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, minx, minx + lx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 1, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, minx, minx + lx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 2, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, minx, minx + lx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 3, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, maxx - lx, maxx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 4, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, maxx - lx, maxx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 5, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, maxx - lx, maxx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 6, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
      if (InsertObject(vCenter, vHalfExtents, Obj, maxx - lx, maxx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 7, uiAddIDChild, uiSubAddIDChild, out_Object))
        return (true);
    }

    out_Object = m_NodeMap.insert(std::pair<aeUInt32, aeObjectData>(uiNodeID, Obj));
    return (true);
  }

  //void aeDynamicOctree::FindVisibleObjects (const aeViewfrustum& Viewfrustum, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough) const
  //{
  //	if (m_NodeMap.empty ())
  //		return;

  //	FindVisibleObjects (Viewfrustum, Callback, pPassThrough, m_fMinX, m_fMaxX, m_fMinY, m_fMaxY, m_fMinZ, m_fMaxZ, 0, m_uiAddIDTopLevel, aeMath::Pow (8, m_uiMaxTreeDepth-1), 0xFFFFFFFF);
  //}

  void aeDynamicOctree::FindObjectsInRange(const aeVec3& vPoint, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough) const
  {
    if (m_NodeMap.empty())
      return;

    if ((vPoint.x < m_fMinX) || (vPoint.x > m_fMaxX))
      return;
    if ((vPoint.y < m_fMinY) || (vPoint.y > m_fMaxY))
      return;
    if ((vPoint.z < m_fMinZ) || (vPoint.z > m_fMaxZ))
      return;

    FindObjectsInRange(vPoint, Callback, pPassThrough, m_fMinX, m_fMaxX, m_fMinY, m_fMaxY, m_fMinZ, m_fMaxZ, 0, m_uiAddIDTopLevel, aeMath::Pow(8, m_uiMaxTreeDepth - 1), 0xFFFFFFFF);
  }


  //void aeDynamicOctree::FindVisibleObjects (const aeViewfrustum& Viewfrustum, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeUInt32 uiNextNodeID) const
  //{
  //	aeVec3 v[8];
  //	v[0].setVector (minx, miny, minz);
  //	v[1].setVector (minx, miny, maxz);
  //	v[2].setVector (minx, maxy, minz);
  //	v[3].setVector (minx, maxy, maxz);
  //	v[4].setVector (maxx, miny, minz);
  //	v[5].setVector (maxx, miny, maxz);
  //	v[6].setVector (maxx, maxy, minz);
  //	v[7].setVector (maxx, maxy, maxz);

  //	aeInt32 iResult = Viewfrustum.isObjectInside (&v[0], 8);

  //	if (iResult == AE_OBJECT_OUTSIDE_VOLUME)
  //		return;

  //	aeDynamicTreeObject it1 = m_NodeMap.lower_bound (uiNodeID);
  //	const aeDynamicTreeObject itend = m_NodeMap.end ();

  //	if ((it1 == itend) || (it1->first >= uiNextNodeID))
  //		return;

  //	if (iResult == AE_OBJECT_INSIDE_VOLUME)
  //	{
  //		const aeDynamicTreeObject itlast = m_NodeMap.lower_bound (uiNextNodeID);

  //		while (it1 != itlast)
  //		{
  //			// first increase the iterator, the user could erase it in the callback
  //			aeDynamicTreeObject temp = it1;
  //			++it1;

  //			Callback (pPassThrough, temp);
  //		}

  //		return;
  //	}
  //	else
  //	if (iResult == AE_OBJECT_INTERSECTS_VOLUME)
  //	{
  //		const aeDynamicTreeObject itlast = m_NodeMap.lower_bound (uiNodeID+1);

  //		while  (it1 != itlast)
  //		{
  //			// first increase the iterator, the user could erase it in the callback
  //			aeDynamicTreeObject temp = it1;
  //			++it1;

  //			Callback (pPassThrough, temp);
  //		}

  //		if (uiAddID > 0)
  //		{
  //			const float lx = ((maxx - minx) * 0.5f) * GC_LOOSE_FACTOR;
  //			const float ly = ((maxy - miny) * 0.5f) * GC_LOOSE_FACTOR;
  //			const float lz = ((maxz - minz) * 0.5f) * GC_LOOSE_FACTOR;

  //			const aeUInt32 uiNodeIDBase = uiNodeID + 1;
  //			const aeUInt32 uiAddIDChild = uiAddID - uiSubAddID;
  //			const aeUInt32 uiSubAddIDChild = uiSubAddID >> 3;

  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, minx, minx + lx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 0, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 1);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, minx, minx + lx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 1, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 2);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, minx, minx + lx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 2, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 3);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, minx, minx + lx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 3, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 4);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, maxx - lx, maxx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 4, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 5);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, maxx - lx, maxx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 5, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 6);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, maxx - lx, maxx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 6, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 7);
  //			FindVisibleObjects (Viewfrustum, Callback, pPassThrough, maxx - lx, maxx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 7, uiAddIDChild, uiSubAddIDChild, uiNextNodeID);
  //		}
  //	}
  //}

  void aeDynamicOctree::RemoveObject(aeDynamicTreeObject obj)
  {
    m_NodeMap.erase(obj);
  }

  void aeDynamicOctree::RemoveObject(aeInt32 iObjectType, aeInt32 iObjectInstance)
  {
    const aeDynamicTreeObject itend = m_NodeMap.end();
    for (aeDynamicTreeObject it = m_NodeMap.begin(); it != itend; ++it)
    {
      if ((it->second.m_iObjectInstance == iObjectInstance) && (it->second.m_iObjectType == iObjectType))
      {
        m_NodeMap.erase(it);
        return;
      }
    }
  }

  void aeDynamicOctree::RemoveObjectsOfType(aeInt32 iObjectType)
  {
    const aeDynamicTreeObject itend = m_NodeMap.end();
    for (aeDynamicTreeObject it = m_NodeMap.begin(); it != itend; ++it)
    {
      if (it->second.m_iObjectType == iObjectType)
      {
        aeDynamicTreeObject itold = it;
        ++it;

        m_NodeMap.erase(itold);
      }
      else
        ++it;
    }
  }



  bool aeDynamicOctree::FindObjectsInRange(const aeVec3& vPoint, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeUInt32 uiNextNodeID) const
  {
    if (vPoint.x < minx)
      return (true);
    if (vPoint.x > maxx)
      return (true);
    if (vPoint.y < miny)
      return (true);
    if (vPoint.y > maxy)
      return (true);
    if (vPoint.z < minz)
      return (true);
    if (vPoint.z > maxz)
      return (true);

    aeDynamicTreeObjectConst it1 = m_NodeMap.lower_bound(uiNodeID);
    const aeDynamicTreeObjectConst itend = m_NodeMap.end();

    if ((it1 == itend) || (it1->first >= uiNextNodeID))
      return (true);

    {
      {
        const aeDynamicTreeObjectConst itlast = m_NodeMap.lower_bound(uiNodeID + 1);

        while (it1 != itlast)
        {
          // first increase the iterator, the user could erase it in the callback
          aeDynamicTreeObjectConst temp = it1;
          ++it1;

          if (!Callback(pPassThrough, temp))
            return (false);
        }
      }

      if (uiAddID > 0)
      {
        const float lx = ((maxx - minx) * 0.5f) * GC_LOOSE_FACTOR;
        const float ly = ((maxy - miny) * 0.5f) * GC_LOOSE_FACTOR;
        const float lz = ((maxz - minz) * 0.5f) * GC_LOOSE_FACTOR;

        const aeUInt32 uiNodeIDBase = uiNodeID + 1;
        const aeUInt32 uiAddIDChild = uiAddID - uiSubAddID;
        const aeUInt32 uiSubAddIDChild = uiSubAddID >> 3;

        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, minx, minx + lx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 0, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 1))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, minx, minx + lx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 1, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 2))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, minx, minx + lx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 2, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 3))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, minx, minx + lx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 3, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 4))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, maxx - lx, maxx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 4, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 5))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, maxx - lx, maxx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 5, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 6))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, maxx - lx, maxx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 6, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 7))
          return (false);
        if (!FindObjectsInRange(vPoint, Callback, pPassThrough, maxx - lx, maxx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 7, uiAddIDChild, uiSubAddIDChild, uiNextNodeID))
          return (false);
      }
    }

    return (true);
  }

  void aeDynamicOctree::FindObjectsInRange(const aeVec3& vPoint, float fRadius, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough) const
  {
    AE_CHECK_DEV(m_uiMaxTreeDepth > 0, "aeDynamicOctree::FindObjectsInRange: You have to first create the tree.");

    if (m_NodeMap.empty())
      return;

    if ((vPoint.x + fRadius < m_fMinX) || (vPoint.x - fRadius > m_fMaxX))
      return;
    if ((vPoint.y + fRadius < m_fMinY) || (vPoint.y - fRadius > m_fMaxY))
      return;
    if ((vPoint.z + fRadius < m_fMinZ) || (vPoint.z - fRadius > m_fMaxZ))
      return;

    FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, m_fMinX, m_fMaxX, m_fMinY, m_fMaxY, m_fMinZ, m_fMaxZ, 0, m_uiAddIDTopLevel, aeMath::Pow(8, m_uiMaxTreeDepth - 1), 0xFFFFFFFF);
  }

  bool aeDynamicOctree::FindObjectsInRange(const aeVec3& vPoint, float fRadius, AE_VISIBLE_OBJ_CALLBACK Callback, void* pPassThrough, float minx, float maxx, float miny, float maxy, float minz, float maxz, aeUInt32 uiNodeID, aeUInt32 uiAddID, aeUInt32 uiSubAddID, aeUInt32 uiNextNodeID) const
  {
    if (vPoint.x + fRadius < minx)
      return (true);
    if (vPoint.x - fRadius > maxx)
      return (true);
    if (vPoint.y + fRadius < miny)
      return (true);
    if (vPoint.y - fRadius > maxy)
      return (true);
    if (vPoint.z + fRadius < minz)
      return (true);
    if (vPoint.z - fRadius > maxz)
      return (true);

    aeDynamicTreeObjectConst it1 = m_NodeMap.lower_bound(uiNodeID);
    const aeDynamicTreeObjectConst itend = m_NodeMap.end();

    // if the whole sub-tree doesn't contain any data, no need to check further
    if ((it1 == itend) || (it1->first >= uiNextNodeID))
      return (true);

    {

      // return all objects stored at this node
      {
        while ((it1 != itend) && (it1->first == uiNodeID))
        {
          // first increase the iterator, the user could erase it in the callback
          aeDynamicTreeObjectConst temp = it1;
          ++it1;

          if (!Callback(pPassThrough, temp))
            return (false);
        }
      }

      // if the node has children
      if (uiAddID > 0)
      {
        const float lx = ((maxx - minx) * 0.5f) * GC_LOOSE_FACTOR;
        const float ly = ((maxy - miny) * 0.5f) * GC_LOOSE_FACTOR;
        const float lz = ((maxz - minz) * 0.5f) * GC_LOOSE_FACTOR;

        const aeUInt32 uiNodeIDBase = uiNodeID + 1;
        const aeUInt32 uiAddIDChild = uiAddID - uiSubAddID;
        const aeUInt32 uiSubAddIDChild = uiSubAddID >> 3;

        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, minx, minx + lx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 0, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 1))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, minx, minx + lx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 1, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 2))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, minx, minx + lx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 2, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 3))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, minx, minx + lx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 3, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 4))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, maxx - lx, maxx, miny, miny + ly, minz, minz + lz, uiNodeIDBase + uiAddID * 4, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 5))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, maxx - lx, maxx, miny, miny + ly, maxz - lz, maxz, uiNodeIDBase + uiAddID * 5, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 6))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, maxx - lx, maxx, maxy - ly, maxy, minz, minz + lz, uiNodeIDBase + uiAddID * 6, uiAddIDChild, uiSubAddIDChild, uiNodeIDBase + uiAddID * 7))
          return (false);
        if (!FindObjectsInRange(vPoint, fRadius, Callback, pPassThrough, maxx - lx, maxx, maxy - ly, maxy, maxz - lz, maxz, uiNodeIDBase + uiAddID * 7, uiAddIDChild, uiSubAddIDChild, uiNextNodeID))
          return (false);
      }
    }

    return (true);
  }
} // namespace AE_NS_FOUNDATION
