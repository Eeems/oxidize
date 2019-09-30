#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QGuiApplication>
#include <QQuickView>
#include <eventdevice.h>
#include <events.h>

class Controller : public QDBusAbstractAdaptor{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "codes.eeems.abrade")
public:
    explicit Controller(QGuiApplication* application);
    virtual ~Controller();
    void emitableKeyEvent(QKeyEvent* ke);
    void emitableMouseEvent(QMouseEvent* me);
    void emitableTouchEvent(QTouchEvent* te);
    void emitableTabletEvent(QTabletEvent* te);

private:
    QCoreApplication* app;

signals:
    void aboutToQuit();
    void keyEvent(KeyEvent ke);
    void mouseEvent(MouseEvent me);
    void touchEvent(TouchEvent te);
    void tabletEvent(TabletEvent te);

public slots:
    Q_NOREPLY void quit();

};

#endif // CONTROLLER_H
