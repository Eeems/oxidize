#include <QCoreApplication>
#include <QtDBus>
#include <iostream>
#include <fstream>

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
            return -1;
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
        qWarning("Cannot connect to the D-Bus system bus.\n"
                 "Please check your system settings and try again.\n");
        return -1;
    }
    QDBusConnectionInterface* interface = bus.interface();
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply = interface->registerService("codes.eeems.abrade");
    if(!reply.isValid()){
        QDBusError ex = reply.error();
        qWarning(ex.message().toStdString().c_str());
        return -1;
    }


    QStringList serviceNames = interface->registeredServiceNames();
    qDebug() << serviceNames;

    return app.exec();
    return 0;
}
