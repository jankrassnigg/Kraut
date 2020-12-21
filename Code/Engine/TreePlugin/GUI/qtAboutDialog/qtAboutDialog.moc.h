#ifndef QTABOUTDIALOG_H
#define QTABOUTDIALOG_H

#include <Code/Engine/TreePlugin/ui_qtAboutDialog.h>

class qtAboutDialog : public QDialog, public Ui_AboutDialog
{
  Q_OBJECT

public:
  qtAboutDialog (QWidget *parent = 0);
  ~qtAboutDialog ();

public slots:
  //virtual void on_bt_Export_clicked ();
};


#endif
