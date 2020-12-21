#ifndef QTTTREECONFIGVIEW_H
#define QTTTREECONFIGVIEW_H

#include <QtGui>
#include <QDockWidget>
#include <QGraphicsView>


class qtTreeConfigView : public QGraphicsView
{
  Q_OBJECT

public:
  qtTreeConfigView (QWidget *parent = 0);


private:
  virtual void mouseMoveEvent (QMouseEvent* event);
  virtual void mousePressEvent (QMouseEvent* event);
};

#endif

