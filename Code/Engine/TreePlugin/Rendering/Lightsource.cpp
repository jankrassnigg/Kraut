#include "PCH.h"

#include "../Basics/Plugin.h"

static aeTransformGizmo g_LightsourceTransform;

static void UpdateLightsourcePosition (void* pPassThrough, const void* pEventData)
{
  aeTransformGizmo::EventData* pED = (aeTransformGizmo::EventData*) pEventData;

  if (pED->m_EventType == aeTransformGizmo::Transformed)
  {
    aeVec3* pPos = (aeVec3*) pPassThrough;
    *pPos = pED->m_Gizmo->GetTransform ().GetTranslationVector ();
  }
}

AE_ON_GLOBAL_EVENT_ONCE (aeTreePlugin_Init)
{
  aeMatrix t;
  t.SetTranslationMatrix (g_Globals.s_vPointLightPos);

  g_LightsourceTransform.m_Events.RegisterEventReceiver (UpdateLightsourcePosition, &g_Globals.s_vPointLightPos);
  g_LightsourceTransform.Reset (&g_Globals.s_vPointLightPos, 0, t, true);
}

AE_ON_GLOBAL_EVENT (aeTreePlugin_TreeLoaded)
{
  aeMatrix t;
  t.SetTranslationMatrix (g_Globals.s_vPointLightPos);

  g_LightsourceTransform.m_Events.RegisterEventReceiver (UpdateLightsourcePosition, &g_Globals.s_vPointLightPos);
  g_LightsourceTransform.Reset (&g_Globals.s_vPointLightPos, 0, t, true);
}



