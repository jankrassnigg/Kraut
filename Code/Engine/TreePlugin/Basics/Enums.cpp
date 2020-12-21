#include "PCH.h"

#include "Enums.h"

const char* aeBranchType::EnumNames[aeBranchType::ENUM_COUNT] =
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

const char* aeBranchType::EnumNamesExport[aeBranchType::ENUM_COUNT] =
{
  "Trunk_1",
  "Trunk_2",
  "Trunk_3",
  "Main_Branches_1",
  "Main_Branches_2",
  "Main_Branches_3",
  "Sub_Branches_1",
  "Sub_Branches_2",
  "Sub_Branches_3",
  "Twigs_1",
  "Twigs_2",
  "Twigs_3",
};

const char* aeTargetDir::EnumNames[aeTargetDir::ENUM_COUNT] =
{
  "Straight",
  "0° (Upwards)",
  "22°",   
  "45°",
  "67°",
  "90° (Orthogonal)",
  "112°",
  "135°",
  "157°",
  "180° (Downwards)",
};

const char* aeLod::EnumNames[aeLod::ENUM_COUNT] =
{
  "None",
  "LoD 0",
  "LoD 1",
  "LoD 2",
  "LoD 3",
  "LoD 4"
};

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
  1
};

const char* aeLeafOrientation::EnumNames[aeLeafOrientation::ENUM_COUNT] =
{
  "Upwards",
  "Along Parent",
  "Orthogonal To Parent",
};

const char* aeMeshType::EnumNames[ENUM_COUNT] =
{
  "Branch",
  "Frond",
  "Leaf"
};

aeInt32 aeMeshType::TextureSampler[ENUM_COUNT];
aeInt32 aeMeshType::ImpostorTextureSampler = -1;

const char* aeForceType::EnumNames[ENUM_COUNT] =
{
  "Magnet",
  "Direction",
  "Mesh",
};

const char* aeBranchTypeMode::EnumNames[ENUM_COUNT] =
{
  "Default",
  "Umbrella"
};

const char* aeForceFalloff::EnumNames[ENUM_COUNT] =
{
  "None",
  "Linear",
  "Quadratic",
  "Hard"
};

const char* aeMaterialType::EnumNames[ENUM_COUNT] =
{
  "[Bark]  ",
  "[Frond] ",
  "[Leaf]  ",
  "[Undef] ",
};