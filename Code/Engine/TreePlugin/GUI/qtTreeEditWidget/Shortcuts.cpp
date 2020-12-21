#include "PCH.h"

#include "qtTreeEditWidget.moc.h"

void qtTreeEditWidget::SetupShortcuts (void)
{
  // New File
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_N), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_action_New_triggered ()));
    action_New->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Open File
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_O), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_action_Open_triggered ()));
    action_Open->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Save File
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_S), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_action_Save_triggered ()));
    action_Save->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Save File As..
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::SHIFT + Qt::Key_S), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_action_SaveAs_triggered ()));
    action_SaveAs->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Default
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F1), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderDefault_triggered ()));
    actionRenderDefault->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Wireframe
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F2), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderWireframe_triggered ()));
    actionRenderWireframe->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Skeleton
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F3), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderSkeleton_triggered ()));
    actionRenderSkeleton->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Normals
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F4), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderNormals_triggered ()));
    actionRenderNormals->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Diffuse
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F5), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderDiffuse_triggered ()));
    actionRenderDiffuse->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Light
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F6), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderLight_triggered ()));
    actionRenderLight->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Ambient
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F7), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderAmbient_triggered ()));
    actionRenderAmbient->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Render Mode: Skylight
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_F8), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRenderSkylight_triggered ()));
    actionRenderSkylight->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Delete Selected Branch
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::Key_Delete), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionDeleteBranch_triggered ()));
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Gizmo Mode: Toggle Gizmo
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_G), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionToggleGizmo_triggered ()));
    actionToggleGizmo->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Undo
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_Z), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionUndo_triggered ()));
    actionUndo->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Redo
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_Y), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_actionRedo_triggered ()));
    actionRedo->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }

  // Export
  {
    QShortcut* aShortCut = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_E), this);
    QObject::connect (aShortCut, SIGNAL (activated ()), this, SLOT (on_action_Export_triggered ()));
    action_Export->setShortcutContext(Qt::ApplicationShortcut);
    aShortCut->setContext(Qt::ApplicationShortcut);
  }
}


