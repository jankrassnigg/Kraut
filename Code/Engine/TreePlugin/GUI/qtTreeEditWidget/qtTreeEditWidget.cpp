#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../../Tree/Tree.h"
#include "../../Undo/TreeUndo.h"
#include "../qtResourceEditorWidget/MaterialLibrary.h"
#include <KrautFoundation/Strings/StringFunctions.h>

#include "qtTreeEditWidget.moc.h"

QToolButton* g_pRenderModeButton = nullptr;
QToolButton* g_pOpenFileButton = nullptr;
QToolButton* g_pTransformModeButton = nullptr;
QMenu* g_pRecentFileMenu = nullptr;

qtTreeEditWidget* qtTreeEditWidget::s_pWidget = nullptr;

extern aeCVarFloat CVar_Gamma;
extern bool s_bTreeNeedsUpdate;
extern bool s_bTreeHasSimpleChanges;
extern bool s_bStatsNeedUpdate;
extern aeCVarBool CVar_LeafCardConvertToDDS;
extern aeDeque<aeFilePath> g_LRU_Trees;

extern aeCVarInt CVar_AmbientLowRed;
extern aeCVarInt CVar_AmbientLowGreen;
extern aeCVarInt CVar_AmbientLowBlue;
extern aeCVarInt CVar_AmbientHighRed;
extern aeCVarInt CVar_AmbientHighGreen;
extern aeCVarInt CVar_AmbientHighBlue;

void UpdateAmbientShaderColors(void);

const char* BranchTargetDirEnumNames[Kraut::BranchTargetDir::ENUM_COUNT] =
  {
    "Straight",
    "0° (Upwards)",
    "22°",
    "45°",
    "67°",
    "90° (Orthogonal)",
    "112°",
    "135°",
    "157°",
    "180° (Downwards)",
};

const char* LeafOrientationEnumNames[Kraut::LeafOrientation::ENUM_COUNT] =
  {
    "Upwards",
    "Along Parent",
    "Orthogonal To Parent",
};

const char* BranchTypeModeEnumNames[Kraut::BranchTypeMode::ENUM_COUNT] =
  {
    "Default",
    "Umbrella",
};

qtTreeEditWidget::qtTreeEditWidget(QWidget* parent)
  : QDockWidget(parent)
{
  s_pWidget = this;

  m_CurrentlyEditedType = Kraut::BranchType::Trunk1;

  setupUi(this);
  SetupToolbars();
  SetupShortcuts();

  SetCurrentlyEditedBranchType(Kraut::BranchType::Trunk1, true);

  // editing not supported anymore
  tabs->removeTab(1);

  for (aeUInt32 i = 0; i < Kraut::BranchTargetDir::ENUM_COUNT; ++i)
    combo_BranchTargetDir->addItem(BranchTargetDirEnumNames[i]);
  for (aeUInt32 i = 0; i < Kraut::BranchTargetDir::ENUM_COUNT; ++i)
    combo_BranchTargetDir2->addItem(BranchTargetDirEnumNames[i]);
  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    combo_LoD->addItem(aeLod::EnumNames[i]);
  for (aeUInt32 i = 0; i < Kraut::LeafOrientation::ENUM_COUNT; ++i)
    combo_FrondOrientation->addItem(LeafOrientationEnumNames[i]);
  for (aeUInt32 i = 0; i < aeForceType::ENUM_COUNT; ++i)
    ComboForceType->addItem(aeForceType::EnumNames[i]);
  for (aeUInt32 i = 0; i < aeForceFalloff::ENUM_COUNT; ++i)
    ComboForceFalloff->addItem(aeForceFalloff::EnumNames[i]);
  for (aeUInt32 i = 0; i < Kraut::BranchTypeMode::ENUM_COUNT; ++i)
    ComboBranchTypeMode->addItem(BranchTypeModeEnumNames[i]);

  combo_LoD->setCurrentIndex(aeLod::Lod0);
  ComboBranchTypeMode->setCurrentIndex(Kraut::BranchTypeMode::Default);

  check_ExportDDS->setChecked(CVar_LeafCardConvertToDDS);

  UpdateForcesUI();
}

