// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "OcclusionQueryObject.h"

namespace AE_NS_GRAPHICS
{
	aeArray<QueryObj> aeOcclusionQueryObject::s_QueryObjects;
	aeDeque<int> aeOcclusionQueryObject::s_AvailableQueries;
	aeDeque<aeInt32> aeOcclusionQueryObject::s_QueryQueue;
	bool aeOcclusionQueryObject::s_bDoingConditionalRender = false;
	bool aeOcclusionQueryObject::s_bQueryStarted = false;
	bool aeOcclusionQueryObject::s_bEndConditionalRender = false;

	aeOcclusionQueryObject::aeOcclusionQueryObject ()
	{
		m_iQueryObject = -1;
		m_uiOQResult = 1;
		m_bResultReady = true;
	}

	aeOcclusionQueryObject::aeOcclusionQueryObject (aeOcclusionQueryObject& cc)
	{
		*this = cc;
	}

	const aeOcclusionQueryObject& aeOcclusionQueryObject::operator= (aeOcclusionQueryObject& cc)
	{
		DiscardQuery ();

		m_iQueryObject = cc.m_iQueryObject;
		m_uiOQResult = cc.m_uiOQResult;
		m_bResultReady = cc.m_bResultReady;

		if (m_iQueryObject != -1)
		{
			s_QueryObjects[m_iQueryObject].m_pOQO = this;
		}

		cc.m_iQueryObject = -1;
		cc.m_uiOQResult = 1;
		cc.m_bResultReady = true;

		return (*this);
	}

	aeOcclusionQueryObject::~aeOcclusionQueryObject ()
	{
		DiscardQuery ();
	}

	void aeOcclusionQueryObject::DiscardQuery (void)
	{
		if (m_iQueryObject == -1)
			return;

		m_iQueryObject = -1;
		m_uiOQResult = 1;
		m_bResultReady = true;

    for (aeUInt32 i = 0; i < s_QueryQueue.size(); ++i)
		{
			if (s_QueryQueue[i] == m_iQueryObject)
			{
        s_QueryQueue[i] = -1;
				break;
			}
		}

		s_AvailableQueries.push_back (m_iQueryObject);

		m_iQueryObject = -1;
	}

	void aeOcclusionQueryObject::CreateNewQuery (void)
	{
		QueryObj qo;
		qo.m_pOQO = nullptr;

		#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				glGenQueries (1, &qo.m_uiOpenGLQueryID);
			}
		#endif

