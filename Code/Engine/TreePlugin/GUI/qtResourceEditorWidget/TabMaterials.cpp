#include "PCH.h"

// #define USE_OPENIL

#ifdef USE_OPENIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include "../../Tree/Tree.h"
#include "qtResourceEditorWidget.moc.h"
#include "../qtTreeEditWidget/qtTreeEditWidget.moc.h"

AE_ON_GLOBAL_EVENT (aeResourceEdit_TreeMaterialsChanged)
{
  if (qtResourceEditorWidget::s_pWidget)
    qtResourceEditorWidget::s_pWidget->UpdateMaterialList ();
}

void qtResourceEditorWidget::UpdateMaterialList (void)
{
  const aeMap<aeHybridString<128>, aeTreeMaterial>& Lib = aeTreeMaterialLibrary::GetMaterials ();

  aeSet<aeTreeMaterial> Sorted;
  for (aeMap<aeHybridString<128>, aeTreeMaterial>::const_iterator it = Lib.begin (); it != Lib.end (); ++it)
    Sorted.insert (it.value ());

  const bool b = ComboMaterials->blockSignals (true);

  ComboMaterials->clear ();
  m_MaterialToComboIndex.clear ();

  aeInt32 iIndex = 0;
  for (aeSet<aeTreeMaterial>::const_iterator it = Sorted.begin (); it != Sorted.end (); ++it)
  {
    ComboMaterials->addItem (QString::fromUtf8 (it.key ().GetDescription ().c_str ()), it.key ().m_sTextureDiffuse.c_str ());
    m_MaterialToComboIndex[it.key ().m_sTextureDiffuse] = iIndex;

    ++iIndex;
  }

  if (m_MaterialToComboIndex.find (m_sSelectedTreeMaterial) != m_MaterialToComboIndex.end ())
  {
    ComboMaterials->setCurrentIndex (m_MaterialToComboIndex[m_sSelectedTreeMaterial]);
  }

  ComboMaterials->blockSignals (b);
}

void qtResourceEditorWidget::SetSelectedMaterial (const char* szMaterial)
{
  m_sSelectedTreeMaterial = szMaterial;
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (szMaterial);

  if (pMaterial == NULL)
  {
    GroupTextures->setEnabled (false);
    GroupUsage->setEnabled (false);
    GroupTiling->setEnabled (false);

    LineMaterialDiffuse->setText ("");
    LineMaterialNormal->setText ("");
  }
  else
  {
    GroupTextures->setEnabled (true);
    GroupUsage->setEnabled (true);
    GroupTiling->setEnabled (true);

    LineMaterialDiffuse->setText (pMaterial->m_sTextureDiffuse.c_str ());
    LineMaterialNormal->setText (pMaterial->m_sTextureNormal.c_str ());

    RadioMaterialUndef->setChecked (pMaterial->m_Usage == aeMaterialType::Unknown);
    RadioMaterialBark->setChecked  (pMaterial->m_Usage == aeMaterialType::Bark);
    RadioMaterialFrond->setChecked (pMaterial->m_Usage == aeMaterialType::Frond);
    RadioMaterialLeaf->setChecked  (pMaterial->m_Usage == aeMaterialType::Leaf);

    SpinMaterialTilingX->setValue (pMaterial->m_uiTilingX);
    SpinMaterialTilingY->setValue (pMaterial->m_uiTilingY);

    switch (pMaterial->m_Usage)
    {
    case aeMaterialType::Unknown:
      SpinMaterialTilingX->setEnabled (false);
      SpinMaterialTilingY->setEnabled (false);
      break;
    case aeMaterialType::Bark:
      SpinMaterialTilingX->setEnabled (false);
      SpinMaterialTilingY->setEnabled (false);
      break;
    case aeMaterialType::Frond:
      SpinMaterialTilingX->setEnabled (true);
      SpinMaterialTilingY->setEnabled (false);
      break;
    case aeMaterialType::Leaf:
      SpinMaterialTilingX->setEnabled (true);
      SpinMaterialTilingY->setEnabled (true);
      break;
    }

    aeFilePath sPreviewImage;
    switch (m_iPreviewTexture)
    {
    case 0:
      sPreviewImage = pMaterial->m_sTextureDiffuse;
      break;
    case 1:
      sPreviewImage = pMaterial->m_sTextureNormal;
      break;
    }

    aeFilePath sAbsolutePath;
    if (aeFileSystem::MakeValidPath (sPreviewImage.c_str (), false, &sAbsolutePath, NULL))
    {
      static bool bInit = false;

#ifdef USE_OPENIL
      if (!bInit)
      {
        bInit = true;
        ilInit();
      }

      ILuint Image;
      ilGenImages (1, &Image);
      ilBindImage (Image);
      ilLoadImage (sAbsolutePath.c_str ());
      iluImageParameter (ILU_FILTER, ILU_BILINEAR);
      iluScale (256, 256, 1);

      const aeUInt32 uiWidth = ilGetInteger (IL_IMAGE_WIDTH);
      const aeUInt32 uiHeight= ilGetInteger (IL_IMAGE_HEIGHT);

      if (ilGetData ())
      {
        aeArray<aeUInt8> Data;
        Data.resize (uiWidth * uiHeight * 4);

        ilCopyPixels (0, 0, 0, uiWidth, uiHeight, 1, IL_BGRA, IL_UNSIGNED_BYTE, Data.data ());

        QImage img (Data.data (), uiWidth, uiHeight, QImage::Format_ARGB32);

        QPixmap pm;
        pm.convertFromImage (img);

        LabelPreview->setPixmap (pm);
      }

      ilDeleteImage (Image);
#endif
    }
  }

  if (m_MaterialToComboIndex.find (szMaterial) != m_MaterialToComboIndex.end ())
  {
    ComboMaterials->setCurrentIndex (m_MaterialToComboIndex[szMaterial]);
  }
}

