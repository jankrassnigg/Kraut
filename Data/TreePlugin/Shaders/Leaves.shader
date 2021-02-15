[PORTABLE VERTEX SHADER]

$include <Shaders/Main_vs.include>

ATTRIBUTE (VERTEX_POS, "attr_Position");
ATTRIBUTE (FLOAT3, "attr_TexCoords");
ATTRIBUTE (FLOAT4, "attr_Normal");
ATTRIBUTE (FLOAT4, "attr_Tangent");
ATTRIBUTE (FLOAT4, "attr_BiTangent");
ATTRIBUTE (FLOAT4, "attr_PickingID");
ATTRIBUTE (FLOAT4, "attr_PickingSubID");

VARYING (VERTEX_POS, "var_Position");
VARYING (FLOAT4, "var_TexCoords");
VARYING (FLOAT3, "var_Normal");
VARYING (FLOAT4, "var_AmbientOccXY");
VARYING (FLOAT2, "var_AmbientOccZ");
VARYING (FLOAT3, "var_PosWS");
VARYING (FLOAT3, "var_Tangent");
VARYING (FLOAT3, "var_BiTangent");
VARYING (FLOAT4, "var_PickingID", FLAT);
VARYING (FLOAT4, "var_PickingSubID", FLAT);
VARYING (FLOAT3, "var_NormalDir");
VARYING (FLOAT,  "var_VariationInfluence", FLAT);

UNIFORM (FLOAT3, "unif_CameraPosition");
UNIFORM (FLOAT3, "unif_CameraForwards");
UNIFORM (FLOAT3, "unif_CameraRight");
UNIFORM (FLOAT3, "unif_CameraUp");
UNIFORM (FLOAT3, "unif_TreeCenter");

SHADER_BEGIN
{
  FLOAT2 span = attr_Tangent.zw * 2.0 - 1.0;

  FLOAT3 vNewPos = attr_Position.xyz;
  vNewPos += (span.x * unif_CameraRight + span.y * unif_CameraUp) * attr_TexCoords.z;
  
  var_NormalDir = vNewPos - unif_TreeCenter;
  var_AmbientOccXY = attr_Normal.xyzw;
  var_AmbientOccZ  = attr_Tangent.xy;

	var_Position = transform (vNewPos);
	var_TexCoords = FLOAT4 (attr_TexCoords.xy, 0, 1);
  var_Normal = -unif_CameraForwards;
  var_PosWS = matrix_mult (ra_ObjectToWorldMatrix, FLOAT4 (vNewPos.xyz, 1.0)).xyz;
  var_Tangent = unif_CameraRight;
  var_BiTangent = unif_CameraUp;
  var_PickingID = attr_PickingID;
  var_PickingSubID = attr_PickingSubID;
  var_VariationInfluence = attr_BiTangent.w;

}
SHADER_END


[PORTABLE FRAGMENT SHADER]

$include <Shaders/Main_fs.include>
$include <Shaders/Gamma.include>

VARYING (FRAG_POS, "var_Position");
VARYING (FLOAT4, "var_TexCoords");
VARYING (FLOAT3, "var_Normal");
VARYING (FLOAT4, "var_AmbientOccXY");
VARYING (FLOAT2, "var_AmbientOccZ");
VARYING (FLOAT3, "var_PosWS");
VARYING (FLOAT3, "var_Tangent");
VARYING (FLOAT3, "var_BiTangent");
VARYING (FLOAT4, "var_PickingID", FLAT);
VARYING (FLOAT4, "var_PickingSubID", FLAT);
VARYING (FLOAT3, "var_NormalDir");
VARYING (FLOAT,  "var_VariationInfluence", FLAT);

UNIFORM (FLOAT3, "unif_LightPos");
UNIFORM (FLOAT4, "unif_Selected");
UNIFORM (FLOAT4, "unif_SelectedSubID");

TARGET (0, FLOAT4, "rt_Color");
TARGET (1, FLOAT4, "rt_Normal");

