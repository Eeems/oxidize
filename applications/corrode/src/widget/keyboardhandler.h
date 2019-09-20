#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <Qt>
#include <QQmlApplicationEngine>
#include <QChar>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QDebug>
#include "../view/mainview.h"

class KeyboardHandler : public QObject {
    Q_OBJECT
public:
    explicit KeyboardHandler(QObject *parent = 0): QObject(parent) {}
    Q_INVOKABLE void virtKeyPress(const QChar &character, const Qt::KeyboardModifiers &modifier){
        qDebug() << "Test";
        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, character.unicode(), modifier, QString(character));
        qDebug() << "event";
        QObject *target = TopParent((QObject*)this);
        if(!target){
            qDebug() << "No target";
            return;
        }
        qDebug() << "Posting";
        QCoreApplication::postEvent(target, event);
        qDebug() << "Posted";
    }
private:
    QObject* TopParent(QObject* widget){
        while (widget->parent() != Q_NULLPTR){
            widget = widget->parent();
        }
        return widget;
    }
};

#endif // KEYBOARD_H
