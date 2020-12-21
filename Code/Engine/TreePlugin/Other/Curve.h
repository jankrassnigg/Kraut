#ifndef AE_TREEPLUGIN_CURVE_H
#define AE_TREEPLUGIN_CURVE_H

#include "../Basics/Base.h"

struct aeCurve
{
  aeArray<float> m_Values;
  float m_fMinValue;
  float m_fMaxValue;

  void Initialize (aeUInt32 uiSamples, float fInitVal, float fMin, float fMax);

  float GetValueAt (float fPosition) const;
  float GetValueAtMirrored (float fPosition) const;

  void PasteCurve (const aeCurve& OtherCurve);

  void Save (aeStreamOut& s);
  void Load (aeStreamIn& s, bool bOldFormat = false);
};


#endif

