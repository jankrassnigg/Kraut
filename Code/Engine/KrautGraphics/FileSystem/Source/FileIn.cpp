#include "../FileIn.h"


namespace AE_NS_FOUNDATION
{
  aeFileIn::aeFileIn () : aeStreamIn ()
  {
    m_pCache = nullptr;
  }

  aeFileIn::~aeFileIn ()
  {
    Close ();
  }

  const aeString aeFileIn::GetFilePathInDataDir (void) const
  {
    return aePathFunctions::MakeRelative (m_pReader->GetDataDirectoryHandler ()->GetDataDirectory (), GetFilePath ());
  }

  bool aeFileIn::Open (const char* szFile, aeUInt32 uiCacheSize)
  {
    m_pReader = aeFileSystem::OpenFileToRead (szFile);

    if (!m_pReader)
      return (false);

    m_bIsOpen = true;
    m_uiCacheSize = uiCacheSize;
    m_pCache = new aeUInt8[m_uiCacheSize];

    m_uiCacheReadPosition = 0;
    m_uiBytesCached = m_pReader->Read (&m_pCache[0], m_uiCacheSize);

    return (true);
  }

  void aeFileIn::Close (void)
  {
    if (!m_bIsOpen)
      return;

    m_bIsOpen = false;

    delete[] m_pCache;
    m_pCache = nullptr;

    m_pReader->Close ();
    m_pReader = nullptr;
  }

  aeUInt32 aeFileIn::ReadFromStream (void* pData, aeUInt32 uiSize)
  {
    aeUInt32 uiBufferPosition = 0;	//how much was read, yet
    aeUInt8* pBuffer = (aeUInt8*) pData;

    while (uiSize > 0)
    {
      // determine the chunk size to read
      aeUInt32 uiChunkSize = uiSize;
      if (m_uiBytesCached - m_uiCacheReadPosition < uiSize)
        uiChunkSize = m_uiBytesCached - m_uiCacheReadPosition;

      // copy data into the buffer
      aeMemory::Copy (&m_pCache[m_uiCacheReadPosition], &pBuffer[uiBufferPosition], uiChunkSize);

      // store how much was read and how much is still left to read
      uiBufferPosition += uiChunkSize;
      m_uiCacheReadPosition += uiChunkSize;
      uiSize -= uiChunkSize;



      // if the cache is depleted, refill it
      // this will even be triggered if EXACTLY the amount of available bytes was read
      if (m_uiCacheReadPosition >= m_uiBytesCached)
      {
        m_uiBytesCached = m_pReader->Read (&m_pCache[0], m_uiCacheSize);
        m_uiCacheReadPosition = 0;

        // if nothing else could be read from the file, return the number of bytes that have been read
        if (m_uiBytesCached == 0)
        {
          // if absolutely nothing could be read, we reached the end of the file (and we actually returned everything else,
          // so the file was really read to the end).
          SetEndOfStreamReached ();
          return (uiBufferPosition);
        }
      }
    }

    // return how much was read
    return (uiBufferPosition);
  }

}




