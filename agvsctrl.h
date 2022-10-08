#ifndef AGVSCTRL_H
#define AGVSCTRL_H

#include <QObject>
#include <QThread>

class AGVSCtrl : public QObject
{
    Q_OBJECT
public:
    explicit AGVSCtrl(QObject *parent = 0);

signals:

public slots:
    void agvsCtrlStart();
    void updateTime(QString);
private:
    QThread ctrlThread;
    void logDelect();
    int coutDeleteLog;//主循环次数
};

#endif // AGVSCTRL_H
