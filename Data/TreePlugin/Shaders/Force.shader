[PORTABLE VERTEX SHADER]

$include <Shaders/Main_vs.include>

ATTRIBUTE (VERTEX_POS, "attr_Position");
ATTRIBUTE (FLOAT4, "attr_Color");

VARYING (VERTEX_POS, "var_Position");
VARYING (FLOAT4, "var_Color");

UNIFORM (FLOAT3, "unif_CameraForwards");
UNIFORM (FLOAT3, "unif_CameraRight");
UNIFORM (FLOAT3, "unif_CameraUp");

SHADER_BEGIN
{
  FLOAT3 vNewLocalPos = unif_CameraRight * attr_Position.x + unif_CameraUp * attr_Position.y;
	var_Position = transform (vNewLocalPos);
  var_Color = attr_Color;
}
SHADER_END


[PORTABLE FRAGMENT SHADER]

$include <Shaders/Main_fs.include>

VARYING (FRAG_POS, "var_Position");
VARYING (FLOAT4, "var_Color");

TARGET (0, FLOAT4, "rt_Color");

SHADER_BEGIN
{
	rt_Color = var_Color;
}
SHADER_END
