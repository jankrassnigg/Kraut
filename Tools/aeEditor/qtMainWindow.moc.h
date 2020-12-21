#ifndef QTMAINWINDOW_H
#define QTMAINWINDOW_H

#include <shobjidl.h>
#include <windows.h>
#include <QWidget>
#include <QMainWindow>
#include <Tools/aeEditor/ui_qtMainWindow.h>

class GLWidget;
class qtTreeEditWidget;

struct aeWin7ProgressBarState
{
  enum Enum
  {
    Disabled,   // no progress bar
    Normal,     // green
    Paused,     // yellow
    Error,      // red
    Unknown,    // indeterminate
  };
};

class qtMainWindow : public QMainWindow, public Ui_MainWindow
{
  Q_OBJECT

public:
  qtMainWindow (QWidget *parent = 0);
  ~qtMainWindow ();

  void SaveLayout (const char* szFile) const;
  void LoadLayout (const char* szFile);

  virtual void closeEvent (QCloseEvent* event);
  void SetWin7ProgressBarState (aeWin7ProgressBarState::Enum state, unsigned int uiProMille);

protected:
  void keyPressEvent(QKeyEvent *event);
  void keyReleaseEvent(QKeyEvent *event);
  virtual bool winEvent (MSG* message, long* result);

private:
  QDockWidget* m_pTreeEditDockWidget;
  qtTreeEditWidget* m_pTreeEditWidget;
  UINT taskBarCreatedMessage;
  ITaskbarList3* m_pWin7Taskbar;
};


#endif
