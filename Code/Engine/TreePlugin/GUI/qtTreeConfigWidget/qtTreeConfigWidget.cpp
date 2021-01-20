#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../../Tree/Tree.h"
#include "../qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include <KrautGraphics/FileSystem/FileIn.h>
#include <KrautGraphics/FileSystem/FileOut.h>
#include <KrautGraphics/Time/Time.h>

#include <QGraphicsRectItem>

#include "qtTreeConfigWidget.moc.h"

qtTreeConfigWidget* qtTreeConfigWidget::s_pWidget = nullptr;
static bool g_bUpdateThumbnails = true;
static float g_fLastThumbnailUpdate = -10;
aeUInt32 g_uiThumbnailSize = 128;

const char* BranchTypeEnumNames[Kraut::BranchType::ENUM_COUNT] =
  {
    "Trunk 1",
    "Trunk 2",
    "Trunk 3",
    "Branch 1",
    "Branch 2",
    "Branch 3",
    "Sub-Branch 1",
    "Sub-Branch 2",
    "Sub-Branch 3",
    "Twigs 1",
    "Twigs 2",
    "Twigs 3",
};

qtTreeConfigWidget::qtTreeConfigWidget(QWidget* parent)
  : QDockWidget(parent)
{
  s_pWidget = this;

  setupUi(this);
  SetupToolbars();

  TreeConfigView->setScene(&m_Scene);
  QTransform t = TreeConfigView->transform();
  t.scale(0.5f, 0.5f);

  aeMemory::FillWithZeros(&m_pSpheres[0], sizeof(QGraphicsRectItem*) * Kraut::BranchType::ENUM_COUNT);
}

qtTreeConfigWidget::~qtTreeConfigWidget()
{
  s_pWidget = nullptr;
}

AE_ON_GLOBAL_EVENT(aeTreeEdit_TreeModifiedSimple)
{
  g_bUpdateThumbnails = true;
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeGenerated)
{
  g_bUpdateThumbnails = true;
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeLoaded)
{
  g_bUpdateThumbnails = true;
  g_fLastThumbnailUpdate = -10.0f;
}

void qtTreeConfigWidget::SetupToolbars(void)
{
  QVBoxLayout* pLayout = new QVBoxLayout(widget_Toolbar);
  m_pToolbar = new QToolBar(widget_Toolbar);

  // General Toolbar Setup
  {
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);

    setContentsMargins(0, 0, 0, 0);
    dockWidgetContents->setContentsMargins(0, 0, 0, 0);
    dockWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);

    pLayout->addWidget(m_pToolbar);

    m_pToolbar->setFloatable(false);
    m_pToolbar->setMovable(false);
  }

  m_pToolbar->addAction(actionLoadTreeComponent);
  m_pToolbar->addAction(actionSaveTreeComponent);
  m_pToolbar->addAction(actionRemoveTreeComponent);
  m_pToolbar->addSeparator();
  m_pToolbar->addAction(actionToggleVisibility);
}

static QColor qColSelected(51 / 2, 119 / 2, 34 / 2);
static QColor qColSelectedInvisible(51 / 4, 119 / 4, 34 / 4);
static QColor qHighlighted(255, 140, 0);
static QColor qOther(128 / 2, 128 / 2, 128 / 2);
static QColor qOtherInvisible(128 / 4, 128 / 4, 128 / 4);
static QColor qConnection(50, 66, 125);

struct aeNodePos
{
  bool bUsed;
  aeInt32 iPosX;
  aeInt32 iPosY;
};

aeArray<aeUInt8> Thumbnail[Kraut::BranchType::ENUM_COUNT];
QPixmap* g_pPixmap[Kraut::BranchType::ENUM_COUNT] = {nullptr};

AE_ON_GLOBAL_EVENT(aeFrame_End)
{
  if (qtTreeConfigWidget::s_pWidget && g_bUpdateThumbnails)
    qtTreeConfigWidget::s_pWidget->UpdateDisplay();
}

