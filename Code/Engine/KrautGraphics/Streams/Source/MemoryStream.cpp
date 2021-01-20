#include "../MemoryStream.h"

namespace AE_NS_FOUNDATION
{
  void aeMemoryStream::WriteToStream(const void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBytes = (aeUInt8*)pData;

    for (aeUInt32 ui = 0; ui < uiSize; ++ui)
    {
      m_Data.push_back(pBytes[ui]);
    }
  }

  aeUInt32 aeMemoryStream::ReadFromStream(void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBytes = (aeUInt8*)pData;

    uiSize = aeMath::Min(uiSize, m_Data.size());

    for (aeUInt32 ui = 0; ui < uiSize; ++ui)
    {
      pBytes[ui] = m_Data.front();
      m_Data.pop_front();
    }

    return uiSize;
  }
} // namespace AE_NS_FOUNDATION
