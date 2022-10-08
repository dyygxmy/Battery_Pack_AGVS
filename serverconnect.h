#ifndef SERVERCONNECT_H
#define SERVERCONNECT_H

#include <QObject>
#include <QThread>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include "globalvariable.h"

class serverConnect : public QObject
{
    Q_OBJECT
public:
    explicit serverConnect(QObject *parent = 0);

signals:
    void sendTask(taskStruct);
public slots:
    void serverStart();
private:
    QThread serverThread;
    QNetworkRequest request;
    QNetworkAccessManager *manager;
    QJsonObject handSendMessage();
    QString datetime;
    QString postAndReceState;//请求与接收的状态
    int postErrorCount;//请求错误次数
    int postErrorAllCount;//请求错误次数超时有几次
    bool newTime;//开机同步一次服务器时间
    bool firstPost;//开机post
    bool firstControl;//开机一次不检测控制字

    int cycleIDTemp;//记录本次循环号
    bool IsAutoStationTemp;//记录本次站属性
    bool FinishChargeTemp;//记录本次充电结束状态
    bool PrepareToChargeTemp;//记录本次准备充电状态
    bool ManualModeTemp;//记录本次手动模式状态
    bool AllowToRunTemp;//记录本次允许状态
    bool firstPostSucessFlag;//开机第一次post成功
    int ManualControlTemp;//记录本次手动控制字
    int ManualBranchTemp;//记录本次手动控制分支方向
    int TargetStationIDToWriteTemp;//记录本次目标站
    int TargetDirectionToWriteTemp;//记录本次目标方向
    int SpeedModeTemp;//记录本次速度模式
    int ciSpeed_ATemp;//记录本次A轮子速度
    int ciSpeed_BTemp;//记录本次B轮子速度
    int BranchTagTemp;//记录本次分支站
    int BranchDirectionTemp;//记录本次分支方向
    int ShieldTemp;//记录本次光栅
    int SpeedTemp;//记录本次速度

    taskStruct allTask;
    void handReceMessage(QByteArray);
private slots:
    void replyFinished(QNetworkReply*);
    void handdata();
signals:
    void sendUpdateTime(QString);
};

#endif // SERVERCONNECT_H
