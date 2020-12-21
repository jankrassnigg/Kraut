// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_RENDERAPI_OCCLUSIONQUERYOBJECT_H
#define AE_GRAPHICS_RENDERAPI_OCCLUSIONQUERYOBJECT_H

#include "Declarations.h"
#include <vector>
#include <deque>

namespace AE_NS_GRAPHICS
{
	struct QueryObj
	{
		aeOcclusionQueryObject* m_pOQO;

#ifdef AE_RENDERAPI_OPENGL
		aeUInt32 m_uiOpenGLQueryID;
#endif
	};

	struct QueryQueueElement
	{
		aeInt32 m_QueryID;
		aeUInt32 m_TimeDue;
	};

  class AE_GRAPHICS_DLL aeOcclusionQueryObject
	{
	public:
		aeOcclusionQueryObject ();
		aeOcclusionQueryObject (aeOcclusionQueryObject& cc);
		~aeOcclusionQueryObject ();

		void BeginOcclusionQuery (void);
		void EndOcclusionQuery (void);

		bool isResultReady (void);

		unsigned int getResult (void);

		bool BeginConditionalRender (void);
		void EndConditionalRender (void);

		const aeOcclusionQueryObject& operator= (aeOcclusionQueryObject& cc);

	private:
//		const aeOcclusionQueryObject& operator= (const aeOcclusionQueryObject& cc);

		void DiscardQuery (void);
		static void CreateNewQuery (void);
		static void CheckQueryQueue (void);

		bool m_bResultReady;
		aeUInt32 m_uiOQResult;
		aeInt32 m_iQueryObject;

		static bool s_bDoingConditionalRender;
		static bool s_bQueryStarted;
		static bool s_bEndConditionalRender;

		static aeArray<QueryObj> s_QueryObjects;
		static aeDeque<aeInt32> s_AvailableQueries;
		static aeDeque<aeInt32> s_QueryQueue;
	};



}

#endif

