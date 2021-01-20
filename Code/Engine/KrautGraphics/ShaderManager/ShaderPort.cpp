#include "ShaderPort.h"
#include "Main.h"
#include <KrautGraphics/Utility/LuaWrapper.h>
#include <KrautGraphics/RenderAPI/Declarations.h>
#include <deque>


namespace AE_NS_GRAPHICS
{
  aeString ReplaceKeywordAndArrayIndex(const aeString& sCode, const aeString& sKeyword, const aeString& sReplacement);

  aeLuaWrapper g_PortScript;

  //! \todo DLL errors

  AE_GS_INOUT_TYPE g_GSInputType = AE_GS_IOT_TRIANGLES;
  AE_GS_INOUT_TYPE g_GSOutputType = AE_GS_IOT_TRIANGLE_STRIP;
  aeUInt32 g_GSOutputVertices = 0;

  static aeString g_sCodeBuilt;
  static AE_SHADER_TYPE g_ShaderType = AE_ST_VERTEX;

  std::deque<aeAttribute> g_Attributes;
  std::deque<aeVarying> g_Varyings;
  std::deque<aeTarget> g_Targets;
  std::deque<aeTexture> g_Textures;
  std::map<aeString, aeUInt32> g_TextureNameToID;

  std::deque<aeVarying> g_VaryingsIn;
  std::deque<aeVarying> g_VaryingsOut;


  int LUAFUNC_Const(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeUInt32 uiVar = s.GetIntParameter(0);
    aeString sVar = s.GetStringParameter(1);
    aeString sVal = s.GetStringParameter(2);

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      g_sCodeBuilt = "const ";
    }
#endif

    g_sCodeBuilt += getVariableReplacement((AE_VAR_TYPES)uiVar) + " " + sVar + " = " + getVariableReplacement((AE_VAR_TYPES)uiVar) + " (" + sVal + ")";

    return (s.ReturnToScript());
  }


  int LUAFUNC_Uniform(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeUInt32 uiVar = s.GetIntParameter(0);
    aeString sVar = s.GetStringParameter(1);

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      g_sCodeBuilt = "uniform " + getVariableReplacement((AE_VAR_TYPES)uiVar) + " " + sVar;
    }
