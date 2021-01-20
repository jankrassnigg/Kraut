#ifndef QTEXPORTWIDGET_H
#define QTEXPORTWIDGET_H

#include "../../Basics/Base.h"
#include <Code/Engine/TreePlugin/ui_qtExportWidget.h>
#include <KrautGraphics/Strings/PathFunctions.h>

class qtExportWidget : public QDialog, public Ui_ExportWidget
{
  Q_OBJECT

public:
  // Export State

  static aeString s_sExportPath;
  static bool s_bExportBranches;
  static bool s_bExportFronds;
  static bool s_bExportLeaves;
  static bool s_bCapTrunk;
  static bool s_bCapBranches;
  static bool s_bExportLODs[6];
  static bool s_bWantedToRun;

public:
  qtExportWidget(QWidget* parent = 0);
  ~qtExportWidget();

  static void RunViewer(void);

public slots:
  virtual void on_bt_Export_clicked();
  virtual void on_bt_ExportAndRun_clicked();
  virtual void on_bt_SelectFile_clicked();
  virtual void on_bt_SelectViewerApp_clicked();
  virtual void on_bt_ViewerAppArgsHelp_clicked();

private:
  void Export(void);
};


#endif
