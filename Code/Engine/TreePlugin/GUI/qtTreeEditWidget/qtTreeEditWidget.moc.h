#ifndef QTTREEEDITWIDGET_H
#define QTTREEEDITWIDGET_H

#include "../../Tree/Tree.h"
#include "../../Undo/TreeUndo.h"
#include <Code/Engine/TreePlugin/ui_qtTreeEditWidget.h>
#include <QDockWidget>
#include <QStringListModel>


#define SLIDER_UNDO(name)                                                          \
  void qtTreeEditWidget::on_##name##_sliderPressed() { aeUndo::BeginOperation(); } \
  void qtTreeEditWidget::on_##name##_sliderReleased() { aeUndo::EndOperation(); }


class qtTreeEditWidget : public QDockWidget, public Ui_TreeEditWidget
{
  Q_OBJECT

public:
  qtTreeEditWidget(QWidget* parent = 0);

  static qtTreeEditWidget* s_pWidget;

  void UpdateMaterialBoxes(void);
  void SetCurrentlyEditedBranchType(Kraut::BranchType::Enum Type, bool bForce);
  void SetCurrentlyEditedLoD(aeLod::Enum LoD);
  Kraut::BranchType::Enum GetCurrentlyEditedBranchType(void) const { return m_CurrentlyEditedType; }
  bool SaveTree(void);
  bool SaveTreeAs(void);

  void UpdateStats();
  void UpdateRecentFileMenu(void);

  static aeFilePath s_sExportLeafCardPath;

  static bool SelectFile(aeFilePath& sPrevTexture, aeHybridString<128>& sTargetString, QLineEdit* pLineEdit, const char* szFileTypeFilter, const char* szContentSubFolder);
  static bool SelectTexture(aeFilePath& sPrevTexture, aeHybridString<128>& sTargetString, QLineEdit* pLineEdit);

private:
  void SetupToolbars(void);
  void SetDefaultRenderModeButton(aeTreeRenderMode::Enum Mode);
  void SetupShortcuts(void);
  void GetAllowedLodMeshes(void);

  Kraut::BranchType::Enum m_CurrentlyEditedType;
  Kraut::SpawnNodeDesc* m_pCurNT;


public slots:


  // ***** General *****

  virtual void on_spin_RandomSeed_valueChanged(int i);
  virtual void on_actionOpenRecentFile_triggered(QAction* pAction);


  // ***** LOD Tab *****

  virtual void on_combo_LoD_currentIndexChanged(int index);
  virtual void on_check_LodSelection_clicked();
  virtual void on_SpinCrossFade_valueChanged(double d);

  virtual void on_combo_LodMode_currentIndexChanged(int index);
  virtual void on_spin_TipDetailThreshold_valueChanged(double d);
  virtual void on_spin_CurvatureThreshold_valueChanged(double d);
  virtual void on_spin_ThicknessThreshold_valueChanged(double d);
  virtual void on_spin_RingDetailThreshold_valueChanged(double d);

  virtual void on_spin_LodDistance_valueChanged(int i);
  virtual void on_spin_MaxFrondDetail_valueChanged(int i);
  virtual void on_spin_FrondDetailReduction_valueChanged(int i);



  // ***** Editing Tab *****

  virtual void on_button_MakeEditable_clicked();
  virtual void on_button_DeleteBranch_clicked();
  virtual void on_check_ModifyLocal_clicked();
  virtual void on_slider_ModifyStrength_valueChanged();
  virtual void on_check_VisPaintingPlane_clicked();
  virtual void on_radio_PaintInViewplane_clicked();
  virtual void on_radio_PaintUpwards_clicked();
  virtual void on_radio_PaintPlanar_clicked();
  virtual void on_radio_PaintAlongBranch_clicked();
  virtual void on_radio_PaintOrthogonal_clicked();
  virtual void on_check_CreateProcedural_clicked();
  virtual void on_actionDeleteBranch_triggered();


  // ***** Simulation Tab *****

  virtual void on_check_EnableCollisionAvoidance_clicked();
  virtual void on_check_VisualizeCollisionMesh_clicked();
  virtual void on_spin_ColObjScalingX_valueChanged(double val);
  virtual void on_spin_ColObjScalingY_valueChanged(double val);
  virtual void on_spin_ColObjScalingZ_valueChanged(double val);
  virtual void on_button_AddCollisionObject_clicked();
  virtual void on_button_DeleteCollisionObject_clicked();
  virtual void on_list_CollisionObjects_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
  virtual void on_check_ShowCollisionObjects_clicked();


