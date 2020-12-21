#ifndef TREEPLUGIN_BOUNDINGBOX_H
#define TREEPLUGIN_BOUNDINGBOX_H

#include "../Basics/Base.h"

struct aeBBox
{
  aeBBox ();

  void SetInvalid (void);
  bool IsInvalid (void) const;
  aeVec3 GetSize (void) const;
  void AddBoundary (const aeVec3& vAdd);

  void ExpandToInclude (const aeVec3& v);
  aeVec3 GetCenter (void) const;

  aeVec3 m_vMin;
  aeVec3 m_vMax;
};


#endif

