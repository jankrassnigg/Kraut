#include "PCH.h"

#include "../../Tree/Tree.h"

#include "qtExportWidget.moc.h"
#include <KrautGraphics/Configuration/CVar.h>
#include <KrautGraphics/FileSystem/FileSystem.h>

aeCVarString CVar_ViewerApp("Tree_ViewerApp", "", aeCVarFlags::Save, "Which application to use to view trees.");
aeCVarString CVar_ViewerAppArgs("Tree_ViewerAppArgs", "-tree $TREE$ -textures $CONTENT$", aeCVarFlags::Save, "The command line arguments for the viewer app.");
aeCVarBool CVar_OnExportConvertToDDS("Tree_OnExportConvertToDDS", true, aeCVarFlags::Save, "Whether to convert exported textures to DDS.");

aeString qtExportWidget::s_sExportPath = "";
bool qtExportWidget::s_bExportBranches = true;
bool qtExportWidget::s_bExportFronds = true;
bool qtExportWidget::s_bExportLeaves = true;
bool qtExportWidget::s_bCapTrunk = false;
bool qtExportWidget::s_bCapBranches = false;
bool qtExportWidget::s_bExportLODs[6] = {false, true, true, true, true, true};
bool qtExportWidget::s_bWantedToRun = false;

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeLoaded)
{
  qtExportWidget::s_sExportPath = "";
}

qtExportWidget::qtExportWidget(QWidget* parent)
  : QDialog(parent)
{
  s_bWantedToRun = false;

  setupUi(this);

  if (s_sExportPath.empty())
  {
    aeString sName = aePathFunctions::GetFileName(g_Tree.m_sTreeFile);

    s_sExportPath = "ExportedTrees/";

    if (!sName.empty())
    {
      s_sExportPath += sName;
      s_sExportPath += ".kraut";
    }

    aeString sAbsolutePath;
    if (aeFileSystem::MakeValidPath(s_sExportPath.c_str(), true, &sAbsolutePath, nullptr))
      s_sExportPath = sAbsolutePath;
  }

  SpinMeshScale->setValue(g_Tree.m_Descriptor.m_fExportScale);
  le_FilePath->setText(s_sExportPath.c_str());
  check_ExportBranches->setChecked(s_bExportBranches);
  check_ExportFronds->setChecked(s_bExportFronds);
  check_ExportLeaves->setChecked(s_bExportLeaves);
  //check_TrunkCaps->setChecked (s_bCapTrunk);
  //check_BranchCaps->setChecked (s_bCapBranches);
  check_HighDetail->setChecked(s_bExportLODs[0]);
  check_LOD0->setChecked(s_bExportLODs[1]);
  check_LOD1->setChecked(s_bExportLODs[2]);
  check_LOD2->setChecked(s_bExportLODs[3]);
  check_LOD3->setChecked(s_bExportLODs[4]);
  check_LOD4->setChecked(s_bExportLODs[5]);
  check_ExportDDS->setChecked(CVar_OnExportConvertToDDS);
  combo_ImpostorResolution->setCurrentIndex(aeMath::Clamp<aeInt32>(g_Tree.m_Descriptor.m_iImpostorResolution, 0, 4));

  check_LOD0->setEnabled(g_Tree.m_Descriptor.m_LodData[aeLod::Lod0].m_Mode != Kraut::LodMode::Disabled);
  check_LOD1->setEnabled(g_Tree.m_Descriptor.m_LodData[aeLod::Lod1].m_Mode != Kraut::LodMode::Disabled);
  check_LOD2->setEnabled(g_Tree.m_Descriptor.m_LodData[aeLod::Lod2].m_Mode != Kraut::LodMode::Disabled);
  check_LOD3->setEnabled(g_Tree.m_Descriptor.m_LodData[aeLod::Lod3].m_Mode != Kraut::LodMode::Disabled);
  check_LOD4->setEnabled(g_Tree.m_Descriptor.m_LodData[aeLod::Lod4].m_Mode != Kraut::LodMode::Disabled);

  le_ViewerApp->setText(QString::fromUtf8(CVar_ViewerApp));
  le_ViewerAppArgs->setText(QString::fromUtf8(CVar_ViewerAppArgs));
}

qtExportWidget::~qtExportWidget()
{
}

