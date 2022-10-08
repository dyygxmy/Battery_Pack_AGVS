#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H

#include <QString>
#include <QSettings>
#include <QList>
extern QString versionNUM;
extern QString GPLCIPAddress;//plc ip地址
extern QString GServerUrl;//服务器url
extern int GPLCPort;//plc端口号
extern int GPLCDB;//plc DB号
extern int GPLCDBWWRITE;//plc DBW写
extern int GPLCDBWREAD;//plc DBW读

//PLC发来的数据
extern int GVwStatus;//DBW200 小车 状态字节
extern int GViNum;//DBW202 小车 编号
extern int GViCurStation;//DBW204 小车 当前所在交通站编号
extern int GViRunStatus;//DBW206 小车 运动状态及方向
extern int GVrBatteryVol;//DBD208 小车 当前电池电压
extern int GViErrorCode;//DBW212 小车 故障代码
extern int GViControl;//DBW214 小车 交互控制字
extern int GVrBatteryCul;//DBD216 小车 当前电池电流
extern int GVrBatteryQ;//DBD220 小车 当前电池电量
extern int GViCurDestination;//DBW224 小车当前目标交通站编号
extern int GViDestDirection;//DBW226 目标方向 -1后退，0:NULL，1前进
extern int GViSpeed;//DBW228 当前速度
extern int GViBranStation;//DBW230 当前分支交通站
extern int GViBranDirection;//DBW232 当前分支方向。(面向运动方向来定义左右) -2左弯道，-1左分支，0直行，1右分支，2右弯道，3穿过
extern int GViSpeedMode;//DBW234 速度控制模式
extern int GViSpeed_A;//DBW236 表示当前A轮实时转速
extern int GViSpeed_B;//DBW238 表示当前B轮实时转速
extern int GViChooseshield;//DBW240 区域选择屏蔽
extern int GViBatteryVol;//DBW242 小车当前电池电压
extern int GViBatteryCul;// DBW244 小车当前电池电流
extern int GViBatteryQ;//DBW246 小车当前电池电量


extern bool GPostRecFlag;//post正常
extern bool GPostValid;//post有效（收到数据）
extern bool GEnableSendToHttp;//读取写入PLC的内容，收到回复
extern bool GIsNewCycle;//服务器有发新的任务来
extern bool GIsStop;//地标不对，急停
extern bool GIsFirstWriteViNum;


extern bool GIsRunning;//bit7 电机运转
extern bool GIsCharging;//bit6 充电
extern bool GIsEmpty;//bit5:携带料车
extern bool GIsHaveErr;//bit4:故障
extern bool GIsReady;//bit3:已就绪（可执行任务）
extern bool GIsInTask;//bit2:执行任务中(正在前往目标站)
extern bool GAgvPulse;//bit1:心跳，周期2S，脉宽1S
extern bool GOperateMode;//bit0:操作模式:0手动，1自动

extern bool GIsAutoStation;//bit7:备用 （后加的站属性）
extern bool GFinishCharge;//bit5:充电结束
extern bool GPrepareToCharge;//bit4:准备充电
extern bool GManualMode;//bit3:手动模式

//服务器发来的数据解析
extern int GCycleId;//记录循环号

extern bool GAllowToRun;//允许运行
extern QSettings* GConfigIni;

extern int GCwStatusTemp;//记录上一次成功写入PLC的状态字
extern bool GPLCIsConnectSuccess;//PLC连接成功而且能读到数据

struct taskStruct{
    //int GCwStatus;//DBW300 PC 状态字节
    int GCiManControl;//DBW302 PC 手动控制启停
    int GCiManDirection;//DBW304 PC 手动控制分支方向
    //int GCiControl;//DBW306 PC 交互控制字
    int GCiDestStation;//DBW308 PC 目标交通站
    int GCiDestDirection;//DBW310 PC 目标方向
    int GCiSpeed;//DBW312 当前速度
    //int GCiBranStation;//DBW314 PC 分支交通站
    //int GCiBranDirection;//DBW316 PC 分支方向
    int GCiPartRelease;//DBW318 PC 释放料车
    int GCiSpeedMode;//DBW320 速度控制模式
    int GCiSpeed_A;//DBW322 PC A电机速度
    int GCiSpeed_B;//DBW324 PC B电机速度
    int GCiChooseshield;//DBW326 光栅范围

    QList<int>GMessagePoint;//记录服务器发来的所有站点信息
    QList<int>GBranchTagList;//记录服务器发来的所有转弯点
    QList<int>GBranchDirectionList;//记录服务器发来的所有转弯点方向
};

extern int GCiWifiStatus;//WIFI状态 0：连接断开 1：正常在线
extern bool GSendWifiStatusFlag;//发送WIFI状态

#endif // GLOBALVARIABLE_H
