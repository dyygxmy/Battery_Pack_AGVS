#include "pulsecheck.h"
#include <QTimer>
#include "globalvariable.h"
#include <QDebug>
pulseCheck::pulseCheck(QObject *parent) : QObject(parent)
{
    this->moveToThread(&pulseThread);
    pulseThread.start();

    pulseFlag = false;//心跳情况(有变动才是心跳)
    pulseStopCount = 0;//记录心跳不变的次数

}

/*******************************************************/
//处理PLC重连机制
/*******************************************************/
void pulseCheck::rePlcConnect()
{
    int cycle = 500;//PLC目前心跳是500ms更新，设500ms检测心跳
    //心跳检测********************************************************
    if(pulseFlag == GAgvPulse){//心跳无变化
        pulseStopCount++;
        qDebug()<<"plc pulse abnormality"<<pulseStopCount;
        if(pulseStopCount > 5*(1000/cycle)){//5S plc无心跳重连
            pulseStopCount = 0;
            emit sendPulseStop(true);
        }
    }else{
        pulseStopCount = 0;
        pulseFlag = GAgvPulse;
        emit sendPulseStop(false);
    }
    if(GPLCIsConnectSuccess){//成功读取到数据
        qDebug()<<"ViNum:"<<GViNum<<"ViCurStation:"<<GViCurStation<<"ViTargetDestination:"<<GViCurDestination
               <<"ViErrorCode:"<<GViErrorCode<<"ViControl:"<<GViControl;
    }
    QTimer::singleShot(cycle,this,SLOT(rePlcConnect()));
}
