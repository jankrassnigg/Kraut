#include "PCH.h"
#include "Foundation.h"
#include "qt3DWidget.moc.h"
#include <KrautEditorBasics/RenderAPI.h>
#include <KrautEditorBasics/Plugin.h>
#include <KrautFoundation/ResourceManager/ResourceManager.h>
#include "LastInclude.h"

#include <QWidget>
#include <QResizeEvent>
#include <QTimer>
#include <windows.h>

using namespace AE_NS_FOUNDATION;
using namespace AE_NS_EDITORBASICS;

// CVar that holds the name of the RenderAPI to use
static aeCVarString CVarRenderAPI ("app_RenderAPI", "ezKrautEditorRenderAPI_GL", aeCVarFlags::Restart | aeCVarFlags::Save, "Which RenderAPI-DLL to use.");

qt3DWidget::qt3DWidget (QWidget *parent) : QWidget (parent, 0)
{
  setObjectName ("aeEditor::3DWidget");

  m_bInitializedRenderAPI = false;
  m_bRedrawQueued = false;

  // store the qt 3D window pointer
  QWidget* pWidget = this;
  aeVariableRegistry::StoreRaw ("system/qt/3dwidget", &pWidget, sizeof (QWidget*));

  HWND hWnd = (HWND)winId ();
  aeVariableRegistry::StoreRaw ("system/windows/hwnd", &hWnd, sizeof (HWND));

  // disable lots of Qt stuff to prevent flickering during rendering
	setAutoFillBackground (false);
	setAttribute (Qt::WA_PaintOnScreen, true);
	setAttribute (Qt::WA_NoSystemBackground, true);
  setBackgroundRole (QPalette::NoRole);

  setMouseTracking (true);

  // Load the Plugin DLL that contains the RenderAPI code
  aePluginManager::LoadPlugin (CVarRenderAPI.GetValue (), "");
}

qt3DWidget::~qt3DWidget ()
{
  aeStartup::ShutdownEngine ();

  aeEditorRenderAPI::GetInstance ()->DestroyContext ();
}

extern aeInt32 g_iBlockRedrawRequests;

void qt3DWidget::RedrawTimeout (void)
{
  m_bRedrawQueued = false;
  update ();
}

void qt3DWidget::paintEvent (QPaintEvent *event)
{
  if (g_iBlockRedrawRequests > 0)
    return;

  event->accept ();

  // If necessary initialize the RenderAPI
  InitializeRenderAPI (this->width (), this->height ());

  // send lots of rendering events for plugins to hook into them
  AE_BROADCAST_EVENT (aeFrame_Begin);

  AE_BROADCAST_EVENT (aeGame_Update);

  //aeEditorPlugin::m_RenderHooks_Default.ExecuteComponents ();

  AE_BROADCAST_EVENT (aeFrame_End);

  aeEditorRenderAPI::GetInstance ()->Swap ();

  aeResourceManager::FreeUnusedResources ();
}

void qt3DWidget::resizeEvent ( QResizeEvent * event )
{
  aeLog::Debug ("qt3DWidget::resizeEvent");

  event->accept();

  QWidget::resizeEvent (event);

  aeEditorRenderAPI::GetInstance ()->ResizeFramebuffer (event->size().width(), event->size().height());
}


void qt3DWidget::InitializeRenderAPI (aeUInt32 uiResolutionX, aeUInt32 uiResolutionY)
{
  if (m_bInitializedRenderAPI)
    return;

  HWND hWnd = (HWND)winId ();
  if (hWnd == NULL)
    return;

  HDC hDC = GetDC (hWnd);
  if (hDC == NULL)
    return;

  aeLog::Debug ("qt3DWidget::InitializeRenderAPI");

  // store some info that the RenderAPI might need during initialization
  aeVariableRegistry::StoreRaw ("system/windows/hdc", &hDC, sizeof (HDC));
  aeVariableRegistry::StoreRaw ("system/windows/hwnd", &hWnd, sizeof (HWND));

  m_bInitializedRenderAPI = true;

  aeEditorRenderAPI::GetInstance ()->CreateContext (uiResolutionX, uiResolutionY);

  // tell the engine that now all modules may load graphics related stuff
  aeStartup::StartupEngine ();

  AE_BROADCAST_EVENT (aeEditor_BeforeFirstFrame);
}

void qt3DWidget::QueueRedraw ()
{
  if (m_bRedrawQueued)
    return;

  m_bRedrawQueued = true;

  static QTimer* pTimer = new QTimer (this);

  // queue this rendering request and only fire it every once in a while
  pTimer->singleShot (20, this, SLOT(RedrawTimeout()));
}

void qt3DWidget::mousePressEvent(QMouseEvent* event)
{
  aeEditorPluginEvent e;
  e.m_Type = aeEditorPluginEvent::MousePress;
  e.m_Data.m_MouseEvent = event;

  aeEditorPlugin::s_EditorPluginEvent.RaiseEvent (&e);
}

void qt3DWidget::mouseReleaseEvent(QMouseEvent* event)
{
  aeEditorPluginEvent e;
  e.m_Type = aeEditorPluginEvent::MouseRelease;
  e.m_Data.m_MouseEvent = event;

  aeEditorPlugin::s_EditorPluginEvent.RaiseEvent (&e);
}

void qt3DWidget::mouseMoveEvent(QMouseEvent* event)
{
  aeEditorPluginEvent e;
  e.m_Type = aeEditorPluginEvent::MouseMove;
  e.m_Data.m_MouseEvent = event;

  aeEditorPlugin::s_EditorPluginEvent.RaiseEvent (&e);
}

void qt3DWidget::wheelEvent (QWheelEvent* event)
{
  aeEditorPluginEvent e;
  e.m_Type = aeEditorPluginEvent::MouseWheel;
  e.m_Data.m_WheelEvent = event;

  aeEditorPlugin::s_EditorPluginEvent.RaiseEvent (&e);
}


