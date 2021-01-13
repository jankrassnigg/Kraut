#include "PCH.h"
#include "../qtCurveEditorWidget/qtCurveEditorWidget.moc.h"
#include "qtCurveProperty.moc.h"
#include "../../Tree/Tree.h"
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>

qtCurveProperty::qtCurveProperty (QWidget* parent) : QGraphicsView (parent)
{
  QPen pen;
  pen.setColor (QColor (155, 124, 255));
  pen.setStyle (Qt::DotLine);
  pen.setWidth(0);

  m_pZeroLine = m_Scene.addLine (QLineF (0, 0, 10, 0), pen);
  m_pPath = m_Scene.addPath (QPainterPath (), QPen (QBrush (QColor (71, 137, 17)), 0 ));

  setScene (&m_Scene);

  m_pCurve = nullptr;
  m_szPropertyName = "";

  QTransform t = transform ();
  t.scale (1, -1);
  setTransform (t);

  setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
  setMaximumHeight (50);
}

void qtCurveProperty::SetSamples (Kraut::Curve* pCurve, const char* szPropertyName)
{
  m_szPropertyName = szPropertyName;
  m_pCurve = pCurve;

  UpdateGraph (false);

  setSceneRect (QRectF (0, m_pCurve->m_fMinValue, m_pCurve->m_Values.size () - 1, pCurve->m_fMaxValue - pCurve->m_fMinValue));
  fitInView    (QRectF (0, m_pCurve->m_fMinValue, m_pCurve->m_Values.size () - 1, pCurve->m_fMaxValue - pCurve->m_fMinValue));

  if (isEnabled ())
    m_pPath->setPen (QPen (QColor (71, 137, 17), 0));
  else
    m_pPath->setPen (QPen (QColor (70, 70, 70), 0));
}

void qtCurveProperty::mousePressEvent (QMouseEvent* event)
{
  if (event->buttons () & Qt::LeftButton)
  {
    qtCurveEditorWidget::s_pWidget->SetActiveCurveProperty (this);
  }
}

void qtCurveProperty::resizeEvent (QResizeEvent* event)
{
  if ((!m_pCurve) || (m_pCurve->m_Values.empty ()))
    return;

  fitInView (QRectF (0, m_pCurve->m_fMinValue, m_pCurve->m_Values.size () - 1, m_pCurve->m_fMaxValue - m_pCurve->m_fMinValue));
}

void qtCurveProperty::changeEvent (QEvent* event)
{
  if (event->type () == QEvent::EnabledChange)
  {
    if (!qtCurveEditorWidget::s_pWidget || !qtCurveEditorWidget::s_pWidget->m_pCurveView)
      return;
    
    if (qtCurveEditorWidget::s_pWidget->m_pCurveView->m_pActiveCurve == this)
      qtCurveEditorWidget::s_pWidget->SetActiveCurveProperty (nullptr);

    if (isEnabled ())
      m_pPath->setPen (QPen (QColor (71, 137, 17), 0));
    else
      m_pPath->setPen (QPen (QColor (70, 70, 70), 0));
  }
}

void qtCurveProperty::UpdateGraph (bool bSendModifyEvent)
{
  if ((!m_pCurve) || (m_pCurve->m_Values.empty ()))
    return;

  QPainterPath pp;
  pp.moveTo (QPointF (0.0f, m_pCurve->m_Values[0]));

  for (aeUInt32 i = 1; i < m_pCurve->m_Values.size (); ++i)
  {
    pp.lineTo (QPointF (i, m_pCurve->m_Values[i]));
  }

  m_pPath->setPath (pp);

  if ((m_pCurve->m_fMinValue < 0.0f) && (m_pCurve->m_fMaxValue > 0.0f))
  {
    m_pZeroLine->setLine (QLineF (0, 0, m_pCurve->m_Values.size ()-1, 0));
    m_pZeroLine->setVisible (true);
  }
  else
    m_pZeroLine->setVisible (false);

  if (bSendModifyEvent)
    AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
  
}

void qtCurveProperty::SmoothCurve (void)
{
  const aeArray<float> Vals = m_pCurve->m_Values;
  for (aeInt32 i = 1; i < (aeInt32) Vals.size () - 2; ++i)
  {
    const float f1 = Vals[i - 1] * 0.2f;
    const float f2 = Vals[i] * 0.6f;
    const float f3 = Vals[i + 1] * 0.2f;

    m_pCurve->m_Values[i] = f1 + f2 + f3;
  }

  m_pCurve->m_Values[0] = Vals[0] * 0.9f + Vals[1] * 0.1f;
  m_pCurve->m_Values[Vals.size () - 1] = Vals[Vals.size () - 1] * 0.9f + Vals[Vals.size () - 2] * 0.1f;

  UpdateGraph (true);
}
