#ifndef QTCURVEEDITWIDGET_H
#define QTCURVEEDITWIDGET_H

#include <Code/Engine/TreePlugin/ui_qtCurveEditorWidget.h>
#include <QDockWidget>
#include <QGraphicsView>
#include <QtGui>

class qtCurveProperty;

namespace Kraut
{
  struct Curve;
}

class QToolButton;
class QToolBar;

class qtCurveEditorWidget : public QDockWidget, public Ui_CurveEditorWidget
{
  Q_OBJECT

public:
  qtCurveEditorWidget(QWidget* parent = 0);

  static qtCurveEditorWidget* s_pWidget;

  void SetActiveCurveProperty(qtCurveProperty* pCurve);

  qtCurveProperty* GetProperty() const { return m_pCurveView->m_pActiveCurve; }

  Kraut::Curve* GetCurve() const;

public slots:

  // Curve Presets
  virtual void on_actionPresetMin_triggered();
  virtual void on_actionPresetMax_triggered();
  virtual void on_actionPresetLinearGrowth_triggered();
  virtual void on_actionPresetLinearDecay_triggered();
  virtual void on_actionPresetRound_triggered();

  virtual void on_actionPresetRise_triggered();
  virtual void on_actionPresetDroop_triggered();
  virtual void on_actionPresetBellCurve_triggered();
  virtual void on_actionPresetConical_triggered();
  virtual void on_actionPresetExponentialFalloff_triggered();
  virtual void on_actionPresetExponentialGrowth_triggered();
  virtual void on_actionPresetFadeInOut_triggered();
  virtual void on_actionPresetScurve_triggered();

  virtual void on_actionInvertValues_triggered();
  virtual void on_actionMirrorValues_triggered();

  virtual void on_actionCopyCurve_triggered();
  virtual void on_actionPasteCurve_triggered();
  virtual void on_actionSmoothCurve_triggered();

  virtual void on_actionSavePreset_triggered();
  virtual void on_actionLoadPreset_triggered();


private:
  QToolButton* m_pPresetMenu;
  QToolBar* m_pToolbar;

  void SetupToolbars(void);
};

#endif
