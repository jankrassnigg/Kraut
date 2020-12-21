[PORTABLE VERTEX SHADER]

$include <Shaders/Main_vs.include>

ATTRIBUTE (VERTEX_POS, "attr_Position");
ATTRIBUTE (FLOAT4, "attr_Color");

VARYING (VERTEX_POS, "var_Position");
VARYING (FLOAT4, "var_Color");

SHADER_BEGIN
{
	var_Position = transform (attr_Position);
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
