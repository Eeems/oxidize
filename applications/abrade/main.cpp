#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuick>
#include <QtPlugin>
#include <QtDBus>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/resource.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdlib>
#include "controller.h"
#include "view.h"
#include "keyboard.h"

#ifdef __arm__
Q_IMPORT_PLUGIN(QsgEpaperPlugin)
#endif

const char* confpath = "/etc/dbus-1/system.d/codes.eeems.abrade.conf";
const char *qt_version = qVersion();

bool exists(const std::string& name) {
    std::fstream file(name.c_str());
    return file.good();
}

int get_num_fds(){
     int fd_count;
     char buf[64];
     struct dirent *dp;

     snprintf(buf, 64, "/proc/%i/fd/", getpid());

     fd_count = 0;
     DIR *dir = opendir(buf);
     while ((dp = readdir(dir)) != NULL) {
          fd_count++;
     }
     closedir(dir);
     return fd_count;
}
std::string get_filepath(int fd){
    char procpath[PATH_MAX + 1];
    char filepath[PATH_MAX + 1];
    memset(procpath, 0, PATH_MAX + 1);
    memset(filepath, 0, PATH_MAX + 1);
    snprintf(procpath, PATH_MAX, "/proc/self/fd/%d", fd);
    readlink(procpath, filepath, (size_t)PATH_MAX);
    return std::string(filepath);
}
int ioctl_evdev(const char* path, unsigned long request, int value){
    int res = -1;
    for(int fd = 0; fd < get_num_fds(); fd++){
        if(strcmp(get_filepath(fd).c_str(), path) == 0){
            res = ioctl(fd, request, value);
            if(res != 0){
                return res;
            }
        }
    }
    return res;
}
int grab_evdev(const char* path){
    return ioctl_evdev(path, EVIOCGRAB, 1);
}
int ungrab_evdev(const char* path){
    return ioctl_evdev(path, EVIOCGRAB, 0);
}
void ungrab_wacom(){
    ungrab_evdev("/dev/input/event0");
}
int grab_wacom(){
    int res = grab_evdev("/dev/input/event0");
    if(res == EBUSY){
        qDebug() << "Failed to grab wacom, device is busy";
    }else if(res != 0){
        qDebug() << "Failed to grab wacom: " << res;
    }else{
        qDebug() << "Grabbed wacom";
        std::atexit(ungrab_wacom);
    }
    return res;
}
void ungrab_touchscreen(){
    ungrab_evdev("/dev/input/event1");
}
int grab_touchscreen(){
    int res = grab_evdev("/dev/input/event1");
    if(res == EBUSY){
        qDebug() << "Failed to grab touchscreen, device is busy";
    }else if(res != 0){
        qDebug() << "Failed to grab touchscreen: " << res;
    }else{
        qDebug() << "Grabbed touchscreen";
        std::atexit(ungrab_touchscreen);
    }
    return res;
}
void ungrab_gpio(){
    ungrab_evdev("/dev/input/event2");
}
int grab_gpio(){
     int res = grab_evdev("/dev/input/event2");
     if(res == EBUSY){
         qDebug() << "Failed to grab gpio, device is busy";
     }else if(res != 0){
         qDebug() << "Failed to grab gpio: " << res;
     }else{
         qDebug() << "Grabbed gpio";
         std::atexit(ungrab_gpio);
     }
     return res;
 }

int main(int argc, char *argv[]){
    if (strcmp(qt_version, QT_VERSION_STR) != 0){
        qDebug() << "Version mismatch, Runtime: " << qt_version << ", Build: " << QT_VERSION_STR;
    }
#ifdef __arm__
    // Setup epaper
    qputenv("QMLSCENE_DEVICE", "epaper");
    qputenv("QT_QPA_PLATFORM", "epaper:enable_fonts");
    qputenv("QT_QPA_GENERIC_PLUGINS", "evdevtablet");
    qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "rotate=180");
    // qputenv("QT_DEBUG_BACKINGSTORE", "1");
    // No need to use env for grabbing as we are doing it manually
    // qputenv("QT_QPA_EVDEV_KEYBOARD_PARAMETERS", "grab=1");
    // qputenv("QT_QPA_EVDEV_MOUSE_PARAMETERS", "grab=1");
#endif
    qmlRegisterType<Keyboard>("KeyboardHandler", 1, 0, "KeyboardHandler");
    QGuiApplication app(argc, argv);
    if(!exists(confpath)){
        QFile infile(QString(":/codes.eeems.abrade.conf"));
        if(!infile.open(QIODevice::ReadOnly)) {
            qWarning("Unable to open dbus configuration file");
            return EXIT_FAILURE;
        }
        std::string data = infile.readAll().toStdString();
        std::ofstream outfile;
        outfile.open(confpath);
        outfile << data;
        outfile << std::endl;
        outfile.close();
        system("systemctl reload dbus");
    }
    QDBusConnection bus = QDBusConnection::systemBus();
    if(!bus.isConnected()){
        qWarning("Failed to connect to system bus.");
        return EXIT_FAILURE;
    }
    QDBusMessage message = QDBusMessage::createSignal("/", "codes.eeems.abrade", "ping");
    message << "Hello Word!";
    bus.send(message);
    Controller controller(&app);
    if(!bus.registerObject("/", &app)){
        qWarning("Failed to register object");
    }
    // Register service
    QDBusConnectionInterface* interface = bus.interface();
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply = interface->registerService("codes.eeems.abrade");
    if(!reply.isValid()){
        QDBusError ex = reply.error();
        qWarning(ex.message().toStdString().c_str());
        return EXIT_FAILURE;
    }
    qDebug() << "Successfully registered code.eeems.abrade service";
    QQmlApplicationEngine engine;
    View view(&engine, &controller);
    QQmlContext* context = view.rootContext();
    context->setContextProperty("screenGeometry", app.primaryScreen()->geometry());
    view.setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    QQuickItem* root = view.rootObject();
    if(root->children().isEmpty()){
        qDebug() << "Nothing to display";
        return EXIT_FAILURE;
    }
    QObject::connect((QObject*)view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    Keyboard* keyboard = root->findChild<Keyboard*>("keyboard");
    if(!keyboard){
        qDebug() << "No keyboard";
        return EXIT_FAILURE;
    }
    keyboard->view = &view;
    view.show();
    qDebug() << "PID: " << getpid();
    qDebug() << "Files open: " << get_num_fds();
    if(grab_wacom() || grab_touchscreen() || grab_gpio()){
        return EXIT_FAILURE;
    }
    return app.exec();
}