void qtTreeConfigWidget::UpdateDisplay(void)
{
  const aeInt32 iSize = 160;

  if (g_bUpdateThumbnails && (aeTime::GetAppTime() - g_fLastThumbnailUpdate > 2.0f))
  {
    bool bUpdatedAny = false;

    for (aeUInt32 bt = 0; bt < Kraut::BranchType::ENUM_COUNT; ++bt)
    {
      if (!g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_bUsed)
        continue;

      if (!g_Plugin.CreateBranchTypeThumbnail(bt, Thumbnail[bt]))
        continue;

      bUpdatedAny = true;

      if (g_pPixmap[bt] == nullptr)
        g_pPixmap[bt] = new QPixmap;

      QImage img(Thumbnail[bt].data(), g_uiThumbnailSize, g_uiThumbnailSize, QImage::Format_ARGB32);
      QPixmap temp;
      temp.convertFromImage(img);

      *g_pPixmap[bt] = temp.scaledToWidth(iSize / 2 /*- (iSize / 8)*/, Qt::SmoothTransformation);
    }

    if (bUpdatedAny)
    {
      g_bUpdateThumbnails = false;
      g_fLastThumbnailUpdate = aeTime::GetAppTime();
    }
  }

  m_Scene.clear();

  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  AE_CHECK_DEV((iEditedType >= 0) && (iEditedType < Kraut::BranchType::ENUM_COUNT), "Incorrect Branch Type %d", iEditedType);

  actionToggleVisibility->setChecked(g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bVisible);

  aeNodePos NodePositions[Kraut::BranchType::ENUM_COUNT];

  for (aeUInt32 i = 0; i < Kraut::BranchType::ENUM_COUNT; ++i)
  {
    m_pSpheres[i] = nullptr;

    NodePositions[i].bUsed = g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[i].m_bUsed;

    if (!g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[i].m_bUsed)
      continue;

    const bool bVisible = g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[i].m_bVisible;

    aeInt32 iGroup = (i / 3);

    NodePositions[i].iPosY = (Kraut::BranchType::ENUM_COUNT / 3) * iSize - iGroup * iSize - iSize + iGroup * (iSize / 4);
    NodePositions[i].iPosX = ((i % 3) - 1) * iSize;

    if (i == iEditedType)
      m_pSpheres[i] = m_Scene.addRect(QRectF(QPointF(NodePositions[i].iPosX, NodePositions[i].iPosY), QSizeF((iSize / 2), (iSize / 2))), QPen(qColSelected, 2), QBrush(bVisible ? qColSelected : qColSelectedInvisible));
    else
      m_pSpheres[i] = m_Scene.addRect(QRectF(QPointF(NodePositions[i].iPosX, NodePositions[i].iPosY), QSizeF((iSize / 2), (iSize / 2))), QPen(qOther, 2), QBrush(bVisible ? qOther : qOtherInvisible));

    if (g_pPixmap[i] != nullptr)
    {
      QGraphicsPixmapItem* pPix = m_Scene.addPixmap(*g_pPixmap[i]);
      pPix->setPos(QPointF(NodePositions[i].iPosX /*+ (iSize / 16)*/, NodePositions[i].iPosY + (iSize / 2) /*- (iSize / 16)*/));
      pPix->setZValue(0.5f);
      QTransform t;
      t.scale(1, -1);
      pPix->setTransform(t);
    }

    QGraphicsTextItem* pText = m_Scene.addText(BranchTypeEnumNames[i], QFont("Arial", 8, 1));
    pText->setPos(QPointF(NodePositions[i].iPosX + (iSize / 4) - pText->boundingRect().width() * 0.5f, NodePositions[i].iPosY + (iSize / 2) /* - pText->boundingRect().height() * 0.5f*/));
    pText->setDefaultTextColor(qHighlighted);
    pText->setZValue(1.0f);
  }

  for (aeUInt32 i = 3; i < Kraut::BranchType::ENUM_COUNT; ++i)
  {
    if (!NodePositions[i].bUsed)
      continue;

    const aeInt32 iRoundDown = ((i / 3) - 1) * 3;

    for (aeInt32 iLower = iRoundDown; iLower < iRoundDown + 3; ++iLower)
    {
      if (!NodePositions[iLower].bUsed)
        continue;

      if (g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iLower].m_bAllowSubType[i % 3])
      {
        QGraphicsLineItem* pLine = m_Scene.addLine(NodePositions[iLower].iPosX + (iSize / 4), NodePositions[iLower].iPosY, NodePositions[i].iPosX + (iSize / 4), NodePositions[i].iPosY + (iSize / 2), QPen(qConnection, 2));
        pLine->setZValue(-1);
      }
    }
  }

  const aeInt32 uiGroups = Kraut::BranchType::ENUM_COUNT / 3;
  const aeInt32 iLess = (uiGroups - 1) * (iSize / 4);

  m_Scene.setSceneRect(QRectF(-(iSize / 4) * 5, -(iSize / 4) + iLess, iSize * 3, iSize * uiGroups - iLess));

  UpdateHighlights(false);
}