$if (EXPORT_LEAFCARD)
	TARGET (2, FLOAT4, "rt_Roughness");
$endif

TEXTURE (TYPE_2D, "tex_Diffuse");
TEXTURE (TYPE_2D, "tex_Normal");
TEXTURE (TYPE_2D, "tex_Noise");

$if (EXPORT_LEAFCARD)
	TEXTURE (TYPE_2D, "tex_Roughness");
$endif

UNIFORM (FLOAT3, "unif_AmbientLow");
UNIFORM (FLOAT3, "unif_AmbientHigh");
UNIFORM (FLOAT4, "unif_LeafColor");

UNIFORM (FLOAT3, "unif_CameraPosition");
UNIFORM (FLOAT3, "unif_CameraForwards");
UNIFORM (FLOAT3, "unif_CameraRight");
UNIFORM (FLOAT3, "unif_CameraUp");

UNIFORM (FLOAT, "unif_CrossFadeInDist");
UNIFORM (FLOAT, "unif_CrossFadeInRange");
UNIFORM (FLOAT, "unif_CrossFadeOutDist");
UNIFORM (FLOAT, "unif_CrossFadeOutRange");

UNIFORM (FLOAT3, "unif_TreeCenter");

$if (EXPORT_LEAFCARD)
  UNIFORM (FLOAT3X3, "unif_InverseTBN");
$endif