void qtTreeEditWidget::UpdateRecentFileMenu(void)
{
  g_pRecentFileMenu->clear();

  for (aeUInt32 i = 1; i < g_LRU_Trees.size(); ++i)
  {
    QAction* pFile = new QAction(g_pRecentFileMenu);

    pFile->setText(g_LRU_Trees[i].c_str());
    pFile->setData(QVariant(i));

    g_pRecentFileMenu->addAction(pFile);
  }
}

void qtTreeEditWidget::SetupToolbars(void)
{
  QVBoxLayout* pLayout = new QVBoxLayout(widget_Toolbar);

  QToolBar* toolBar = new QToolBar(widget_Toolbar);

  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->setSpacing(0);

  setContentsMargins(0, 0, 0, 0);
  dockWidgetContents->setContentsMargins(0, 0, 0, 0);
  dockWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);

  pLayout->addWidget(toolBar);

  toolBar->setFloatable(false);
  toolBar->setMovable(false);

  toolBar->addAction(action_New);

  {
    g_pOpenFileButton = new QToolButton(toolBar);
    g_pRecentFileMenu = new QMenu(g_pOpenFileButton);
    g_pOpenFileButton->setMenu(g_pRecentFileMenu);
    g_pOpenFileButton->setPopupMode(QToolButton::MenuButtonPopup);

    connect(g_pRecentFileMenu, SIGNAL(triggered(QAction*)), this, SLOT(on_actionOpenRecentFile_triggered(QAction*)));

    g_pOpenFileButton->setDefaultAction(action_Open);
    toolBar->addWidget(g_pOpenFileButton);

    UpdateRecentFileMenu();
  }

  toolBar->addAction(action_Save);
  toolBar->addAction(action_SaveAs);
  toolBar->addAction(action_Export);
  toolBar->addSeparator();

  {
    g_pRenderModeButton = new QToolButton(toolBar);
    QMenu* pmenu = new QMenu(g_pRenderModeButton);
    g_pRenderModeButton->setMenu(pmenu);
    g_pRenderModeButton->setPopupMode(QToolButton::MenuButtonPopup);

    pmenu->addAction(actionRenderDefault);
    pmenu->addAction(actionRenderWireframe);
    pmenu->addAction(actionRenderSkeleton);
    pmenu->addAction(actionRenderNormals);
    pmenu->addAction(actionRenderDiffuse);
    pmenu->addAction(actionRenderLight);
    pmenu->addAction(actionRenderAmbient);
    pmenu->addAction(actionRenderSkylight);

    SetDefaultRenderModeButton(aeTreeRenderMode::Diffuse);
    toolBar->addWidget(g_pRenderModeButton);
  }

  toolBar->addAction(actionShowHideLeaves);
  actionShowHideLeaves->setChecked(g_Globals.s_bRenderLeaves);

  check_VisualizeCollisionMesh->setChecked(g_Globals.s_bRenderCollisionMesh);
  check_EnableCollisionAvoidance->setChecked(g_Globals.s_bDoPhysicsSimulation);

  toolBar->addAction(actionOrbitCamera);
  actionOrbitCamera->setChecked(g_Globals.s_bOrbitCamera);

  // Rotate / Translate Menu
  {
    g_pTransformModeButton = new QToolButton(toolBar);
    QMenu* pmenu = new QMenu(g_pTransformModeButton);
    g_pTransformModeButton->setMenu(pmenu);
    g_pTransformModeButton->setPopupMode(QToolButton::MenuButtonPopup);

    pmenu->addAction(actionTranslate);
    pmenu->addAction(actionRotate);
    pmenu->addAction(actionHideGizmos);

    g_pTransformModeButton->setDefaultAction(actionToggleGizmo);

    toolBar->addWidget(g_pTransformModeButton);
  }

  // Undo / Redo
  {
    toolBar->addSeparator();
    toolBar->addAction(actionUndo);
    toolBar->addAction(actionRedo);
    toolBar->addSeparator();
  }

  toolBar->addAction(actionAbout);
}


