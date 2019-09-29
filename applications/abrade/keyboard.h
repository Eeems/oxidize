#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QObject>
#include <view.h>

class Keyboard : public QObject
{
    Q_OBJECT
public:
    explicit Keyboard(QObject *parent = 0) : QObject(parent) { }
    View* view;
    Q_INVOKABLE void charPress(const QChar &character, const Qt::KeyboardModifiers &modifier);
    Q_INVOKABLE void keyPress(const Qt::Key &key, const Qt::KeyboardModifiers &modifier, const QString &text);
    Q_INVOKABLE void stringPress(const QString &text, const Qt::KeyboardModifiers &modifier);
};

#endif // KEYBOARD_H
