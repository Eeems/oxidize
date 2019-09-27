#include <QCoreApplication>
#include <QtDBus>
#include <iostream>
#include <fstream>
#include "controller.h"

const char* confpath = "/etc/dbus-1/system.d/codes.eeems.abrade.conf";

bool exists(const std::string& name) {
    std::fstream file(name.c_str());
    return file.good();
}

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);
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

    new Controller(&app);
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
    return app.exec();
}