void qtResourceEditorWidget::on_ButtonMaterialAdd_clicked ()
{
  static aeFilePath sPrevTexture = "";

  aeHybridString<128> sResult;
  if (!qtTreeEditWidget::SelectTexture (sPrevTexture, sResult, LineMaterialDiffuse))
    return;

  aeTreeMaterialLibrary::AddMaterial (sResult.c_str ());

  SetSelectedMaterial (sResult.c_str ());

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtResourceEditorWidget::on_ButtonMaterialDelete_clicked ()
{
  aeTreeMaterialLibrary::DeleteMaterial (m_sSelectedTreeMaterial.c_str ());

  if (aeTreeMaterialLibrary::GetMaterials ().empty ())
  {
    SetSelectedMaterial ("");
    return;
  }

  aeMap<aeHybridString<128>, aeTreeMaterial>::const_iterator it = aeTreeMaterialLibrary::GetMaterials ().lower_bound (m_sSelectedTreeMaterial);

  if (it == aeTreeMaterialLibrary::GetMaterials ().end ())
    SetSelectedMaterial (aeTreeMaterialLibrary::GetMaterials ().begin ().key ().c_str ());
  else
    SetSelectedMaterial (it.key ().c_str ());

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtResourceEditorWidget::on_ComboMaterials_currentIndexChanged (int index)
{
  SetSelectedMaterial (ComboMaterials->itemData (index).toString ().toUtf8 ().data ());
}

void qtResourceEditorWidget::on_ButtonSelectTextureNormal_clicked ()
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  static aeFilePath sPrevTexture = "";

  if (!qtTreeEditWidget::SelectTexture (sPrevTexture, pMaterial->m_sTextureNormal, LineMaterialNormal))
    return;
}

void qtResourceEditorWidget::on_RadioMaterialUndef_clicked ()
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  SpinMaterialTilingX->setEnabled (false);
  SpinMaterialTilingY->setEnabled (false);
  SpinMaterialTilingX->setValue (1);
  SpinMaterialTilingY->setValue (1);

  pMaterial->m_Usage = aeMaterialType::Unknown;
  pMaterial->m_uiTilingX = 1;
  pMaterial->m_uiTilingY = 1;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);

  UpdateMaterialList ();
}

void qtResourceEditorWidget::on_RadioMaterialBark_clicked ()
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  SpinMaterialTilingX->setEnabled (false);
  SpinMaterialTilingY->setEnabled (false);
  SpinMaterialTilingX->setValue (1);
  SpinMaterialTilingY->setValue (1);

  pMaterial->m_Usage = aeMaterialType::Bark;
  pMaterial->m_uiTilingX = 1;
  pMaterial->m_uiTilingY = 1;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);

  UpdateMaterialList ();
}

void qtResourceEditorWidget::on_RadioMaterialFrond_clicked ()
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  SpinMaterialTilingX->setEnabled (true);
  SpinMaterialTilingY->setEnabled (false);
  SpinMaterialTilingY->setValue (1);

  pMaterial->m_Usage = aeMaterialType::Frond;
  pMaterial->m_uiTilingY = 1;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);

  UpdateMaterialList ();
}

void qtResourceEditorWidget::on_RadioMaterialLeaf_clicked ()
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  SpinMaterialTilingX->setEnabled (true);
  SpinMaterialTilingY->setEnabled (true);

  pMaterial->m_Usage = aeMaterialType::Leaf;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);

  UpdateMaterialList ();
}

void qtResourceEditorWidget::on_SpinMaterialTilingX_valueChanged (int i)
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  pMaterial->m_uiTilingX = i;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtResourceEditorWidget::on_SpinMaterialTilingY_valueChanged (int i)
{
  aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetMaterial (m_sSelectedTreeMaterial.c_str ());

  if (!pMaterial)
    return;

  pMaterial->m_uiTilingY = i;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

bool qtResourceEditorWidget::eventFilter (QObject* object, QEvent* event)
{
  if (object == LineMaterialDiffuse && event->type() == QEvent::MouseButtonPress) 
  {
    m_iPreviewTexture = 0;
    SetSelectedMaterial (m_sSelectedTreeMaterial.c_str ());
    return false;
  }

  if (object == LineMaterialNormal && event->type() == QEvent::MouseButtonPress) 
  {
    m_iPreviewTexture = 1;
    SetSelectedMaterial (m_sSelectedTreeMaterial.c_str ());
    return false;
  }

  return false;
}

//void qtResourceEditorWidget::on_LineMaterialDiffuse_cursorPositionChanged ()
//{
//  m_iPreviewTexture = 0;
//  SetSelectedMaterial (m_sSelectedTreeMaterial.c_str ());
//}
//
//void qtResourceEditorWidget::on_LineMaterialNormal_cursorPositionChanged ()
//{
//  m_iPreviewTexture = 1;
//  SetSelectedMaterial (m_sSelectedTreeMaterial.c_str ());
//}
