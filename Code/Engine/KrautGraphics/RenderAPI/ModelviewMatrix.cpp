// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "RenderAPI_Impl.h"



namespace AE_NS_GRAPHICS
{
	aeMatrixStack& aeRenderAPI::GetMatrix_ObjectToWorld (void)
	{
		return (aeRenderAPI_Impl::m_MatrixObjectToWorld);
	}

	aeMatrixStack& aeRenderAPI::GetMatrix_WorldToCamera (void)
	{
		return (aeRenderAPI_Impl::m_MatrixWorldToCamera);
	}

	aeMatrixStack& aeRenderAPI::GetMatrix_CameraToScreen (void)
	{
		return (aeRenderAPI_Impl::m_MatrixCameraToScreen);
	}
}