void qtTreeConfigWidget::on_actionToggleVisibility_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bVisible = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bVisible;

  UpdateDisplay();

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeConfigWidget::on_actionRemoveTreeComponent_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  Kraut::SpawnNodeDesc& Type = g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType];

  if (!Type.m_bUsed)
    return;

  if (QMessageBox::question(this, "Kraut", "Do you really want to completely remove this branch type?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  Type.m_bUsed = false;

  for (aeUInt32 t = 0; t < Kraut::BranchType::ENUM_COUNT; ++t)
  {
    if (g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[t].m_bUsed)
    {
      qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType((Kraut::BranchType::Enum)t, false);
      break;
    }
  }

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

static aeInt32 GetChildID(aeInt32 iBranchType, aeInt32 iChild)
{
  return ((iBranchType / 3) + 1) * 3 + iChild;
}

static bool HasChild(aeInt32 iBranchType, aeInt32 iChild)
{
  aeInt32 iChildID = GetChildID(iBranchType, iChild);
  return (iChildID < Kraut::BranchType::ENUM_COUNT) && g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iChildID].m_bUsed;
}

static aeInt32 GetParentID(aeInt32 iBranchType, aeInt32 iParent)
{
  return (((iBranchType / 3) - 1) * 3) + iParent;
}

static bool HasParent(aeInt32 iBranchType, aeInt32 iParent)
{
  aeInt32 iParentID = GetParentID(iBranchType, iParent);
  return (iParentID >= 0 && g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bUsed);
}

static bool HasConnectionToChild(aeInt32 iBranchType, aeInt32 iChild)
{
  aeInt32 iChildID = GetChildID(iBranchType, iChild);
  return (iChildID < Kraut::BranchType::ENUM_COUNT) && (g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iBranchType].m_bAllowSubType[iChild] && g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iChildID].m_bUsed);
}

static bool HasConnectionToParent(aeInt32 iBranchType, aeInt32 iParent)
{
  aeInt32 iChild = iBranchType % 3;
  aeInt32 iParentID = GetParentID(iBranchType, iParent);
  return (iParentID >= 0) && (g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iChild] && g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bUsed);
}


