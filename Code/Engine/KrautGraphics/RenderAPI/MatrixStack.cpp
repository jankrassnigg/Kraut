// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "MatrixStack.h"



namespace AE_NS_GRAPHICS
{
	aeMatrixStack::aeMatrixStack ()
	{
		m_bStateFlag = false;
		LoadIdentity ();
	}

	void aeMatrixStack::PushMatrix (void)
	{
		m_MatrixStack.push (m_CurrentMatrix);
	}

	void aeMatrixStack::PopMatrix (void)
	{
		m_bStateFlag = false;

		m_CurrentMatrix = m_MatrixStack.top ();
		m_MatrixStack.pop ();
	}

	void aeMatrixStack::LoadIdentity (void)
	{
		m_bStateFlag = false;

		m_CurrentMatrix.m_bInverseValid = true;
		m_CurrentMatrix.m_Matrix.SetIdentity ();
		m_CurrentMatrix.m_Inverse.SetIdentity ();
	}

	void aeMatrixStack::LoadMatrix (const aeMatrix& m)
	{
		m_bStateFlag = false;

		m_CurrentMatrix.m_bInverseValid = false;

		m_CurrentMatrix.m_Matrix = m;
	}

	void aeMatrixStack::MultMatrix (const aeMatrix& m)
	{
		m_bStateFlag = false;

		m_CurrentMatrix.m_bInverseValid = false;

		m_CurrentMatrix.m_Matrix = m_CurrentMatrix.m_Matrix * m;
	}

	aeMatrix aeMatrixStack::GetMatrix (void) const
	{
		return (m_CurrentMatrix.m_Matrix);
	}

	aeMatrix aeMatrixStack::GetInverse (void)
	{
		if (!m_CurrentMatrix.m_bInverseValid)
		{
			m_CurrentMatrix.m_bInverseValid = true;
			m_CurrentMatrix.m_Inverse = m_CurrentMatrix.m_Matrix.GetInverse ();
		}

		return (m_CurrentMatrix.m_Inverse);
	}

	void aeMatrixStack::Translate (float x, float y, float z)
	{
		m_bStateFlag = false;

		m_CurrentMatrix.m_bInverseValid = false;

		aeMatrix t;
		t.SetTranslationMatrix (aeVec3 (x, y, z));
		
		m_CurrentMatrix.m_Matrix = m_CurrentMatrix.m_Matrix * t;

	}

	void aeMatrixStack::Rotate (float fAngle, float x, float y, float z)
	{
		m_bStateFlag = false;

		m_CurrentMatrix.m_bInverseValid = false;

		aeMatrix t;
		t.SetRotationMatrix (aeVec3 (x, y, z), fAngle);

		m_CurrentMatrix.m_Matrix = m_CurrentMatrix.m_Matrix * t;

	}

	void aeMatrixStack::Scale (float x, float y, float z)
	{
		m_bStateFlag = false;

		m_CurrentMatrix.m_bInverseValid = false;

		aeMatrix t;
		t.SetScalingMatrix (x, y, z);

		m_CurrentMatrix.m_Matrix = m_CurrentMatrix.m_Matrix * t;

	}

	void aeMatrixStack::Scale (float xyz)
	{
		m_bStateFlag = false;
		m_CurrentMatrix.m_bInverseValid = false;

		aeMatrix t;
		t.SetScalingMatrix (xyz, xyz, xyz);

		m_CurrentMatrix.m_Matrix = m_CurrentMatrix.m_Matrix * t;
	}

	void aeMatrixStack::Frustum (float left, float right, float bottom, float top, float fnear, float ffar)
	{
		m_bStateFlag = false;
		m_CurrentMatrix.m_bInverseValid = false;

		m_CurrentMatrix.m_Matrix.SetZero ();


		m_CurrentMatrix.m_Matrix.m_fColumn[0][0] = 2.0f * fnear / (right -left);
		m_CurrentMatrix.m_Matrix.m_fColumn[1][1] = 2.0f * fnear / (top - bottom);
		m_CurrentMatrix.m_Matrix.m_fColumn[2][2] = - (ffar + fnear) / (ffar - fnear);
                                         
		m_CurrentMatrix.m_Matrix.m_fColumn[2][0] = (right + left) / (right - left);
		m_CurrentMatrix.m_Matrix.m_fColumn[2][1] = (top + bottom) / (top - bottom);
		m_CurrentMatrix.m_Matrix.m_fColumn[3][2] = -2.0f * ffar * fnear / (ffar - fnear);
		m_CurrentMatrix.m_Matrix.m_fColumn[2][3] = -1.0f;

	}

