#ifndef KRAUTIMPORT_H
#define KRAUTIMPORT_H

#include "VertexBuffer.h"
#include "Texture.h"
#include <vector>
#include <string>
using namespace std;

struct kvMaterial
{
  string m_sDiffuseTexture;
  string m_sNormalMap;

  kvTexture m_Diffuse;
  kvTexture m_NormalMap;
  unsigned char m_uiVariationColor[4];
};

struct kvTreeMesh
{
  int m_iMaterialType;
  int m_iMaterial;
  kvVertexBuffer m_VertexBuffer;
};

struct kvTreeLod
{
  float m_fLodDistance;
  unsigned char m_uiLodType; // 0 == full mesh, 1 == 4 quads, 2 == 2 quads, 3 == Billboard
  vector<kvTreeMesh> m_Meshes[3]; // currently there are never more than 3 material types (branch / frond / leaf), but impostors always only use the frond-type
};

struct kvTree
{
  kvMaterial m_ImpostorMaterial;
  vector<kvMaterial> m_Materials[3];
  vector<kvTreeLod> m_LODs;
  VEC3 m_BBoxMin;
  VEC3 m_BBoxMax;
};

void ImportKrautFile (const char* szFile, kvTree& out_Tree);

extern kvTree g_Tree;

#endif

