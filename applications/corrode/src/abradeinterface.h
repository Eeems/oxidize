#ifndef ABRADEINTERFACE_H
#define ABRADEINTERFACE_H


#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "events.h"
#include "view/mainview.h"

class AbradeInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    MainView* view;
    static inline const char *staticInterfaceName() {
        return "codes.eeems.abrade";
    }
    AbradeInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);
    ~AbradeInterface();
signals:
    void aboutToQuit();
    void keyEvent(KeyEvent ke);
public slots:
    void keyEventRecieved(KeyEvent ke);
private:
    void postEvent(QEvent* event);
};

#endif // ABRADEINTERFACE_H
