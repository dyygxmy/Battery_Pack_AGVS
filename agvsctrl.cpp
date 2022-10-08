#include "agvsctrl.h"
#include <QDebug>
#include "globalvariable.h"
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDate>
#include "unistd.h"
AGVSCtrl::AGVSCtrl(QObject *parent) : QObject(parent)
{
    this->moveToThread(&ctrlThread);
    ctrlThread.start();
    coutDeleteLog = 0;//主循环次数
}

/*
*PLC循环逻辑：
    *1.发读指令获取上次写给PLC的数据。DBW300(开机一次)
    *
    *2.发写指令给PLC。DBW300（成功后发服务器状态字）
    *3.发读指令获取PLC处理完成后的数据。DBW200
    *
*服务器循环逻辑：
    *1.将PLC保存在DWB300的数据发给服务器。
    *2.post服务器。
    *3.接收解析服务器发来的数据。
*/

void AGVSCtrl::agvsCtrlStart()
{
    int cycle = 100;


    if(coutDeleteLog >= 1*60*60*(1000/cycle)){//100ms一循环，一小时删除一次
        coutDeleteLog = 0;
        logDelect();
    }
    coutDeleteLog++;

    QTimer::singleShot(cycle,this,SLOT(agvsCtrlStart()));
}

/*******************************************************/
//更新时间
/*******************************************************/
void AGVSCtrl::updateTime(QString datetime)
{
    system((QString("date -s \"") +datetime+QString("\" &")).toLatin1().data());//设置系统时间
    system("hwclock -w &");//同步系统时钟和硬件时钟
    qDebug()<<"settime:"<<datetime<<"Current version:"<<versionNUM;
}


/*******************************************************/
//删除日志处理
/*******************************************************/
void AGVSCtrl::logDelect()
{
    qDebug()<<"this is logDelect";
    QDir dir("/agvs/log");
    if (!dir.exists())
        return ;
    QFileInfoList list = dir.entryInfoList();
    int i=0;

    do{
        QFileInfo fileInfo = list.at(i);
        if(fileInfo.fileName()=="."||fileInfo.fileName()=="..")
        {
            i++;
            continue;
        }
        //        qDebug()<<"find "<<fileInfo.filePath();
        QDate date = QDate::currentDate();
        QDate logDate = QDate::fromString(fileInfo.baseName(),"yyyy-MM-dd");
        QDate wifiLogDate = QDate::fromString(fileInfo.baseName().remove("wifi"),"yyyy-MM-dd");
        if((logDate < date.addDays(-2)) && (logDate > date.addDays(-3600)) ||
                (wifiLogDate < date.addDays(-2) && wifiLogDate > date.addDays(-3600)))
        {
            qDebug() << "delete file "+fileInfo.fileName();
            unlink(fileInfo.filePath().toLocal8Bit().data());
        }
        i++;
    }while(i<list.size());
}
