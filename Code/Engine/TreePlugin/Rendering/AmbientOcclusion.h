#ifndef AE_TREEPLUGIN_AMBIENTOCCLUSION_H
#define AE_TREEPLUGIN_AMBIENTOCCLUSION_H

#include "../Basics/Base.h"
#include "../Basics/Plugin.h"
#include <KrautGenerator/Infrastructure/BoundingBox.h>

class aeAOGrid
{
public:
  struct aeDirection
  {
    aeDirection() {}
    aeDirection(float f)
    {
      for (int i = 0; i < 6; ++i)
        m_fDir[i] = f;
    }
    const aeDirection operator+(const aeDirection& rhs) const
    {
      aeDirection res;
      for (int i = 0; i < 6; ++i)
        res.m_fDir[i] = m_fDir[i] + rhs.m_fDir[i];

      return res;
    }

    const aeDirection operator*(float f) const
    {
      aeDirection res;
      for (int i = 0; i < 6; ++i)
        res.m_fDir[i] = m_fDir[i] * f;

      return res;
    }

    float m_fDir[6];
  };

  static aeAOGrid::aeDirection SampleGrid(const aeVec3& vPos);
  static void Update(Kraut::BoundingBox box, float fCellSize);
  static void Clear(void)
  {
    m_Cells.clear();
    m_uiCells[0] = 0;
    m_uiCells[1] = 0;
    m_uiCells[2] = 0;
  }
  static float GetAmbientOcclusionAt(const aeVec3& vPos, const aeVec3& vNormal);

  static void Save(aeStreamOut& stream);
  static void Load(aeStreamIn& stream);

private:
  static inline aeUInt32 GetCellIndex(aeUInt32 x, aeUInt32 y, aeUInt32 z)
  {
    const aeUInt32 uiIndex = (z * m_uiCells[1] + y) * m_uiCells[0] + x;
    return uiIndex;
  }
  static inline aeDirection GetCell(aeUInt32 x, aeUInt32 y, aeUInt32 z) { return m_Cells[GetCellIndex(x, y, z)]; }
  static inline void SetCell(aeUInt32 x, aeUInt32 y, aeUInt32 z, aeDirection fValue) { m_Cells[GetCellIndex(x, y, z)] = fValue; }
  static void UpdatePosition(aeUInt32 x, aeUInt32 y, aeUInt32 z);
  static void RenderCubeFace(aeInt32 iFace, const aeVec3& vFromPos, const aeVec3& vDir, const aeVec3& vUp);
  static aeAOGrid::aeDirection GetRenderedResult(void);

  // to be saved
  static Kraut::BoundingBox m_BBox;
  static aeUInt32 m_uiCells[3];
  static aeArray<aeDirection> m_Cells;
  static float m_fCellSize;

  // temp data
  static aeArray<aeUInt8> m_RenderedImage;
};

#endif
