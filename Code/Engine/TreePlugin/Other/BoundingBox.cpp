#include "PCH.h"
#include "BoundingBox.h"

aeBBox::aeBBox ()
{
  SetInvalid ();
}

void aeBBox::SetInvalid (void)
{
  m_vMin.SetVector (aeMath::Infinity ());
  m_vMax.SetVector (-aeMath::Infinity ());
}

bool aeBBox::IsInvalid (void) const
{
  return ((m_vMin.x > m_vMax.x) || (m_vMin.y > m_vMax.y) || (m_vMin.z > m_vMax.z));
}

aeVec3 aeBBox::GetSize (void) const
{
  return (m_vMax - m_vMin);
}

void aeBBox::AddBoundary (const aeVec3& vAdd)
{
  m_vMin -= vAdd;
  m_vMax += vAdd;
}

void aeBBox::ExpandToInclude (const aeVec3& v)
{
  if (v.x > m_vMax.x)
    m_vMax.x = v.x;
  if (v.y > m_vMax.y)
    m_vMax.y = v.y;
  if (v.z > m_vMax.z)
    m_vMax.z = v.z;

  if (v.x < m_vMin.x)
    m_vMin.x = v.x;
  if (v.y < m_vMin.y)
    m_vMin.y = v.y;
  if (v.z < m_vMin.z)
    m_vMin.z = v.z;
}

aeVec3 aeBBox::GetCenter (void) const
{
  return m_vMin + (m_vMax - m_vMin) * 0.5f;
}