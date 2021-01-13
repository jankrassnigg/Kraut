#include "PCH.h"

#include "../../Tree/Tree.h"
#include "qtTreeEditWidget.moc.h"
#include "../../Basics/Plugin.h"
#include <KrautFoundation/FileSystem/FileSystem.h>

aeCVarBool CVar_LeafCardConvertToDDS ("Tree_LeafCardConvertToDDS", true, aeCVarFlags::Save, "Whether to convert exported textures to DDS.");

void qtTreeEditWidget::on_check_LeafCardMode_clicked ()
{
  g_Tree.m_Descriptor.m_StructureDesc.m_bLeafCardMode = check_LeafCardMode->isChecked();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_pb_ExportLeafCard_clicked ()
{
  s_sExportLeafCardPath = le_LeafCardPath->text().toUtf8().data();

  if (!g_Plugin.ExportLeafCards (s_sExportLeafCardPath.c_str (), CVar_LeafCardConvertToDDS))
  {
    QMessageBox::critical (this, "Kraut", "The Leaf-Card export failed. Please select another file.");
  }
  else
  {
    //QMessageBox::information (this, "Kraut", "The Leaf-Cards were exported successfully.");
  }

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_check_PreviewLeafCard_clicked ()
{
  g_Globals.s_bPreviewLeafCard = check_PreviewLeafCard->isChecked ();

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_check_ExportDDS_clicked ()
{
  CVar_LeafCardConvertToDDS = check_ExportDDS->isChecked ();
}

aeFilePath qtTreeEditWidget::s_sExportLeafCardPath;

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeLoaded)
{
  qtTreeEditWidget::s_sExportLeafCardPath = "Textures/LeafCards/";

  if (!g_Tree.m_sTreeFile.empty ())
  {
    qtTreeEditWidget::s_sExportLeafCardPath += aePathFunctions::GetFileName (g_Tree.m_sTreeFile);
    qtTreeEditWidget::s_sExportLeafCardPath += ".tga";
  }

  aeFilePath sAbsolutePath, sRelativePath;
  aeFileSystem::MakeValidPath (qtTreeEditWidget::s_sExportLeafCardPath.c_str (), true, &sAbsolutePath, &sRelativePath);

  qtTreeEditWidget::s_pWidget->le_LeafCardPath->setText (sAbsolutePath.c_str ());
}

void qtTreeEditWidget::on_tb_SelectLeafCardPath_clicked ()
{
  aeFilePath sAbsolutePath, sRelativePath;

  if (s_sExportLeafCardPath.empty ())
  {
    s_sExportLeafCardPath = "Textures/LeafCards/";
    s_sExportLeafCardPath += aePathFunctions::GetFileName (g_Tree.m_sTreeFile);
    le_LeafCardPath->setText (s_sExportLeafCardPath.c_str ());
  }

  aeFileSystem::MakeValidPath (s_sExportLeafCardPath.c_str (), true, &sAbsolutePath, &sRelativePath);

  QString result = QFileDialog::getSaveFileName (this, 
    QFileDialog::tr ("Save Leaf Card to"), sAbsolutePath.c_str (),
    QFileDialog::tr ("Texture Files (*.tga)"), 0); 

  if (result.size() == 0)
    return;

  s_sExportLeafCardPath = result.toLatin1 ().data ();

  le_LeafCardPath->setText (s_sExportLeafCardPath.c_str ());
}

void qtTreeEditWidget::on_combo_MaxLeafCardResolution_currentIndexChanged (int index)
{
  g_Tree.m_Descriptor.m_LeafCardResolution = (aeLeafCardResolution::Enum) index;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_combo_LeafCardMipmapResolution_currentIndexChanged (int index)
{
  g_Tree.m_Descriptor.m_LeafCardMipmaps = (aeLeafCardMipmapResolution::Enum) index;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_combo_LeafCardColorDilation_currentIndexChanged (int index)
{
  g_Tree.m_Descriptor.m_uiLeafCardTexelDilation = index;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_check_FromAbove_clicked ()
{
  g_Tree.m_Descriptor.m_bSnapshotFromAbove = check_FromAbove->isChecked ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}
