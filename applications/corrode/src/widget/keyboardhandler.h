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
    explicit KeyboardHandler(QObject *parent = 0) : QObject(parent) {}
    MainView* view;
    Q_INVOKABLE void virtKeyPress(const QChar &character, const Qt::KeyboardModifiers &modifier){
        QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, character.unicode(), modifier, QString(character));
//        QObject *target = TopParent((QObject*)this);
//        QObject *target = MainView::activeFocusItem();
        QObject* target = view->focusObject();
        if(!target){
            qDebug() << "No target";
            return;
        }
        qDebug() << target;
        QCoreApplication::postEvent(target, event);
        qDebug() << "Key pressed: " << character;
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
