#version 140

smooth in vec2 var_TexCoord;

uniform sampler2D tex_Diffuse;

out vec4 fg_FragColor;

void main(void)
{
  vec4 vColor = texture2D (tex_Diffuse, var_TexCoord.xy);
  fg_FragColor = vColor;
}