	void aeMatrixStack::Ortho (float left, float right, float bottom, float top, float fnear, float ffar)
	{
		m_bStateFlag = false;
		m_CurrentMatrix.m_bInverseValid = false;

		m_CurrentMatrix.m_Matrix.SetZero ();

		m_CurrentMatrix.m_Matrix.m_fColumn[0][0] = 2.0f / (right - left);
		m_CurrentMatrix.m_Matrix.m_fColumn[1][1] = 2.0f / (top - bottom);
		//m_CurrentMatrix.m_Matrix.m_fColumn[2][2] = -2.0f / (ffar - fnear);
		m_CurrentMatrix.m_Matrix.m_fColumn[3][3] = 1.0f;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			m_CurrentMatrix.m_Matrix.m_fColumn[2][2] = -2.0f / (ffar - fnear);
			m_CurrentMatrix.m_Matrix.m_fColumn[3][2] = -(ffar + fnear) / (ffar - fnear);
		}
#endif

    m_CurrentMatrix.m_Matrix.m_fColumn[3][0] = -(right + left) / (right - left);
		m_CurrentMatrix.m_Matrix.m_fColumn[3][1] = -(top + bottom) / (top - bottom);
		//m_CurrentMatrix.m_Matrix.m_fColumn[3][2] = -(ffar + fnear) / (ffar - fnear);

	}

	void aeMatrixStack::Perspective (float fovy, float aspect, float zNear, float zFar)
	{
		m_bStateFlag = false;
		m_CurrentMatrix.m_bInverseValid = false;
/*
		m_CurrentMatrix.m_Matrix.SetIdentity ();
		
		const float radians = fovy / 2.0f * aeMath::fPI / 180.0f;

		const float deltaZ = zFar - zNear;
		const float sine = aeMath::SinRad (radians);

		const float cotangent = aeMath::CosRad (radians) / sine;

		m_CurrentMatrix.m_Matrix.m_fColumn[0][0] = cotangent / aspect;
		m_CurrentMatrix.m_Matrix.m_fColumn[1][1] = cotangent;
		m_CurrentMatrix.m_Matrix.m_fColumn[2][2] = -(zFar + zNear) / deltaZ;

		m_CurrentMatrix.m_Matrix.m_fColumn[2][3] = -1.0f;
		m_CurrentMatrix.m_Matrix.m_fColumn[3][2] = -2.0f * zNear * zFar / deltaZ;

		m_CurrentMatrix.m_Matrix.m_fColumn[3][3] = 0.0f;*/

      const float radians = aeMath::DegToRad (fovy / 2.0f);

	   float xmin, xmax, ymin, ymax;

	   ymax = zNear * aeMath::TanRad (radians);
	   ymin = -ymax;

	   xmin = ymin * aspect;
	   xmax = ymax * aspect;

	   Frustum (xmin, xmax, ymin, ymax, zNear, zFar);
	}

	void aeMatrixStack::LookAt (float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz)
	{
		m_bStateFlag = false;
		m_CurrentMatrix.m_bInverseValid = false;

		m_CurrentMatrix.m_Matrix.SetIdentity ();

		const aeVec3 center (centerx, centery, centerz);
		const aeVec3 eye (eyex, eyey, eyez);
		aeVec3 up (upx, upy, upz);

		const aeVec3 forward = (center - eye).GetNormalized ();

		const aeVec3 side = forward.Cross (up).GetNormalized ();

		/* Recompute up as: up = side x forward */
		up = side.Cross (forward);

		m_CurrentMatrix.m_Matrix.m_fColumn[0][0] = side.x;
		m_CurrentMatrix.m_Matrix.m_fColumn[1][0] = side.y;
		m_CurrentMatrix.m_Matrix.m_fColumn[2][0] = side.z;
                                         
		m_CurrentMatrix.m_Matrix.m_fColumn[0][1] = up.x;
		m_CurrentMatrix.m_Matrix.m_fColumn[1][1] = up.y;
		m_CurrentMatrix.m_Matrix.m_fColumn[2][1] = up.z;
                                         
		m_CurrentMatrix.m_Matrix.m_fColumn[0][2] = -forward.x;
		m_CurrentMatrix.m_Matrix.m_fColumn[1][2] = -forward.y;
		m_CurrentMatrix.m_Matrix.m_fColumn[2][2] = -forward.z;

		Translate (-eyex, -eyey, -eyez);
	}
}