#endif

    return (s.ReturnToScript());
  }


  int LUAFUNC_SetGSInputType(lua_State* state)
  {
    aeLuaWrapper s(state);

    g_GSInputType = (AE_GS_INOUT_TYPE)s.GetIntParameter(0);

    return (s.ReturnToScript());
  }

  int LUAFUNC_SetGSOutputType(lua_State* state)
  {
    aeLuaWrapper s(state);

    g_GSOutputType = (AE_GS_INOUT_TYPE)s.GetIntParameter(0);

    return (s.ReturnToScript());
  }

  int LUAFUNC_SetGSOutputVertices(lua_State* state)
  {
    aeLuaWrapper s(state);

    g_GSOutputVertices = s.GetIntParameter(0);

    return (s.ReturnToScript());
  }

  aeString ReplaceUniforms(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "UNIFORM", sPrev, sPost, sBetween, false))
    {
      //aeLog::Dev ("Found Uniform: (%s)", sBetween.c_str ());

      aeString sExecute = "Uniform (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + g_sCodeBuilt + sPost;
    }

    return (sResult);
  }

  aeString ReplaceConsts(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "CONST", sPrev, sPost, sBetween, false))
    {
      //aeLog::Dev ("Found Uniform: (%s)", sBetween.c_str ());

      aeString sExecute = "Const (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + g_sCodeBuilt + sPost;
    }

    return (sResult);
  }

  int LUAFUNC_Attribute(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeAttribute attr;
    attr.m_Type = (AE_VAR_TYPES)s.GetIntParameter(0);
    attr.m_sSemantic = s.GetStringParameter(1);

    g_Attributes.push_back(attr);

    return (s.ReturnToScript());
  }

  aeString FindAttributes(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "ATTRIBUTE", sPrev, sPost, sBetween, true))
    {
      //aeLog::Dev ("Found Attribute: (%s)", sBetween.c_str ());

      aeString sExecute = "Attribute (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  int LUAFUNC_Varyings(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeVarying var;
    var.m_Type = (AE_VAR_TYPES)s.GetIntParameter(0);
    var.m_sSemantic = s.GetStringParameter(1);
    var.m_Modifier = AE_VM_DEFAULT;

    if (s.GetNumberOfFunctionParameters() >= 3)
      var.m_Modifier = (AE_VARYING_MODIFIERS)s.GetIntParameter(2);

    g_Varyings.push_back(var);

    return (s.ReturnToScript());
  }

  int LUAFUNC_VaryingsIn(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeVarying var;
    var.m_Type = (AE_VAR_TYPES)s.GetIntParameter(0);
    var.m_sSemantic = s.GetStringParameter(1);
    var.m_Modifier = AE_VM_DEFAULT;

    if (s.GetNumberOfFunctionParameters() >= 3)
      var.m_Modifier = (AE_VARYING_MODIFIERS)s.GetIntParameter(2);

    g_VaryingsIn.push_back(var);

    return (s.ReturnToScript());
  }

  int LUAFUNC_VaryingsOut(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeVarying var;
    var.m_Type = (AE_VAR_TYPES)s.GetIntParameter(0);
    var.m_sSemantic = s.GetStringParameter(1);
    var.m_Modifier = AE_VM_DEFAULT;

    if (s.GetNumberOfFunctionParameters() >= 3)
      var.m_Modifier = (AE_VARYING_MODIFIERS)s.GetIntParameter(2);

    g_VaryingsOut.push_back(var);

    return (s.ReturnToScript());
  }


  aeString FindVaryings(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "VARYING", sPrev, sPost, sBetween, true))
    {
      //aeLog::Dev ("Found Varying: (%s)", sBetween.c_str ());

      aeString sExecute = "Varying (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  aeString FindVaryingsIn(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "VAR_IN", sPrev, sPost, sBetween, true))
    {
      //aeLog::Dev ("Found Varying: (%s)", sBetween.c_str ());

      aeString sExecute = "VaryingIn (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  aeString FindVaryingsOut(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "VAR_OUT", sPrev, sPost, sBetween, true))
    {
      //aeLog::Dev ("Found Varying: (%s)", sBetween.c_str ());

      aeString sExecute = "VaryingOut (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  int LUAFUNC_Target(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeTarget var;
    var.m_uiRenderTarget = s.GetIntParameter(0);
    var.m_Type = (AE_VAR_TYPES)s.GetIntParameter(1);
    var.m_sSemantic = s.GetStringParameter(2);

    g_Targets.push_back(var);

    return (s.ReturnToScript());
  }

  aeString FindTargets(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "TARGET", sPrev, sPost, sBetween, true))
    {
      //aeLog::Dev ("Found Target: (%s)", sBetween.c_str ());

      aeString sExecute = "Target (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  int LUAFUNC_Texture(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeTexture var;
    var.m_Type = (AE_TEXTURE_TYPES)s.GetIntParameter(0);
    var.m_sName = s.GetStringParameter(1);

    g_TextureNameToID[var.m_sName] = (aeUInt32)g_Textures.size();
    g_Textures.push_back(var);

    return (s.ReturnToScript());
  }

  aeString FindTextures(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "TEXTURE", sPrev, sPost, sBetween, true))
    {
      //aeLog::Dev ("Found Texture: (%s)", sBetween.c_str ());

      aeString sExecute = "Texture (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  aeString GetGSInputType(const aeString& sCode)
  {
    g_GSInputType = AE_GS_IOT_TRIANGLES;

    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "SET_GS_INPUT_TYPE", sPrev, sPost, sBetween, true))
    {
      aeString sExecute = "SetGSInputType (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  aeString GetGSOutputType(const aeString& sCode)
  {
    g_GSOutputType = AE_GS_IOT_TRIANGLE_STRIP;

    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "SET_GS_OUTPUT_TYPE", sPrev, sPost, sBetween, true))
    {
      aeString sExecute = "SetGSOutputType (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  aeString GetGSOutputVertices(const aeString& sCode)
  {
    g_GSOutputVertices = 0;

    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "SET_GS_OUTPUT_VERTICES", sPrev, sPost, sBetween, true))
    {
      aeString sExecute = "SetGSOutputVertices (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + sPost;
    }

    return (sResult);
  }

  int LUAFUNC_Sample(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeString sTexture = s.GetStringParameter(0);
    aeString sTexCoord = s.GetStringParameter(1);

    char temp[256];

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      const int iTex = g_TextureNameToID[sTexture];

      aeString sType = "_VS";
      if (g_ShaderType == AE_ST_FRAGMENT)
        sType = "_FS";
      else if (g_ShaderType == AE_ST_GEOMETRY)
        sType = "_GS";

      switch (g_Textures[iTex].m_Type)
      {
        case AE_TEX_2D:
          aeStringFunctions::Format(temp, 256, "texture2D (%s%s, %s)", sTexture.c_str(), sType.c_str(), sTexCoord.c_str());
          break;
        case AE_TEX_3D:
          aeStringFunctions::Format(temp, 256, "texture3D (%s%s, %s)", sTexture.c_str(), sType.c_str(), sTexCoord.c_str());
          break;
        case AE_TEX_CUBE:
          aeStringFunctions::Format(temp, 256, "textureCube (%s%s, %s)", sTexture.c_str(), sType.c_str(), sTexCoord.c_str());
          break;
      }
      g_sCodeBuilt = temp;
    }
