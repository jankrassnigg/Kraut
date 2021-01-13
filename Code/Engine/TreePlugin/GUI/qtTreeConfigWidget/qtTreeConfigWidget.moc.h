#ifndef QTTTREECONFIGWIDGET_H
#define QTTTREECONFIGWIDGET_H

#include "../../Basics/Enums.h"
#include <Code/Engine/TreePlugin/ui_qtTreeConfigWidget.h>
#include <KrautGenerator/Description/DescriptionEnums.h>

class QToolBar;
class QGraphicsRectItem;

class qtTreeConfigWidget : public QDockWidget, public Ui_TreeConfigWidget
{
  Q_OBJECT

public:
  qtTreeConfigWidget(QWidget* parent = 0);
  ~qtTreeConfigWidget();

  static qtTreeConfigWidget* s_pWidget;

  void UpdateDisplay(void);
  void UpdateHighlights(bool bClick, QMouseEvent* event = nullptr);
  void EnsureVisible(void);

public slots:
  virtual void on_actionToggleVisibility_triggered();
  virtual void on_actionRemoveTreeComponent_triggered();
  virtual void on_actionAddTreeComponent_triggered();
  virtual void on_actionAddTrunkComponent_triggered();
  virtual void on_actionSaveTreeComponent_triggered();
  virtual void on_actionLoadTreeComponent_triggered();
  virtual void on_actionToggleUpConnection1_triggered();
  virtual void on_actionToggleUpConnection2_triggered();
  virtual void on_actionToggleUpConnection3_triggered();
  virtual void on_actionToggleDownConnection1_triggered();
  virtual void on_actionToggleDownConnection2_triggered();
  virtual void on_actionToggleDownConnection3_triggered();

private:
  QToolBar* m_pToolbar;
  void SetupToolbars(void);

  QGraphicsScene m_Scene;
  QGraphicsRectItem* m_pSpheres[Kraut::BranchType::ENUM_COUNT];
};

#endif
