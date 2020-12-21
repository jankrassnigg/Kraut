#ifndef QTCURVEVIEW_H
#define QTCURVEVIEW_H

#include <QGraphicsView>
#include <KrautFoundation/Containers/Array.h>

using namespace AE_NS_FOUNDATION;

class qtCurveProperty;

class qtCurveView : public QGraphicsView
{
  Q_OBJECT

public:
  qtCurveView (QWidget *parent = 0);

  void SetActiveCurve (qtCurveProperty* pCurve);

  void UpdateGraph (bool bSendModifyEvent);

  qtCurveProperty* m_pActiveCurve;

private:
  QGraphicsPathItem* m_pPath;
  
  virtual void 	mouseMoveEvent (QMouseEvent* event);
  virtual void 	mouseReleaseEvent (QMouseEvent* event);
  virtual void resizeEvent (QResizeEvent* event);
};

#endif