#endif

    return (s.ReturnToScript());
  }

  aeString ReplaceSamples(const aeString& sCode)
  {
    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "SAMPLE", sPrev, sPost, sBetween, false))
    {
      //aeLog::Dev ("Found Sample: (%s)", sBetween.c_str ());

      aeString sExecute = "Sample (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + g_sCodeBuilt + sPost;
    }

    return (sResult);
  }

  int LUAFUNC_SamplerPair(lua_State* state)
  {
    aeLuaWrapper s(state);

    aeString sVar = s.GetStringParameter(0);

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      switch (g_ShaderType)
      {
        case AE_ST_VERTEX:
          g_sCodeBuilt = sVar + "_VS";
          break;
        case AE_ST_FRAGMENT:
          g_sCodeBuilt = sVar + "_FS";
          break;
        case AE_ST_GEOMETRY:
          g_sCodeBuilt = sVar + "_GS";
          break;
      }
    }
#endif

    return (s.ReturnToScript());
  }

  aeString ReplaceSamplerPairs(const aeString& sCode)
  {
    //aeLog::Log ("Replacing Sampler Pairs.");
    //aeLog::Flush ();

    aeString sResult, sPrev, sPost, sBetween;
    sResult = sCode;

    while (FindPortFunction(sResult, "TEX2SL", sPrev, sPost, sBetween, false))
    {
      //aeLog::Dev ("Found SamplerPair: (%s)", sBetween.c_str ());
      //aeLog::Flush ();

      aeString sExecute = "SamplerPair (" + sBetween;
      g_PortScript.ExecuteString(sExecute);

      sResult = sPrev + g_sCodeBuilt + sPost;
    }

    return (sResult);
  }

  aeString CreateShaderMainVS(const aeString& sCode0)
  {
    aeString sCode = sCode0;
    aeString sMain = "";


    if (!g_Textures.empty())
    {
      for (aeUInt32 ui = 0; ui < g_Textures.size(); ++ui)
      {
        char temp[256];

#ifdef AE_RENDERAPI_OPENGL
        if (g_RenderAPI == AE_RA_OPENGL)
        {
          aeStringFunctions::Format(temp, 256, "%s %s_VS;\n", getTextureTypeName(g_Textures[ui].m_Type).c_str(), g_Textures[ui].m_sName.c_str());
          sMain += temp;
        }
#endif
      }
    }

    //if (!g_Attributes.empty ())
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        for (aeUInt32 ui = 0; ui < g_Attributes.size(); ++ui)
        {
          char temp[256];
          if (g_Attributes[ui].m_Type == AE_INSTANCE_ID)
            continue;
          if (g_Attributes[ui].m_Type == AE_VERTEX_POSITION)
            continue;

          aeStringFunctions::Format(temp, 256, "attribute %s %s;\n", getVariableReplacement(g_Attributes[ui].m_Type).c_str(), g_Attributes[ui].m_sSemantic.c_str());

          sMain += temp;
        }
      }
