#ifndef AE_TREEPLUGIN_ENUMS_H
#define AE_TREEPLUGIN_ENUMS_H

#include "Base.h"
#include <KrautGenerator/Description/DescriptionEnums.h>

struct aePaintingPlaneMode
{
  enum Enum
  {
    InViewPlane,
    Upwards,
    Planar,
    AlongBranch,
    OrthogonalToBranch,
  };
};

struct aeTreeRenderMode
{
  enum Enum
  {
    Default,
    Wireframe,
    Skeleton,
    Normals,
    Diffuse,
    Light,
    Ambient,
    Skylight,
    ENUM_COUNT
  };
};

struct aeLod
{
  enum Enum
  {
    None,
    Lod0,
    Lod1,
    Lod2,
    Lod3,
    Lod4,

    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
};

struct aeBranchGeometryType
{
  static aeInt32 TextureSampler[Kraut::BranchGeometryType::ENUM_COUNT];
  static aeInt32 ImpostorTextureSampler;
};

struct aeLeafCardResolution
{
  enum Enum
  {
    Tex2048,
    Tex1024,
    Tex512,
    Tex256,
    Tex128,
    Tex64,
    ENUM_COUNT
  };

  static aeUInt32 Values[aeLeafCardResolution::ENUM_COUNT];
};

struct aeLeafCardMipmapResolution
{
  enum Enum
  {
    None,
    Tex1024,
    Tex512,
    Tex256,
    Tex128,
    Tex64,
    Tex32,
    Tex16,
    Tex8,
    Tex4,
    Tex2,
    Tex1,
    ENUM_COUNT
  };

  static aeUInt32 Values[aeLeafCardMipmapResolution::ENUM_COUNT];
};

struct aeColor
{
  aeUInt8 r;
  aeUInt8 g;
  aeUInt8 b;
  aeUInt8 a;
};

struct aeForceType
{
  enum Enum
  {
    Position,
    Direction,
    Mesh,
    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
};

struct aeForceFalloff
{
  enum Enum
  {
    None,
    Linear,
    Quadratic,
    Hard,
    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
};

struct aeMaterialType
{
  enum Enum
  {
    Bark,
    Frond,
    Leaf,
    Unknown,
    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
};

#endif
