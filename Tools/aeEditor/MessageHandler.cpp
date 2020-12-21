#include "PCH.h"
#include "Foundation.h"
#include <QApplication>
#include "qtMainWindow.moc.h"
#include "qt3DWidget.moc.h"
#include <KrautEditorBasics/Plugin.h>
#include "LastInclude.h"

using namespace AE_NS_EDITORBASICS;

aeInt32 g_iBlockRedrawRequests = 0;

AE_ON_GLOBAL_EVENT (aeEditor_BlockRedraw)
{
  ++g_iBlockRedrawRequests;

  AE_CHECK_DEV (g_iBlockRedrawRequests <= 100, "Too many aeEditor_BlockRedraw events.");
}

AE_ON_GLOBAL_EVENT (aeEditor_UnblockRedraw)
{
  --g_iBlockRedrawRequests;

  AE_CHECK_DEV (g_iBlockRedrawRequests >= 0, "Too many aeEditor_UnblockRedraw events.");

  if (g_iBlockRedrawRequests == 0)
    AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}


AE_ON_GLOBAL_EVENT (aeEditor_RedrawNow)
{
  if (g_iBlockRedrawRequests > 0)
    return;

  static bool bRedrawing = false;

  qt3DWidget* pGLWidget;
  aeVariableRegistry::RetrieveRaw ("system/qt/3dwidget", &pGLWidget, sizeof (QGLWidget*));

  if (bRedrawing)
  {
    pGLWidget->QueueRedraw ();
    return;
  }

  bRedrawing = true;
  pGLWidget->update ();
  bRedrawing = false;
}

AE_ON_GLOBAL_EVENT (aeEditor_QueueRedraw)
{
  if (g_iBlockRedrawRequests > 0)
    return;

  qt3DWidget* pGLWidget;
  aeVariableRegistry::RetrieveRaw ("system/qt/3dwidget", &pGLWidget, sizeof (QGLWidget*));

  pGLWidget->QueueRedraw ();
}

AE_ON_GLOBAL_EVENT (aeEditor_SetWindowTitle)
{
  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  aeFilePath sTitle;
  sTitle.Format ("%s - [ %s ]", aeEditorPlugin::s_EditorName.c_str (), param0.Text);

  pMainWindow->setWindowTitle (sTitle.c_str ());
}

AE_ON_GLOBAL_EVENT (aeEditor_WorkspaceModified)
{
  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  aeString sTitle = pMainWindow->windowTitle().toUtf8().data();

  if (!sTitle.EndsWith ("*"))
  {
    sTitle += "*";
    pMainWindow->setWindowTitle (sTitle.c_str ());
  }
}

AE_ON_GLOBAL_EVENT (aeEditor_WorkspaceUnmodified)
{
  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  aeString sTitle = pMainWindow->windowTitle().toUtf8().data();

  if (sTitle.EndsWith ("*"))
  {
    sTitle.resize (sTitle.length () - 1);
    pMainWindow->setWindowTitle (sTitle.c_str ());
  }
}