// Updating the UI

void qtTreeEditWidget::UpdateStats()
{
  lcd_Bones->display((int)g_Tree.GetNumBones(g_Globals.s_CurLoD));
  lcd_Polygons->display((int)g_Tree.GetNumAllTriangles(g_Globals.s_CurLoD));
  lcd_PolygonsBranch->display((int)g_Tree.GetNumTriangles(g_Globals.s_CurLoD, Kraut::BranchGeometryType::Branch));
  lcd_PolygonsFrond->display((int)g_Tree.GetNumTriangles(g_Globals.s_CurLoD, Kraut::BranchGeometryType::Frond));
  lcd_PolygonsLeaves->display((int)g_Tree.GetNumTriangles(g_Globals.s_CurLoD, Kraut::BranchGeometryType::Leaf));
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_SelectedBranchChanged)
{
  if (param0.iInt32 != -1)
  {
    const Kraut::BranchStructure& branch = g_Tree.m_TreeStructure.m_BranchStructures[param0.iInt32];

    qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType(branch.m_Type, false);
  }
}

AE_ON_GLOBAL_EVENT(aeResourceEdit_TreeMaterialsChanged)
{
  if (qtTreeEditWidget::s_pWidget)
    qtTreeEditWidget::s_pWidget->UpdateMaterialBoxes();
}

void qtTreeEditWidget::UpdateMaterialBoxes(void)
{
  AE_PREVENT_RECURSION;

  if (m_pCurNT)
  {
    aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch].c_str());
    aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond].c_str());
    aeTreeMaterialLibrary::GetOrAddMaterial(m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf].c_str());
  }

  const aeMap<aeHybridString<128>, aeTreeMaterial>& Lib = aeTreeMaterialLibrary::GetMaterials();

  const bool b1 = ComboBranchTexture->blockSignals(true);
  const bool b2 = ComboFrondTexture->blockSignals(true);
  const bool b3 = ComboLeafTexture->blockSignals(true);

  ComboBranchTexture->clear();
  ComboFrondTexture->clear();
  ComboLeafTexture->clear();

  m_MapBarkTextureToCombo.clear();
  m_MapFrondTextureToCombo.clear();
  m_MapLeafTextureToCombo.clear();

  aeUInt32 uiNumBark = 0;
  aeUInt32 uiNumFrond = 0;
  aeUInt32 uiNumLeaf = 0;

  for (aeMap<aeHybridString<128>, aeTreeMaterial>::const_iterator it = Lib.begin(); it != Lib.end(); ++it)
  {
    switch (it.value().m_Usage)
    {
      case aeMaterialType::Unknown:
        ComboBranchTexture->addItem(it.value().GetDescription().c_str(), it.value().m_sTextureDiffuse.c_str());
        ComboFrondTexture->addItem(it.value().GetDescription().c_str(), it.value().m_sTextureDiffuse.c_str());
        ComboLeafTexture->addItem(it.value().GetDescription().c_str(), it.value().m_sTextureDiffuse.c_str());

        m_MapBarkTextureToCombo[it.value().m_sTextureDiffuse] = uiNumBark;
        m_MapFrondTextureToCombo[it.value().m_sTextureDiffuse] = uiNumFrond;
        m_MapLeafTextureToCombo[it.value().m_sTextureDiffuse] = uiNumLeaf;

        ++uiNumBark;
        ++uiNumFrond;
        ++uiNumLeaf;
        break;
      case aeMaterialType::Bark:
        ComboBranchTexture->addItem(it.value().GetDescription().c_str(), it.value().m_sTextureDiffuse.c_str());
        m_MapBarkTextureToCombo[it.value().m_sTextureDiffuse] = uiNumBark;
        ++uiNumBark;
        break;
      case aeMaterialType::Frond:
        ComboFrondTexture->addItem(it.value().GetDescription().c_str(), it.value().m_sTextureDiffuse.c_str());
        m_MapFrondTextureToCombo[it.value().m_sTextureDiffuse] = uiNumFrond;
        ++uiNumFrond;
        break;
      case aeMaterialType::Leaf:
        ComboLeafTexture->addItem(it.value().GetDescription().c_str(), it.value().m_sTextureDiffuse.c_str());
        m_MapLeafTextureToCombo[it.value().m_sTextureDiffuse] = uiNumLeaf;
        ++uiNumLeaf;
        break;
    }
  }

  if (m_pCurNT)
  {
    ComboBranchTexture->setCurrentIndex(m_MapBarkTextureToCombo[m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch]]);
    ComboFrondTexture->setCurrentIndex(m_MapFrondTextureToCombo[m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond]]);
    ComboLeafTexture->setCurrentIndex(m_MapLeafTextureToCombo[m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf]]);
  }

  ComboBranchTexture->blockSignals(b1);
  ComboFrondTexture->blockSignals(b2);
  ComboLeafTexture->blockSignals(b3);
}

