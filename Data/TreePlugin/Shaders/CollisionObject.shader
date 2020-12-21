[PORTABLE VERTEX SHADER]

$include <Shaders/Main_vs.include>

ATTRIBUTE (VERTEX_POS, "attr_Position");
ATTRIBUTE (FLOAT4, "attr_Color");
ATTRIBUTE (FLOAT4, "attr_PickingID");
ATTRIBUTE (FLOAT4, "attr_PickingSubID");

VARYING (VERTEX_POS, "var_Position");
VARYING (FLOAT4, "var_Color");
VARYING (FLOAT4, "var_PickingID", FLAT);
VARYING (FLOAT4, "var_PickingSubID", FLAT);
VARYING (FLOAT3, "var_OSPosition");

SHADER_BEGIN
{
  var_OSPosition = attr_Position.xyz;
	var_Position = transform (attr_Position);
  var_Color = attr_Color;
  var_PickingID = attr_PickingID;
  var_PickingSubID = attr_PickingSubID;
}
SHADER_END


[PORTABLE FRAGMENT SHADER]

$include <Shaders/Main_fs.include>

VARYING (FRAG_POS, "var_Position");
VARYING (FLOAT4, "var_Color");
VARYING (FLOAT4, "var_PickingID", FLAT);
VARYING (FLOAT4, "var_PickingSubID", FLAT);
VARYING (FLOAT3, "var_OSPosition");

TARGET (0, FLOAT4, "rt_Color");

$if (PICKING)
  TARGET (1, FLOAT4, "rt_Normal");
$endif

UNIFORM (FLOAT4, "unif_Selected");
UNIFORM (FLOAT4, "unif_SelectedSubID");

UNIFORM (FLOAT, "unif_ColorScale");

SHADER_BEGIN
{
  $if (PICKING)
    rt_Color = var_PickingID;
    rt_Normal = var_PickingSubID;
    return;
  $endif

  FLOAT3 vLine1 = fract (var_OSPosition / FLOAT3 (0.5));
  FLOAT3 vLine2 = 1.0 - fract (var_OSPosition / FLOAT3 (0.5));

  FLOAT3 vLine = max (vLine1, vLine2);

  vLine = pow (vLine, FLOAT3 (100.0));
  vLine = FLOAT3 (1.0) - vLine;
  float fBlack = min (vLine.x, min (vLine.y, vLine.z));
  float fFog = min (1.0, (1.0 - var_Position.z) * 500.0);
  
  $if (SELECTED)
    rt_Color.xyz = FLOAT3 (255.0 / 255.0, 150.0 / 255.0, 100.0 / 255.0) * fBlack * 0.85 * fFog;
  $else
    if ((var_PickingID == unif_Selected) && (var_PickingSubID == unif_SelectedSubID))
      rt_Color.xyz = FLOAT3 (255.0 / 255.0, 180.0 / 255.0, 150.0 / 255.0) * fBlack * 0.85 * fFog;
    else
      rt_Color.xyz = FLOAT3 (255.0 / 255.0, 245.0 / 255.0, 235.0 / 255.0) * fBlack * 0.85 * fFog;
  $endif

}
SHADER_END


