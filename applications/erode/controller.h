#pragma once

#include <QObject>
#include "controller.h"

class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(QObject* parent = 0) : QObject(parent){}
    Q_INVOKABLE QList<QObject*> getTasks();

};
