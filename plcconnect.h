#ifndef PLCCONNECT_H
#define PLCCONNECT_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include "globalvariable.h"

class PLCConnect : public QObject
{
    Q_OBJECT
public:
    explicit PLCConnect(QObject *parent = 0);

signals:

public slots:
    void plcStart();
    void pulseStop(bool);
    void getTask(taskStruct);
private:
    QThread plcThread;
    QTcpSocket *plcSocket;
    QByteArray startCmdBuf1;
    QByteArray startCmdBuf2;
    QByteArray startCmdBuf3;
    QByteArray startCmdBuf4;
    QByteArray sendWriteDataTemp;
    QString readPLCTemp;
    bool PLCIsConnectTCP;
    bool allowReadFlag;
    //bool isWriteToPLC;
//    bool isFirstON;//刚开机
    bool firstTaskFlag;//开机后第一次收到任务
    bool isFistReceCycle;//开机收到新循环号
    bool isViControl;//状态控制中
    bool allowDebugReadCmd;//允许打印读指令
    bool dataIsChange;//plc数据有变化，用于去重复打印
    //bool directlyWrite;//直接写入PLC
    int DBWLen;
    int DBWRLen;
    int connectCount;
    int sendCmd;//通讯序号
    int curSendPLCCycle;//1.发读DBW200 2.发写DBW300
    int readOrWriteTemp;//读1 写2
    //int curServerCycleID;//记录本次服务器的发来的循环号
    int ViCurStationTemp;//记录当前站点
    int writeBuf[100];//存储准备写入PLC的数据
    int ctrlCycle;//主控制周期
    int liveCount;//记录PLC线程在跑，10s一次打印

    void writeCmdToPLC(QByteArray,bool);
    void writePLCFunc(int,int,int,int*);
    void readPLCFunc(int,int,int);
    void handReveMessage(QString,bool);
    void plcConnect();
    void writeDataCtrl();
    int CwStatus;//记录写给PLC的状态字
    //taskStruct allTask;
    QList<int>GMessagePoint;//记录服务器发来的所有站点信息
    QList<int>GBranchTagList;//记录服务器发来的所有转弯点
    QList<int>GBranchDirectionList;//记录服务器发来的所有转弯点方向
private slots:
    void readPLCData(QString);
    void disPLCSocket();
    void ctrlPLC();
    void allowedToRead();
};

#endif // PLCCONNECT_H
