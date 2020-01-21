#ifndef EVENTS_H
#define EVENTS_H

#include <QDBusMetaType>
#include <QKeyEvent>
#include <QMouseEvent>

struct KeyEvent {
    KeyEvent() {}
    KeyEvent(QKeyEvent* ke){
        type = ke->type();
        key = ke->key();
        modifiers = (int)ke->modifiers();
        text = ke->text();
        isAutoRepeat = ke->isAutoRepeat();
        count = ke->count();
    }
    int type;
    int key;
    int modifiers;
    QString text;
    bool isAutoRepeat;
    int count;
    QKeyEvent event(){
        return QKeyEvent((QEvent::Type)type, key,
                (Qt::KeyboardModifiers)modifiers, text, isAutoRepeat, count);
    }
};
Q_DECLARE_METATYPE(KeyEvent)
inline QDBusArgument &operator<<(QDBusArgument &a, const KeyEvent &e){
    a.beginStructure();
    a << e.type << e.key << e.modifiers << e.text << e.isAutoRepeat << e.count;
    a.endStructure();
    return a;
}
inline const QDBusArgument &operator>>(const QDBusArgument &a, KeyEvent &e){
    a.beginStructure();
    a >> e.type >> e.key >> e.modifiers >> e.text >> e.isAutoRepeat >> e.count;
    a.endStructure();
    return a;
}

