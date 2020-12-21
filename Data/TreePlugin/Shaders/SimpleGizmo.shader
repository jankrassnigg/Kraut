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

UNIFORM (FLOAT4, "unif_PickingID");
UNIFORM (FLOAT4, "unif_PickingSubID");

UNIFORM (FLOAT4, "unif_Selected");
UNIFORM (FLOAT4, "unif_SelectedSubID");

TARGET (0, FLOAT4, "rt_Color");

$if (PICKING)
  TARGET (1, FLOAT4, "rt_Normal");
$endif

SHADER_BEGIN
{
  $if (PICKING)
    rt_Color = unif_PickingID;
    rt_Normal = unif_PickingSubID;
    return;
  $endif
  
  if ((unif_PickingID == unif_Selected) && (unif_PickingSubID == unif_SelectedSubID))
  {
    rt_Color = var_Color;
  }  
  else
  {
    rt_Color = var_Color;
  }
}
SHADER_END
