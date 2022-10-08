#ifndef PULSECHECK_H
#define PULSECHECK_H

#include <QObject>
#include <QThread>
class pulseCheck : public QObject
{
    Q_OBJECT
public:
    explicit pulseCheck(QObject *parent = 0);
private:
    bool pulseFlag;//心跳情况(有变动才是心跳)
    int pulseStopCount;//记录心跳不变的次数
    QThread pulseThread;
signals:
    void sendPulseStop(bool);
private slots:
public slots:
    void rePlcConnect();
};

#endif // PULSECHECK_H
