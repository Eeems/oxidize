#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QtPlugin>
#include <QtQuick>
#include <QQuickView>
#include "view/mainview.h"

#ifdef __arm__
Q_IMPORT_PLUGIN(QsgEpaperPlugin)
#endif


int main(int argc, char *argv[])
{
#ifdef __arm__
    // Setup epaper
    qputenv("QMLSCENE_DEVICE", "epaper");
    qputenv("QT_QPA_PLATFORM", "epaper:enable_fonts");
    qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "rotate=180");
    qputenv("QT_QPA_GENERIC_PLUGINS", "evdevtablet");
#endif

    QGuiApplication app(argc, argv);

    MainView view;

    view.rootContext()->setContextProperty("screenGeometry", app.primaryScreen()->geometry());
    view.rootContext()->setContextProperty("cwd", "/");
    view.setSource(QUrl(QStringLiteral("qrc:/src/view/main.qml")));
    QObject::connect((QObject*)view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    view.show();

    qDebug() << "view shown";
    return app.exec();
}