#endif
    }

    //if (!g_Varyings.empty ())
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        for (aeUInt32 ui = 0; ui < g_Varyings.size(); ++ui)
        {
          char temp[256];
          if (g_Varyings[ui].m_Type == AE_VERTEX_POSITION)
            continue;

          if (g_Varyings[ui].m_Modifier == AE_VM_DEFAULT)
            aeStringFunctions::Format(temp, 256, "varying %s %s;\n", getVariableReplacement(g_Varyings[ui].m_Type).c_str(), g_Varyings[ui].m_sSemantic.c_str());
          else if (g_Varyings[ui].m_Modifier == AE_VM_FLAT)
            aeStringFunctions::Format(temp, 256, "flat varying %s %s;\n", getVariableReplacement(g_Varyings[ui].m_Type).c_str(), g_Varyings[ui].m_sSemantic.c_str());

          sMain += temp;
        }
      }
#endif
    }


    if (aeStringFunctions::FindFirstStringPos(sCode.c_str(), "SHADER_HEADER") != -1)
    {
      sCode = ReplaceKeyword(sCode, "SHADER_HEADER", sMain);
      sMain = "";
    }

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sMain += "void main ()\n{\n\n";
    }
#endif

    aeString sResult = ReplaceKeyword(sCode, "SHADER_BEGIN", sMain);

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sMain = "\n}\n\n";
    }
#endif

    sResult = ReplaceKeyword(sResult, "SHADER_END", sMain);

    return (sResult);
  }


  aeString CreateShaderMainFS(const aeString& sCode0)
  {
    aeString sCode = sCode0;
    aeString sMain = "";

    if (!g_Textures.empty())
    {
      for (aeUInt32 ui = 0; ui < g_Textures.size(); ++ui)
      {
        char temp[256];

#ifdef AE_RENDERAPI_OPENGL
        if (g_RenderAPI == AE_RA_OPENGL)
        {
          aeStringFunctions::Format(temp, 256, "%s %s_FS;\n", getTextureTypeName(g_Textures[ui].m_Type).c_str(), g_Textures[ui].m_sName.c_str());
          sMain += temp;
        }
#endif
      }
    }

    //if (!g_Varyings.empty ())
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        for (aeUInt32 ui = 0; ui < g_Varyings.size(); ++ui)
        {
          char temp[256];
          if (g_Varyings[ui].m_Type == AE_FRAGMENT_POSITION)
            continue;

          if (g_Varyings[ui].m_Modifier == AE_VM_DEFAULT)
            aeStringFunctions::Format(temp, 256, "varying %s %s;\n", getVariableReplacement(g_Varyings[ui].m_Type).c_str(), g_Varyings[ui].m_sSemantic.c_str());
          else if (g_Varyings[ui].m_Modifier == AE_VM_FLAT)
            aeStringFunctions::Format(temp, 256, "flat varying %s %s;\n", getVariableReplacement(g_Varyings[ui].m_Type).c_str(), g_Varyings[ui].m_sSemantic.c_str());

          sMain += temp;
        }
      }
#endif
    }

    //if (!g_Targets.empty ())
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        // targets don't need to be declared
      }
#endif
    }


    if (aeStringFunctions::FindFirstStringPos(sCode.c_str(), "SHADER_HEADER") != -1)
    {
      sCode = ReplaceKeyword(sCode, "SHADER_HEADER", sMain);
      sMain = "";
    }

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sMain += "void main ()\n{\n\n";
    }
#endif

    aeString sResult = ReplaceKeyword(sCode, "SHADER_BEGIN", sMain);

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sMain = "\n}\n\n";
    }
