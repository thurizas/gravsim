#ifndef _2Dgraphicsview_h_
#define _2Dgraphicsview_h_

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QKeyEvent>

class TwoDGeomView : public QGraphicsView
{
  Q_OBJECT
public:
  explicit TwoDGeomView(QWidget* parent = nullptr);


protected Q_SLOTS:
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void resizeEvent(QResizeEvent* event) override;
};

#endif

