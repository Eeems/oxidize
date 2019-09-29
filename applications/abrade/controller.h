#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QGuiApplication>
#include <eventdevice.h>

class Controller : public QDBusAbstractAdaptor{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "codes.eeems.abrade")
public:
    explicit Controller(QGuiApplication* application);
    virtual ~Controller();

private:
    QCoreApplication* app;
    EventDevice* gpio_evdev;

signals:
    void aboutToQuit();

public slots:
    Q_NOREPLY void quit();

};

#endif // CONTROLLER_H
