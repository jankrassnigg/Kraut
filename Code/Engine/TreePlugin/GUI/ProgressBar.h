#ifndef AE_TREEPLUGIN_PROGRESSBAR_H
#define AE_TREEPLUGIN_PROGRESSBAR_H

#include "../Basics/Base.h"

class aeProgressBar
{
public:
  aeProgressBar (const char* szText, aeUInt32 uiItems, aeUInt32 uiTimeInvisibleInMS = 0);
  ~aeProgressBar ();

  static void Update (const char* szText = nullptr, aeUInt32 uiFinishedItems = 1);

  static bool WasProgressBarCanceled (void);

private:
  static QProgressDialog* s_pQtProgressBar;
  static aeProgressBar* s_pActiveRange;

  aeProgressBar* m_pParentRange;
  double m_dPercentageRange;
  double m_dBaseValue;
  aeUInt32 m_uiMaxItems;
  aeUInt32 m_uiCurItems;
};


#endif