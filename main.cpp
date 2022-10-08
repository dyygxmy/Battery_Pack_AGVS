#include "mainwindow.h"
#include <QApplication>
#include "globalvariable.h"
#include "agvsctrl.h"
#include "plcconnect.h"
#include "serverconnect.h"
#include "pulsecheck.h"
#include <QTimer>
#include <QTime>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSettings>

QString versionNUM = "VersionNUM:V2020.06.18_1";
int GPLCDB = 2;//DB块
int GPLCDBWWRITE = 300;//DB块写区域
int GPLCDBWREAD = 200;//DB块读区域

int GVwStatus = 0;//DBW200 小车 状态字节
int GViNum = 0;//DBW202 小车 编号 （注：只通过PLC获取用来传送给服务器）
int GViCurStation = 0;//DBW204 小车 当前所在交通站编号
int GViRunStatus = 0;//DBW206 小车 运动状态及方向
int GVrBatteryVol = 0;//DBD208 小车 当前电池电压
int GViErrorCode = 0;//DBW212 小车 故障代码
int GViControl = 0;//DBW214 小车 交互控制字
int GVrBatteryCul = 0;//DBD216 小车 当前电池电流
int GVrBatteryQ = 0;//DBD220 小车 当前电池电量
int GViCurDestination = 0;//DBW224 小车当前目标交通站编号
int GViDestDirection = 0;//DBW226 目标方向 -1后退，0:NULL，1前进
int GViSpeed = 0;//DBW228 当前速度
int GViBranStation = 0;//DBW230 当前分支交通站
int GViBranDirection = 0;//DBW232 当前分支方向。(面向运动方向来定义左右) -2左弯道，-1左分支，0直行，1右分支，2右弯道，3穿过
int GViSpeedMode = 0;//DBW234 速度控制模式
int GViSpeed_A = 0;//DBW236 表示当前A轮实时转速
int GViSpeed_B = 0;//DBW238 表示当前B轮实时转速
int GViChooseshield = 0;//DBW240 区域选择屏蔽
int GViBatteryVol = 0;//DBW242 小车当前电池电压
int GViBatteryCul = 0;// DBW244 小车当前电池电流
int GViBatteryQ = 0;//DBW246 小车当前电池电量


bool GPostRecFlag = true;//post正常
bool GPostValid = true;//post有效（收到数据）
bool GEnableSendToHttp = false;//读取写入PLC的内容，收到回复
bool GIsNewCycle = false;//服务器有发新的任务来
bool GIsFirstWriteViNum = true;

bool GIsRunning = true;//bit7 电机是否在运转
bool GIsCharging = true;//bit6 充电
bool GIsEmpty = true;//bit5:携带料车
bool GIsHaveErr = true;//bit4:故障
bool GIsReady = true;//bit3:已就绪（可执行任务）
bool GIsInTask = true;//bit2:执行任务中(正在前往目标站)
bool GAgvPulse = true;//bit1:心跳，周期2S，脉宽1S
bool GOperateMode;//bit0:操作模式:0手动，1自动

bool GIsAutoStation = false;//bit7:备用 （后加的站属性）
bool GFinishCharge = true;//bit5:充电结束
bool GPrepareToCharge = true;//bit4:准备充电
bool GManualMode = true;//bit3:手动模式
bool GIsStop = false;//地标不对，急停

int GCycleId = 0;//记录循环号

bool GAllowToRun = false;//允许运行
bool GPLCIsConnectSuccess = false;//PLC连接成功而且能读到数据


int GCwStatusTemp = 0;//记录上一次成功写入PLC的状态字

int GCiWifiStatus = 0;//WIFI状态 0：连接断开 1：正常在线
bool GSendWifiStatusFlag = false;//发送WIFI状态

QSettings* GConfigIni = new QSettings("/agvs/agvsConfig.ini",QSettings::IniFormat);
int GPLCPort = GConfigIni->value("baseinfo/AgvPost").toInt();
QString GPLCIPAddress = "192.168.2." + QString::number(GConfigIni->value("baseinfo/ViNum").toInt());//PLC的IP
QString GServerUrl = GConfigIni->value("baseinfo/UrlAddress").toString();//服务器url



/******************日志打印级别函数****************************/
void outputMessage(QtMsgType type,const QMessageLogContext &context,const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text = "";
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        abort();
    }
    if(text != QString("Warning:")){
        QString message = QString("[%1] %2 %3").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(text).arg(msg);
        QDateTime time = QDateTime::currentDateTime();
        QString date = time.toString("yyyy-MM-dd");

        QFile file(QString("/agvs/log/")+date+QString(".log"));
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << message << endl;
        file.flush();
        file.close();
    }
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //注册MessageHandler
    qInstallMessageHandler(outputMessage);
    qDebug()<<versionNUM;
    //MainWindow w;
    //w.show();

    //公司测试PLC
#if 0
    GPLCPort = 102;
    GPLCIPAddress = "192.168.1.150";
#endif
    qRegisterMetaType<taskStruct>("taskStruct");
    AGVSCtrl agvsCtrl;
    PLCConnect plcConnect;
    pulseCheck pulseChe;
    serverConnect servConnect;

    QObject::connect(&servConnect,SIGNAL(sendUpdateTime(QString)),&agvsCtrl,SLOT(updateTime(QString)));
    QObject::connect(&pulseChe,SIGNAL(sendPulseStop(bool)),&plcConnect,SLOT(pulseStop(bool)));
    QObject::connect(&servConnect,SIGNAL(sendTask(taskStruct)),&plcConnect,SLOT(getTask(taskStruct)));

    QTimer::singleShot(0,&agvsCtrl,SLOT(agvsCtrlStart()));
    QTimer::singleShot(500,&plcConnect,SLOT(plcStart()));
    QTimer::singleShot(0,&pulseChe,SLOT(rePlcConnect()));
    QTimer::singleShot(30000,&servConnect,SLOT(serverStart()));

    return a.exec();
}


