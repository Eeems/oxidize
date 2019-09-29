#include <QDebug>
#include "controller.h"

Controller::Controller(QGuiApplication* application)
: QDBusAbstractAdaptor(application),
  app(application){
    gpio_evdev = new EventDevice("/dev/input/event2", O_RDONLY);
//    if(gpio_evdev->open_device() != -1){
//        int res = gpio_evdev->grab();
//        if(res == EBUSY){
//            qDebug() << "Device is busy, unable to grab";
//        }else if(res != 0){
//            qDebug() << "Unknown error while trying to grab device: " << res;
//        }else{
//            qDebug() << "Grabbed device";
//        }
//    }else{
//        qDebug() << "Failed to open device";
//    }
    connect(app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
    connect(app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
}
Controller::~Controller(){
//    if(!gpio_evdev->ungrab()){
//        qDebug() << "Failed to release device";
//    }else{
//        qDebug() << "Released device";
//    }
//    gpio_evdev->close_device();
}
void Controller::quit(){
    app->quit();
}
