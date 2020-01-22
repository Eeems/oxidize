#include "abradeinterface.h"

AbradeInterface::AbradeInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject* parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qDBusRegisterMetaType<KeyEvent>();
    qDBusRegisterMetaType<float>();
}

AbradeInterface::~AbradeInterface() { }

void AbradeInterface::keyEventRecieved(KeyEvent ke) {
    QKeyEvent event = ke.event();
    this->event(&event);
}

void AbradeInterface::postEvent(QEvent* event){
    QObject* target = view->focusObject();
    if(!target){
        target = view;
    }
    qDebug() << target;
    QCoreApplication::postEvent(target, event);
}