#endif

    sResult = ReplaceKeyword(sResult, "SHADER_END", sMain);

    return (sResult);
  }



  aeString CreateShaderMainGS(const aeString& sCode0)
  {
    aeString sCode = sCode0;
    aeString sMain = "";


    if (!g_Textures.empty())
    {
      for (aeUInt32 ui = 0; ui < g_Textures.size(); ++ui)
      {
        char temp[256];

#ifdef AE_RENDERAPI_OPENGL
        if (g_RenderAPI == AE_RA_OPENGL)
        {
          aeStringFunctions::Format(temp, 256, "%s %s_VS;\n", getTextureTypeName(g_Textures[ui].m_Type).c_str(), g_Textures[ui].m_sName.c_str());
          sMain += temp;
        }
#endif
      }
    }

    // input
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        for (aeUInt32 ui = 0; ui < g_VaryingsIn.size(); ++ui)
        {
          char temp[256];
          if (g_VaryingsIn[ui].m_Type == AE_VERTEX_POSITION)
            continue;

          if (g_VaryingsIn[ui].m_Modifier == AE_VM_DEFAULT)
            aeStringFunctions::Format(temp, 256, "varying in %s %s[];\n", getVariableReplacement(g_VaryingsIn[ui].m_Type).c_str(), g_VaryingsIn[ui].m_sSemantic.c_str());
          else if (g_VaryingsIn[ui].m_Modifier == AE_VM_FLAT)
            aeStringFunctions::Format(temp, 256, "flat varying in %s %s[];\n", getVariableReplacement(g_VaryingsIn[ui].m_Type).c_str(), g_VaryingsIn[ui].m_sSemantic.c_str());

          sMain += temp;
        }
      }
#endif
    }


    // output
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        for (aeUInt32 ui = 0; ui < g_VaryingsOut.size(); ++ui)
        {
          char temp[256];
          if (g_VaryingsOut[ui].m_Type == AE_VERTEX_POSITION)
            continue;

          if (g_VaryingsOut[ui].m_Modifier == AE_VM_DEFAULT)
            aeStringFunctions::Format(temp, 256, "varying out %s %s;\n", getVariableReplacement(g_VaryingsOut[ui].m_Type).c_str(), g_VaryingsOut[ui].m_sSemantic.c_str());
          else if (g_VaryingsOut[ui].m_Modifier == AE_VM_FLAT)
            aeStringFunctions::Format(temp, 256, "flat varying out %s %s;\n", getVariableReplacement(g_VaryingsOut[ui].m_Type).c_str(), g_VaryingsOut[ui].m_sSemantic.c_str());

          sMain += temp;
        }
      }
#endif
    }


    if (aeStringFunctions::FindFirstStringPos(sCode.c_str(), "SHADER_HEADER") != -1)
    {
      sCode = ReplaceKeyword(sCode, "SHADER_HEADER", sMain);
      sMain = "";
    }

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sMain += "void main ()\n{\n\n";
    }
#endif

    aeString sResult = ReplaceKeyword(sCode, "SHADER_BEGIN", sMain);

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sMain = "\n}\n\n";
    }
#endif

    sResult = ReplaceKeyword(sResult, "SHADER_END", sMain);

    return (sResult);
  }



  aeString ReplaceVaryings(const aeString& sCode, bool bInput)
  {
    aeString sResult = sCode;

    for (aeUInt32 ui = 0; ui < g_Varyings.size(); ++ui)
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        if ((!bInput) && (g_Varyings[ui].m_Type == AE_VERTEX_POSITION))
          sResult = ReplaceKeyword(sResult, g_Varyings[ui].m_sSemantic, "gl_Position");
        if ((bInput) && (g_Varyings[ui].m_Type == AE_FRAGMENT_POSITION))
          sResult = ReplaceKeyword(sResult, g_Varyings[ui].m_sSemantic, "gl_FragCoord");
      }