void qtTreeEditWidget::SetCurrentlyEditedBranchType(Kraut::BranchType::Enum Type, bool bForce)
{
  AE_PREVENT_RECURSION

  if ((!bForce) && (Type == m_CurrentlyEditedType))
    return;

  bool bNeedsUpdate = s_bTreeNeedsUpdate;
  bool bSimpleChange = s_bTreeHasSimpleChanges;

  m_CurrentlyEditedType = Type;
  m_pCurNT = &g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[m_CurrentlyEditedType];

  // General
  {
    spin_RandomSeed->setValue(g_Tree.m_Descriptor.m_StructureDesc.m_uiRandomSeed);
    check_LodSelection->setChecked(g_Globals.s_bSelectLodAutomatically);
    actionShowHideLeaves->setChecked(g_Globals.s_bRenderLeaves);
    SpinCrossFade->setValue(g_Tree.m_Descriptor.m_fLodCrossFadeTransition);

    SpinCrossFade->setEnabled(check_LodSelection->isChecked());
  }

  // Branch Type
  {
    SpinBranchSegmentLength->setValue(m_pCurNT->m_iSegmentLengthCM);

    SpinMinBranchThickness->setValue(m_pCurNT->m_uiMinBranchThicknessInCM);
    SpinMaxBranchThickness->setValue(m_pCurNT->m_uiMaxBranchThicknessInCM);

    //le_BranchTexture->setText (m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Branch].c_str ());
    //le_BranchTextureN->setText (m_pCurNT->m_sTextureN[Kraut::BranchGeometryType::Branch].c_str ());

    SliderLowerBound->setValue(m_pCurNT->m_uiLowerBound);
    SliderUpperBound->setValue(m_pCurNT->m_uiUpperBound);

    spin_BBoxAdjustment->setValue(g_Tree.m_Descriptor.m_fBBoxAdjustment);
  }

  // Simulation
  {
    check_VisualizeCollisionMesh->setChecked(g_Globals.s_bRenderCollisionMesh);
    check_EnableCollisionAvoidance->setChecked(g_Globals.s_bDoPhysicsSimulation);
    check_ShowCollisionObjects->setChecked(g_Globals.s_bShowCollisionObjects);
  }

  // Growth
  {
    slider_BranchGrowDirDev->setValue(m_pCurNT->m_fGrowMaxTargetDirDeviation);
    spin_BranchSegmentDirChange->setValue(m_pCurNT->m_fGrowMaxDirChangePerSegment);

    check_TargetDirRelative->setChecked(m_pCurNT->m_bTargetDirRelative);
    combo_BranchTargetDir->setCurrentIndex(m_pCurNT->m_TargetDirection);
    combo_BranchTargetDir2->setCurrentIndex(m_pCurNT->m_TargetDirection2);

    combo_BranchSecondDirMode->setCurrentIndex(m_pCurNT->m_TargetDir2Uage);

    slider_SecondDirUsage->setEnabled(m_pCurNT->m_TargetDir2Uage != Kraut::BranchTargetDir2Usage::Off);
    combo_BranchTargetDir2->setEnabled(m_pCurNT->m_TargetDir2Uage != Kraut::BranchTargetDir2Usage::Off);

    slider_SecondDirUsage->setValue((aeInt32)(m_pCurNT->m_fTargetDir2Usage * 100.0f));

    //slider_BranchMinThickness->setValue (aeMath::Clamp ((int) (m_pCurNT->m_fMinBranchThickness * 100.0f), 1, 100));
    SpinMinBranchThickness->setValue(m_pCurNT->m_uiMinBranchThicknessInCM);
    SpinMaxBranchThickness->setValue(m_pCurNT->m_uiMaxBranchThicknessInCM);

    slider_BranchRoundness->setValue(m_pCurNT->m_fRoundnessFactor * 100.0f);

    check_ActAsObstacle->setChecked(m_pCurNT->m_bActAsObstacle);
    check_EnableObstacleAvoidance->setChecked(m_pCurNT->m_bDoPhysicalSimulation);
    slider_ObstacleLookAhead->setValue(m_pCurNT->m_fPhysicsLookAhead * 100.0f);
    slider_ObstacleEvasionAngle->setValue(m_pCurNT->m_fPhysicsEvasionAngle);
  }

  // Shape
  {
    curve_MaxBranchLengthParent->SetSamples(&m_pCurNT->m_MaxBranchLengthParentScale, "Max Length relative to Parent");

    curve_BranchContour->SetSamples(&m_pCurNT->m_BranchContour, "Branch Contour");

    spin_MinBranchLength->setValue(m_pCurNT->m_uiMinBranchLengthInCM / 100.0);
    spin_MaxBranchLength->setValue(m_pCurNT->m_uiMaxBranchLengthInCM / 100.0);

    SpinBranchlessPartStart->setValue(m_pCurNT->m_fBranchlessPartABS);
    SpinBranchlessPartEnd->setValue(m_pCurNT->m_fBranchlessPartEndABS);
  }

  // Branching
  {
    SpinFreeSpaceBeforeNode->setValue(m_pCurNT->m_fNodeSpacingBefore);
    SpinFreeSpaceAfterNode->setValue(m_pCurNT->m_fNodeSpacingAfter);

    SpinMinBranchesPerNode->setValue(m_pCurNT->m_uiMinBranches);
    SpinMaxBranchesPerNode->setValue(m_pCurNT->m_uiMaxBranches);
    SpinNodeHeight->setValue(m_pCurNT->m_fNodeHeight);
    ComboBranchTypeMode->setCurrentIndex((int)m_pCurNT->m_BranchTypeMode);

    slider_BranchRotationalDeviation->setValue(m_pCurNT->m_fMaxRotationalDeviation);
    slider_BranchAngle->setValue(m_pCurNT->m_fBranchAngle);
    slider_BranchAngleDeviation->setValue(m_pCurNT->m_fMaxBranchAngleDeviation);
    slider_BranchDirectionDeviation->setValue(m_pCurNT->m_fMaxTargetDirDeviation);
  }

  // Leaf Cards
  {
    check_LeafCardMode->setChecked(g_Tree.m_Descriptor.m_StructureDesc.m_bLeafCardMode);
    check_FromAbove->setChecked(g_Tree.m_Descriptor.m_bSnapshotFromAbove);

    check_PreviewLeafCard->setChecked(g_Globals.s_bPreviewLeafCard);

    combo_MaxLeafCardResolution->setCurrentIndex((int)g_Tree.m_Descriptor.m_LeafCardResolution);
    combo_LeafCardMipmapResolution->setCurrentIndex((int)g_Tree.m_Descriptor.m_LeafCardMipmaps);
    combo_LeafCardColorDilation->setCurrentIndex((int)g_Tree.m_Descriptor.m_uiLeafCardTexelDilation);
  }

  //// Manual Editing
  //{
  //  check_ModifyLocal->setChecked(g_Globals.s_bModifyLocally);
  //  slider_ModifyStrength->setValue(g_Globals.s_uiModifyStrength);

  //  check_VisPaintingPlane->setChecked(g_Globals.s_bVisualizePaintingPlane);

  //  radio_PaintInViewplane->setChecked(g_Globals.s_PaintingPlaneMode == aePaintingPlaneMode::InViewPlane);
  //  radio_PaintUpwards->setChecked(g_Globals.s_PaintingPlaneMode == aePaintingPlaneMode::Upwards);
  //  radio_PaintPlanar->setChecked(g_Globals.s_PaintingPlaneMode == aePaintingPlaneMode::Planar);
  //  radio_PaintAlongBranch->setChecked(g_Globals.s_PaintingPlaneMode == aePaintingPlaneMode::AlongBranch);
  //  radio_PaintOrthogonal->setChecked(g_Globals.s_PaintingPlaneMode == aePaintingPlaneMode::OrthogonalToBranch);

  //  check_CreateProcedural->setChecked(g_Tree.m_Descriptor.m_StructureDesc.m_bGrowProceduralTrunks);
  //}

  // Fronds
  {
    UpdateBranchTypeModeGUI();

    check_EnableFronds->setChecked(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Frond]);
    widgetFrondContent->setVisible(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Frond]);
    check_RestrictGrowToUpPlane->setChecked(m_pCurNT->m_bRestrictGrowthToFrondPlane);
    combo_FrondOrientation->setCurrentIndex((int)m_pCurNT->m_FrondUpOrientation);
    combo_FrondContourMode->setCurrentIndex((int)m_pCurNT->m_FrondContourMode);
    slider_FrondRotation->setValue(m_pCurNT->m_uiMaxFrondOrientationDeviation);

    //le_FrondTexture->setText (m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Frond].c_str ());
    //le_FrondTextureN->setText (m_pCurNT->m_sTextureN[Kraut::BranchGeometryType::Frond].c_str ());

    spin_Fronds->setValue(m_pCurNT->m_uiNumFronds);
    spin_FrondDetail->setValue(m_pCurNT->m_uiFrondDetail);
    curve_FrondContour->SetSamples(&m_pCurNT->m_FrondContour, "Frond Contour");
    SpinFrondWidth->setValue(m_pCurNT->m_fFrondWidth);
    SpinFrondHeight->setValue(m_pCurNT->m_fFrondHeight);
    curve_FrondWidth->SetSamples(&m_pCurNT->m_FrondWidth, "Frond Width");
    curve_FrondHeight->SetSamples(&m_pCurNT->m_FrondHeight, "Frond Contour Scale");
    spinFrondTextureRepeat->setValue(m_pCurNT->m_fTextureRepeat);
    checkAlignFronds->setChecked(m_pCurNT->m_bAlignFrondsOnSurface);

    // Frond Color
    {
      QColor col;
      col.setRgb(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][0], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][1], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][2], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3]);
      const QString COLOR_STYLE("QPushButton { background-color : %1 }");
      button_FrondColor->setStyleSheet(COLOR_STYLE.arg(col.name()));
      sliderFrondAlpha->setValue(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Frond][3]);
    }

    // Leaf Color
    {
      QColor col;
      col.setRgb(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2], m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3]);
      const QString COLOR_STYLE("QPushButton { background-color : %1 }");
      ButtonLeafColor->setStyleSheet(COLOR_STYLE.arg(col.name()));
      SliderLeafAlpha->setValue(m_pCurNT->m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3]);
    }
  }

  // Leaves
  {
    CheckEnableLeaves->setChecked(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Leaf]);
    WidgetLeafContent->setVisible(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Leaf]);
    SpinLeafSize->setValue(m_pCurNT->m_fLeafSize);
    SpinLeafInterval->setValue(m_pCurNT->m_fLeafInterval);
    //EditLeafTexture->setText (m_pCurNT->m_sTexture[Kraut::BranchGeometryType::Leaf].c_str ());
    //EditLeafTextureN->setText (m_pCurNT->m_sTextureN[Kraut::BranchGeometryType::Leaf].c_str ());
    CurveLeafScale->SetSamples(&m_pCurNT->m_LeafScale, "Leaf Scale");
    CheckBillboardLeaves->setChecked(m_pCurNT->m_bBillboardLeaves);
  }

  // Flares
  {
    spin_NumFlares->setValue(m_pCurNT->m_uiFlares);
    spin_FlareWidth->setValue(m_pCurNT->m_fFlareWidth);
    curve_FlareWidth->SetSamples(&m_pCurNT->m_FlareWidthCurve, "Flare Scale");
    spin_FlareRotation->setValue((int)m_pCurNT->m_fFlareRotation);
    check_RotateTexCoords->setChecked(m_pCurNT->m_bRotateTexCoords);
  }

  // Branch
  {
    check_EnableBranchMesh->setChecked(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Branch]);
    widgetBranchContent->setVisible(m_pCurNT->m_bEnable[Kraut::BranchGeometryType::Branch]);
  }

  // Ambient Occlusion
  {
    groupAmbientOcclusion->setChecked(g_Tree.m_Descriptor.m_bUseAO);
    spinAOSampleSize->setValue(g_Tree.m_Descriptor.m_fAOSampleSize);
    spinAOContrast->setValue(g_Tree.m_Descriptor.m_fAOContrast);
  }

  // Extras
  {
    spinGamma->setValue(CVar_Gamma);

    UpdateAmbientShaderColors();
  }

  // Forces
  {
    UpdateForcesUI();
  }

  UpdateMaterialBoxes();

  SetCurrentlyEditedLoD(g_Globals.s_CurLoD);

  s_bTreeNeedsUpdate = bNeedsUpdate;
  s_bTreeHasSimpleChanges = bSimpleChange;

  AE_BROADCAST_EVENT(aeTreePlugin_SelectedBranchTypeChanged);
}

