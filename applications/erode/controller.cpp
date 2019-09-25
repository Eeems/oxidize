#include "controller.h"
#include "taskitem.h"
#include <QIODevice>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QSet>
#include <QDebug>

QList<QObject*> Controller::getTasks(){
    QList<QObject*> result;
    QDir directory("/proc");
    if (!directory.exists() || directory.isEmpty()){
        qCritical() << "Unable to access /proc";
        return result;
    }
    directory.setFilter( QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    auto processes = directory.entryInfoList(QDir::NoFilter, QDir::SortFlag::Name);
    foreach(QFileInfo fi, processes) {
        TaskItem* task = new TaskItem(fi.baseName().toInt());
        if(task->ok()){
            result.append(task);
        }
    }
    return result;
}