#endif
    }

    return (sResult);
  }

  aeString ReplaceVaryingsIn(const aeString& sCode)
  {
    aeString sResult = sCode;

    for (aeUInt32 ui = 0; ui < g_VaryingsIn.size(); ++ui)
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        if (g_VaryingsIn[ui].m_Type == AE_VERTEX_POSITION)
          sResult = ReplaceKeyword(sResult, g_VaryingsIn[ui].m_sSemantic, "gl_PositionIn");
        if (g_VaryingsIn[ui].m_Type == AE_FRAGMENT_POSITION)
          sResult = ReplaceKeyword(sResult, g_VaryingsIn[ui].m_sSemantic, "gl_FragCoord");
      }
#endif
    }

    return (sResult);
  }


  aeString ReplaceVaryingsOut(const aeString& sCode)
  {
    aeString sResult = sCode;

    for (aeUInt32 ui = 0; ui < g_VaryingsOut.size(); ++ui)
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        if (g_VaryingsOut[ui].m_Type == AE_VERTEX_POSITION)
          sResult = ReplaceKeyword(sResult, g_VaryingsOut[ui].m_sSemantic, "gl_Position");
      }
#endif
    }

    return (sResult);
  }

  aeString ReplaceAttributes(const aeString& sCode)
  {
    aeString sResult = sCode;

    for (aeUInt32 ui = 0; ui < g_Attributes.size(); ++ui)
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        if (g_Attributes[ui].m_Type == AE_VERTEX_POSITION)
          sResult = ReplaceKeyword(sResult, g_Attributes[ui].m_sSemantic, "gl_Vertex.xyz");
        else if (g_Attributes[ui].m_Type == AE_INSTANCE_ID)
          sResult = ReplaceKeyword(sResult, g_Attributes[ui].m_sSemantic, "gl_InstanceID");
      }
