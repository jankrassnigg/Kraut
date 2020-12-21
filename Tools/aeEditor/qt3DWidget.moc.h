#ifndef QT3DWIDGET_H
#define QT3DWIDGET_H

#include <QWidget>

class qt3DWidget : public QWidget
{
  Q_OBJECT

public:
  qt3DWidget (QWidget *parent = 0);
  ~qt3DWidget ();

  void QueueRedraw ();
  virtual void paintEvent (QPaintEvent* pEvent);
  virtual void resizeEvent (QResizeEvent* event);
  virtual QPaintEngine* paintEngine () const { return NULL; }

public slots:
  virtual void RedrawTimeout (void);

signals:

protected:
  void InitializeRenderAPI (unsigned int uiResolutionX, unsigned int uiResolutionY);

  virtual void mousePressEvent (QMouseEvent* event);
  virtual void mouseReleaseEvent (QMouseEvent* event);
  virtual void mouseMoveEvent (QMouseEvent* event);
  virtual void wheelEvent (QWheelEvent* event);

private:
  bool m_bRedrawQueued;
  bool m_bInitializedRenderAPI;
};

#endif
