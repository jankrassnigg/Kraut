#include "PCH.h"
#include "ProgressBar.h"

aeProgressBar* aeProgressBar::s_pActiveRange = NULL;
QProgressDialog* aeProgressBar::s_pQtProgressBar = NULL;

struct aeWin7ProgressBarState
{
  enum Enum
  {
    Disabled,   // no progress bar
    Normal,     // green
    Paused,     // yellow
    Error,      // red
    Unknown,    // indeterminate
  };
};

aeProgressBar::aeProgressBar (const char* szText, aeUInt32 uiItems, aeUInt32 uiTimeInvisibleInMS)
{
  m_dBaseValue = 0.0;
  m_dPercentageRange = 100.0;
  m_uiMaxItems = uiItems;
  m_uiCurItems = 0;
  m_pParentRange = s_pActiveRange;

  if (m_pParentRange)
  {
    m_dPercentageRange = m_pParentRange->m_dPercentageRange / m_pParentRange->m_uiMaxItems;
    m_dBaseValue = m_pParentRange->m_dBaseValue + m_dPercentageRange * m_pParentRange->m_uiCurItems;
  }
  else
    AE_BROADCAST_EVENT (aeEditor_BlockRedraw);

  if (!s_pQtProgressBar)
  {
    QMainWindow* pMainWindow = NULL;
    aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

    QApplication::setOverrideCursor (Qt::WaitCursor);

    s_pQtProgressBar = new QProgressDialog ("                                                                                ", 
      "Abort", 0, 1000, pMainWindow);

    s_pQtProgressBar->setWindowModality(Qt::WindowModal);
    s_pQtProgressBar->setMinimumDuration(uiTimeInvisibleInMS);
    s_pQtProgressBar->setAutoReset (false);
    s_pQtProgressBar->setAutoClose (false);
    s_pQtProgressBar->setLabelText (szText);
  }

  if (szText)
    s_pQtProgressBar->setLabelText (szText);

  s_pActiveRange = this;
}

aeProgressBar::~aeProgressBar ()
{
  s_pActiveRange = m_pParentRange;

  if (!s_pActiveRange)
  {
    delete s_pQtProgressBar;
    s_pQtProgressBar = NULL;

    QMainWindow* pMainWindow = NULL;
    aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

    QApplication::restoreOverrideCursor();

    AE_BROADCAST_EVENT (aeEditor_SetTaskbarProgress, aeGlobalEventParam ((aeInt32) aeWin7ProgressBarState::Disabled), aeGlobalEventParam ((aeInt32) 1000));
    AE_BROADCAST_EVENT (aeEditor_UnblockRedraw);
  }
}

void aeProgressBar::Update (const char* szText, aeUInt32 uiFinishedItems)
{
  if (!s_pActiveRange || !s_pQtProgressBar)
    return;

  if (szText)
    s_pQtProgressBar->setLabelText (szText);

  s_pActiveRange->m_uiCurItems = aeMath::Min<aeUInt32> (s_pActiveRange->m_uiCurItems + uiFinishedItems, s_pActiveRange->m_uiMaxItems);

  double dStep = s_pActiveRange->m_dPercentageRange / s_pActiveRange->m_uiMaxItems;
  double dPercentage = s_pActiveRange->m_dBaseValue + s_pActiveRange->m_uiCurItems * dStep;

  aeUInt32 uiProMille = (aeUInt32) (dPercentage * 10.0);

  s_pQtProgressBar->setValue (uiProMille);

  AE_BROADCAST_EVENT (aeEditor_SetTaskbarProgress, aeGlobalEventParam ((aeInt32) aeWin7ProgressBarState::Normal), aeGlobalEventParam ((aeInt32) uiProMille));
}

bool aeProgressBar::WasProgressBarCanceled (void)
{
  if (s_pQtProgressBar)
    return s_pQtProgressBar->wasCanceled ();

  return false;
}