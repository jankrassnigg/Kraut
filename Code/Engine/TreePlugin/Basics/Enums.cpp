#include "PCH.h"

#include "Enums.h"

const char* aeLod::EnumNames[aeLod::ENUM_COUNT] =
  {
    "None",
    "LoD 0",
    "LoD 1",
    "LoD 2",
    "LoD 3",
    "LoD 4"};

aeUInt32 aeLeafCardResolution::Values[aeLeafCardResolution::ENUM_COUNT] =
  {
    2048,
    1024,
    512,
    256,
    128,
    64,
};

aeUInt32 aeLeafCardMipmapResolution::Values[aeLeafCardMipmapResolution::ENUM_COUNT] =
  {
    2048,
    1024,
    512,
    256,
    128,
    64,
    32,
    16,
    8,
    4,
    2,
    1};

const char* aeForceType::EnumNames[ENUM_COUNT] =
  {
    "Magnet",
    "Direction",
    "Mesh",
};

const char* aeForceFalloff::EnumNames[ENUM_COUNT] =
  {
    "None",
    "Linear",
    "Quadratic",
    "Hard"};

const char* aeMaterialType::EnumNames[ENUM_COUNT] =
  {
    "[Bark]  ",
    "[Frond] ",
    "[Leaf]  ",
    "[Undef] ",
};

aeInt32 aeBranchGeometryType::TextureSampler[Kraut::BranchGeometryType::ENUM_COUNT];
aeInt32 aeBranchGeometryType::ImpostorTextureSampler = -1;
