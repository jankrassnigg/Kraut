// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_RENDERAPI_MATRIXSTACK_H
#define AE_GRAPHICS_RENDERAPI_MATRIXSTACK_H

#include "Declarations.h"
#include <KrautFoundation/Math/Matrix.h>
#include <KrautFoundation/Containers/Stack.h>

namespace AE_NS_GRAPHICS
{
  class AE_GRAPHICS_DLL aeMatrixStack
	{
	public:
		aeMatrixStack ();

		void PushMatrix (void);
		void PopMatrix (void);

		void LoadIdentity (void);
		void LoadMatrix (const aeMatrix& m);
		void MultMatrix (const aeMatrix& m);

		aeMatrix GetMatrix (void) const;
		aeMatrix GetInverse (void);

		void Translate (float x, float y, float z);
		void Rotate (float fAngle, float x, float y, float z);
		void Scale (float x, float y, float z);
		void Scale (float xyz);

		void Frustum (float left, float right, float bottom, float top, float zNear, float zFar);
		void Ortho (float left, float right, float bottom, float top, float znear, float zfar); 
		void Perspective (float fovy, float aspect, float zNear, float zFar);
		void LookAt (float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz);


		void UnflagCurrentState (void) {m_bStateFlag = false;}

		//! To flag the current state, for later checks for modifications.
		void FlagCurrentState (void) {m_bStateFlag = true;}

		//! To check whether the matrix has been modified since it was flagged.
		bool IsStateFlagged (void) const {return (m_bStateFlag);}


	private:
		struct aeMat
		{
			aeMat () : m_bInverseValid (false)
			{
			}

			aeMatrix m_Matrix;
			aeMatrix m_Inverse;
			bool m_bInverseValid;
		};

		bool m_bStateFlag;
		aeMat m_CurrentMatrix;
		aeStack<aeMat> m_MatrixStack;
	};


}

#endif