struct MouseEvent {
    MouseEvent() {}
    MouseEvent(QMouseEvent* me){
        type = me->type();
        localPos = me->localPos();
        screenPos = me->screenPos();
        button = (int)me->button();
        buttons = (int)me->buttons();
        modifiers = (int)me->modifiers();
    }
    int type;
    QPointF localPos;
    QPointF screenPos;
    int button;
    int buttons;
    int modifiers;
    QMouseEvent event(){
        return QMouseEvent((QEvent::Type)type, localPos, screenPos,
                (Qt::MouseButton)button, (Qt::MouseButtons)buttons,
                (Qt::KeyboardModifiers)modifiers);
    }
};
Q_DECLARE_METATYPE(MouseEvent)
inline QDBusArgument &operator<<(QDBusArgument &a, const MouseEvent &e){
    a.beginStructure();
    a << e.type << e.localPos << e.screenPos << e.button << e.buttons << e.modifiers;
    a.endStructure();
    return a;
}
inline const QDBusArgument &operator>>(const QDBusArgument &a, MouseEvent &e){
    a.beginStructure();
    a >> e.type >> e.localPos >> e.screenPos >> e.button >> e.buttons >> e.modifiers;
    a.endStructure();
    return a;
}
Q_DECLARE_METATYPE(float)
inline QDBusArgument &operator<<(QDBusArgument &a, const float &f){
    a.beginStructure();
    a << (double)f;
    a.endStructure();
    return a;
}
inline const QDBusArgument &operator>>(const QDBusArgument &a, float &f){
    a.beginStructure();
    double d;
    a >> d;
    f = (float)d;
    a.endStructure();
    return a;
}
struct TouchPoint {
    TouchPoint() {}
    TouchPoint(QTouchEvent::TouchPoint* te){
        flags = (int)te->flags();
        id = te->id();
        lastNormalizedPos = te->lastNormalizedPos();
        lastPos = te->lastPos();
        lastScenePos = te->lastScenePos();
        lastScreenPos = te->lastScreenPos();
        normalizedPos = te->normalizedPos();
        pos = te->pos();
        pressure = te->pressure();
        rawScreenPositions = te->rawScreenPositions();
        rotation = te->rotation();
        scenePos = te->scenePos();
        screenPos = te->screenPos();
        startNormalizedPos = te->startNormalizedPos();
        startPos = te->startPos();
        startScenePos = te->startScenePos();
        startScreenPos = te->startScreenPos();
        state = (int)te->state();
        uniqueId = te->uniqueId().numericId();
        velocityX = te->velocity().x();
        velocityY = te->velocity().y();
    }
    int flags;
    int id;
    QPointF lastNormalizedPos;
    QPointF lastPos;
    QPointF lastScenePos;
    QPointF lastScreenPos;
    QPointF normalizedPos;
    QPointF pos;
    qreal pressure;
    QVector<QPointF> rawScreenPositions;
    qreal rotation;
    QPointF scenePos;
    QPointF screenPos;
    QPointF startNormalizedPos;
    QPointF startPos;
    QPointF startScenePos;
    QPointF startScreenPos;
    int state;
    qint64 uniqueId;
    float velocityX;
    float velocityY;
    QTouchEvent::TouchPoint touchPoint(){
        QTouchEvent::TouchPoint* tp = new QTouchEvent::TouchPoint();
        tp->setFlags((QTouchEvent::TouchPoint::InfoFlags)flags);
        tp->setId(id);
        tp->setLastNormalizedPos(lastNormalizedPos);
        tp->setLastPos(lastPos);
        tp->setLastScenePos(lastScenePos);
        tp->setLastScreenPos(lastScreenPos);
        tp->setNormalizedPos(normalizedPos);
        tp->setPos(pos);
        tp->setPressure(pressure);
        tp->setRawScreenPositions(rawScreenPositions);
        tp->setRotation(rotation);
        tp->setScenePos(scenePos);
        tp->setScreenPos(screenPos);
        tp->setStartNormalizedPos(startNormalizedPos);
        tp->setStartPos(startPos);
        tp->setStartScenePos(startScenePos);
        tp->setStartScreenPos(startScreenPos);
        tp->setState((Qt::TouchPointState)state);
        tp->setUniqueId(uniqueId);
        tp->setVelocity(QVector2D(velocityX, velocityY));
        return *tp;
    }
};
Q_DECLARE_METATYPE(TouchPoint)
inline QDBusArgument &operator<<(QDBusArgument &a, const TouchPoint &e){
    a.beginStructure();
    a << e.flags << e.id << e.lastNormalizedPos << e.lastPos << e.lastScenePos << e.lastScreenPos
            << e.normalizedPos << e.pos << e.pressure << e.rawScreenPositions << e.rotation
            << e.scenePos << e.screenPos << e.startNormalizedPos << e.startPos << e.startScenePos
            << e.startScreenPos << e.state << e.uniqueId << e.velocityX << e.velocityY;
    a.endStructure();
    return a;
}
inline const QDBusArgument &operator>>(const QDBusArgument &a, TouchPoint &e){
    a.beginStructure();
    a >> e.flags >> e.id >> e.lastNormalizedPos >> e.lastPos >> e.lastScenePos >> e.lastScreenPos
            >> e.normalizedPos >> e.pos >> e.pressure >> e.rawScreenPositions >> e.rotation
            >> e.scenePos >> e.screenPos >> e.startNormalizedPos >> e.startPos >> e.startScenePos
            >> e.startScreenPos >> e.state >> e.uniqueId >> e.velocityX >> e.velocityY;
    a.endStructure();
    return a;
}
struct TouchEvent {
    TouchEvent() {}
    TouchEvent(QTouchEvent* te){
        type = (int)te->type();
        modifiers = (int)te->modifiers();
        touchPointStates = (int)te->touchPointStates();
        device = te->device()->name();
        QList<QTouchEvent::TouchPoint> tp = te->touchPoints();
        touchPoints = QList<TouchPoint>();
        for(int i = 0; i < tp.length(); i++){
            touchPoints.append(TouchPoint(&tp[i]));
        }
    }
    int type;
    int modifiers;
    int touchPointStates;
    QString device;
    QList<TouchPoint> touchPoints;
    QTouchEvent event(){
        QTouchDevice td;
        QList<const QTouchDevice*> devices = QTouchDevice::devices();
        for(int i = 0; i < devices.length(); i++){
            if(devices[i]->name() == device){
                td = *devices[i];
                break;
            }
        }
        QList<QTouchEvent::TouchPoint> tp = QList<QTouchEvent::TouchPoint>();
        for(int i = 0; i < touchPoints.length(); i++){
            tp.append(touchPoints[i].touchPoint());
        }
        return QTouchEvent((QEvent::Type)type, &td, (Qt::KeyboardModifiers)modifiers,
                (Qt::TouchPointStates)touchPointStates, (QList<QTouchEvent::TouchPoint>)tp);
    }
};
Q_DECLARE_METATYPE(TouchEvent)
inline QDBusArgument &operator<<(QDBusArgument &a, const TouchEvent &e){
    a.beginStructure();
    a << e.type << e.modifiers << e.touchPointStates << e.device << e.touchPoints;
    a.endStructure();
    return a;
}
inline const QDBusArgument &operator>>(const QDBusArgument &a, TouchEvent &e){
    a.beginStructure();
    a >> e.type >> e.modifiers >> e.touchPointStates >> e.device >> e.touchPoints;
    a.endStructure();
    return a;
}
struct TabletEvent {
    TabletEvent() {}
    TabletEvent(QTabletEvent* te){
        type = (int)te->type();
        pos = te->pos();
        globalPos = te->globalPos();
        device = te->device();
        pointerType = te->pointerType();
        pressure = te->pressure();
        xTilt = te->xTilt();
        yTilt = te->yTilt();
        tangentialPressure = te->tangentialPressure();
        rotation = te->rotation();
        z = te->z();
        keyState = (int)te->modifiers();
        uniqueId = te->uniqueId();
        button = (int)te->button();
        buttons = (int)te->buttons();
    }
    int type;
    QPointF pos;
    QPointF globalPos;
    int device;
    int pointerType;
    qreal pressure;
    int xTilt;
    int yTilt;
    qreal tangentialPressure;
    qreal rotation;
    int z;
    int keyState;
    qint64 uniqueId;
    int button;
    int buttons;
    QTabletEvent event(){
        return QTabletEvent((QEvent::Type)type, pos, globalPos, device, pointerType,
                pressure, xTilt, yTilt, tangentialPressure, rotation, z,
                (Qt::KeyboardModifiers)keyState, uniqueId, (Qt::MouseButton)button,
                (Qt::MouseButtons)buttons);
    }
};
Q_DECLARE_METATYPE(TabletEvent)
inline QDBusArgument &operator<<(QDBusArgument &a, const TabletEvent &e){
    a.beginStructure();
    a << e.type;
    a.endStructure();
    return a;
}
inline const QDBusArgument &operator>>(const QDBusArgument &a, TabletEvent &e){
    a.beginStructure();
    a >> e.type;
    a.endStructure();
    return a;
}
#endif // EVENTS_H
