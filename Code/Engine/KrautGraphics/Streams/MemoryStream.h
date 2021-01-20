#ifndef AE_FOUNDATION_STREAMS_MEMORYSTREAM_H
#define AE_FOUNDATION_STREAMS_MEMORYSTREAM_H

#include <KrautFoundation/Containers/Deque.h>
#include <KrautFoundation/Streams/Streams.h>

namespace AE_NS_FOUNDATION
{
  class aeMemoryStream : public aeStreamIn, public aeStreamOut
  {
  public:
    aeMemoryStream() {}
    aeMemoryStream(const aeMemoryStream& rhs) { *this = rhs; }
    void operator=(const aeMemoryStream& rhs) { m_Data = rhs.m_Data; }

    const aeDeque<aeUInt8>& GetDataStream() const { return m_Data; }

  private:
    AE_FOUNDATION_DLL void WriteToStream(const void* pData, aeUInt32 uiSize);
    AE_FOUNDATION_DLL aeUInt32 ReadFromStream(void* pData, aeUInt32 uiSize);

    aeDeque<aeUInt8> m_Data;
  };
} // namespace AE_NS_FOUNDATION

#endif
