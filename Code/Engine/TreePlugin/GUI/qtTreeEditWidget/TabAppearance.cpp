#include "PCH.h"

#include "../../Undo/TreeUndo.h"
#include "../qtResourceEditorWidget/MaterialLibrary.h"
#include "qtTreeEditWidget.moc.h"

#include <KrautGraphics/FileSystem/FileIn.h>

#include <QColorDialog>



// ***** Branch Mesh *****

void qtTreeEditWidget::on_check_EnableBranchMesh_clicked()
{
  m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Branch] = check_EnableBranchMesh->isChecked();
  widgetBranchContent->setVisible(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Branch]);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

static aeString sPrevBranchTexture = "";

void qtTreeEditWidget::on_tb_SelectBranchTexture_clicked()
{
  aeTreeMaterial* pPrevMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch].c_str());

  if (!SelectTexture(sPrevBranchTexture, m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch], nullptr))
    return;

  // this will alreay make sure the combo box contains the material
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch].c_str());

  ComboBranchTexture->setCurrentIndex(m_MapBarkTextureToCombo[m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch]]);

  if ((pPrevMaterial->m_uiTilingX != pMaterial->m_uiTilingX) ||
      (pPrevMaterial->m_uiTilingY != pMaterial->m_uiTilingY))
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

SLIDER_UNDO(slider_BranchRoundness);

void qtTreeEditWidget::on_slider_BranchRoundness_valueChanged()
{
  m_pCurNT->m_fRoundnessFactor = slider_BranchRoundness->value() / 100.0f;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


// ***** Flares *****

void qtTreeEditWidget::on_spin_NumFlares_valueChanged(int i)
{
  m_pCurNT->m_uiFlares = i;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_FlareWidth_valueChanged(double d)
{
  m_pCurNT->m_fFlareWidth = (float)d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


void qtTreeEditWidget::on_spin_FlareRotation_valueChanged(int d)
{
  m_pCurNT->m_fFlareRotation = (float)d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_check_RotateTexCoords_clicked()
{
  m_pCurNT->m_bRotateTexCoords = check_RotateTexCoords->isChecked();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


// ***** Fronds ******

void qtTreeEditWidget::on_check_EnableFronds_clicked()
{
  m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Frond] = check_EnableFronds->isChecked();
  widgetFrondContent->setVisible(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Frond]);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_combo_FrondOrientation_currentIndexChanged(int index)
{
  m_pCurNT->m_FrondUpOrientation = (Kraut::LeafOrientation::Enum)index;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

SLIDER_UNDO(slider_FrondRotation);

void qtTreeEditWidget::on_slider_FrondRotation_valueChanged()
{
  m_pCurNT->m_uiMaxFrondOrientationDeviation = slider_FrondRotation->value();

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


static aeString sPrevFrondTexture = "";

void qtTreeEditWidget::on_tb_SelectFrondTexture_clicked()
{
  aeTreeMaterial* pPrevMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond].c_str());

  if (!SelectTexture(sPrevFrondTexture, m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond], nullptr))
    return;

  // this will alreay make sure the combo box contains the material
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond].c_str());

  ComboFrondTexture->setCurrentIndex(m_MapFrondTextureToCombo[m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond]]);

  if ((pPrevMaterial->m_uiTilingX != pMaterial->m_uiTilingX) ||
      (pPrevMaterial->m_uiTilingY != pMaterial->m_uiTilingY))
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


void qtTreeEditWidget::on_spin_Fronds_valueChanged(int i)
{
  m_pCurNT->m_uiNumFronds = i;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_FrondDetail_valueChanged(int i)
{
  m_pCurNT->m_uiFrondDetail = i;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinFrondWidth_valueChanged(double d)
{
  m_pCurNT->m_fFrondWidth = d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinFrondHeight_valueChanged(double d)
{
  m_pCurNT->m_fFrondHeight = d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_combo_FrondContourMode_currentIndexChanged(int index)
{
  m_pCurNT->m_FrondContourMode = (Kraut::SpawnNodeDesc::FrondContourMode)index;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

static aeUInt8 g_uiLastFrondColor[4] = {0, 0, 0, 0};

void qtTreeEditWidget::on_button_FrondColor_clicked()
{
  aeUndo::BeginOperation();

  g_uiLastFrondColor[0] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0];
  g_uiLastFrondColor[1] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1];
  g_uiLastFrondColor[2] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2];
  g_uiLastFrondColor[3] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3];

  QColor col;
  col.setRgb(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3]);

  QColorDialog* dlg = new QColorDialog(col, this);
  dlg->setOption(QColorDialog::ShowAlphaChannel, true);
  connect(dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(on_FrondColor_changed(const QColor&)));
  connect(dlg, SIGNAL(rejected()), this, SLOT(on_FrondColor_reset()));
  connect(dlg, SIGNAL(accepted()), this, SLOT(on_FrondColor_accepted()));
  dlg->open(this, SLOT(on_FrondColor_changed(const QColor&)));
}

void qtTreeEditWidget::on_FrondColor_changed(const QColor& chosenColor)
{
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0] = chosenColor.red();
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1] = chosenColor.green();
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2] = chosenColor.blue();
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3] = chosenColor.alpha();

  sliderFrondAlpha->setValue(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3]);


  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  button_FrondColor->setStyleSheet(COLOR_STYLE.arg(chosenColor.name()));

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_FrondColor_reset()
{
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0] = g_uiLastFrondColor[0];
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1] = g_uiLastFrondColor[1];
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2] = g_uiLastFrondColor[2];
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3] = g_uiLastFrondColor[3];

  sliderFrondAlpha->setValue(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3]);

  QColor col;
  col.setRgb(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3]);

  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  button_FrondColor->setStyleSheet(COLOR_STYLE.arg(col.name()));

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);

  aeUndo::EndOperation();
}