void qtTreeConfigWidget::UpdateHighlights(bool bClick, QMouseEvent* event)
{
  aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  bool bUnderAny = false;

  for (aeUInt32 i = 0; i < Kraut::BranchType::ENUM_COUNT; ++i)
  {
    if (m_pSpheres[i])
    {
      bool bUnder = m_pSpheres[i]->isUnderMouse();

      bUnderAny |= bUnder;

      if (bUnder && bClick)
        iEditedType = i;

      const bool bVisible = g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[i].m_bVisible;

      if (iEditedType == i)
      {
        m_pSpheres[i]->setPen(QPen(qColSelected, 2));
        m_pSpheres[i]->setBrush(QBrush(bVisible ? qColSelected : qColSelectedInvisible));
      }
      else
      {
        m_pSpheres[i]->setPen(QPen(qOther, 2));
        m_pSpheres[i]->setBrush(QBrush(bVisible ? qOther : qOtherInvisible));
      }

      if (bUnder)
        m_pSpheres[i]->setPen(QPen(qHighlighted, 2));
    }
  }

  if (bClick && bUnderAny && m_pSpheres[iEditedType])
    TreeConfigView->ensureVisible(m_pSpheres[iEditedType]);

  qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType((Kraut::BranchType::Enum)iEditedType, false);

  if ((event) && (event->button() == Qt::RightButton) && bClick)
  {
    QMenu menu(this);

    if (bUnderAny)
    {
      menu.addAction(actionToggleVisibility);
      menu.addSeparator();

      if (iEditedType < Kraut::BranchType::ENUM_COUNT - 3)
        menu.addAction(actionAddTreeComponent);

      menu.addAction(actionLoadTreeComponent);
      menu.addAction(actionSaveTreeComponent);
      menu.addAction(actionRemoveTreeComponent);

      aeHybridString<32> s;

      {
        menu.addSeparator();

        QAction* pActions[3] = {actionToggleUpConnection1, actionToggleUpConnection2, actionToggleUpConnection3};

        for (aeInt32 c = 0; c < 3; ++c)
        {
          if (HasChild(iEditedType, c))
          {
            if (HasConnectionToChild(iEditedType, c))
            {
              s.Format("Remove Connection to '%s'", BranchTypeEnumNames[GetChildID(iEditedType, c)]);
              pActions[c]->setIcon(QIcon(":/Icons/Icons/Delete.png"));
            }
            else
            {
              s.Format("Add Connection to '%s'", BranchTypeEnumNames[GetChildID(iEditedType, c)]);
              pActions[c]->setIcon(QIcon(":/Icons/Icons/Add.png"));
            }

            pActions[c]->setText(s.c_str());
            menu.addAction(pActions[c]);
          }
        }
      }

      {
        menu.addSeparator();

        QAction* pActions[3] = {actionToggleDownConnection1, actionToggleDownConnection2, actionToggleDownConnection3};

        for (aeInt32 c = 0; c < 3; ++c)
        {
          if (HasParent(iEditedType, c))
          {
            if (HasConnectionToParent(iEditedType, c))
            {
              s.Format("Remove Connection to '%s'", BranchTypeEnumNames[GetParentID(iEditedType, c)]);
              pActions[c]->setIcon(QIcon(":/Icons/Icons/Delete.png"));
            }
            else
            {
              s.Format("Add Connection to '%s'", BranchTypeEnumNames[GetParentID(iEditedType, c)]);
              pActions[c]->setIcon(QIcon(":/Icons/Icons/Add.png"));
            }

            pActions[c]->setText(s.c_str());
            menu.addAction(pActions[c]);
          }
        }
      }
    }
    else
      menu.addAction(actionAddTrunkComponent);

    menu.exec(event->globalPos());
  }
}

void qtTreeConfigWidget::EnsureVisible(void)
{
  if ((!qtTreeEditWidget::s_pWidget) || (!TreeConfigView))
    return;

  aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  if (m_pSpheres[iEditedType])
  {
    TreeConfigView->ensureVisible(m_pSpheres[iEditedType]);
  }
}

AE_ON_GLOBAL_EVENT(aeTreePlugin_SelectedBranchTypeChanged)
{
  if (qtTreeEditWidget::s_pWidget && qtTreeConfigWidget::s_pWidget)
  {
    aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();
    qtTreeConfigWidget::s_pWidget->actionToggleVisibility->setChecked(g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bVisible);

    qtTreeConfigWidget::s_pWidget->UpdateDisplay();
    qtTreeConfigWidget::s_pWidget->EnsureVisible();
  }
}

static aeString sPrevPreset = "";

void qtTreeConfigWidget::on_actionSaveTreeComponent_triggered()
{
  aeString sAbsolutePath;
  aeFileSystem::MakeValidPath(sPrevPreset.c_str(), true, &sAbsolutePath, nullptr);

  QString result = QFileDialog::getSaveFileName(this,
    QFileDialog::tr("Save Branch Preset"), sAbsolutePath.c_str(),
    QFileDialog::tr("Branch-Files (*.branch)"), 0);

  if (result.size() == 0)
    return;

  sPrevPreset = result.toLatin1().data();

  aeFileOut File;
  if (!File.Open(sPrevPreset.c_str()))
  {
    QMessageBox::warning(this, "Kraut", "The selected file could not be opened for writing.");
    return;
  }

  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  aeUInt8 uiVersion = 1;
  File << uiVersion;

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].Serialize(File);
}

