#include "PCH.h"
#include "Foundation.h"
#include <QtGui>
#include "qt3DWidget.moc.h"
#include "qtMainWindow.moc.h"
#include "LastInclude.h"
#include <KrautEditorBasics/Plugin.h>

using namespace AE_NS_EDITORBASICS;

qtMainWindow::qtMainWindow (QWidget *parent) : QMainWindow (parent)
{
  m_pWin7Taskbar = NULL;

  qtMainWindow* pMainWindow = this;
  pMainWindow->setObjectName ("aeEditor::MainWidget");

  // store the qt mainwindow pointer
  aeVariableRegistry::StoreRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  // Request task bar.
  taskBarCreatedMessage = RegisterWindowMessage("TaskbarButtonCreated");

  setupUi(this);

  setCentralWidget (m_p3DWidget);

  AE_BROADCAST_EVENT (aeMainWindow_SetupUI_Begin);

  AE_BROADCAST_EVENT (aeMainWindow_SetupUI_End);

  LoadLayout ("<APP>/Layout.qt");
}

qtMainWindow::~qtMainWindow ()
{
}

bool qtMainWindow::winEvent (MSG* message, long* result)
{
  if (message->message == taskBarCreatedMessage)
  {
    HRESULT hr = CoCreateInstance (CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&m_pWin7Taskbar));

    *result = hr;
    return true;
  }

  return false;
}

void qtMainWindow::SetWin7ProgressBarState (aeWin7ProgressBarState::Enum state, unsigned int uiProMille)
{
  if (m_pWin7Taskbar) 
  {
    switch (state)
    {
    case aeWin7ProgressBarState::Disabled:
      m_pWin7Taskbar->SetProgressValue ((HWND)winId(), 0, 1000);
      m_pWin7Taskbar->SetProgressState ((HWND)winId(), TBPF_NOPROGRESS);
      break;
    case aeWin7ProgressBarState::Normal:
      m_pWin7Taskbar->SetProgressValue ((HWND)winId(), uiProMille, 1000);
      m_pWin7Taskbar->SetProgressState ((HWND)winId(), TBPF_NORMAL);
      break;
    case aeWin7ProgressBarState::Paused:
      m_pWin7Taskbar->SetProgressValue ((HWND)winId(), uiProMille, 1000);
      m_pWin7Taskbar->SetProgressState ((HWND)winId(), TBPF_PAUSED);
      break;
    case aeWin7ProgressBarState::Error:
      m_pWin7Taskbar->SetProgressValue ((HWND)winId(), uiProMille, 1000);
      m_pWin7Taskbar->SetProgressState ((HWND)winId(), TBPF_ERROR);
      break;
    case aeWin7ProgressBarState::Unknown:
      m_pWin7Taskbar->SetProgressState ((HWND)winId(), TBPF_INDETERMINATE);
      break;
    }
  }
}

AE_ON_GLOBAL_EVENT (aeEditor_SetTaskbarProgress)
{
  qtMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  pMainWindow->SetWin7ProgressBarState ((aeWin7ProgressBarState::Enum) param0.iInt32, param1.iInt32);
}

void qtMainWindow::keyPressEvent(QKeyEvent* event)
{
  //if (event->key() == Qt::Key_Escape)
  //  close();
  //else
  {
    aeEditorPluginEvent e;
    e.m_Type = aeEditorPluginEvent::KeyPress;
    e.m_Data.m_KeyEvent = event;

    aeEditorPlugin::s_EditorPluginEvent.RaiseEvent (&e);

    QWidget::keyPressEvent(event);
  }
}

void qtMainWindow::keyReleaseEvent(QKeyEvent* event)
{
  {
    aeEditorPluginEvent e;
    e.m_Type = aeEditorPluginEvent::KeyRelease;
    e.m_Data.m_KeyEvent = event;

    aeEditorPlugin::s_EditorPluginEvent.RaiseEvent (&e);

    QWidget::keyReleaseEvent (event);
  }
}

void qtMainWindow::closeEvent (QCloseEvent* event)
{
  SaveLayout ("<APP>/Layout.qt");

  bool bClose = true;

  AE_BROADCAST_EVENT (aeEditor_Close, &bClose);

  if (bClose)
    event->accept ();
  else
    event->ignore ();
}

void qtMainWindow::SaveLayout (const char* szFile) const
{
  aeFileOut File;
  if (File.Open (szFile))
  {
    aeUInt32 uiVersion = 1;
    File << uiVersion;

    QByteArray store1 = saveState ();
    QByteArray store2 = saveGeometry ();

    aeUInt32 uiBytes1 = store1.size ();
    File << uiBytes1;

    for (int i = 0; i < store1.size (); ++i)
    {
      unsigned char uc = store1[i];
      File << uc;
    }

    aeUInt32 uiBytes2 = store2.size ();
    File << uiBytes2;

    for (int i = 0; i < store2.size (); ++i)
    {
      unsigned char uc = store2[i];
      File << uc;
    }
  }
}

void qtMainWindow::LoadLayout (const char* szFile)
{
  aeFileIn File;
  if (File.Open (szFile))
  {
    aeUInt32 uiVersion = 0;
    File >> uiVersion;

    if (uiVersion != 1)
      return;

    aeUInt32 uiBytes1;
    aeUInt32 uiBytes2;

    File >> uiBytes1;

    QByteArray store1;

    for (aeUInt32 ui = 0; ui < uiBytes1; ++ui)
    {
      unsigned char uc;
      File >> uc;;
      store1.append (uc);
    }

    restoreState (store1);

    File >> uiBytes2;

    QByteArray store2;

    for (aeUInt32 ui = 0; ui < uiBytes2; ++ui)
    {
      unsigned char uc;
      File >> uc;;
      store2.append (uc);
    }

    restoreGeometry (store2);
  }
}



