#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QCoreApplication>

class Controller : public QDBusAbstractAdaptor{
    Q_OBJECT
public:
    explicit Controller(QCoreApplication* application);

private:
    QCoreApplication* app;

signals:

public slots:
    Q_NOREPLY void quit(){
        app->quit();
    }
};

#endif // CONTROLLER_H