void qtTreeEditWidget::on_FrondColor_accepted()
{
  if ((m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0] != g_uiLastFrondColor[0]) ||
      (m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1] != g_uiLastFrondColor[1]) ||
      (m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2] != g_uiLastFrondColor[2]) ||
      (m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3] != g_uiLastFrondColor[3]))
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);

  aeUndo::EndOperation();
}

SLIDER_UNDO(sliderFrondAlpha);

void qtTreeEditWidget::on_sliderFrondAlpha_valueChanged(int i)
{
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3] = i;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_spinFrondTextureRepeat_valueChanged(double d)
{
  m_pCurNT->m_fTextureRepeat = (float)d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_checkAlignFronds_clicked()
{
  m_pCurNT->m_bAlignFrondsOnSurface = checkAlignFronds->isChecked();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


// ***** Leaves *****

void qtTreeEditWidget::on_CheckEnableLeaves_clicked()
{
  m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Leaf] = CheckEnableLeaves->isChecked();
  WidgetLeafContent->setVisible(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Leaf]);
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinLeafSize_valueChanged(double d)
{
  m_pCurNT->m_fLeafSize = d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinLeafInterval_valueChanged(double d)
{
  m_pCurNT->m_fLeafInterval = d;
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

static aeString sPrevLeafTexture = "";

void qtTreeEditWidget::on_ButtonSelectLeafTexture_clicked()
{
  aeTreeMaterial* pPrevMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf].c_str());

  if (!SelectTexture(sPrevLeafTexture, m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf], nullptr))
    return;

  // this will alreay make sure the combo box contains the material
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf].c_str());

  ComboLeafTexture->setCurrentIndex(m_MapLeafTextureToCombo[m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf]]);

  if ((pPrevMaterial->m_uiTilingX != pMaterial->m_uiTilingX) ||
      (pPrevMaterial->m_uiTilingY != pMaterial->m_uiTilingY))
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

static aeUInt8 g_uiLastLeafColor[4] = {0, 0, 0, 0};

