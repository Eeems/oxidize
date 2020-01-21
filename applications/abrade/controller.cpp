#include <QDebug>
#include "controller.h"

Controller::Controller(QGuiApplication* application)
: QDBusAbstractAdaptor(application),
  app(application){
    qDBusRegisterMetaType<MouseEvent>();
    qDBusRegisterMetaType<KeyEvent>();
    qDBusRegisterMetaType<float>();
    qDBusRegisterMetaType<TouchPoint>();
    qDBusRegisterMetaType<TouchEvent>();
//    qDBusRegisterMetaType<TabletEvent>();
    setAutoRelaySignals(true);
//    connect(app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
}
Controller::~Controller(){ }
void Controller::emitableKeyEvent(QKeyEvent* ke){
    emit keyEvent(KeyEvent(ke));
}
void Controller::emitableMouseEvent(QMouseEvent* me){
    emit mouseEvent(MouseEvent(me));
}
void Controller::emitableTouchEvent(QTouchEvent* te){
    emit touchEvent(TouchEvent(te));
}
void Controller::emitableTabletEvent(QTabletEvent* te){
    emit tabletEvent(TabletEvent(te));
}
void Controller::quit(){
    app->quit();
}
