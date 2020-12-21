#include "PCH.h"

#include "../../Basics/Plugin.h"

#include "qtTreeConfigWidget.moc.h"

qtTreeConfigView::qtTreeConfigView (QWidget* parent) : QGraphicsView (parent)
{
}

void qtTreeConfigView::mouseMoveEvent (QMouseEvent* event)
{
  if (qtTreeConfigWidget::s_pWidget)
    qtTreeConfigWidget::s_pWidget->UpdateHighlights (false);
}


void qtTreeConfigView::mousePressEvent (QMouseEvent* event)
{
  if (qtTreeConfigWidget::s_pWidget)
  {
    qtTreeConfigWidget::s_pWidget->UpdateHighlights (true, event);
    qtTreeConfigWidget::s_pWidget->UpdateHighlights (false, event);
  }
}

