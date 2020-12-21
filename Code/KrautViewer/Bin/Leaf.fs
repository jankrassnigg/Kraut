#version 140

uniform vec4 unif_CameraPos;
uniform vec4 unif_CameraDir;
uniform vec4 unif_CameraRight;
uniform vec4 unif_CameraUp;
uniform vec4 unif_LightPosition;
uniform vec4 unif_VariationColor;
uniform vec4 unif_RenderMode;

uniform sampler2D tex_Diffuse;
uniform sampler2D tex_NormalMap;
uniform sampler2D tex_Ambient;

smooth in vec3 var_Position;
smooth in vec3 var_TexCoord;
smooth in vec3 var_Normal;
smooth in vec3 var_Tangent;
smooth in vec4 var_Color;
flat   in vec3 var_TreeCenter;

out vec4 var_FragColor;

void main(void)
{
  vec3 vNormal    = -unif_CameraDir.xyz;
  vec3 vTangent   = unif_CameraRight.xyz;
  vec3 vBiTangent = unif_CameraUp.xyz;
  
  mat3 mTBN = mat3 (vTangent, vBiTangent, vNormal);

  vec4 vColor     = texture2D (tex_Diffuse,   var_TexCoord.xy);
  vec4 vNormalMap = texture2D (tex_NormalMap, var_TexCoord.xy);
  
  if (vColor.a < 0.1)
    discard;

  vColor.rgb = mix (vColor.rgb, unif_VariationColor.rgb, var_Color.a * unif_VariationColor.a);
  vColor.rgb *= var_Color.rgb;
    
  vec3 vDir = normalize (var_Position - var_TreeCenter.xyz);

  vec3 vNormalTBN = normalize (mTBN * (vNormalMap.xyz * 2.0 - 1.0));  
  vec3 vFinalNormal = normalize (vNormalTBN + vDir * 0.8); // the factor, how strong you weigh which normal, might need to be tweaked
  
  float fDistToLight= length (unif_LightPosition.xyz - var_Position);
  vec3 vDirToLight = normalize (unif_LightPosition.xyz - var_Position);
  float fLight = max (0.2, dot (vDirToLight, vFinalNormal));
  fLight *= max (0.0, 1.0 - fDistToLight / 25.0);
  
  vec3 vLookupDir = normalize (1 * vDir + vFinalNormal * 2.8);
  float fLut = dot (vec3 (0, 1, 0), vLookupDir) * 0.5 + 0.5;
  
  vec3 vAmbient = texture2D (tex_Ambient, vec2 (0.5, fLut)).rgb * 0.5;
  
  
  var_FragColor = vColor * (vec4 (vAmbient, 0.0) + vec4 (fLight));
  
  if (unif_RenderMode.x == 1.0) // Diffuse Color only
    var_FragColor.rgb = vColor.rgb;
  else
  if (unif_RenderMode.x == 2.0) // Pixel Normal
    var_FragColor.rgb = vFinalNormal.rgb * 0.5 + 0.5;
  else
  if (unif_RenderMode.x == 3.0) // Face Normal
    var_FragColor.rgb = vNormal.rgb * 0.5 + 0.5;
  else
  if (unif_RenderMode.x == 4.0) // Tangent
    var_FragColor.rgb = vTangent.rgb * 0.5 + 0.5;
  else
  if (unif_RenderMode.x == 5.0) // BiTangent
    var_FragColor.rgb = vBiTangent.rgb * 0.5 + 0.5;
  else
  if (unif_RenderMode.x == 6.0) // Diffuse Light only
    var_FragColor.rgb = vec3 (fLight);
  else
  if (unif_RenderMode.x == 7.0) // Ambient Light only
    var_FragColor.rgb = vAmbient.rgb;
  else
  if (unif_RenderMode.x == 8.0) // Material Type
    var_FragColor.rgb = vec3(1,1,0);
  else
  if (unif_RenderMode.x == 9.0) // Tree Center Direction
    var_FragColor.rgb = vDir * 0.5 + 0.5;
}