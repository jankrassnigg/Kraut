#include "PCH.h"
#include <KrautFoundation/FileSystem/FileOut.h>
#include <KrautFoundation/FileSystem/FileIn.h>

#include "qtResourceEditorWidget.moc.h"
#include "MaterialLibrary.h"

aeMap<aeHybridString<128>, aeTreeMaterial> aeTreeMaterialLibrary::s_Materials;

AE_ON_GLOBAL_EVENT_ONCE (aeTreePlugin_TreeSaved)
{
  aeTreeMaterialLibrary::Save ();
}

AE_ON_GLOBAL_EVENT_ONCE (aeStartup_ShutdownEngine_Begin)
{
  aeTreeMaterialLibrary::Save ();
}

aeTreeMaterial::aeTreeMaterial ()
{
  m_Usage = aeMaterialType::Unknown;
  m_uiTilingX = 1;
  m_uiTilingY = 1;
}

bool aeTreeMaterial::operator< (const aeTreeMaterial& rhs) const
{
  return GetDescription () < rhs.GetDescription ();
}

const aeHybridString<128> aeTreeMaterial::GetDescription (void) const
{
  aeHybridString<128> sText = aeMaterialType::EnumNames[m_Usage];

  sText += m_sTextureDiffuse;
  return sText;
}

void aeTreeMaterialLibrary::AddMaterial (const char* szDiffuse)
{
  if (s_Materials.find (szDiffuse) != s_Materials.end ())
    return;

  aeTreeMaterial m;
  m.m_sTextureDiffuse = szDiffuse;

  s_Materials[szDiffuse] = m;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
  AE_BROADCAST_EVENT (aeResourceEdit_TreeMaterialsChanged);
}

void aeTreeMaterialLibrary::DeleteMaterial (const char* szDiffuse)
{
  s_Materials.erase (szDiffuse);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
  AE_BROADCAST_EVENT (aeResourceEdit_TreeMaterialsChanged);
}

aeTreeMaterial* aeTreeMaterialLibrary::GetOrAddMaterial (const char* szDiffuse)
{
  aeTreeMaterial* pMaterial = GetMaterial (szDiffuse);

  if (pMaterial)
    return pMaterial;

  s_Materials[szDiffuse].m_sTextureDiffuse = szDiffuse;
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
  AE_BROADCAST_EVENT (aeResourceEdit_TreeMaterialsChanged);

  return &s_Materials[szDiffuse];
}

aeTreeMaterial* aeTreeMaterialLibrary::GetMaterial (const char* szDiffuse)
{
  aeMap<aeHybridString<128>, aeTreeMaterial>::iterator it = s_Materials.find (szDiffuse);

  if (it == s_Materials.end ())
    return nullptr;

  return &it.value ();
}

const aeMap<aeHybridString<128>, aeTreeMaterial>& aeTreeMaterialLibrary::GetMaterials (void)
{
  return s_Materials;
}

void aeTreeMaterialLibrary::Save (void)
{
  aeFileOut File;
  if (!File.Open ("Materials.Library"))
  {
    QMessageBox::critical (qtResourceEditorWidget::s_pWidget, "Kraut", "Could not save the material library!");
    return;
  }

  const aeUInt8 uiVersion = 1;
  File << uiVersion;

  const aeUInt32 uiMaterials = s_Materials.size ();
  File << uiMaterials;

  for (aeMap<aeHybridString<128>, aeTreeMaterial>::iterator it = s_Materials.begin (); it != s_Materials.end (); ++it)
  {
    const aeTreeMaterial& m = it.value ();

    File << m.m_sTextureDiffuse;
    File << m.m_sTextureNormal;
    File << m.m_uiTilingX;
    File << m.m_uiTilingY;
    const aeUInt8 uiUsage = m.m_Usage;
    File << uiUsage;
  }
}

void aeTreeMaterialLibrary::Load (void)
{
  aeFileIn File;
  if (!File.Open ("Materials.Library"))
    return;

  aeUInt8 uiVersion = 0;
  File >> uiVersion;

  aeUInt32 uiMaterials;
  File >> uiMaterials;

  for (aeUInt32 i = 0; i < uiMaterials; ++i)
  {
    aeString sDiffuse, sNormal;

    aeTreeMaterial m;

    File >> sDiffuse; m.m_sTextureDiffuse = sDiffuse;
    File >> sNormal;  m.m_sTextureNormal  = sNormal;
    File >> m.m_uiTilingX;
    File >> m.m_uiTilingY;
    aeUInt8 uiUsage;
    File >> uiUsage;
    m.m_Usage = (aeMaterialType::Enum) uiUsage;

    s_Materials[sDiffuse] = m;
  }
}

