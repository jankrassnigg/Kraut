// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "ShaderObject.h"
#include "ShaderManager.h"
#include <KrautGraphics/ResourceManager/ResourceManager.h>
#include <KrautGraphics/RenderAPI/Main.h>

namespace AE_NS_GRAPHICS
{
  aeString PortShader(const aeString& sCode, AE_SHADER_TYPE ShaderType);

  extern AE_GS_INOUT_TYPE g_GSInputType;
  extern AE_GS_INOUT_TYPE g_GSOutputType;
  extern aeUInt32 g_GSOutputVertices;

  void SplitString(const char* szString, char cSplit, std::vector<aeString>& out_sResults)
  {
    aeString sResult;
    int iPos = 0;

    while (szString[iPos] != '\0')
    {
      if (szString[iPos] == cSplit)
      {
        if (!sResult.empty())
          out_sResults.push_back(sResult);

        sResult.clear();
      }
      else
        sResult += szString[iPos];

      ++iPos;
    }

    if (!sResult.empty())
      out_sResults.push_back(sResult);
  }

  void LogShaderSource(const char* szHeader, const aeString& sSource, aeInt32 iErrorLine = -1)
  {
    AE_LOG_BLOCK(szHeader);

    std::vector<aeString> sResults;
    SplitString(sSource.c_str(), '\n', sResults);

    for (aeUInt32 i = 0; i < sResults.size(); ++i)
    {
      if (i + 1 == iErrorLine)
        aeLog::Error("%03d: %s", i + 1, sResults[i].c_str());
      else
        aeLog::Log("%03d: %s", i + 1, sResults[i].c_str());
    }

    if (sResults.empty())
    {
      aeLog::Error("Shader Source seems empty: \"%s\"", sSource.c_str());
    }

    //aeLog::Flush ();
  }

  void LogErrorString(const char* szString)
  {
    std::vector<aeString> sResults;
    SplitString(szString, '\n', sResults);

    for (aeUInt32 i = 0; i < sResults.size(); ++i)
      aeLog::Log(sResults[i].c_str());
  }

  aeShaderObject::aeShaderObject()
  {
  }

  void aeShaderObject::setShaderCode(const char* szSource)
  {
    m_RootCondition.SetVariable("BENDERISTHEGREATEST", false);
    aeShaderManager::setShaderBuilderVariable("BENDERISTHEGREATEST", "1", true);

    m_RootCondition.ParseForConditions(szSource, m_UsedVariables);
  }

  aeString aeShaderObject::BuildShaderCode(void)
  {
    aeString sResult;
    m_RootCondition.BuildShaderCode(sResult, false);

    return (sResult);
  }

  aeShaderTypeData* aeShaderObject::getShaderID(AE_SHADER_TYPE ShaderType, aeResourceHandle hShader)
  {
    aeSet<aeStaticString<32>>::iterator it = m_UsedVariables.begin();
    const aeSet<aeStaticString<32>>::iterator itend = m_UsedVariables.end();

    aeString sKey;

    for (; it != itend; ++it)
      sKey += it.key() + aeCondition::getShaderVariable(it.key().c_str());

    aeMap<aeString, aeShaderTypeData>::iterator its = m_ShaderCombos.find(sKey);

    if (its != m_ShaderCombos.end())
      return (&its.value());

    aeShaderTypeData id;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      id = CreateShaderOGL(ShaderType, hShader);
    }
#endif

    m_ShaderCombos[sKey] = id;

    return (&m_ShaderCombos[sKey]);
  }

  static aeInt32 getErrorLine(const char* szLog)
  {

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      AE_GPU_VENDOR::Enum Vendor = aeShaderManager::getGPUVendor();

      if (Vendor == AE_GPU_VENDOR::AE_VENDOR_NVIDIA)
      {
        aeInt32 iStart = aeStringFunctions::FindFirstStringPos(szLog, "(") + 1;
        aeInt32 iEnd = aeStringFunctions::FindFirstStringPos(szLog, ")");

        if ((iStart != -1) && (iEnd != -1) && (iEnd > iStart) && (iEnd - iStart < 4))
        {
          char szNumber[8] = "";

          for (aeInt32 i = 0; i < iEnd - iStart; ++i)
          {
            szNumber[i] = szLog[iStart + i];
            szNumber[i + 1] = '\0';
          }

          return (atoi(szNumber));
        }
      }
      else if (Vendor == AE_GPU_VENDOR::AE_VENDOR_ATI)
      {
        aeInt32 iStart = aeStringFunctions::FindFirstStringPos(szLog, "ERROR: ");
        aeInt32 iEnd = aeStringFunctions::GetLength(szLog);

        if (iStart == -1)
          return (-1);

        aeInt32 iStart2 = iStart + 7;
        for (aeInt32 i = iStart2; i < iEnd; ++i)
        {
          if (szLog[i] == ':')
          {
            iStart2 = i + 1;
            break;
          }
        }

        char szNumber[8] = "";

        for (aeInt32 i = 0; i < aeMath::Min(7, iEnd - iStart2); ++i)
        {
          if (szLog[iStart2 + i] == ':')
            return (atoi(szNumber));

          szNumber[i] = szLog[iStart2 + i];
          szNumber[i + 1] = '\0';
        }

        return (-1);
      }
    }