void qtTreeEditWidget::on_ButtonLeafColor_clicked()
{
  aeUndo::BeginOperation();

  g_uiLastLeafColor[0] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0];
  g_uiLastLeafColor[1] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1];
  g_uiLastLeafColor[2] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2];
  g_uiLastLeafColor[3] = m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3];

  QColor col;
  col.setRgb(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3]);

  QColorDialog* dlg = new QColorDialog(col, this);
  dlg->setOption(QColorDialog::ShowAlphaChannel, true);
  connect(dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(on_LeafColor_changed(const QColor&)));
  connect(dlg, SIGNAL(rejected()), this, SLOT(on_LeafColor_reset()));
  connect(dlg, SIGNAL(accepted()), this, SLOT(on_LeafColor_accepted()));
  dlg->open(this, SLOT(on_LeafColor_changed(const QColor&)));
}

void qtTreeEditWidget::on_LeafColor_changed(const QColor& chosenColor)
{
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0] = chosenColor.red();
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1] = chosenColor.green();
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2] = chosenColor.blue();
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3] = chosenColor.alpha();

  SliderLeafAlpha->setValue(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3]);


  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  ButtonLeafColor->setStyleSheet(COLOR_STYLE.arg(chosenColor.name()));

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_LeafColor_reset()
{
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0] = g_uiLastLeafColor[0];
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1] = g_uiLastLeafColor[1];
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2] = g_uiLastLeafColor[2];
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3] = g_uiLastLeafColor[3];

  SliderLeafAlpha->setValue(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3]);

  QColor col;
  col.setRgb(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3]);

  const QString COLOR_STYLE("QPushButton { background-color : %1 }");
  ButtonLeafColor->setStyleSheet(COLOR_STYLE.arg(col.name()));

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);

  aeUndo::EndOperation();
}

void qtTreeEditWidget::on_LeafColor_accepted()
{
  if ((m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0] != g_uiLastLeafColor[0]) ||
      (m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1] != g_uiLastLeafColor[1]) ||
      (m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2] != g_uiLastLeafColor[2]) ||
      (m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3] != g_uiLastLeafColor[3]))
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);

  aeUndo::EndOperation();
}

SLIDER_UNDO(SliderLeafAlpha);

void qtTreeEditWidget::on_SliderLeafAlpha_valueChanged(int i)
{
  m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3] = i;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_CheckBillboardLeaves_clicked()
{
  m_pCurNT->m_bBillboardLeaves = CheckBillboardLeaves->isChecked();
  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_ComboBranchTexture_currentIndexChanged(int index)
{
  if ((m_pCurNT) && (index >= 0))
  {
    aeTreeMaterial* pPrevMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch].c_str());
    m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch] = ComboBranchTexture->itemData(index).toString().toUtf8().data();
    aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch].c_str());

    if ((pPrevMaterial->m_uiTilingX != pMaterial->m_uiTilingX) ||
        (pPrevMaterial->m_uiTilingY != pMaterial->m_uiTilingY))
    {
      AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
    }
    else
    {
      AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
    }
  }
}

void qtTreeEditWidget::on_ComboFrondTexture_currentIndexChanged(int index)
{
  if ((m_pCurNT) && (index >= 0))
  {
    aeTreeMaterial* pPrevMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond].c_str());
    m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond] = ComboFrondTexture->itemData(index).toString().toUtf8().data();
    aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond].c_str());

    if ((pPrevMaterial->m_uiTilingX != pMaterial->m_uiTilingX) ||
        (pPrevMaterial->m_uiTilingY != pMaterial->m_uiTilingY))
    {
      AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
    }
    else
    {
      AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
    }
  }
}

void qtTreeEditWidget::on_ComboLeafTexture_currentIndexChanged(int index)
{
  if ((m_pCurNT) && (index >= 0))
  {
    aeTreeMaterial* pPrevMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf].c_str());
    m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf] = ComboLeafTexture->itemData(index).toString().toUtf8().data();
    aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf].c_str());

    if ((pPrevMaterial->m_uiTilingX != pMaterial->m_uiTilingX) ||
        (pPrevMaterial->m_uiTilingY != pMaterial->m_uiTilingY))
    {
      AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
    }
    else
    {
      AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
    }
  }
}