void qtTreeEditWidget::SetCurrentlyEditedLoD(aeLod::Enum LoD)
{
  AE_PREVENT_RECURSION

  g_Tree.m_Descriptor.ClampLodValues(g_Globals.s_CurLoD);

  bool bNeedsUpdate = s_bTreeNeedsUpdate;
  bool bSimpleChanges = s_bTreeHasSimpleChanges;
  g_Globals.s_CurLoD = LoD;

  const bool bDisableLodSettings = (g_Globals.s_CurLoD == aeLod::None) || (Kraut::LodMode::IsImpostorMode(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_Mode) || g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_Mode == Kraut::LodMode::Disabled);

  group_LODSettings->setDisabled(bDisableLodSettings);
  group_AllowBranchTypes->setDisabled(bDisableLodSettings);
  group_AllowFrondTypes->setDisabled(bDisableLodSettings);
  group_AllowLeafTypes->setDisabled(bDisableLodSettings);
  combo_LodMode->setDisabled(g_Globals.s_CurLoD == aeLod::None || g_Globals.s_CurLoD == aeLod::Lod0);

  combo_LodMode->setCurrentIndex((int)g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_Mode);
  spin_CurvatureThreshold->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fCurvatureThreshold);
  spin_ThicknessThreshold->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fThicknessThreshold);
  spin_RingDetailThreshold->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fVertexRingDetail);
  spin_TipDetailThreshold->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fTipDetail);
  spin_LodDistance->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_uiLodDistance);
  spin_MaxFrondDetail->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_iMaxFrondDetail);
  spin_FrondDetailReduction->setValue(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_iFrondDetailReduction);


  QCheckBox* cbLod[Kraut::BranchGeometryType::ENUM_COUNT][Kraut::BranchType::ENUM_COUNT] =
    {
      {check_LodTrunk1, check_LodTrunk2, check_LodTrunk3, check_LodMainBranches1, check_LodMainBranches2, check_LodMainBranches3, check_LodSubBranches1, check_LodSubBranches2, check_LodSubBranches3, check_LodTwigs1, check_LodTwigs2, check_LodTwigs3},
      {check_LodTrunk1_2, check_LodTrunk2_2, check_LodTrunk3_2, check_LodMainBranches1_2, check_LodMainBranches2_2, check_LodMainBranches3_2, check_LodSubBranches1_2, check_LodSubBranches2_2, check_LodSubBranches3_2, check_LodTwigs1_2, check_LodTwigs2_2, check_LodTwigs3_2},
      {check_LodTrunk1_3, check_LodTrunk2_3, check_LodTrunk3_3, check_LodMainBranches1_3, check_LodMainBranches2_3, check_LodMainBranches3_3, check_LodSubBranches1_3, check_LodSubBranches2_3, check_LodSubBranches3_3, check_LodTwigs1_3, check_LodTwigs2_3, check_LodTwigs3_3},
    };

  for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
  {
    for (aeUInt32 bt = 0; bt < Kraut::BranchType::ENUM_COUNT; ++bt)
    {
      cbLod[mt][bt]->setChecked(g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_AllowTypes[mt].IsAnyFlagSet(1 << bt));
      cbLod[mt][bt]->setVisible(g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_bUsed && g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_bEnable[mt]);
    }
  }

  s_bTreeNeedsUpdate = bNeedsUpdate;
  s_bTreeHasSimpleChanges = bSimpleChanges;

  s_bStatsNeedUpdate = true;
  g_Globals.s_bUpdatePickingBuffer = true;
}

