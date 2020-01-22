#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuick>
#include <QtPlugin>
#include <QtDBus>
#include <unistd.h>
#include "evdev.h"
#include "controller.h"
#include "view.h"
#include "keyboard.h"
#include "fb2png.h"

#ifdef __arm__
Q_IMPORT_PLUGIN(QsgEpaperPlugin)
#endif

const char* confpath = "/etc/dbus-1/system.d/codes.eeems.abrade.conf";
const char* pngpath = "/tmp/fb.png";
const char* qt_version = qVersion();

bool exists(const std::string& name) {
    std::fstream file(name.c_str());
    return file.good();
}

void remove_fb_png(){
    qDebug() << "Removing framebuffer image";
    remove(pngpath);
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
//     qputenv("QT_QPA_EVDEV_MOUSE_PARAMETERS", "grab=1");
#endif
    qmlRegisterType<Keyboard>("Keyboard", 1, 0, "Keyboard");
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
    if(exists(pngpath)){
        remove_fb_png();
    }
//    std::atexit(remove_fb_png);
    qDebug() << "Generating png from framebuffer...";
    int res = fb2png_defaults();
    if(res){
        qDebug() << "Failed:" << res;
        return EXIT_FAILURE;
    }
    QDBusConnection bus = QDBusConnection::systemBus();
    if(!bus.isConnected()){
        qWarning("Failed to connect to system bus.");
        return EXIT_FAILURE;
    }
    // Register service
    QDBusConnectionInterface* interface = bus.interface();
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply = interface->registerService("codes.eeems.abrade");
    if(!reply.isValid()){
        QDBusError ex = reply.error();
        qWarning("%s", ex.message().toStdString().c_str());
        return EXIT_FAILURE;
    }
    qDebug() << "Successfully registered code.eeems.abrade service";
    Controller controller(&app);
    if(!bus.registerObject("/", &app)){
        qWarning("Failed to register object");
    }
    QQmlApplicationEngine engine;
    QRect geometry = app.primaryScreen()->geometry();
    View view(&engine, &controller, geometry);
    QQmlContext* context = view.rootContext();
    context->setContextProperty("screenGeometry", geometry);
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
    keyboard->init(&view, &bus);
    if (!bus.registerObject("/keyboard", keyboard, QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllSlots)) {
        qFatal("Unable to register keyboard at DBus");
        return EXIT_FAILURE;
    }
    qDebug() << "PID: " << getpid();
    qDebug() << "Files open: " << get_num_fds();
//    std::atexit(ungrab_wacom);
//    std::atexit(ungrab_touchscreen);
//    std::atexit(ungrab_gpio);
//    if(grab_wacom() || grab_touchscreen() || grab_gpio()){
//        return EXIT_FAILURE;
//    }
    QTimer::singleShot(1000, [&view](){
        qDebug() << "Showing main window";
        view.show();
    });
    qDebug() << "Starting main event loop";
    return app.exec();
}
