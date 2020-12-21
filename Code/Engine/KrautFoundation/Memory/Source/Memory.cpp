#include "../../Containers/Array.h"
#include "../../Logging/Log.h"



namespace AE_NS_FOUNDATION
{
  struct aeTempBuffer
  {
    aeUInt32 m_uiSize;
    void* m_pBuffer;
    bool m_bAccquired;

    aeTempBuffer ()
    {
      m_uiSize = 0;
      m_pBuffer = NULL;
      m_bAccquired = false;
    }
  };

  static aeArray<aeTempBuffer> g_TempBuffers;

  void aeMemory::ClearTempBuffers (void)
  {
    for (aeUInt32 ui = 0; ui < g_TempBuffers.size (); ++ui)
      AE_MEMORY_DEALLOCATE_NODEBUG (g_TempBuffers[ui].m_pBuffer, true);

    g_TempBuffers.clear ();
  }

  void* aeMemory::AccquireTempBuffer (aeUInt32 uiSize)
  {
    aeInt32 iBestFitIndex = -1;
    aeUInt32 uiBestFitSize = 0xFFFFFFFF;

    for (aeUInt32 ui = 0; ui < g_TempBuffers.size (); ++ui)
    {
      if ((!g_TempBuffers[ui].m_bAccquired) && (g_TempBuffers[ui].m_uiSize >= uiSize))
      {
        if (g_TempBuffers[ui].m_uiSize < uiBestFitSize)
        {
          uiBestFitSize = g_TempBuffers[ui].m_uiSize;
          iBestFitIndex = ui;
        }
      }
    }

    if (iBestFitIndex >= 0)
    {
      g_TempBuffers[iBestFitIndex].m_bAccquired = true;
      return (g_TempBuffers[iBestFitIndex].m_pBuffer);
    }

    AE_CHECK_DEV (g_TempBuffers.size () < 100, "aeMemory::AccquireTempBuffer: Using too many buffer.");

    aeTempBuffer NewBuffer;
    NewBuffer.m_bAccquired = true;
    NewBuffer.m_uiSize  = aeMath::Max (aeMath::PowerOfTwo_Ceil (uiSize), 4096);
    NewBuffer.m_pBuffer = AE_MEMORY_ALLOCATE_NODEBUG (NewBuffer.m_uiSize, true);

    g_TempBuffers.push_back (NewBuffer);

    AE_IF_DEVELOPING
    {
      LogTempBufferStats ();
    }

    return (NewBuffer.m_pBuffer);
  }

  void aeMemory::ReleaseTempBuffer (void* pBuffer)
  {
    for (aeUInt32 ui = 0; ui < g_TempBuffers.size (); ++ui)
    {
      if (g_TempBuffers[ui].m_pBuffer == pBuffer)
      {
        g_TempBuffers[ui].m_bAccquired = false;
        return;
      }
    }

    AE_CHECK_DEV (false, "aeMemory::ReleaseTempBuffer: The given Temp-Buffer %p does not exist.", pBuffer);
  }

  void aeMemory::LogTempBufferStats (void)
  {
    // make sure this function is never call recursively, to prevent endless recursions
    static bool bRecursive = false;
    {
      if (bRecursive)
        return;

      bRecursive = true;
    }


    AE_LOG_BLOCK ("aeMemory::LogTempBufferStats");

    aeUInt32 uiSummedMemory = 0;
    for (aeUInt32 ui = 0; ui < g_TempBuffers.size (); ++ui)
    {
      uiSummedMemory += g_TempBuffers[ui].m_uiSize;
      aeLog::Log ("Temp-Buffer #%d. Size: %d KB (%.3f MB) (%s).", ui, g_TempBuffers[ui].m_uiSize / 1024, (float) (g_TempBuffers[ui].m_uiSize) / (1024.0f * 1024.0f), g_TempBuffers[ui].m_bAccquired ? "accquired" : "free");
    }

    if (g_TempBuffers.size () > 10)
      aeLog::SeriousWarning ("Using %d Temp-Buffers.", g_TempBuffers.size ());
    else
    if (g_TempBuffers.size () > 4)
      aeLog::Warning ("Using %d Temp-Buffers.", g_TempBuffers.size ());
    else
      aeLog::Log ("Using %d Temp-Buffers.", g_TempBuffers.size ());

    if (uiSummedMemory > 1024 * 1024 * 20)
      aeLog::SeriousWarning ("Using %.3f MB of Memory for Temp-Buffers!", (float) (uiSummedMemory) / (1024.0f * 1024.0f));
    else
    if (uiSummedMemory > 1024 * 1024 * 10)
      aeLog::Warning ("Using %.3f MB of Memory for Temp-Buffers!", (float) (uiSummedMemory) / (1024.0f * 1024.0f));
    else
      aeLog::Log ("Using %.3f MB of Memory for Temp-Buffers.", (float) (uiSummedMemory) / (1024.0f * 1024.0f));

    bRecursive = false;
  }
}