  // ***** Leafcards Tab *****

  virtual void on_check_LeafCardMode_clicked();
  virtual void on_pb_ExportLeafCard_clicked();
  virtual void on_check_PreviewLeafCard_clicked();
  virtual void on_tb_SelectLeafCardPath_clicked();
  virtual void on_combo_MaxLeafCardResolution_currentIndexChanged(int index);
  virtual void on_combo_LeafCardMipmapResolution_currentIndexChanged(int index);
  virtual void on_combo_LeafCardColorDilation_currentIndexChanged(int index);
  virtual void on_check_FromAbove_clicked();
  virtual void on_check_ExportDDS_clicked();


  // ***** Branch Type Tab *****

  virtual void on_SpinBranchSegmentLength_valueChanged(int i);
  virtual void on_SpinBranchlessPartStart_valueChanged(double d);
  virtual void on_SpinBranchlessPartEnd_valueChanged(double d);
  virtual void on_spin_BBoxAdjustment_valueChanged(double d);
  virtual void on_ComboBranchTypeMode_currentIndexChanged(int index);

  virtual void on_SpinMinBranchThickness_valueChanged(int i);
  virtual void on_SpinMaxBranchThickness_valueChanged(int i);

  virtual void on_SliderLowerBound_valueChanged();
  virtual void on_SliderLowerBound_sliderPressed();
  virtual void on_SliderLowerBound_sliderReleased();

  virtual void on_SliderUpperBound_valueChanged();
  virtual void on_SliderUpperBound_sliderPressed();
  virtual void on_SliderUpperBound_sliderReleased();

  // Branch Type: Spawn Nodes:
  virtual void on_SpinMinBranchesPerNode_valueChanged(int i);
  virtual void on_SpinMaxBranchesPerNode_valueChanged(int i);
  virtual void on_SpinFreeSpaceBeforeNode_valueChanged(double d);
  virtual void on_SpinFreeSpaceAfterNode_valueChanged(double d);
  virtual void on_SpinNodeHeight_valueChanged(double d);


  // ***** Growth Tab *****

  virtual void on_spin_MinBranchLength_valueChanged(double d);
  virtual void on_spin_MaxBranchLength_valueChanged(double d);
  virtual void on_spin_BranchSegmentDirChange_valueChanged(int i);
  virtual void on_check_RestrictGrowToUpPlane_clicked();

  virtual void on_slider_BranchGrowDirDev_valueChanged();
  virtual void on_slider_BranchGrowDirDev_sliderPressed();
  virtual void on_slider_BranchGrowDirDev_sliderReleased();

  virtual void on_slider_BranchRotationalDeviation_valueChanged();
  virtual void on_slider_BranchRotationalDeviation_sliderPressed();
  virtual void on_slider_BranchRotationalDeviation_sliderReleased();

  virtual void on_slider_BranchAngle_valueChanged();
  virtual void on_slider_BranchAngle_sliderPressed();
  virtual void on_slider_BranchAngle_sliderReleased();

  virtual void on_slider_BranchAngleDeviation_valueChanged();
  virtual void on_slider_BranchAngleDeviation_sliderPressed();
  virtual void on_slider_BranchAngleDeviation_sliderReleased();

  virtual void on_slider_BranchDirectionDeviation_valueChanged();
  virtual void on_slider_BranchDirectionDeviation_sliderPressed();
  virtual void on_slider_BranchDirectionDeviation_sliderReleased();

  // Growth: Target Direction
  virtual void on_check_TargetDirRelative_clicked();

  virtual void on_combo_BranchTargetDir_currentIndexChanged(int index);
  virtual void on_combo_BranchTargetDir2_currentIndexChanged(int index);

  virtual void on_combo_BranchSecondDirMode_currentIndexChanged(int index);

  virtual void on_slider_SecondDirUsage_valueChanged();
  virtual void on_slider_SecondDirUsage_sliderPressed();
  virtual void on_slider_SecondDirUsage_sliderReleased();

  // Growth: Obstacle Avoidance:
  virtual void on_check_ActAsObstacle_clicked();
  virtual void on_check_EnableObstacleAvoidance_clicked();

