#ifndef AE_FOUNDATION_STREAMS_MEMORYSTREAM_H
#define AE_FOUNDATION_STREAMS_MEMORYSTREAM_H

#include "../Defines.h"
#include "../Streams/Streams.h"
#include "../Containers/Deque.h"

namespace AE_NS_FOUNDATION
{
  class aeMemoryStream : public aeStreamIn, public aeStreamOut
  {
  public:
    aeMemoryStream () {}
    aeMemoryStream (const aeMemoryStream& rhs) { *this = rhs; }
    void operator= (const aeMemoryStream& rhs) { m_Data = rhs.m_Data; }

    const aeDeque<aeUInt8>& GetDataStream () const { return m_Data; }

  private:
    AE_FOUNDATION_DLL void WriteToStream (const void* pData, aeUInt32 uiSize);
    AE_FOUNDATION_DLL aeUInt32 ReadFromStream (void* pData, aeUInt32 uiSize);

    aeDeque<aeUInt8> m_Data;
  };


  class aeInPlaceMemoryStream : public aeStreamIn, public aeStreamOut
  {
  public:
    aeInPlaceMemoryStream () : m_pBuffer(NULL), m_iBufferSize(0), m_iPos(0) {}
    aeInPlaceMemoryStream (const aeInPlaceMemoryStream& rhs) { *this = rhs; }
    void operator= (const aeInPlaceMemoryStream& rhs)
    {
      m_pBuffer = rhs.m_pBuffer;
      m_iBufferSize = rhs.m_iBufferSize;
      m_iPos = rhs.m_iPos;
    }

    void SetDataStream(aeUInt8* pBuffer, aeUInt32 iBufferSize)
    {m_pBuffer = pBuffer; m_iBufferSize = iBufferSize;}

    aeUInt8* GetDataStream(void) {return m_pBuffer;}
    aeUInt32 GetDataStreamSize(void) {return m_iBufferSize;}
    aeUInt32 GetCurrentPos(void) {return m_iPos;}
    void SetCurrentPos(aeUInt32 iPos) {m_iPos = iPos;}

  private:
    AE_FOUNDATION_DLL void WriteToStream (const void* pData, aeUInt32 uiSize);
    AE_FOUNDATION_DLL aeUInt32 ReadFromStream (void* pData, aeUInt32 uiSize);

    aeUInt8* m_pBuffer;
    aeUInt32 m_iBufferSize;
    aeUInt32 m_iPos;
  };
}

#endif

