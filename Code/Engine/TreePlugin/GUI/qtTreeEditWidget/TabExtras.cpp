#include "PCH.h"

#include "../../Tree/Tree.h"
#include "../../Undo/TreeUndo.h"
#include "qtTreeEditWidget.moc.h"

#include <KrautGraphics/Configuration/CVar.h>
#include <QColorDialog>

extern aeCVarFloat CVar_Gamma;

aeCVarInt CVar_AmbientLowRed("Tree_AmbientLowRed", 72, aeCVarFlags::Save, "Ambient Ground Color - Red.");
aeCVarInt CVar_AmbientLowGreen("Tree_AmbientLowGreen", 65, aeCVarFlags::Save, "Ambient Ground Color - Green.");
aeCVarInt CVar_AmbientLowBlue("Tree_AmbientLowBlue", 58, aeCVarFlags::Save, "Ambient Ground Color - Blue.");

aeCVarInt CVar_AmbientHighRed("Tree_AmbientHighRed", 49, aeCVarFlags::Save, "Ambient Sky Color - Red.");
aeCVarInt CVar_AmbientHighGreen("Tree_AmbientHighGreen", 75, aeCVarFlags::Save, "Ambient Sky Color - Green.");
aeCVarInt CVar_AmbientHighBlue("Tree_AmbientHighBlue", 79, aeCVarFlags::Save, "Ambient Sky Color - Blue.");

void UpdateAmbientShaderColors(void)
{
  aeShaderManager::setUniformFloat("unif_AmbientLow", 3, CVar_AmbientLowRed / 255.0f, CVar_AmbientLowGreen / 255.0f, CVar_AmbientLowBlue / 255.0f);
  aeShaderManager::setUniformFloat("unif_AmbientHigh", 3, CVar_AmbientHighRed / 255.0f, CVar_AmbientHighGreen / 255.0f, CVar_AmbientHighBlue / 255.0f);

  QColor col;
  col.setRgb(CVar_AmbientLowRed, CVar_AmbientLowGreen, CVar_AmbientLowBlue);
  const QString COLOR_STYLE1("QPushButton { background-color : %1 }");
  qtTreeEditWidget::s_pWidget->buttonAmbientColorLow->setStyleSheet(COLOR_STYLE1.arg(col.name()));

  col.setRgb(CVar_AmbientHighRed, CVar_AmbientHighGreen, CVar_AmbientHighBlue);
  const QString COLOR_STYLE2("QPushButton { background-color : %1 }");
  qtTreeEditWidget::s_pWidget->buttonAmbientColorHigh->setStyleSheet(COLOR_STYLE2.arg(col.name()));
}

void ComputeAmbientOcclusion(void);
void SetShaderGammaValue(void);

