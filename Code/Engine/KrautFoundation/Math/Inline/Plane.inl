#ifndef AE_FOUNDATION_MATHS_PLANE_INL
#define AE_FOUNDATION_MATHS_PLANE_INL

#include "../Math/Matrix.h"

namespace AE_NS_FOUNDATION
{
  AE_INLINE aePlane::aePlane (const aeVec3 &vNormal, float fDistToPlane)
  {
    CreatePlane (vNormal, fDistToPlane);
  }

  AE_INLINE aePlane::aePlane (const aeVec3 &vNormal, const aeVec3& vPointOnPlane)
  {
    CreatePlane (vNormal, vPointOnPlane);
  }

  AE_INLINE aePlane::aePlane (const aeVec3& v1, const aeVec3& v2, const aeVec3& v3)
  {
    CreatePlane (v1, v2, v3);
  }

  AE_INLINE aePlane::aePlane (const aeVec3* const pVertices)
  {
    CreatePlane (pVertices);
  }

  AE_INLINE aePlane::aePlane (const aeVec3* const pVertices, aeUInt32 iMaxVertices)
  {
    CreatePlane (pVertices, iMaxVertices);
  }

  AE_INLINE void aePlane::CreatePlane (const aeVec3 &vNormal, const aeVec3& vPointOnPlane)
  {
    m_vNormal = vNormal; 
    m_fDistance = m_vNormal.Dot (vPointOnPlane);
  }

  AE_INLINE void aePlane::CreatePlane (const aeVec3 &vNormal, float fDistToPlane)
  {
    m_vNormal = vNormal;
    m_fDistance = fDistToPlane;
  }

  AE_INLINE void aePlane::CreatePlane (const aeVec3& v1, const aeVec3& v2, const aeVec3& v3)
  {
    m_vNormal.CalculateNormal (v1, v2, v3);
    m_fDistance = m_vNormal.Dot (v1);
  }

  AE_INLINE void aePlane::CreatePlane (const aeVec3* const pVertices)
  {
    m_vNormal.CalculateNormal (pVertices[0], pVertices[1], pVertices[2]);
    m_fDistance = m_vNormal.Dot (pVertices[0]);
  }

  AE_INLINE void aePlane::TransformPlane (const aeMatrix& m)
  {
    aeVec3 vPointOnPlane = m_vNormal * m_fDistance;

    // rotate the normal, translate the point
    CreatePlane (m.TransformDirection (m_vNormal), m * vPointOnPlane);
  }

  AE_INLINE void aePlane::FlipPlane (void)
  {
    m_fDistance = -m_fDistance;
    m_vNormal = -m_vNormal;
  }

  AE_INLINE float aePlane::GetDistanceToPoint (const aeVec3& vPoint) const
  {
    return (m_vNormal.Dot (vPoint) - m_fDistance);
  }

  AE_INLINE aePositionOnPlane::Enum aePlane::GetPointPosition (const aeVec3& vPoint) const
  {
    return (m_vNormal.Dot (vPoint) < m_fDistance ? aePositionOnPlane::Back : aePositionOnPlane::Front);
  }

  AE_INLINE aePositionOnPlane::Enum aePlane::GetPointPosition (const aeVec3& vPoint, float fPlaneHalfWidth) const
  {
    const float f = m_vNormal.Dot (vPoint);

    if (f + fPlaneHalfWidth < m_fDistance)
      return (aePositionOnPlane::Back);

    if (f - fPlaneHalfWidth > m_fDistance)
      return (aePositionOnPlane::Front);

    return (aePositionOnPlane::OnPlane);
  }

  AE_INLINE const aeVec3 aePlane::ProjectOntoPlane (const aeVec3& vPoint) const
  {
    return vPoint - m_vNormal * (m_vNormal.Dot (vPoint) - m_fDistance);
  }
}

#endif

