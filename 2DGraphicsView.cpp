#include "2DGraphicsView.h"



TwoDGeomView::TwoDGeomView(QWidget* parent) : QGraphicsView(parent)
{
  setDragMode(QGraphicsView::ScrollHandDrag);
}


void TwoDGeomView::wheelEvent(QWheelEvent* event)
{
  if (event->angleDelta().y() > 0)
    scale(1.25, 1.25);
  else
    scale(0.8, 0.8);
}

void TwoDGeomView::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Left)
    rotate(1);
  else if (event->key() == Qt::Key_Right)
    rotate(-1);
}

void TwoDGeomView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);

  //if(scene() != nullptr)
  //  fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}