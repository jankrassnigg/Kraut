#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../qtCurveProperty/qtCurveProperty.moc.h"
#include "qtCurveEditorWidget.moc.h"
#include <KrautFoundation/FileSystem/FileIn.h>
#include <KrautFoundation/FileSystem/FileOut.h>
#include <KrautGenerator/Infrastructure/Curve.h>

qtCurveEditorWidget* qtCurveEditorWidget::s_pWidget = nullptr;

qtCurveEditorWidget::qtCurveEditorWidget(QWidget* parent)
  : QDockWidget(parent)
{
  s_pWidget = this;

  setupUi(this);
  SetupToolbars();

  SetActiveCurveProperty(nullptr);
}

void qtCurveEditorWidget::SetupToolbars(void)
{
  QVBoxLayout* pLayout = new QVBoxLayout(widget_Toolbar);
  m_pToolbar = new QToolBar(widget_Toolbar);

  // General Toolbar Setup
  {
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);

    setContentsMargins(0, 0, 0, 0);
    dockWidgetContents->setContentsMargins(0, 0, 0, 0);
    dockWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);

    pLayout->addWidget(m_pToolbar);

    m_pToolbar->setFloatable(false);
    m_pToolbar->setMovable(false);
  }

  // Preset Menu
  {
    m_pPresetMenu = new QToolButton(m_pToolbar);
    QMenu* pMenu = new QMenu(m_pPresetMenu);

    m_pPresetMenu->setMenu(pMenu);
    m_pPresetMenu->setPopupMode(QToolButton::InstantPopup);

    // Add all the presets
    pMenu->addAction(actionPresetMax);
    pMenu->addAction(actionPresetMin);
    pMenu->addAction(actionPresetLinearGrowth);
    pMenu->addAction(actionPresetLinearDecay);
    pMenu->addAction(actionPresetRound);

    pMenu->addAction(actionPresetRise);
    pMenu->addAction(actionPresetDroop);
    pMenu->addAction(actionPresetBellCurve);
    pMenu->addAction(actionPresetConical);
    pMenu->addAction(actionPresetExponentialFalloff);
    pMenu->addAction(actionPresetExponentialGrowth);
    pMenu->addAction(actionPresetFadeInOut);
    pMenu->addAction(actionPresetScurve);

    m_pPresetMenu->setDefaultAction(actionSelectCurve);

    pMenu->setActiveAction(actionPresetMin);
    m_pToolbar->addWidget(m_pPresetMenu);
  }

  m_pToolbar->addSeparator();

  m_pToolbar->addAction(actionSmoothCurve);
  m_pToolbar->addAction(actionInvertValues);
  m_pToolbar->addAction(actionMirrorValues);


  m_pToolbar->addSeparator();

  m_pToolbar->addAction(actionCopyCurve);
  m_pToolbar->addAction(actionPasteCurve);

  actionPasteCurve->setDisabled(true);

  m_pToolbar->addSeparator();

  m_pToolbar->addAction(actionSavePreset);
  m_pToolbar->addAction(actionLoadPreset);
}

Kraut::Curve* qtCurveEditorWidget::GetCurve() const
{
  if (!m_pCurveView->m_pActiveCurve)
    return nullptr;

  return m_pCurveView->m_pActiveCurve->m_pCurve;
}