#endif
    }

    return (sResult);
  }

  aeString ReplaceTargets(const aeString& sCode)
  {
    aeString sResult = sCode;

    char temp[256];

    for (aeUInt32 ui = 0; ui < g_Targets.size(); ++ui)
    {
#ifdef AE_RENDERAPI_OPENGL
      if (g_RenderAPI == AE_RA_OPENGL)
      {
        switch (g_Targets[ui].m_Type)
        {
          case AE_FLOAT:
            aeStringFunctions::Format(temp, 256, "gl_FragData[%d].r", g_Targets[ui].m_uiRenderTarget);
            break;
          case AE_FLOAT2:
            aeStringFunctions::Format(temp, 256, "gl_FragData[%d].rg", g_Targets[ui].m_uiRenderTarget);
            break;
          case AE_FLOAT3:
            aeStringFunctions::Format(temp, 256, "gl_FragData[%d].rgb", g_Targets[ui].m_uiRenderTarget);
            break;
          case AE_FLOAT4:
            aeStringFunctions::Format(temp, 256, "gl_FragData[%d]", g_Targets[ui].m_uiRenderTarget);
            break;
        }
      }
#endif

      sResult = ReplaceKeyword(sResult, g_Targets[ui].m_sSemantic, temp);
    }

    return (sResult);
  }


  void InitPortScript(void)
  {
    g_PortScript.SetVariable("VERTEX_POS", AE_VERTEX_POSITION);
    g_PortScript.SetVariable("FRAG_POS", AE_FRAGMENT_POSITION);
    g_PortScript.SetVariable("FLOAT", AE_FLOAT);
    g_PortScript.SetVariable("FLOAT2", AE_FLOAT2);
    g_PortScript.SetVariable("FLOAT3", AE_FLOAT3);
    g_PortScript.SetVariable("FLOAT4", AE_FLOAT4);
    g_PortScript.SetVariable("FLOAT2X2", AE_FLOAT2X2);
    g_PortScript.SetVariable("FLOAT3X3", AE_FLOAT3X3);
    g_PortScript.SetVariable("FLOAT4X4", AE_FLOAT4X4);
    g_PortScript.SetVariable("INSTANCE_ID", AE_INSTANCE_ID);
    g_PortScript.SetVariable("INT", AE_INT);
    g_PortScript.SetVariable("INT2", AE_INT2);
    g_PortScript.SetVariable("INT3", AE_INT3);
    g_PortScript.SetVariable("INT4", AE_INT4);

    g_PortScript.SetVariable("DEFAULT", AE_VM_DEFAULT);
    g_PortScript.SetVariable("FLAT", AE_VM_FLAT);

    g_PortScript.SetVariable("TYPE_2D", AE_TEX_2D);
    g_PortScript.SetVariable("TYPE_3D", AE_TEX_3D);
    g_PortScript.SetVariable("TYPE_CUBE", AE_TEX_CUBE);

    g_PortScript.RegisterCFunction("Uniform", LUAFUNC_Uniform);
    g_PortScript.RegisterCFunction("Const", LUAFUNC_Const);
    g_PortScript.RegisterCFunction("Attribute", LUAFUNC_Attribute);
    g_PortScript.RegisterCFunction("Varying", LUAFUNC_Varyings);
    g_PortScript.RegisterCFunction("VaryingIn", LUAFUNC_VaryingsIn);
    g_PortScript.RegisterCFunction("VaryingOut", LUAFUNC_VaryingsOut);
    g_PortScript.RegisterCFunction("Target", LUAFUNC_Target);
    g_PortScript.RegisterCFunction("Texture", LUAFUNC_Texture);
    g_PortScript.RegisterCFunction("Sample", LUAFUNC_Sample);
    g_PortScript.RegisterCFunction("SamplerPair", LUAFUNC_SamplerPair);


    g_PortScript.SetVariable("POINTS", AE_GS_IOT_POINTS);
    g_PortScript.SetVariable("LINES", AE_GS_IOT_LINES);
    g_PortScript.SetVariable("TRIANGLES", AE_GS_IOT_TRIANGLES);
    g_PortScript.SetVariable("LINE_STRIP", AE_GS_IOT_LINE_STRIP);
    g_PortScript.SetVariable("TRIANGLE_STRIP", AE_GS_IOT_TRIANGLE_STRIP);

    g_PortScript.RegisterCFunction("SetGSInputType", LUAFUNC_SetGSInputType);
    g_PortScript.RegisterCFunction("SetGSOutputType", LUAFUNC_SetGSOutputType);
    g_PortScript.RegisterCFunction("SetGSOutputVertices", LUAFUNC_SetGSOutputVertices);
  }

  aeString ReplaceGeometryShaderStuff(const aeString& sCode)
  {
    aeString sResult = sCode;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      sResult = ReplaceKeyword(sResult, "EMIT_VERTEX", "EmitVertex ();");
      sResult = ReplaceKeyword(sResult, "NEXT_PRIMITIVE", "EndPrimitive ();");
    }
