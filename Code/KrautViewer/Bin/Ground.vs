#version 140

uniform mat4 kvObjectToWorld;
uniform mat4 kvWorldToCamera;
uniform mat4 kvCameraToProjection;

in vec3 attr_Position;
in vec2 attr_TexCoord;

smooth out vec2 var_TexCoord;

void main()
{
  gl_Position = kvCameraToProjection * (kvWorldToCamera * (kvObjectToWorld * vec4 (attr_Position.xyz, 1.0)));

  var_TexCoord = attr_TexCoord.xy;
}