void qtTreeEditWidget::on_groupAmbientOcclusion_clicked()
{
  g_Tree.m_Descriptor.m_bUseAO = groupAmbientOcclusion->isChecked();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_buttonUpdateAO_clicked()
{
  ComputeAmbientOcclusion();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spinAOSampleSize_valueChanged(double d)
{
  g_Tree.m_Descriptor.m_fAOSampleSize = spinAOSampleSize->value();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_spinAOContrast_valueChanged(double d)
{
  g_Tree.m_Descriptor.m_fAOContrast = spinAOContrast->value();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spinGamma_valueChanged(double d)
{
  CVar_Gamma = d;

  SetShaderGammaValue();
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

static aeUInt8 g_uiLastAmbientColorLow[3] = {0, 0, 0};

void qtTreeEditWidget::on_buttonAmbientColorLow_clicked()
{
  g_uiLastAmbientColorLow[0] = CVar_AmbientLowRed;
  g_uiLastAmbientColorLow[1] = CVar_AmbientLowGreen;
  g_uiLastAmbientColorLow[2] = CVar_AmbientLowBlue;

  QColor col;
  col.setRgb(g_uiLastAmbientColorLow[0], g_uiLastAmbientColorLow[1], g_uiLastAmbientColorLow[2]);

  QColorDialog* dlg = new QColorDialog(col, this);
  connect(dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(on_AmbientColorLow_changed(const QColor&)));
  connect(dlg, SIGNAL(rejected()), this, SLOT(on_AmbientColorLow_reset()));
  connect(dlg, SIGNAL(accepted()), this, SLOT(on_AmbientColorLow_accepted()));
  dlg->open(this, SLOT(on_AmbientColorLow_changed(const QColor&)));
}

void qtTreeEditWidget::on_AmbientColorLow_changed(const QColor& chosenColor)
{
  CVar_AmbientLowRed = chosenColor.red();
  CVar_AmbientLowGreen = chosenColor.green();
  CVar_AmbientLowBlue = chosenColor.blue();

  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  buttonAmbientColorLow->setStyleSheet(COLOR_STYLE.arg(chosenColor.name()));

  UpdateAmbientShaderColors();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_AmbientColorLow_reset()
{
  CVar_AmbientLowRed = g_uiLastAmbientColorLow[0];
  CVar_AmbientLowGreen = g_uiLastAmbientColorLow[1];
  CVar_AmbientLowBlue = g_uiLastAmbientColorLow[2];

  QColor col;
  col.setRgb(g_uiLastAmbientColorLow[0], g_uiLastAmbientColorLow[1], g_uiLastAmbientColorLow[2]);

  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  buttonAmbientColorLow->setStyleSheet(COLOR_STYLE.arg(col.name()));

  UpdateAmbientShaderColors();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

static aeUInt8 g_uiLastAmbientColorHigh[3] = {0, 0, 0};

void qtTreeEditWidget::on_buttonAmbientColorHigh_clicked()
{
  g_uiLastAmbientColorHigh[0] = CVar_AmbientHighRed;
  g_uiLastAmbientColorHigh[1] = CVar_AmbientHighGreen;
  g_uiLastAmbientColorHigh[2] = CVar_AmbientHighBlue;

  QColor col;
  col.setRgb(g_uiLastAmbientColorHigh[0], g_uiLastAmbientColorHigh[1], g_uiLastAmbientColorHigh[2]);

  QColorDialog* dlg = new QColorDialog(col, this);
  connect(dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(on_AmbientColorHigh_changed(const QColor&)));
  connect(dlg, SIGNAL(rejected()), this, SLOT(on_AmbientColorHigh_reset()));
  connect(dlg, SIGNAL(accepted()), this, SLOT(on_AmbientColorHigh_accepted()));
  dlg->open(this, SLOT(on_AmbientColorHigh_changed(const QColor&)));
}

void qtTreeEditWidget::on_AmbientColorHigh_changed(const QColor& chosenColor)
{
  CVar_AmbientHighRed = chosenColor.red();
  CVar_AmbientHighGreen = chosenColor.green();
  CVar_AmbientHighBlue = chosenColor.blue();

  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  buttonAmbientColorHigh->setStyleSheet(COLOR_STYLE.arg(chosenColor.name()));

  UpdateAmbientShaderColors();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_AmbientColorHigh_reset()
{
  CVar_AmbientHighRed = g_uiLastAmbientColorHigh[0];
  CVar_AmbientHighGreen = g_uiLastAmbientColorHigh[1];
  CVar_AmbientHighBlue = g_uiLastAmbientColorHigh[2];

  QColor col;
  col.setRgb(g_uiLastAmbientColorHigh[0], g_uiLastAmbientColorHigh[1], g_uiLastAmbientColorHigh[2]);

  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  buttonAmbientColorHigh->setStyleSheet(COLOR_STYLE.arg(col.name()));

  UpdateAmbientShaderColors();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_buttonAmbientLowDefault_clicked()
{
  CVar_AmbientLowRed = 72;
  CVar_AmbientLowGreen = 65;
  CVar_AmbientLowBlue = 58;

  UpdateAmbientShaderColors();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_buttonAmbientHighDefault_clicked()
{
  CVar_AmbientHighRed = 49;
  CVar_AmbientHighGreen = 75;
  CVar_AmbientHighBlue = 79;

  UpdateAmbientShaderColors();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}