bool qtTreeEditWidget::SelectFile(aeFilePath& sPrevFile, aeHybridString<128>& sTargetString, QLineEdit* pLineEdit, const char* szFileTypeFilter, const char* szContentSubFolder)
{
  // get the current files's full path
  {
    aeFileIn File;
    if (File.Open(sTargetString.c_str()))
      sPrevFile = File.GetFilePath().c_str();
    else
      aeFileSystem::MakeValidPath(szContentSubFolder, false, &sPrevFile, nullptr);
  }

  QString result = QFileDialog::getOpenFileName(qtTreeEditWidget::s_pWidget,
    QFileDialog::tr("Select File"), sPrevFile.c_str(),
    QFileDialog::tr(szFileTypeFilter), 0);

  if (result.size() == 0)
    return false;

  sPrevFile = result.toLatin1().data();

  aeFilePath sNewFilePath;

  // get its relative path and check that it is in a data directory
  {
    aeFileIn File;
    if (File.Open(sPrevFile.c_str()))
      sNewFilePath = File.GetFilePathInDataDir().c_str();
  }

  if ((sNewFilePath.empty()) || (aePathFunctions::IsAbsolutePath(sNewFilePath.c_str())))
  {
    QMessageBox::warning(qtTreeEditWidget::s_pWidget, "Kraut", "The selected file must be located in a data-directory of the editor.\nPlease select a file somewhere in the 'Content' or 'TreePlugin' folder.");
    return false;
  }

  if (pLineEdit)
    pLineEdit->setText(sNewFilePath.c_str());

  sTargetString = sNewFilePath;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);

  return true;
}