#endif

    return (sResult);
  }

  aeString ReplaceGlobalVariablePlaceholders(const aeString& sCode, AE_SHADER_TYPE ShaderType)
  {
    aeString sResult = sCode;
    aeString sGlobalParam, sGlobalPassThrough;

    if (g_RenderAPI == AE_RA_D3D11)
    {
      switch (ShaderType)
      {
        case AE_ST_VERTEX:
        case AE_ST_FRAGMENT:
          sGlobalParam = " , inout SHADER_OUTPUT output, inout SHADER_INPUT input ";
          sGlobalPassThrough = " , output, input ";
          break;
        case AE_ST_GEOMETRY:
        {
          sGlobalParam = " , inout SHADER_OUTPUT output, inout SHADER_INPUT input";

          switch (g_GSInputType)
          {
            case AE_GS_IOT_POINTS:
              break;
            case AE_GS_IOT_LINES:
              sGlobalParam += "[2]";
              break;
            case AE_GS_IOT_TRIANGLES:
              sGlobalParam += "[3]";
              break;
          }

          switch (g_GSOutputType)
          {
            case AE_GS_IOT_POINTS:
              sGlobalParam += ", inout PointStream<SHADER_OUTPUT> OutStream ";
              break;
            case AE_GS_IOT_LINE_STRIP:
              sGlobalParam += ", inout LineStream<SHADER_OUTPUT> OutStream ";
              break;
            case AE_GS_IOT_TRIANGLE_STRIP:
              sGlobalParam += ", inout TriangleStream<SHADER_OUTPUT> OutStream ";
              break;
          }

          sGlobalPassThrough = " , output, input, OutStream ";
        }
        break;
      }
    }

    sResult = ReplaceKeyword(sResult, "PARAM_GLOBALS", sGlobalParam);
    sResult = ReplaceKeyword(sResult, "PASS_GLOBALS", sGlobalPassThrough);

    return (sResult);
  }

  aeString PortShader(const aeString& sCode, AE_SHADER_TYPE ShaderType)
  {
    g_ShaderType = ShaderType;

    g_Attributes.clear();
    g_Varyings.clear();
    g_VaryingsIn.clear();
    g_VaryingsOut.clear();
    g_Targets.clear();
    g_Textures.clear();
    g_TextureNameToID.clear();

    InitPortScript();

    aeString sResult = sCode;

    sResult = ReplaceKeyword2(sResult, "FLOAT", getVariableReplacement(AE_FLOAT));
    sResult = ReplaceKeyword2(sResult, "FLOAT2", getVariableReplacement(AE_FLOAT2));
    sResult = ReplaceKeyword2(sResult, "FLOAT3", getVariableReplacement(AE_FLOAT3));
    sResult = ReplaceKeyword2(sResult, "FLOAT4", getVariableReplacement(AE_FLOAT4));
    sResult = ReplaceKeyword2(sResult, "INT", getVariableReplacement(AE_INT));
    sResult = ReplaceKeyword2(sResult, "INT2", getVariableReplacement(AE_INT2));
    sResult = ReplaceKeyword2(sResult, "INT3", getVariableReplacement(AE_INT3));
    sResult = ReplaceKeyword2(sResult, "INT4", getVariableReplacement(AE_INT4));

    sResult = ReplaceKeyword2(sResult, "FLOAT2X2", getVariableReplacement(AE_FLOAT2X2));
    sResult = ReplaceKeyword2(sResult, "FLOAT3X3", getVariableReplacement(AE_FLOAT3X3));
    sResult = ReplaceKeyword2(sResult, "FLOAT4X4", getVariableReplacement(AE_FLOAT4X4));

    sResult = ReplaceUniforms(sResult);
    sResult = ReplaceConsts(sResult);
    sResult = FindAttributes(sResult);
    sResult = FindVaryings(sResult);
    sResult = FindVaryingsIn(sResult);
    sResult = FindVaryingsOut(sResult);
    sResult = FindTargets(sResult);
    sResult = FindTextures(sResult);

    sResult = ReplaceVaryings(sResult, ShaderType != AE_ST_VERTEX);
    sResult = ReplaceVaryingsIn(sResult);
    sResult = ReplaceVaryingsOut(sResult);

    sResult = ReplaceSamples(sResult);
    sResult = ReplaceSamplerPairs(sResult);

    switch (ShaderType)
    {
      case AE_ST_VERTEX:
        sResult = ReplaceAttributes(sResult);
        sResult = ReplaceGlobalVariablePlaceholders(sResult, AE_ST_VERTEX);
        sResult = CreateShaderMainVS(sResult);
        break;
      case AE_ST_FRAGMENT:
        sResult = ReplaceTargets(sResult);
        sResult = ReplaceGlobalVariablePlaceholders(sResult, AE_ST_FRAGMENT);
        sResult = CreateShaderMainFS(sResult);
        break;
      case AE_ST_GEOMETRY:
        sResult = GetGSInputType(sResult);
        sResult = GetGSOutputType(sResult);
        sResult = GetGSOutputVertices(sResult);
        sResult = ReplaceGeometryShaderStuff(sResult);
        sResult = ReplaceGlobalVariablePlaceholders(sResult, AE_ST_GEOMETRY);
        sResult = CreateShaderMainGS(sResult);
        break;
    }

    return (sResult);
  }


} // namespace AE_NS_GRAPHICS
