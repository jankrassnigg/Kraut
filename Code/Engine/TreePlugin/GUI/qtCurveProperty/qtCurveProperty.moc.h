#ifndef QTCURVEPROPERTY_H
#define QTCURVEPROPERTY_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <KrautFoundation/Containers/Array.h>

using namespace AE_NS_FOUNDATION;

struct aeCurve;

class QGraphicsPathItem;
class QGraphicsLineItem;

class qtCurveProperty : public QGraphicsView
{
  Q_OBJECT

public:
  qtCurveProperty (QWidget *parent = 0);

  void SetSamples (aeCurve* pCurve, const char* szPropertyName);

  void SmoothCurve (void);

  aeCurve* m_pCurve;
  const char* m_szPropertyName;

  void UpdateGraph (bool bSendModifyEvent);

  QGraphicsPathItem* m_pPath;
  QGraphicsLineItem* m_pZeroLine;
  QGraphicsScene m_Scene;

  virtual void 	mousePressEvent (QMouseEvent* event);
  virtual void resizeEvent (QResizeEvent* event);

  virtual void changeEvent (QEvent* event);
};

#endif