		s_QueryObjects.push_back (qo);
		s_AvailableQueries.push_back ((int) (s_QueryObjects.size ()) - 1);
	}

	void aeOcclusionQueryObject::CheckQueryQueue (void)
	{
		while (!s_QueryQueue.empty ())
		{
			aeInt32 iQO = s_QueryQueue[0];
			if (iQO == -1)
			{
				s_QueryQueue.pop_front ();
				continue;
			}
			
			aeUInt32 uiPixels = 0;

			#ifdef AE_RENDERAPI_OPENGL
				if (g_RenderAPI == AE_RA_OPENGL)
				{
					aeInt32 iReady = 1;

					glGetQueryObjectiv (s_QueryObjects[iQO].m_uiOpenGLQueryID, GL_QUERY_RESULT_AVAILABLE, &iReady);

					if (iReady == 0)
						return;

					glGetQueryObjectuiv (s_QueryObjects[iQO].m_uiOpenGLQueryID, GL_QUERY_RESULT, &uiPixels);
				}
			#endif

			s_QueryObjects[iQO].m_pOQO->m_uiOQResult = uiPixels;
			s_QueryObjects[iQO].m_pOQO->m_bResultReady = true;
			s_QueryObjects[iQO].m_pOQO = nullptr;

			s_QueryQueue.pop_front ();
			s_AvailableQueries.push_back (iQO);
		}
	}

	void aeOcclusionQueryObject::BeginOcclusionQuery (void)
	{
		AE_CHECK_DEV (!s_bQueryStarted, "aeOcclusionQueryObject::BeginOcclusionQuery: You already started a query previously.");
		AE_CHECK_DEV (!s_bDoingConditionalRender, "aeOcclusionQueryObject::BeginOcclusionQuery: You are currently doing a conditional render.");

		// if there are no queries available
		if (s_AvailableQueries.empty ())
		{
			// first check, whether any previous query has finished
			CheckQueryQueue ();

			// if not, create a new query
			if (s_AvailableQueries.empty ())
				CreateNewQuery ();
		}

		s_bQueryStarted = true;

		m_bResultReady = false;
		m_uiOQResult = 1;
		m_iQueryObject = s_AvailableQueries[0];
		s_AvailableQueries.pop_front ();

		s_QueryObjects[m_iQueryObject].m_pOQO = this;

		#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				glBeginQuery (GL_SAMPLES_PASSED, s_QueryObjects[m_iQueryObject].m_uiOpenGLQueryID);
			}
		#endif
	}

	void aeOcclusionQueryObject::EndOcclusionQuery (void)
	{
		AE_CHECK_DEV (s_bQueryStarted, "aeOcclusionQueryObject::EndOcclusionQuery: You didn't start a query previously.");

		#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				glEndQuery (GL_SAMPLES_PASSED);
			}
		#endif

		s_QueryQueue.push_back (m_iQueryObject);

		s_bQueryStarted = false;
	}

	bool aeOcclusionQueryObject::isResultReady (void)
	{
		AE_CHECK_DEV (!s_bQueryStarted, "aeOcclusionQueryObject::isResultReady: You didn't finish the current query.");

		if (m_bResultReady)
			return (true);

		CheckQueryQueue ();
		
		return (m_bResultReady);
	}

	unsigned int aeOcclusionQueryObject::getResult (void)
	{
		AE_CHECK_DEV (!s_bQueryStarted, "aeOcclusionQueryObject::getResult: You didn't finish the current query.");

		if (m_bResultReady)
			return (m_uiOQResult);

		AE_CHECK_DEV (m_iQueryObject != -1, "aeOcclusionQueryObject::getResult: You need to do a query before you can check for any result.");

		while (!m_bResultReady)
			CheckQueryQueue ();

		return (m_uiOQResult);
	}

	bool aeOcclusionQueryObject::BeginConditionalRender (void)
	{
		AE_CHECK_DEV (!s_bQueryStarted, "aeOcclusionQueryObject::BeginConditionalRender: You didn't finish the current query.");
		AE_CHECK_DEV (!s_bDoingConditionalRender, "aeOcclusionQueryObject::BeginConditionalRender: You already started a conditional render previously.");

		s_bDoingConditionalRender = true;

		if (isResultReady ())
		{
			if (m_uiOQResult == 0)
			{
				s_bDoingConditionalRender = false;
				return (false);
			}

			return (true);
		}

		s_bEndConditionalRender = true;

		#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				if (GLEW_NV_conditional_render)
					glBeginConditionalRenderNV (s_QueryObjects[m_iQueryObject].m_uiOpenGLQueryID, GL_QUERY_BY_REGION_NO_WAIT_NV);
			}
		#endif

		return (true);
	}

	void aeOcclusionQueryObject::EndConditionalRender (void)
	{
		AE_CHECK_DEV (s_bDoingConditionalRender, "aeOcclusionQueryObject::EndConditionalRender: You didn't start a conditional render previously. If BeginConditionalRender returns false, you are not supposed to send any rendering commands and call this function, because the queries result was zero.");

		s_bDoingConditionalRender = false;
		
		if (s_bEndConditionalRender)
		{
			s_bEndConditionalRender = false;

#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				if (GLEW_NV_conditional_render)
					glEndConditionalRenderNV ();
			}
#endif
		}
	}
}