SHADER_BEGIN
{
  $if (CROSSFADE)
    FLOAT fDistance = length (unif_TreeCenter - unif_CameraPosition);
    //FLOAT fNoise = SAMPLE ("tex_Noise", "var_TexCoords.xy / var_TexCoords.w").x;
    FLOAT fNoise = SAMPLE ("tex_Noise", "var_Position.xy / 512.0").x;
  
    FLOAT fDissolveOut = (fDistance - unif_CrossFadeOutDist) / unif_CrossFadeOutRange;
    FLOAT fDissolveIn  = (fDistance - unif_CrossFadeInDist) / unif_CrossFadeInRange;
  
    if (fDissolveOut > fNoise)
      discard;
    if (fDissolveIn <= fNoise)
      discard;
  $endif

  $if (WIREFRAME)
    rt_Color = FLOAT4 (1, 1, 0, 1);
    return;
  $endif
    
  FLOAT3 vFaceNormal    = normalize (var_Normal);
  FLOAT3 vTangent   = normalize (var_Tangent);
  FLOAT3 vBiTangent = -normalize (var_BiTangent);

  FLOAT3X3 mTBN = FLOAT3X3 (vTangent, vBiTangent, vFaceNormal);

  FLOAT4 vTexNormal = SAMPLE ("tex_Normal", "var_TexCoords.xy");
  FLOAT3 vNormal = normalize (matrix_mult (mTBN, vTexNormal.xyz * 2.0 - 1.0));
  
  FLOAT3 vNormalDir = normalize (var_NormalDir);
  vNormal = normalize (vNormal + vNormalDir * 0.8);

	FLOAT4 vTexDiffuse = SAMPLE ("tex_Diffuse", "var_TexCoords.xy");
  vTexDiffuse.rgb = GetGammaCorrected (vTexDiffuse.rgb);
  vTexDiffuse.rgb = mix(vTexDiffuse.rgb, GetGammaCorrected (unif_LeafColor.rgb), unif_LeafColor.a * var_VariationInfluence);

  if (vTexDiffuse.a < 0.1)
    discard;
  
  $if (PICKING)
    rt_Color = var_PickingID;
    rt_Normal = var_PickingSubID;
    return;
  $endif
    
  float fPickingColorScale = 1.0;
  float fMinColor = 0.0;

  if ((var_PickingID == unif_Selected) && (var_PickingSubID == unif_SelectedSubID))
  {
    //if ((var_PickingID != 0) && (var_PickingSubID != 0))
    {
      fPickingColorScale = 2.0;
      fMinColor = 0.2;
    }
  }

  FLOAT3 vFragToLight = normalize (unif_LightPos - var_PosWS);
  FLOAT fLight = max (0, dot (vFragToLight, vNormal));
  
  $if (EXPORT_LEAFCARD)
    fLight = 1.0;
    vNormal.xyz = matrix_mult (unif_InverseTBN, vNormal.xyz);
  $endif
  
  FLOAT3 vLight;
  vLight = FLOAT3 (fLight);
  
  FLOAT3 vSphereNormal = normalize (vNormal);
  
  FLOAT3 vAmbientLight = FLOAT3 (0, 0, 0);

  // Compute Ambient Light
  $ifnot (EXPORT_LEAFCARD)
  
    FLOAT3 vAmbientNormal = normalize (vSphereNormal.xyz);
    float fAmbientLookup = dot (vAmbientNormal, FLOAT3 (0, 1, 0));
    fAmbientLookup *= 0.5;
    fAmbientLookup += 0.5;
    FLOAT3 vAmbientColor = mix (GetGammaCorrected (unif_AmbientLow), GetGammaCorrected (unif_AmbientHigh), fAmbientLookup);
    
    float fAmbientStrength = 1.0;
    
    vAmbientLight = vAmbientColor * fAmbientStrength;
    vLight = max (vAmbientLight, vLight);
  $endif
  
  // compute per pixel AO
  float var_AmbientOcc = 1.0;
  {
    float px = max (0, dot (vNormal, vec3 ( 1, 0, 0))); px *= px; px *= var_AmbientOccXY.x;
    float nx = max (0, dot (vNormal, vec3 (-1, 0, 0))); nx *= nx; nx *= var_AmbientOccXY.y;
    float py = max (0, dot (vNormal, vec3 ( 0, 1, 0))); py *= py; py *= var_AmbientOccXY.z;
    float ny = max (0, dot (vNormal, vec3 ( 0,-1, 0))); ny *= ny; ny *= var_AmbientOccXY.w;
    float pz = max (0, dot (vNormal, vec3 ( 0, 0, 1))); pz *= pz; pz *= var_AmbientOccZ.x;
    float nz = max (0, dot (vNormal, vec3 ( 0, 0,-1))); nz *= nz; nz *= var_AmbientOccZ.y;
    
    var_AmbientOcc = px + nx + py + ny + pz + nz;
  }
  
  
  
  // output 

  FLOAT4 out_Color;
  
  rt_Normal.rgb = vNormal.rgb * 0.5 + 0.5;
  rt_Normal.a = vTexNormal.a;

  out_Color.rgb = var_AmbientOcc * vTexDiffuse.rgb * vLight.rgb;
  out_Color.a = 1;//vTexDiffuse.a;

  $if (EXPORT_LEAFCARD)
	rt_Roughness.rgba = texture2DProj (tex_Roughness_FS, var_TexCoords).rgba;
  $endif
  
  $if (OUTPUT_DIFFUSE_ONLY)
    out_Color.rgb = var_AmbientOcc * vTexDiffuse.rgb;
  $endif
  
  $if (OUTPUT_LIGHT_ONLY)
    out_Color.rgb = var_AmbientOcc * vLight.rgb;
  $endif
  
  $if (OUTPUT_SKYLIGHT_ONLY)
    out_Color.rgb = var_AmbientOcc * vAmbientLight.rgb;
  $endif
  
  $if (OUTPUT_AMBIENT_ONLY)
    out_Color.rgb = FLOAT3 (var_AmbientOcc);
  $endif
  
  $if (OUTPUT_NORMALS_ONLY)
    out_Color.rgb = GetGammaCorrected (vNormal.xyz * 0.5 + 0.5);
  $endif

  $if (NON_EDITABLE)
    out_Color.xyz = FLOAT3 (0.7, 0.7, 0.7) * vLight;
  $endif

  out_Color.rgb = max (FLOAT3 (fMinColor), out_Color.rgb * fPickingColorScale);
  
  rt_Color = ApplyGamma (out_Color);
}
SHADER_END




