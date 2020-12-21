#ifndef QTRESOURCEEDITORWIDGET_H
#define QTRESOURCEEDITORWIDGET_H

#include <QDockWidget>
#include <Code/Engine/TreePlugin/ui_qtResourceEditorWidget.h>
#include <QStringListModel>
#include "../../Undo/TreeUndo.h"
#include "../../Tree/Tree.h"
#include "MaterialLibrary.h"


class qtResourceEditorWidget : public QDockWidget, public Ui_ResourceEditorWidget
{
  Q_OBJECT

public:
  qtResourceEditorWidget (QWidget* parent = 0);

  static qtResourceEditorWidget* s_pWidget;

public slots:

  // *** Tab: Materials ***
  virtual void on_ButtonMaterialAdd_clicked ();
  virtual void on_ButtonMaterialDelete_clicked ();
  virtual void on_ComboMaterials_currentIndexChanged (int index);
  virtual void on_ButtonSelectTextureNormal_clicked ();
  virtual void on_RadioMaterialUndef_clicked ();
  virtual void on_RadioMaterialBark_clicked ();
  virtual void on_RadioMaterialFrond_clicked ();
  virtual void on_RadioMaterialLeaf_clicked ();
  virtual void on_SpinMaterialTilingX_valueChanged (int i);
  virtual void on_SpinMaterialTilingY_valueChanged (int i);
  //virtual void on_LineMaterialDiffuse_cursorPositionChanged ();
  //virtual void on_LineMaterialNormal_cursorPositionChanged ();


public:
  void UpdateMaterialList (void);

private:
  virtual bool eventFilter (QObject* object, QEvent* event);
  void SetSelectedMaterial (const char* szMaterial);

  aeHybridString<128> m_sSelectedTreeMaterial;
  aeMap<aeHybridString<128>, aeInt32> m_MaterialToComboIndex;

  aeInt8 m_iPreviewTexture;
};


#endif

