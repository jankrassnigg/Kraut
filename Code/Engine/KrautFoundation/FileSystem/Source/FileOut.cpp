#include "../FileOut.h"


namespace AE_NS_FOUNDATION
{
  aeFileOut::aeFileOut () : aeStreamOut ()
  {
    m_pCache = NULL;
  }

  aeFileOut::~aeFileOut ()
  {
    Close ();
  }

  const aeFilePath aeFileOut::GetFilePathInDataDir (void) const
  {
    return aePathFunctions::MakeRelative (m_pWriter->GetDataDirectoryHandler ()->GetDataDirectory (), GetFilePath ());
  }

  bool aeFileOut::Open (const char* szFile, aeUInt32 uiCacheSize)
  {
    m_sStreamName = szFile;
    m_pWriter = aeFileSystem::OpenFileToWrite (szFile);

    if (!m_pWriter)
      return (false);

    m_bIsOpen = true;
    m_uiCacheSize = uiCacheSize;
    m_pCache = new aeUInt8[m_uiCacheSize];

    m_uiCacheWritePosition = 0;

    return (true);
  }

  void aeFileOut::Close (void)
  {
    if (!m_bIsOpen)
      return;

    FlushWriteCache ();

    m_bIsOpen = false;

    delete[] m_pCache;
    m_pCache = NULL;

    m_pWriter->Close ();
    m_pWriter = NULL;
  }

  void aeFileOut::FlushWriteCache (void)
  {
    m_pWriter->Write (&m_pCache[0], m_uiCacheWritePosition);
    m_uiCacheWritePosition = 0;
  }

  void aeFileOut::WriteToStream (const void* pData, aeUInt32 uiSize)
  {
    aeUInt8* pBuffer = (aeUInt8*) pData;

    while (uiSize > 0)
    {
      // determine chunk size to be written
      aeUInt32 uiChunkSize = uiSize;
      if (m_uiCacheSize - m_uiCacheWritePosition < uiSize)
        uiChunkSize = m_uiCacheSize - m_uiCacheWritePosition;

      // copy memory
      aeMemory::Copy (pBuffer, &m_pCache[m_uiCacheWritePosition], uiChunkSize);

      pBuffer += uiChunkSize;
      m_uiCacheWritePosition += uiChunkSize;
      uiSize -= uiChunkSize;

      // if the cache is full or nearly full, flush it to disk
      if (m_uiCacheWritePosition + 32 >= m_uiCacheSize)
        FlushWriteCache ();
    }

  }

}




