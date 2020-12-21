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

SHADER_BEGIN
{
  FLOAT3 vGizmoCenter = matrix_mult (ra_ObjectToWorldMatrix, FLOAT4 (0, 0, 0, 1)).xyz;
  FLOAT fDistanceToCamera = length (ra_CameraPosition - vGizmoCenter);
  FLOAT fScale = fDistanceToCamera * 0.2;

	var_Position = transform (attr_Position * fScale);
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

  float fPickingColorScale = 1.0;
  float fMinColor = 0.0;

  if ((var_PickingID == unif_Selected) && (var_PickingSubID == unif_SelectedSubID))
  {
    fPickingColorScale = 2.0;
    fMinColor = 0.2;
  }

	rt_Color = var_Color * fPickingColorScale;

  rt_Color = max (FLOAT4 (fMinColor), rt_Color * unif_ColorScale);
}
SHADER_END
