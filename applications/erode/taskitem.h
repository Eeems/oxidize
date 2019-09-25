#ifndef TASKITEM_H
#define TASKITEM_H

#include <QObject>
#include <QFileInfo>
#include <string>

class TaskItem : public QObject {
    Q_OBJECT
public:
    explicit TaskItem(int pid);
    Q_PROPERTY(QString name MEMBER _name NOTIFY nameChanged)
    Q_PROPERTY(QString path MEMBER _path NOTIFY pathChanged)
    Q_PROPERTY(int pid MEMBER _pid NOTIFY pidChanged)
    bool ok();
    std::string getprop(std::string name);

signals:
    void nameChanged();
    void pathChanged();
    void pidChanged();

private:
    QString _name;
    QString _path;
    int _pid;
};

#endif // TASKITEM_H
