#ifndef AE_MATERIALLIBRARY_H
#define AE_MATERIALLIBRARY_H

#include "../../Basics/Base.h"

class aeTreeMaterial
{
public:
  aeTreeMaterial ();
  bool operator< (const aeTreeMaterial& rhs) const;

  const aeHybridString<128> GetDescription (void) const;

  aeHybridString<128> m_sTextureDiffuse;
  aeHybridString<128> m_sTextureNormal;
  aeMaterialType::Enum m_Usage;
  aeUInt8 m_uiTilingX;
  aeUInt8 m_uiTilingY;
};

class aeTreeMaterialLibrary
{
public:
  static void AddMaterial (const char* szDiffuse);
  static void DeleteMaterial (const char* szDiffuse);

  static aeTreeMaterial* GetMaterial (const char* szDiffuse);
  static aeTreeMaterial* GetOrAddMaterial (const char* szDiffuse);
  static const aeMap<aeHybridString<128>, aeTreeMaterial>& GetMaterials (void);

  static void Save (void);
  static void Load (void);

private:
  static aeMap<aeHybridString<128>, aeTreeMaterial> s_Materials;
};

#endif

