#include "../Streams.h"
#include "../MemoryStream.h"



namespace AE_NS_FOUNDATION
{

  void aeStreamOut::WriteLine (const char* szString)
  {
    int iPos = 0;
    while ((szString[iPos] != '\0') && (szString[iPos] != '\n'))
    {
      if (szString[iPos] != '\r')
        Write (&szString[iPos], sizeof (char));

      ++iPos;
    }

    char cTerminator = '\n';
    Write (&cTerminator, sizeof (char));
  }

  void aeStreamIn::ReadLine (aeString& out_sString)
  {
    out_sString.clear ();

    char c;
    while (!IsEndOfStream ())
    {
      if (Read (&c, sizeof (char)) == 0)
        return;

      if ((c == '\n') || (c == '\0'))
        return;

      if (c != '\r')
        out_sString += c;
    }
  }

  void aeMemoryStream::WriteToStream (const void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBytes = (aeUInt8*) pData;

    for (aeUInt32 ui = 0; ui < uiSize; ++ui)
    {
      m_Data.push_back (pBytes[ui]);
    }
  }

  aeUInt32 aeMemoryStream::ReadFromStream (void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBytes = (aeUInt8*) pData;

    uiSize = aeMath::Min (uiSize, m_Data.size ());

    for (aeUInt32 ui = 0; ui < uiSize; ++ui)
    {
      pBytes[ui] = m_Data.front ();
      m_Data.pop_front ();
    }

    return uiSize;
  }

  void aeInPlaceMemoryStream::WriteToStream (const void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBytes = (aeUInt8*) pData;

    for (aeUInt32 ui = 0; ui < uiSize; ++ui)
    {
      if (m_iPos < m_iBufferSize)
      {
        m_pBuffer[m_iPos] = pBytes[ui];
        ++m_iPos;
      }
    }
  }

  aeUInt32 aeInPlaceMemoryStream::ReadFromStream (void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBytes = (aeUInt8*) pData;

    uiSize = aeMath::Min (uiSize, m_iBufferSize - m_iPos);

    for (aeUInt32 ui = 0; ui < uiSize; ++ui)
    {
      pBytes[ui] = m_pBuffer[m_iPos];
      ++m_iPos;
    }

    return uiSize;
  }
}