  virtual void on_slider_ObstacleLookAhead_valueChanged();
  virtual void on_slider_ObstacleLookAhead_sliderPressed();
  virtual void on_slider_ObstacleLookAhead_sliderReleased();

  virtual void on_slider_ObstacleEvasionAngle_valueChanged();
  virtual void on_slider_ObstacleEvasionAngle_sliderPressed();
  virtual void on_slider_ObstacleEvasionAngle_sliderReleased();



  // ***** Appearance Tab *****

  // Appearance: Branch Mesh:
  virtual void on_check_EnableBranchMesh_clicked();
  virtual void on_tb_SelectBranchTexture_clicked();
  //virtual void on_tb_SelectBranchTextureN_clicked ();
  virtual void on_slider_BranchRoundness_valueChanged();
  virtual void on_slider_BranchRoundness_sliderPressed();
  virtual void on_slider_BranchRoundness_sliderReleased();
  virtual void on_ComboBranchTexture_currentIndexChanged(int index);

  // Appearance: Flares:
  virtual void on_spin_NumFlares_valueChanged(int i);
  virtual void on_spin_FlareWidth_valueChanged(double d);
  virtual void on_spin_FlareRotation_valueChanged(int d);
  virtual void on_check_RotateTexCoords_clicked();

  // Appearance: Leaves:
  virtual void on_CheckEnableLeaves_clicked();
  virtual void on_SpinLeafSize_valueChanged(double d);
  virtual void on_SpinLeafInterval_valueChanged(double d);
  virtual void on_ButtonSelectLeafTexture_clicked();
  //virtual void on_ButtonSelectLeafTextureN_clicked ();
  virtual void on_ButtonLeafColor_clicked();
  virtual void on_LeafColor_changed(const QColor& color);
  virtual void on_LeafColor_reset();
  virtual void on_LeafColor_accepted();
  virtual void on_SliderLeafAlpha_valueChanged(int i);
  virtual void on_SliderLeafAlpha_sliderPressed();
  virtual void on_SliderLeafAlpha_sliderReleased();
  virtual void on_CheckBillboardLeaves_clicked();
  virtual void on_ComboLeafTexture_currentIndexChanged(int index);


  // Appearance: Fronds:
  virtual void on_check_EnableFronds_clicked();
  virtual void on_combo_FrondOrientation_currentIndexChanged(int index);
  virtual void on_slider_FrondRotation_valueChanged();
  virtual void on_slider_FrondRotation_sliderPressed();
  virtual void on_slider_FrondRotation_sliderReleased();
  virtual void on_tb_SelectFrondTexture_clicked();
  //virtual void on_tb_SelectFrondTextureN_clicked ();
  virtual void on_spin_Fronds_valueChanged(int i);
  virtual void on_spin_FrondDetail_valueChanged(int i);
  virtual void on_SpinFrondWidth_valueChanged(double d);
  virtual void on_SpinFrondHeight_valueChanged(double d);
  virtual void on_combo_FrondContourMode_currentIndexChanged(int index);
  virtual void on_button_FrondColor_clicked();
  virtual void on_FrondColor_changed(const QColor& color);
  virtual void on_FrondColor_reset();
  virtual void on_FrondColor_accepted();
  virtual void on_sliderFrondAlpha_valueChanged(int i);
  virtual void on_sliderFrondAlpha_sliderPressed();
  virtual void on_sliderFrondAlpha_sliderReleased();
  virtual void on_spinFrondTextureRepeat_valueChanged(double d);
  virtual void on_checkAlignFronds_clicked();
  virtual void on_ComboFrondTexture_currentIndexChanged(int index);

  // ***** Extras Tab *****

  virtual void on_groupAmbientOcclusion_clicked();
  virtual void on_buttonUpdateAO_clicked();
  virtual void on_spinAOSampleSize_valueChanged(double d);
  virtual void on_spinAOContrast_valueChanged(double d);
  virtual void on_spinGamma_valueChanged(double d);
  virtual void on_buttonAmbientColorLow_clicked();
  virtual void on_AmbientColorLow_changed(const QColor& color);
  virtual void on_AmbientColorLow_reset();
  virtual void on_buttonAmbientColorHigh_clicked();
  virtual void on_AmbientColorHigh_changed(const QColor& color);
  virtual void on_AmbientColorHigh_reset();
  virtual void on_buttonAmbientLowDefault_clicked();
  virtual void on_buttonAmbientHighDefault_clicked();


