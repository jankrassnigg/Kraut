#include "PCH.h"

#include "../../Tree/Tree.h"
#include "../../Basics/Plugin.h"
#include "../../Undo/TreeUndo.h"
#include <KrautFoundation/Strings/StringFunctions.h>

#include "qtResourceEditorWidget.moc.h"

qtResourceEditorWidget* qtResourceEditorWidget::s_pWidget = NULL;

qtResourceEditorWidget::qtResourceEditorWidget (QWidget* parent) : QDockWidget (parent)
{
  s_pWidget = this;
  m_iPreviewTexture = 0;

  setupUi (this);

  LineMaterialDiffuse->installEventFilter (this);
  LineMaterialNormal->installEventFilter (this);

  aeTreeMaterialLibrary::Load ();

  UpdateMaterialList ();

  if (!aeTreeMaterialLibrary::GetMaterials ().empty ())
    SetSelectedMaterial (aeTreeMaterialLibrary::GetMaterials ().begin ().key ().c_str ());
}