static bool LoadBranchType(aeInt32 iBranchType, QWidget* pParent)
{
  aeFileSystem::MakeValidPath("BranchTypes/", false, &sPrevPreset, nullptr);

  QString result = QFileDialog::getOpenFileName(pParent,
    QFileDialog::tr("Load Branch Preset"), sPrevPreset.c_str(),
    QFileDialog::tr("Curve-Files (*.branch);;All Files (*.*)"), 0);

  if (result.size() == 0)
    return false;

  sPrevPreset = result.toLatin1().data();

  aeFileIn File;
  if (!File.Open(sPrevPreset.c_str()))
  {
    QMessageBox::warning(pParent, "Kraut", "The selected file could not be opened.");
    return false;
  }

  aeUInt8 uiVersion = 0;
  File >> uiVersion;

  if (uiVersion > 1)
  {
    QMessageBox::warning(pParent, "Kraut", "The selected branch-file has an incompatible file-version.");
    return false;
  }

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iBranchType].Deserialize(File);

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iBranchType].m_bUsed = true;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iBranchType].m_bAllowSubType[0] = false;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iBranchType].m_bAllowSubType[1] = false;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iBranchType].m_bAllowSubType[2] = false;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);

  return true;
}

void qtTreeConfigWidget::on_actionLoadTreeComponent_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  LoadBranchType(iEditedType, this);
}


void qtTreeConfigWidget::on_actionAddTreeComponent_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  if (!g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bUsed)
  {
    QMessageBox::information(this, "Kraut", "No active branch type is selected. Please select a branch type to add a sub-branch to it.");
    return;
  }

  const aeInt32 iNextType = ((iEditedType / 3) + 1) * 3;

  if (iNextType >= Kraut::BranchType::ENUM_COUNT)
  {
    QMessageBox::information(this, "Kraut", "The selected branch type is already the most detailed branch type possible. No additional branch type can be added to it.");
    return;
  }

  aeInt32 iNext = iNextType;
  for (; iNext < iNextType + 3; ++iNext)
  {
    if (!g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bUsed)
      goto found;
  }

  QMessageBox::information(this, "Kraut", "The selected branch-type already has 3 sub-branch types.");
  return;

found:
  aeInt32 iSubType = iNext - iNextType;

  for (aeInt32 i = 0; i < 3; ++i)
    g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[(iEditedType / 3) * 3 + i].m_bAllowSubType[iSubType] = false;

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[iSubType] = true;

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].Reset();

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bUsed = true;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bAllowSubType[0] = false;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bAllowSubType[1] = false;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bAllowSubType[2] = false;

  qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType((Kraut::BranchType::Enum)iNext, false);

  UpdateDisplay();

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeConfigWidget::on_actionAddTrunkComponent_triggered()
{
  aeInt32 iNext = 0;
  for (; iNext < 3; ++iNext)
  {
    if (!g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bUsed)
      goto found;
  }

  QMessageBox::information(this, "Kraut", "There are already enough trunk types in this tree.");
  return;

found:
  //LoadBranchType (iNext, this);

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].Reset();

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bUsed = true;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bAllowSubType[0] = false;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bAllowSubType[1] = false;
  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iNext].m_bAllowSubType[2] = false;

  qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType((Kraut::BranchType::Enum)iNext, false);

  UpdateDisplay();

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeConfigWidget::on_actionToggleUpConnection1_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[0] = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[0];

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


void qtTreeConfigWidget::on_actionToggleUpConnection2_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[1] = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[1];

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


void qtTreeConfigWidget::on_actionToggleUpConnection3_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[2] = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iEditedType].m_bAllowSubType[2];

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeConfigWidget::on_actionToggleDownConnection1_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  if (!HasParent(iEditedType, 0))
    return;

  aeInt32 iParentID = GetParentID(iEditedType, 0);

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iEditedType % 3] = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iEditedType % 3];

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeConfigWidget::on_actionToggleDownConnection2_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  if (!HasParent(iEditedType, 1))
    return;

  aeInt32 iParentID = GetParentID(iEditedType, 1);

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iEditedType % 3] = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iEditedType % 3];

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeConfigWidget::on_actionToggleDownConnection3_triggered()
{
  const aeInt32 iEditedType = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();

  if (!HasParent(iEditedType, 2))
    return;

  aeInt32 iParentID = GetParentID(iEditedType, 2);

  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iEditedType % 3] = !g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[iParentID].m_bAllowSubType[iEditedType % 3];

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}
