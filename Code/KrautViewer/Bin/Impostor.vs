#version 140

uniform mat4 kvObjectToWorld;
uniform mat4 kvWorldToCamera;
uniform mat4 kvCameraToProjection;
uniform vec4 unif_TreeCenter;

in vec3 attr_Position;
in vec3 attr_TexCoord;
in vec4 attr_PackedNormal;
in vec4 attr_PackedTangent;
in vec4 attr_Color;

smooth out vec3 var_Position;
smooth out vec3 var_TexCoord;
smooth out vec3 var_Normal;
smooth out vec3 var_Tangent;
smooth out vec4 var_Color;
flat out vec3 var_TreeCenter;

void main()
{
  var_Position = (kvObjectToWorld * vec4 (attr_Position.xyz, 1)).xyz;
  gl_Position = kvCameraToProjection * (kvWorldToCamera * vec4 (var_Position.xyz, 1.0));
  var_Normal  = (kvObjectToWorld * vec4 (attr_PackedNormal.xyz  * 2.0 - 1.0, 0)).xyz;
  var_Tangent  = (kvObjectToWorld * vec4 (attr_PackedTangent.xyz  * 2.0 - 1.0, 0)).xyz;
  var_Color = attr_Color;
  var_TreeCenter = (kvObjectToWorld * vec4 (0, 0, 0, 1)).xyz + unif_TreeCenter.xyz;

  var_TexCoord = attr_TexCoord;
}