bool qtTreeEditWidget::SelectTexture(aeFilePath& sPrevTexture, aeHybridString<128>& sTargetString, QLineEdit* pLineEdit)
{
  return SelectFile(sPrevTexture, sTargetString, pLineEdit, "DDS-Files (*.dds);;TGA-Files (*.tga);;Texture-Files (*.tga;*.dds);;All Files (*.*)", "Textures/");
}

void qtTreeEditWidget::UpdateBranchTypeModeGUI(void)
{
  const bool bEnableGUI = m_pCurNT->m_BranchTypeMode != Kraut::BranchTypeMode::Umbrella;

  if (m_pCurNT->m_BranchTypeMode == Kraut::BranchTypeMode::Umbrella)
    m_pCurNT->m_FrondContourMode = Kraut::SpawnNodeDesc::Symetric;

  combo_FrondOrientation->setCurrentIndex(m_pCurNT->m_FrondContourMode);

  checkAlignFronds->setEnabled(bEnableGUI);
  slider_FrondRotation->setEnabled(bEnableGUI);
  combo_FrondOrientation->setEnabled(bEnableGUI);
  spin_Fronds->setEnabled(bEnableGUI);
  combo_FrondContourMode->setEnabled(bEnableGUI);
  SpinFrondWidth->setEnabled(bEnableGUI);
  curve_FrondWidth->setEnabled(bEnableGUI);
}
