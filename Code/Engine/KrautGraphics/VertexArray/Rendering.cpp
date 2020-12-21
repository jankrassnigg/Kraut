#include "VertexArrayResource.h"
#include <KrautFoundation/Math/IncludeAll.h>
#include <KrautFoundation/Containers/Map.h>

namespace AE_NS_GRAPHICS
{
	void aeVertexArrayResource::QueueDrawcall (aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, aeUInt32 uiMaterial, aeOcclusionQueryObject* pOcclusionQuery)
	{
		if (uiTriangleCount == 0)
			return;

		if (s_bIgnoreMaterialChange)
			uiMaterial = 0xFFFFFFFF;

    std::deque<aeDrawcall>& Queue = m_DrawcallQueues[uiMaterial];

		if (!Queue.empty ())
		{
			aeDrawcall c = *Queue.rbegin ();

			if ((c.m_pConditionalRender == pOcclusionQuery) && (c.m_uiFirstTriangle + c.m_uiTriangleCount == uiFirstTriangle))
			{
				// store the merged drawcall
				StoreDrawcall (c.m_uiFirstTriangle, c.m_uiTriangleCount + uiTriangleCount, Queue, uiMaterial, true, pOcclusionQuery);
				return;
			}
		}
		
		// store the new / non-merged drawcall
		StoreDrawcall (uiFirstTriangle, uiTriangleCount, Queue, uiMaterial, false, pOcclusionQuery);
	}

	void aeVertexArrayResource::StoreDrawcall (aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, std::deque<aeDrawcall>& Queue, aeUInt32 uiMaterial, bool bReplace, aeOcclusionQueryObject* pOcclusionQuery)
	{
		aeDrawcall c;
		c.m_uiFirstTriangle = uiFirstTriangle;
		c.m_uiTriangleCount = uiTriangleCount;
		c.m_pConditionalRender = pOcclusionQuery;

		if (bReplace)
			(*Queue.rbegin ()) = c;
		else
			Queue.push_back (c);

		if (Queue.size () >= 128)
		{
			m_bBufferZeroIsBound = false;
			FlushMaterialQueue (Queue, uiMaterial);
		}
	}


	void aeVertexArrayResource::FlushDrawcallQueues (void)
	{
		m_bBufferZeroIsBound = false;

    aeMap<aeUInt32, std::deque<aeDrawcall> >::iterator it = m_DrawcallQueues.begin ();
		const aeMap<aeUInt32, std::deque<aeDrawcall> >::iterator itend = m_DrawcallQueues.end ();

		for (; it != itend; ++it)
		{
			if (!it.value().empty ())
				FlushMaterialQueue (it.value(), it.key());
		}
	}

	void aeVertexArrayResource::FlushMaterialQueue (std::deque<aeDrawcall>& Queue, aeUInt32 uiMaterial)
	{
		if ((!s_bIgnoreMaterialChange) && (m_MaterialChangeCallback != NULL))
			m_MaterialChangeCallback (uiMaterial, m_pMaterialChangePassThrough);

    std::deque<aeDrawcall>::iterator qit = Queue.begin ();
		const std::deque<aeDrawcall>::iterator qit_end = Queue.end ();

		for (; qit != qit_end; ++qit)
			IssueDrawcall (*qit);

		Queue.clear ();
	}

	void aeVertexArrayResource::IssueDrawcall (const aeDrawcall& dc)
	{
		if (dc.m_pConditionalRender != NULL)
		{
			if (!dc.m_pConditionalRender->BeginConditionalRender ())
				return;
		}

		const aeUInt32 uiPartitions = (aeUInt32) m_BufferPartitions.size ();

		if (uiPartitions > 1)
		{
			aeUInt32 uiFirstTriangle = dc.m_uiFirstTriangle;
			aeUInt32 uiLastTriangle = dc.m_uiFirstTriangle + dc.m_uiTriangleCount;

			for (aeUInt32 part = 0; part < uiPartitions; ++part)
			{
				const aeUInt32 uiPartFirstTriangle = m_BufferPartitions[part].m_uiFirstTriangle;
				const aeUInt32 uiPartLastTri = uiPartFirstTriangle + m_BufferPartitions[part].m_uiTriangleCount;

				if ((uiPartFirstTriangle <= uiFirstTriangle) && (uiPartLastTri > uiFirstTriangle))
				{
					BindBufferPartition (part);

					aeUInt32 uiMaxTri = aeMath::Min (uiPartLastTri, uiLastTriangle);

					const aeUInt32 uiCount = uiMaxTri - uiFirstTriangle;

					aeUInt32 uiOffset;
					uiOffset = (uiFirstTriangle - m_BufferPartitions[part].m_uiFirstTriangle) * 3;

					aeRenderAPI::DrawIndexed (AE_RENDER_TRIANGLES, uiOffset, uiCount * 3, (m_BufferPartitions[part].m_uiBytesPerIndex == 2) ? AE_UNSIGNED_SHORT : AE_UNSIGNED_INT);

					uiFirstTriangle += uiCount;

					if (uiFirstTriangle >= uiLastTriangle)
						break;
				}
			}
		}
		else
		{
			BindBufferPartition (0);

			aeRenderAPI::DrawIndexed (AE_RENDER_TRIANGLES, dc.m_uiFirstTriangle * 3, dc.m_uiTriangleCount * 3, m_BufferPartitions[0].m_uiBytesPerIndex == 2 ? AE_UNSIGNED_SHORT : AE_UNSIGNED_INT);
		}

		if (dc.m_pConditionalRender != NULL)
		{
			dc.m_pConditionalRender->EndConditionalRender ();
		}
	}

	void aeVertexArrayResource::RenderInstances (aeUInt32 uiInstances, aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount)
	{
		AE_CHECK_DEV (m_BufferPartitions.size () == 1, "aeVertexArrayResource::RenderInstances: You can only render instances through Vertex-Arrays that don't use buffer-splitting.");

		m_bBufferZeroIsBound = false;
		FlushDrawcallQueues ();

		BindBufferPartition (0);

		aeRenderAPI::DrawIndexedInstanced (AE_RENDER_TRIANGLES, uiFirstTriangle * 3, uiTriangleCount * 3, uiInstances, 
			m_BufferPartitions[0].m_uiBytesPerIndex == 2 ? AE_UNSIGNED_SHORT : AE_UNSIGNED_INT);
	}

	void aeVertexArrayResource::RenderWithBaseIndex (aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, aeUInt32 uiBaseIndex)
	{
		AE_CHECK_DEV (m_BufferPartitions.size () == 1, "aeVertexArrayResource::RenderInstances: You can only render instances through Vertex-Arrays that don't use buffer-splitting.");

		m_bBufferZeroIsBound = false;
		FlushDrawcallQueues ();

		BindBufferPartition (0);

		aeRenderAPI::DrawIndexedWithBase (AE_RENDER_TRIANGLES, uiFirstTriangle * 3, uiTriangleCount * 3, uiBaseIndex, 
			m_BufferPartitions[0].m_uiBytesPerIndex == 2 ? AE_UNSIGNED_SHORT : AE_UNSIGNED_INT);

	}
}

