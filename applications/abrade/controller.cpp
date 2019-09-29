#include <QDebug>
#include "controller.h"

Controller::Controller(QGuiApplication* application)
: QDBusAbstractAdaptor(application),
  app(application){
    connect(app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
    connect(app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
}
Controller::~Controller(){
}
void Controller::quit(){
    app->quit();
}
