[PORTABLE VERTEX SHADER]

$include <Shaders/Main_vs.include>

ATTRIBUTE (VERTEX_POS, "attr_Position");
ATTRIBUTE (FLOAT4, "attr_Color");
ATTRIBUTE (FLOAT2, "attr_TexCoords");

VARYING (VERTEX_POS, "var_Position");
VARYING (FLOAT4, "var_Color");
VARYING (FLOAT2, "var_TexCoords");

SHADER_BEGIN
{
	var_Position = transform (attr_Position);
  var_Color = attr_Color;
  var_TexCoords = attr_TexCoords;
}
SHADER_END


[PORTABLE FRAGMENT SHADER]

$include <Shaders/Main_fs.include>

VARYING (FRAG_POS, "var_Position");
VARYING (FLOAT4, "var_Color");
VARYING (FLOAT2, "var_TexCoords");

TEXTURE (TYPE_2D, "tex_Diffuse");

TARGET (0, FLOAT4, "rt_Color");

$if (PICKING)
  TARGET (1, FLOAT4, "rt_PickingID");
$endif

SHADER_BEGIN
{
  $if (GROUNDPLANE_DISCARD)
    discard;
  $endif

  FLOAT4 vTexDiffuse = SAMPLE ("tex_Diffuse", "var_TexCoords");
	rt_Color = vTexDiffuse;

  $if (PICKING)
    rt_Color = FLOAT4 (0, 0, 0, 0);
    rt_PickingID = FLOAT4 (0, 0, 0, 0);
  $endif
}
SHADER_END
