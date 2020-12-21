#ifndef AE_TREEPLUGIN_ENUMS_H
#define AE_TREEPLUGIN_ENUMS_H

#include "Base.h"

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

struct aeTargetDir
{
  enum Enum
  {
    Straight,   // along the start direction
    Upwards,    // to the sky!
    Degree22,
    Degree45,
    Degree67,
    Degree90,
    Degree112,
    Degree135,
    Degree157,
    Downwards,  // to the ground
    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
};

struct aeTargetDir2Usage
{
  enum Enum
  {
    Off,
    Relative,
    Absolute,
  };
};

struct aeBranchType
{
  enum Enum
  {
    None          =-1,
    Trunk1        = 0,
    Trunk2        = 1,
    Trunk3        = 2,
    MainBranches1 = 3,
    MainBranches2 = 4,
    MainBranches3 = 5,
    SubBranches1  = 6,
    SubBranches2  = 7,
    SubBranches3  = 8,
    Twigs1        = 9,
    Twigs2        =10,
    Twigs3        =11,

    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
  static const char* EnumNamesExport[ENUM_COUNT];
};

struct aeLeafOrientation
{
  enum Enum
  {
    Upwards,
    AlongBranch,
    OrthogonalToBranch,
    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
};

struct aeMeshType
{
  enum Enum
  {
    Branch,
    Frond,
    Leaf,
    ENUM_COUNT
  };

  static const char* EnumNames[ENUM_COUNT];
  static aeInt32 TextureSampler[ENUM_COUNT];
  static aeInt32 ImpostorTextureSampler;
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

struct aeLodMode
{
  enum Enum
  {
    Full,
    FourQuads,
    TwoQuads,
    Billboard,
    Disabled,
    ENUM_COUNT
  };

  static bool IsImpostorMode (aeLodMode::Enum mode)
  {
    return (mode != aeLodMode::Full && mode != aeLodMode::Disabled);
  }

  static bool IsMeshMode (aeLodMode::Enum mode)
  {
    return (mode == aeLodMode::Full);
  }
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

struct aeBranchTypeMode
{
  enum Enum
  {
    Default,
    Umbrella,
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

