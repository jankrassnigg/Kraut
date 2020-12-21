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

smooth out vec3 var_Position;
smooth out vec3 var_TexCoord;
flat   out vec3 var_TreeCenter;

void main()
{
  float span = attr_TexCoord.z;

  vec3 vPosition = (kvObjectToWorld * vec4 (attr_Position.xyz, 1)).xyz;
  
  var_TreeCenter = (kvObjectToWorld * vec4 (0, 0, 0, 1)).xyz + unif_TreeCenter.xyz;
  
  vPosition += span * unif_CameraRight.xyz;
  
  var_Position = vPosition.xyz;

  var_TexCoord = vec3 (attr_TexCoord.xy, 1.0);
  gl_Position = kvCameraToProjection * kvWorldToCamera * vec4 (vPosition, 1.0);
  
  
}

