#include "PCH.h"

#include "../qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include "qtCurveView.moc.h"

qtCurveView::qtCurveView (QWidget* parent) : QGraphicsView (parent)
{
  QTransform t = transform ();
  t.scale (1, -1);
  setTransform (t);

  m_pActiveCurve = nullptr;
}

void qtCurveView::mouseMoveEvent (QMouseEvent* event)
{
  if (event->buttons () & Qt::LeftButton)
  {
    if (m_pActiveCurve && m_pActiveCurve->m_pCurve)
    {
      QPointF pos = mapToScene (event->pos ());

      aeInt32 iSample = aeMath::Clamp<aeInt32> ((aeInt32) (pos.x () + 0.5f), 0, m_pActiveCurve->m_pCurve->m_Values.size () - 1);
      float fValue = aeMath::Clamp ((float) pos.y (), m_pActiveCurve->m_pCurve->m_fMinValue, m_pActiveCurve->m_pCurve->m_fMaxValue);

      m_pActiveCurve->m_pCurve->m_Values[iSample] = fValue;

      UpdateGraph (false);
    }
  }
}

void qtCurveView::mouseReleaseEvent (QMouseEvent* event)
{
  if (event->button () & Qt::LeftButton)
  {
    if (m_pActiveCurve)
      AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
  }
}

void qtCurveView::resizeEvent (QResizeEvent* event)
{
  if ((!m_pActiveCurve) || (!m_pActiveCurve->m_pCurve))
    return;

  setSceneRect (QRectF (0, m_pActiveCurve->m_pCurve->m_fMinValue, m_pActiveCurve->m_pCurve->m_Values.size () - 1, m_pActiveCurve->m_pCurve->m_fMaxValue - m_pActiveCurve->m_pCurve->m_fMinValue));
  fitInView (QRectF (0, m_pActiveCurve->m_pCurve->m_fMinValue, m_pActiveCurve->m_pCurve->m_Values.size () - 1, m_pActiveCurve->m_pCurve->m_fMaxValue - m_pActiveCurve->m_pCurve->m_fMinValue));
}

void qtCurveView::UpdateGraph (bool bSendModifyEvent)
{
  if ((!m_pActiveCurve) || (!m_pActiveCurve->m_pCurve))
    return;

  m_pActiveCurve->UpdateGraph (bSendModifyEvent);
}

void qtCurveView::SetActiveCurve (qtCurveProperty* pCurve)
{
  if ((!pCurve) || (!pCurve->m_pCurve))
  {
    m_pActiveCurve = nullptr;
    setScene (nullptr);
  }
  else
  {
    m_pActiveCurve = pCurve;
    setScene (&m_pActiveCurve->m_Scene);

    setSceneRect (QRectF (0, m_pActiveCurve->m_pCurve->m_fMinValue, m_pActiveCurve->m_pCurve->m_Values.size () - 1, m_pActiveCurve->m_pCurve->m_fMaxValue - m_pActiveCurve->m_pCurve->m_fMinValue));
    fitInView    (QRectF (0, m_pActiveCurve->m_pCurve->m_fMinValue, m_pActiveCurve->m_pCurve->m_Values.size () - 1, m_pActiveCurve->m_pCurve->m_fMaxValue - m_pActiveCurve->m_pCurve->m_fMinValue));
  }
}


