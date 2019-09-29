#include "view.h"

View::View(QQmlEngine *engine, Controller* controller)
: QQuickView(engine, Q_NULLPTR),
  _controller(controller) { }

void View::keyPressEvent(QKeyEvent* ke){
    qDebug() << Q_FUNC_INFO;
    qDebug() << "mw" << ke->key() << "down";
//    QQuickView::keyPressEvent(ke); // base class implementation
}

void View::mouseMoveEvent(QMouseEvent* me){
    qDebug() << Q_FUNC_INFO;
    qDebug() << me->pos();
//    QQuickView::mouseMoveEvent(me);
}

void View::mousePressEvent(QMouseEvent* me){
    qDebug() << Q_FUNC_INFO;
    qDebug() << me->pos();
//    QQuickView::mousePressEvent(me);
}

void View::mouseReleaseEvent(QMouseEvent* me){
    qDebug() << Q_FUNC_INFO;
    qDebug() << me->pos();
//    QQuickView::mouseReleaseEvent(me);
}

void View::tabletEvent(QTabletEvent* te){
    qDebug() << Q_FUNC_INFO;
    qDebug() << te;
//    QQuickView::tabletEvent(te);
}

void View::touchEvent(QTouchEvent* te){
    qDebug() << Q_FUNC_INFO;
    qDebug() << te;
//    QQuickView::touchEvent(te);
}

void View::quit(){

}