#endif


    return (-1);
  }

#ifdef AE_RENDERAPI_OPENGL
  aeShaderTypeData aeShaderObject::CreateShaderOGL(AE_SHADER_TYPE ShaderType, aeResourceHandle hShader)
  {
    aeRenderAPI::CheckErrors("Before aeShaderObject::CreateShader");

    aeLog::Dev("CreateShaderOGL");

    const aeString sCode0 = BuildShaderCode();

    //LogShaderSource ("ORIGINAL", sCode0);

    const aeString sCode = PortShader(sCode0, ShaderType);

    //LogShaderSource ("PORTED", sCode);



    aeString sType;

    aeShaderTypeData uiHandle;
    switch (ShaderType)
    {
      case AE_ST_VERTEX:
        sType = "Vertex Shader";
        uiHandle.m_ShaderOGL = glCreateShader(GL_VERTEX_SHADER);
        break;
      case AE_ST_GEOMETRY:
        sType = "Geometry Shader";
        uiHandle.m_ShaderOGL = glCreateShader(GL_GEOMETRY_SHADER);
        uiHandle.m_GSInputType = g_GSInputType;
        uiHandle.m_GSOutputType = g_GSOutputType;
        uiHandle.m_GSOutputVertices = g_GSOutputVertices;
        break;
      case AE_ST_FRAGMENT:
        sType = "Fragment Shader";
        uiHandle.m_ShaderOGL = glCreateShader(GL_FRAGMENT_SHADER);
        break;
      default:
        AE_CHECK_ALWAYS(false, "Unknown Shader Type %d", ShaderType);
        break;
    }

    // set shader source and compile it
    aeInt32 iLength = aeInt32(sCode.length());
    const char* szPointer = sCode.c_str();
    glShaderSource(uiHandle.m_ShaderOGL, 1, &szPointer, &iLength);
    glCompileShader(uiHandle.m_ShaderOGL);

    int iStatus = 0;
    glGetShaderiv(uiHandle.m_ShaderOGL, GL_COMPILE_STATUS, &iStatus);

    if (iStatus == GL_FALSE)
    {
      // get error messages
      int iLogLength;
      glGetShaderiv(uiHandle.m_ShaderOGL, GL_INFO_LOG_LENGTH, &iLogLength);

      char* szLog = new char[iLogLength + 1];

      glGetShaderInfoLog(uiHandle.m_ShaderOGL, iLogLength, &iLogLength, szLog);

      aeString sShaderFile = aeResourceManager::GetFileResourceWasLoadedFrom(hShader);

      AE_LOG_BLOCK(sShaderFile.c_str());

      aeLog::Error("Compilation of %s failed.", sType.c_str());
      LogErrorString(szLog);

      LogShaderSource("Shader Source", sCode.c_str(), getErrorLine(szLog));

      delete[] szLog;
    }

    aeRenderAPI::CheckErrors("After aeShaderObject::CreateShader");

    aeShaderManager::ReflectShaderForConstantBuffersOGL(uiHandle);

    return (uiHandle);
  }
#endif

  void aeShaderObject::UnloadAllShaders(void)
  {
    m_RootCondition.UnloadCondition();

    aeMap<aeString, aeShaderTypeData>::iterator it = m_ShaderCombos.begin();
    const aeMap<aeString, aeShaderTypeData>::iterator itend = m_ShaderCombos.end();

    for (; it != itend; ++it)
    {
      aeShaderManager::UnloadProgramsUsingShader(it.value());

#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        glDeleteProgram(it.value().m_ShaderOGL);
      }
#endif
    }

    m_ShaderCombos.clear();
    m_UsedVariables.clear();
  }
} // namespace AE_NS_GRAPHICS