void qtExportWidget::Export(void)
{
  s_sExportPath = le_FilePath->text().toUtf8().data();

  if (s_sExportPath.empty())
  {
    QMessageBox::warning(this, "Kraut", "You did not select a proper file to export to.");
    return;
  }

  if (!aePathFunctions::IsAbsolutePath(s_sExportPath.c_str()))
  {
    QMessageBox::warning(this, "Kraut", "Please select an absolute path for the file to be exported to.");
    return;
  }

  const float fScale = (float)SpinMeshScale->value();
  if (g_Tree.m_Descriptor.m_fExportScale != fScale)
  {
    g_Tree.m_Descriptor.m_fExportScale = fScale;
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
  }

  s_bExportBranches = check_ExportBranches->isChecked();
  s_bExportFronds = check_ExportFronds->isChecked();
  s_bExportLeaves = check_ExportLeaves->isChecked();
  //s_bCapTrunk = check_TrunkCaps->isChecked ();
  //s_bCapBranches = check_BranchCaps->isChecked ();

  s_bExportLODs[0] = check_HighDetail->isChecked();
  s_bExportLODs[1] = check_LOD0->isChecked();
  s_bExportLODs[2] = check_LOD1->isChecked();
  s_bExportLODs[3] = check_LOD2->isChecked();
  s_bExportLODs[4] = check_LOD3->isChecked();
  s_bExportLODs[5] = check_LOD4->isChecked();
  CVar_OnExportConvertToDDS = check_ExportDDS->isChecked();

  const aeInt8 iImpostorResolution = combo_ImpostorResolution->currentIndex();
  if (iImpostorResolution != g_Tree.m_Descriptor.m_iImpostorResolution)
  {
    g_Tree.m_Descriptor.m_iImpostorResolution = iImpostorResolution;
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
  }

  CVar_ViewerApp = le_ViewerApp->text().toUtf8().data();
  CVar_ViewerAppArgs = le_ViewerAppArgs->text().toUtf8().data();
}

void qtExportWidget::on_bt_Export_clicked()
{
  Export();
  accept();
}

void qtExportWidget::on_bt_ExportAndRun_clicked()
{
  Export();
  accept();

  s_bWantedToRun = true;
}

QStringList GetCommandLine(const char* szCmdLine)
{
  QStringList result;

  QString sCur;
  bool bInString = false;

  while (*szCmdLine != '\0')
  {
    if (bInString)
    {
      if (*szCmdLine == '\"')
      {
        bInString = false;
        result.append(sCur);
        sCur = "";
      }
      else
        sCur += *szCmdLine;
    }
    else
    {
      if (*szCmdLine == ' ')
      {
        result.append(sCur);
        sCur = "";
      }
      else if (*szCmdLine == '\"')
        bInString = true;
      else
        sCur += *szCmdLine;
    }

    ++szCmdLine;
  }

  return result;
}

void qtExportWidget::RunViewer(void)
{
  if (!s_bWantedToRun)
    return;
  if (aeStringFunctions::IsNullOrEmpty(CVar_ViewerApp))
    return;

  aeString sContentPath;
  aeFileSystem::MakeValidPath("", false, &sContentPath, nullptr);
  sContentPath = "\"" + sContentPath + "\"";

  aeString sExportPath;
  aeFileSystem::MakeValidPath(s_sExportPath.c_str(), false, &sExportPath, nullptr);
  sExportPath = "\"" + sExportPath + "\"";

  aeString sArguments = CVar_ViewerAppArgs;
  sArguments.ReplaceAll_NoCase("$CONTENT$", sContentPath.c_str());
  sArguments.ReplaceAll_NoCase("$TREE$", sExportPath.c_str());
  sArguments.ReplaceAll("/", "\\");

  aeString sWorkingDir = aePathFunctions::GetFileDirectory(CVar_ViewerApp.GetValue());
  sWorkingDir.ReplaceAll("/", "\\");

  aeString sPath = "\"";
  sPath += CVar_ViewerApp;
  sPath += "\"";
  sPath.ReplaceAll("/", "\\");

  if (!QProcess::startDetached(QString::fromUtf8(sPath.c_str()), GetCommandLine(sArguments.c_str()), QString::fromUtf8(sWorkingDir.c_str())))
    QMessageBox::warning(nullptr, "Kraut", QString("Could not run the external application:\n") + QString(sPath.c_str()));
}

void qtExportWidget::on_bt_SelectFile_clicked()
{
  aeString sAbsolutePath;
  aeFileSystem::MakeValidPath(s_sExportPath.c_str(), true, &sAbsolutePath, nullptr);

  QString result = QFileDialog::getSaveFileName(this,
    QFileDialog::tr("Export Mesh"), sAbsolutePath.c_str(),
    QFileDialog::tr("Kraut-Files (*.kraut);;FBX-Files (*.fbx);;OBJ-Files (*.obj)"), 0);

  if (result.size() == 0)
    return;

  le_FilePath->setText(result.toLatin1().data());
}

void qtExportWidget::on_bt_SelectViewerApp_clicked()
{
  aeString sAbsolutePath;
  aeFileSystem::MakeValidPath(CVar_ViewerApp, true, &sAbsolutePath, nullptr);

  QString result = QFileDialog::getOpenFileName(this,
    QFileDialog::tr("Select Viewer Application"), sAbsolutePath.c_str(),
    QFileDialog::tr("Application (*.exe)"), 0);

  if (result.size() == 0)
    return;

  le_ViewerApp->setText(result);
}

void qtExportWidget::on_bt_ViewerAppArgsHelp_clicked()
{
  QMessageBox::information(this, "Kraut",
    "The following command-line arguments will be automatically replaced by Kraut:\n\n\
$TREE$\n\
  -> Will be replaced by the full path to the exported Kraut-file.\n\n\
$CONTENT$\n\
  -> Will be replaced by the full path to Kraut's Content folder.\n\
");
}