void qtCurveEditorWidget::on_actionPresetMin_triggered()
{
  if (!GetCurve())
    return;

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue;

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetMax_triggered()
{
  if (!GetCurve())
    return;

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMaxValue;

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetLinearGrowth_triggered()
{
  if (!GetCurve())
    return;

  const float fStep = (GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue) / (GetCurve()->m_Values.size() - 1);

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + i * fStep;

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetLinearDecay_triggered()
{
  if (!GetCurve())
    return;

  const float fStep = (GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue) / (GetCurve()->m_Values.size() - 1);

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMaxValue - i * fStep;

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetRound_triggered()
{
  if (!GetCurve())
    return;

  const float fStep = 180.0f / (GetCurve()->m_Values.size() - 1);

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = aeMath::SinDeg(i * fStep);

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetRise_triggered()
{
  if (!GetCurve())
    return;

  const float fStep = (GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue) * 0.5f / (GetCurve()->m_Values.size() - 1);

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + i * fStep;

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetDroop_triggered()
{
  if (!GetCurve())
    return;

  const float fStep = (GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue) * 0.5f / (GetCurve()->m_Values.size() - 1);

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMaxValue - i * fStep;

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetExponentialFalloff_triggered()
{
  if (!GetCurve())
    return;

  const float fRange = GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue;

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
  {
    float fPos = (float)i / (float)(GetCurve()->m_Values.size() - 1);

    fPos = 1.0f - aeMath::Pow(fPos, 6.0f);

    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + fPos * fRange;
  }

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetExponentialGrowth_triggered()
{
  if (!GetCurve())
    return;

  const float fRange = GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue;

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
  {
    float fPos = (float)i / (float)(GetCurve()->m_Values.size() - 1);

    fPos = aeMath::Pow(fPos, 6.0f);

    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + fPos * fRange;
  }

  m_pCurveView->UpdateGraph(true);
}

float SCurve(float f);

static void CreateCurveS(Kraut::Curve& c, aeUInt32 uiStart, aeUInt32 uiRange, bool bInvert, float fPower)
{
  //for (aeUInt32 i = 0; i < c.m_Values.size (); ++i)
  //{
  //  c.m_Values[i] = SCurve (i / (float) c.m_Values.size ());
  //}

  //return;

  aeUInt32 i = 0;
  aeUInt32 uiHalf = uiRange / 2;

  const float fRange = c.m_fMaxValue - c.m_fMinValue;

  for (; i < uiHalf; ++i)
  {
    float fPos = (float)i / (float)(uiHalf - 1);

    fPos = aeMath::Pow(fPos, fPower);

    if (bInvert)
      c.m_Values[uiStart + i] = c.m_fMinValue + fRange * 0.5f + (1.0f - fPos) * fRange * 0.5f;
    else
      c.m_Values[uiStart + i] = c.m_fMinValue + fPos * fRange * 0.5f;
  }

  for (; i < uiRange; ++i)
  {
    float fPos = 1.0f - ((float)(i - uiHalf + 1) / (float)(uiHalf));

    fPos = 1.0f - aeMath::Pow(fPos, fPower);

    if (bInvert)
      c.m_Values[uiStart + i] = c.m_fMinValue + (1.0f - fPos) * fRange * 0.5f;
    else
      c.m_Values[uiStart + i] = c.m_fMinValue + fRange * 0.5f + fPos * fRange * 0.5f;
  }
}

void qtCurveEditorWidget::on_actionPresetFadeInOut_triggered()
{
  if (!GetCurve())
    return;

  aeUInt32 uiSize = GetCurve()->m_Values.size();

  for (aeUInt32 i = 0; i < uiSize; ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMaxValue;

  CreateCurveS(*GetCurve(), 0, uiSize / 4, false, 2.0f);
  CreateCurveS(*GetCurve(), (uiSize / 4) * 3, uiSize - ((uiSize / 4) * 3), true, 2.0f);

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetBellCurve_triggered()
{
  if (!GetCurve())
    return;

  CreateCurveS(*GetCurve(), 0, GetCurve()->m_Values.size() / 2, false, 3.0f);
  CreateCurveS(*GetCurve(), GetCurve()->m_Values.size() / 2, GetCurve()->m_Values.size() / 2, true, 3.0f);

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetConical_triggered()
{
  if (!GetCurve())
    return;

  const float fRange = GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue;
  const aeUInt32 uiSize = GetCurve()->m_Values.size();

  aeUInt32 i = 0;
  aeUInt32 uiFifth = uiSize / 4;

  for (; i < uiFifth; ++i)
  {
    float fPos = (float)i / (float)(uiFifth - 1);

    fPos = 1.0f - aeMath::Pow(1.0f - fPos, 4.0f);

    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + fPos * fRange;
  }

  for (; i < GetCurve()->m_Values.size(); ++i)
  {
    float fPos = (float)(i - uiFifth) / (float)(uiSize - uiFifth - 1);

    fPos = 1.0f - aeMath::Pow(fPos, 2.0f);

    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + fPos * fRange;
  }

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionPresetScurve_triggered()
{
  if (!GetCurve())
    return;

  aeUInt32 uiSize = GetCurve()->m_Values.size();

  CreateCurveS(*GetCurve(), 0, uiSize, false, 4.0f);

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionInvertValues_triggered()
{
  if (!GetCurve())
    return;

  const float fRange = GetCurve()->m_fMaxValue - GetCurve()->m_fMinValue;

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = GetCurve()->m_fMinValue + (fRange - (GetCurve()->m_Values[i] - GetCurve()->m_fMinValue));

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionMirrorValues_triggered()
{
  if (!GetProperty())
    return;

  aeArray<float> Copy = GetCurve()->m_Values;

  for (aeUInt32 i = 0; i < GetCurve()->m_Values.size(); ++i)
    GetCurve()->m_Values[i] = Copy[Copy.size() - i - 1];

  m_pCurveView->UpdateGraph(true);
}

static Kraut::Curve g_CopiedCurve;

void qtCurveEditorWidget::on_actionCopyCurve_triggered()
{
  if (!GetCurve())
    return;

  g_CopiedCurve = *GetCurve();

  actionPasteCurve->setDisabled(false);
}

void qtCurveEditorWidget::on_actionPasteCurve_triggered()
{
  if (!GetCurve())
    return;

  if (g_CopiedCurve.m_Values.empty())
    return;

  GetCurve()->PasteCurve(g_CopiedCurve);

  m_pCurveView->UpdateGraph(true);
}

void qtCurveEditorWidget::on_actionSmoothCurve_triggered()
{
  if (!GetCurve())
    return;

  GetProperty()->SmoothCurve();
}

void qtCurveEditorWidget::SetActiveCurveProperty(qtCurveProperty* pCurve)
{
  if (pCurve)
  {
    aeHybridString<32> sName;
    sName.Format("Curve Editor - %s", pCurve->m_szPropertyName);

    setWindowTitle(sName.c_str());
    setEnabled(true);
    m_pToolbar->setEnabled(true);

    qtCurveEditorWidget::s_pWidget->raise();
  }
  else
  {
    setWindowTitle("Curve Editor");
    setEnabled(false);
    m_pToolbar->setEnabled(false);
  }

  m_pCurveView->SetActiveCurve(pCurve);
}

static aeFilePath sPrevPreset = "";

void qtCurveEditorWidget::on_actionSavePreset_triggered()
{
  if (!GetCurve())
    return;

  aeFilePath sAbsolutePath;
  aeFileSystem::MakeValidPath(sPrevPreset.c_str(), true, &sAbsolutePath, nullptr);

  QString result = QFileDialog::getSaveFileName(this,
    QFileDialog::tr("Save Curve Preset"), sAbsolutePath.c_str(),
    QFileDialog::tr("Curve-Files (*.curve)"), 0);

  if (result.size() == 0)
    return;

  sPrevPreset = result.toLatin1().data();

  aeFileOut File;
  if (!File.Open(sPrevPreset.c_str()))
  {
    QMessageBox::warning(this, "Kraut", "The selected file could not be opened for writing.");
    return;
  }

  GetCurve()->Save(File);
}


void qtCurveEditorWidget::on_actionLoadPreset_triggered()
{
  if (!GetCurve())
    return;

  aeFileSystem::MakeValidPath("Curves/", false, &sPrevPreset, nullptr);

  QString result = QFileDialog::getOpenFileName(this,
    QFileDialog::tr("Load Curve Preset"), sPrevPreset.c_str(),
    QFileDialog::tr("Curve-Files (*.curve);;All Files (*.*)"), 0);

  if (result.size() == 0)
    return;

  sPrevPreset = result.toLatin1().data();

  aeFileIn File;
  if (!File.Open(sPrevPreset.c_str()))
  {
    QMessageBox::warning(this, "Kraut", "The selected file could not be opened.");
    return;
  }

  GetCurve()->Load(File);

  m_pCurveView->UpdateGraph(true);
}