  // ***** Toolbar Actions *****

  virtual void on_action_New_triggered();
  virtual void on_action_Open_triggered();
  virtual void on_action_Save_triggered();
  virtual void on_action_SaveAs_triggered();
  virtual void on_action_Export_triggered();

  virtual void on_actionRenderDefault_triggered();
  virtual void on_actionRenderWireframe_triggered();
  virtual void on_actionRenderSkeleton_triggered();
  virtual void on_actionRenderNormals_triggered();
  virtual void on_actionRenderDiffuse_triggered();
  virtual void on_actionRenderSkylight_triggered();
  virtual void on_actionRenderLight_triggered();
  virtual void on_actionRenderAmbient_triggered();
  virtual void on_actionToggleRenderMode_triggered();

  virtual void on_actionShowHideLeaves_triggered();
  virtual void on_actionOrbitCamera_triggered();

  virtual void on_actionToggleGizmo_triggered();
  virtual void on_actionTranslate_triggered();
  virtual void on_actionRotate_triggered();
  virtual void on_actionHideGizmos_triggered();

  virtual void on_actionUndo_triggered();
  virtual void on_actionRedo_triggered();

  virtual void on_actionAbout_triggered();



  // ***** LOD Checkboxes: Branch Meshes *****

  virtual void on_check_LodTrunk1_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTrunk2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTrunk3_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodMainBranches1_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodMainBranches2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodMainBranches3_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodSubBranches1_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodSubBranches2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodSubBranches3_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodTwigs1_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTwigs2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTwigs3_clicked() { GetAllowedLodMeshes(); }


  // ***** LOD Checkboxes: Frond Meshes *****

  virtual void on_check_LodTrunk1_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTrunk2_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTrunk3_2_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodMainBranches1_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodMainBranches2_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodMainBranches3_2_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodSubBranches1_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodSubBranches2_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodSubBranches3_2_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodTwigs1_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTwigs2_2_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTwigs3_2_clicked() { GetAllowedLodMeshes(); }


  // ***** LOD Checkboxes: Leaf Meshes *****

  virtual void on_check_LodTrunk1_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTrunk2_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTrunk3_3_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodMainBranches1_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodMainBranches2_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodMainBranches3_3_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodSubBranches1_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodSubBranches2_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodSubBranches3_3_clicked() { GetAllowedLodMeshes(); }

  virtual void on_check_LodTwigs1_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTwigs2_3_clicked() { GetAllowedLodMeshes(); }
  virtual void on_check_LodTwigs3_3_clicked() { GetAllowedLodMeshes(); }

  // ***** Tab: Forces *****

  virtual void UpdateForcesUI(void);
  virtual void on_ButtonAddForce_clicked();
  virtual void on_ButtonDeleteForce_clicked();
  virtual void on_ListForces_itemSelectionChanged();
  virtual void on_ListForces_itemChanged(QListWidgetItem* item);
  virtual void on_ComboForceType_currentIndexChanged(int index);
  virtual void on_ComboForceFalloff_currentIndexChanged(int index);
  virtual void on_SpinForceStrength_valueChanged(double d);
  virtual void on_SpinForceMinRadius_valueChanged(double d);
  virtual void on_SpinForceMaxRadius_valueChanged(double d);
  virtual void on_ButtonSelectForceMesh_clicked();

  virtual void on_CheckForceTrunk1_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceTrunk2_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceTrunk3_clicked() { GetForceInfluences(); }

  virtual void on_CheckForceBranches1_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceBranches2_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceBranches3_clicked() { GetForceInfluences(); }

  virtual void on_CheckForceSubBranches1_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceSubBranches2_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceSubBranches3_clicked() { GetForceInfluences(); }

  virtual void on_CheckForceTwigs1_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceTwigs2_clicked() { GetForceInfluences(); }
  virtual void on_CheckForceTwigs3_clicked() { GetForceInfluences(); }

private:
  void GetForceInfluences(void);
  void UpdateBranchTypeModeGUI(void);

  aeMap<aeHybridString<128>, aeUInt32> m_MapBarkTextureToCombo;
  aeMap<aeHybridString<128>, aeUInt32> m_MapFrondTextureToCombo;
  aeMap<aeHybridString<128>, aeUInt32> m_MapLeafTextureToCombo;
};


#endif
