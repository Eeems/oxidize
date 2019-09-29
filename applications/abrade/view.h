#ifndef VIEW_H
#define VIEW_H

#include <QQuickView>
#include <QQmlEngine>
#include "controller.h"


class View : public QQuickView {
public:
    View(QQmlEngine* engine, Controller* controller, QRect geometry);
public slots:
    void keyPressEvent(QKeyEvent*);
    void mouseMoveEvent(QMouseEvent* me);
    void mousePressEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    void tabletEvent(QTabletEvent* te);
    void touchEvent(QTouchEvent* te);
    void quit();
signals:
    void aboutToQuit();
private:
    Controller* _controller;
};

#endif // VIEW_H
