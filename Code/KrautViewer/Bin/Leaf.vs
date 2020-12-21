#version 140

uniform mat4 kvObjectToWorld;
uniform mat4 kvWorldToCamera;
uniform mat4 kvCameraToProjection;
uniform vec4 unif_TreeCenter;

uniform vec4 unif_CameraDir;
uniform vec4 unif_CameraRight;
uniform vec4 unif_CameraUp;

in vec3 attr_Position;
in vec3 attr_TexCoord;
in vec4 attr_Color;

smooth out vec3 var_Position;
smooth out vec3 var_TexCoord;
smooth out vec4 var_Color;
flat   out vec3 var_TreeCenter;

void main()
{
  vec2 span = attr_TexCoord.xy * 2.0 - 1.0;
  float fSize = attr_TexCoord.z;

  vec3 vPosition = (kvObjectToWorld * vec4 (attr_Position.xyz, 1)).xyz;
  
  var_TreeCenter = (kvObjectToWorld * vec4 (0, 0, 0, 1)).xyz + unif_TreeCenter.xyz;
  
  vPosition += span.x * unif_CameraRight.xyz * fSize;
  vPosition += span.y * unif_CameraUp.xyz * fSize;
  
  var_Position = vPosition.xyz;

  var_TexCoord = attr_TexCoord;
  gl_Position = kvCameraToProjection * kvWorldToCamera * vec4 (vPosition, 1.0);
  
  var_Color = attr_Color;
}

