#include "serverconnect.h"
#include <QDebug>
#include <QUrl>
#include <QTimer>
#include <QJsonParseError>
#include <QTextCodec>
#include <QList>
#include <QJsonArray>
#include <QMutex>
#include "mymsleep.h"

serverConnect::serverConnect(QObject *parent) : QObject(parent)
{
    this->moveToThread(&serverThread);
    serverThread.start();
    qDebug()<<"this server url"<<GServerUrl;
}

/*******************************************************/
//进程开始
/*******************************************************/
void serverConnect::serverStart()
{
    postErrorCount = 0;//请求错误次数
    postErrorAllCount = 0;//请求错误次数超时有几次
    datetime = "";
    postAndReceState = "initial";//请求与接收的状态
    newTime = true;//开机同步一次服务器时间
    firstPost = true;//开机post
    firstControl = false;//开机一次不检测控制字

    cycleIDTemp = 0;//记录本次循环号
    IsAutoStationTemp = false;//记录本次站属性
    FinishChargeTemp = true;//记录本次充电结束状态
    PrepareToChargeTemp = false;//记录本次准备充电状态
    ManualModeTemp = false;//记录本次手动模式状态
    AllowToRunTemp = false;//记录本次允许状态
    ManualControlTemp = 0;//记录本次手动控制字
    ManualBranchTemp = 0;//记录本次手动控制分支方向
    TargetStationIDToWriteTemp = 0;//记录本次目标站
    TargetDirectionToWriteTemp = 0;//记录本次目标方向
    SpeedModeTemp = 0;//记录本次速度模式
    ciSpeed_ATemp = 0;//记录本次A轮子速度
    ciSpeed_BTemp = 0;//记录本次B轮子速度
    BranchTagTemp = 0;//记录本次分支站
    BranchDirectionTemp = 0;//记录本次分支方向
    ShieldTemp = 0;//记录本次光栅
    SpeedTemp = 0;//记录本次速度
    firstPostSucessFlag = false;//开机第一次post成功

    request.setUrl(GServerUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));

    handdata();
}

/*******************************************************/
//获取服务器的数据
/*******************************************************/
void serverConnect::replyFinished(QNetworkReply *reply)
{
    postAndReceState = "receStart";
    if(reply->error()== QNetworkReply::NoError){
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QString recMessage = codec->toUnicode(reply->readAll());//data is json type.Need to translate to what we need
        QByteArray messageBuf = recMessage.toLatin1();
        qDebug()<<"this is source message"<<messageBuf<<messageBuf.size();
        if(messageBuf.size() > 2){
            GPostValid = true;
            if(firstControl){//第一次开机不检测控制字，必须获取服务器数据
                int myCount = 0;
                while(GViControl == 0){//上次写给PLC的任务还没结束，先等一会再处理
                    if(myCount > 1*1000){
                        break;
                    }
                    myMsleep::mSleep(1);
                    myCount++;
                }
                handReceMessage(messageBuf);//解析服务器发来的数据
            }
            firstControl = true;
        }else{
            GPostValid = false;
        }
        GPostRecFlag = true;
    }else{
        qDebug()<<"this is receive httpMessage error"<<reply->errorString();
        reply->abort();
    }
    reply->deleteLater();
    postAndReceState = "receOFF";
}


/*******************************************************/
//同步小车时钟 并将收集到的数据发到服务器的逻辑循环
/*******************************************************/
void serverConnect::handdata()
{
    int cycle = 500;
    if(GEnableSendToHttp){//开机读取到PLC缓存的上次写入DWB300的内容 一次性
        QJsonDocument sendJsonDocu;
        sendJsonDocu.setObject(handSendMessage());
        QByteArray array = sendJsonDocu.toJson(QJsonDocument::Compact);
        if(GPostRecFlag){//post连接正常
            if(postAndReceState.contains("receOFF") || postAndReceState.contains("initial")){
                if(postAndReceState.contains("receOFF")){
                    if(0 == GCiWifiStatus){//刚开机第一次GPostRecFlag=true是为了开机要post
                        GCiWifiStatus = 1;
//                        GIsNewCycle = true;
                        GSendWifiStatusFlag = true;//网络状态变化需要发PLC
                    }
                }
                qDebug()<<"send to http:"
                       <<"CycleId:"<<GCycleId<<"IsRunning:"<<GIsRunning<<"IsCharging:"<<GIsCharging
                      <<"IsEmpty:"<<GIsEmpty<<"IsHaveErr:"<<GIsHaveErr<<"IsReady:"<<GIsReady
                     <<"IsInTask:"<<GIsInTask<<"AgvPulse:"<<GAgvPulse<<"OperateMode:"<<GOperateMode
                    <<"ViNum:"<<GViNum<<"ViCurStation:"<<GViCurStation<<"ViRunStatus:"<<GViRunStatus
                   <<"ViCurDestination:"<<GViCurDestination<<"ViDestDirection:"<<GViDestDirection<<"ViSpeed:"<<GViSpeed
                  <<"ViBranStation:"<<GViBranStation<<"ViErrorCode:"<<GViErrorCode<<"ViControl:"<<GViControl
                 <<"ViBatteryVol:"<<GViBatteryVol<<"ViBatteryCul:"<<GViBatteryCul<<"ViBatteryQ:"<<GViBatteryQ
                <<"ViChooseshield:"<<GViChooseshield<<"CwStatusTemp:"<<GCwStatusTemp;
                //postAndReceState = "postStart";
                //firstPost = false;
                GPostRecFlag = false;
                GPostValid = false;
                postAndReceState = "postOFF";
                postErrorCount = 0;
                postErrorAllCount = 0;
                manager->post(request,array);//发送数据到服务器
            }
        }else{
            postErrorCount++;
            if(postErrorAllCount >= 20/4){//20S还接不到就重新连接了
                if(1 == GCiWifiStatus){
                    GCiWifiStatus = 0;
//                    GIsNewCycle = true;
                    GSendWifiStatusFlag = true;//网络状态变化需要发PLC
                }
                postErrorAllCount = 0;
                manager->destroyed();
                //QThread::sleep(1);
                myMsleep::mSleep(1);
                delete manager;
                manager = new QNetworkAccessManager(this);
                connect(manager, SIGNAL(finished(QNetworkReply*)),
                        this, SLOT(replyFinished(QNetworkReply*)));
                qDebug()<<"this is receive outtime error";
            }
            if(postErrorCount >= 4*(1000/cycle)){//4S超时
                qDebug()<<"post timeout:"
                       <<"CycleId:"<<GCycleId<<"IsRunning:"<<GIsRunning<<"IsCharging:"<<GIsCharging
                      <<"IsEmpty:"<<GIsEmpty<<"IsHaveErr:"<<GIsHaveErr<<"IsReady:"<<GIsReady
                     <<"IsInTask:"<<GIsInTask<<"AgvPulse:"<<GAgvPulse<<"OperateMode:"<<GOperateMode
                    <<"ViNum:"<<GViNum<<"ViCurStation:"<<GViCurStation<<"ViRunStatus:"<<GViRunStatus
                   <<"ViCurDestination:"<<GViCurDestination<<"ViDestDirection:"<<GViDestDirection<<"ViSpeed:"<<GViSpeed
                  <<"ViBranStation:"<<GViBranStation<<"ViErrorCode:"<<GViErrorCode<<"ViControl:"<<GViControl
                 <<"ViBatteryVol:"<<GViBatteryVol<<"ViBatteryCul:"<<GViBatteryCul<<"ViBatteryQ:"<<GViBatteryQ
                <<"ViChooseshield:"<<GViChooseshield<<"CwStatusTemp:"<<GCwStatusTemp;
                postErrorAllCount++;
                postErrorCount = 0;
                GPostRecFlag = false;
                GPostValid = false;
                postAndReceState = "postOFF";
                manager->post(request,array);
            }
        }
    }
    QTimer::singleShot(cycle,this,SLOT(handdata()));
}


/*******************************************************/
//处理准备发到服务器的数据
/*******************************************************/
QJsonObject serverConnect::handSendMessage()
{
    /*
    *CycleID	int	命令循环号
    *IP	string	IP地址
    *Bit15	bool	备用位
    *Bit14	bool	备用位
    *Bit13	bool	备用位
    *Bit12	bool	备用位
    *Bit11	bool	备用位
    *Bit10	bool	备用位
    *Bit9	bool	备用位
    *Bit8	bool	备用位
    *IsRunning	bool	运动中
    *IsCharging	bool	充电中
    *IsEmpty	bool	携带料车
    *IsHaveErr	bool	故障信号
    *IsReady	bool	已就绪
    *IsInTask	bool	执行任务中
    *AgvPulse	bool	小车心跳
    *OperateMode	bool	操作模式，false手动，true自动
    *CarID	int	小车编号
    *LocatedStationID	int	当前地标
    *RunningDirection	int	运动方向
    *TargetStationID	int	目标地标
    *TargetDirection	int	目标方向
    *Speed	int	当前速度
    *BranchStationID	int	分支地标
    *BranchDirection	int	分支方向
    *ErrorCode	int	故障代码
    *ViControl	int	交互控制字
    *Voltage	string	电压
    *Current	string	电流
    *BatteryQ	string	电量
    *Shield	int	屏蔽程序号
    *CwStatus	int	写入状态字
    */
    QJsonObject sendaJsonMessage;
    sendaJsonMessage.insert("CycleID",GCycleId);//命令循环号
    sendaJsonMessage.insert("IP","192.168.31.103");//IP地址
    sendaJsonMessage.insert("IsRunning",GIsRunning);//运动中
    sendaJsonMessage.insert("IsCharging",GIsCharging);//充电中
    sendaJsonMessage.insert("IsEmpty",GIsEmpty);//携带料车
    sendaJsonMessage.insert("IsHaveErr",GIsHaveErr);//故障信号
    sendaJsonMessage.insert("IsReady",GIsReady);//已就绪
    sendaJsonMessage.insert("IsInTask",GIsInTask);//执行任务中
    sendaJsonMessage.insert("AgvPulse",GAgvPulse);//小车心跳
    sendaJsonMessage.insert("OperateMode",GOperateMode);//操作模式，false手动，true自动
    sendaJsonMessage.insert("CarID",GViNum);//小车编号
    sendaJsonMessage.insert("LocatedStationID",GViCurStation);//当前地标
    sendaJsonMessage.insert("RunningDirection",GViRunStatus);//运动方向
    sendaJsonMessage.insert("TargetStationID",GViCurDestination);//目标地标
    sendaJsonMessage.insert("TargetDirection",GViDestDirection);//目标方向
    sendaJsonMessage.insert("Speed",GViSpeed);//当前速度
    sendaJsonMessage.insert("BranchStationID",GViBranStation);//分支地标
    sendaJsonMessage.insert("BranchDirection",GViBranDirection);//分支方向
    sendaJsonMessage.insert("ErrorCode",GViErrorCode);//故障代码
    sendaJsonMessage.insert("ViControl",GViControl);//交互控制字
    sendaJsonMessage.insert("Voltage",GViBatteryVol);//电压
    sendaJsonMessage.insert("Current",GViBatteryCul);//电流
    sendaJsonMessage.insert("BatteryQ",GViBatteryQ);//电量
    sendaJsonMessage.insert("Shield",GViChooseshield);//屏蔽程序号
    sendaJsonMessage.insert("CwStatus",GCwStatusTemp);//写入状态字（上一次写入PLC的状态）
    return sendaJsonMessage;
}


/*******************************************************/
//解析服务器发来的数据
/*******************************************************/
/*
*CycleID	int	命令循环号
*Bit15ToWrite	bool	预留位
*Bit14ToWrite	bool	预留位
*Bit13ToWrite	bool	预留位
*Bit12ToWrite	bool	预留位
*Bit11ToWrite	bool	预留位
*Bit10ToWrite	bool	预留位
*Bit9ToWrite	bool	预留位
*Bit8ToWrite	bool	预留位
*IsAutoStation	bool	目标站点是否是精定位站点
*FinishCharge	bool	结束充电
*PrepareToCharge	bool	准备充电
*ManualMode	bool	手动模式
*CrossMaterialRack	bool	穿过料架
*PcPulse	bool	PC心跳
*AllowToRun	bool	允许运行
*ManualControl	int	手动控制
*ManualBranch	int	手动控制分支方向
*CiControl	int	交互控制字
*TargetStationIDToWrite	int	要写入的目标交通站
*TargetDirectionToWrite	int	要写入的目标方向
*ReleaseMaterialCar	int	释放料车
*SpeedMode	int	速度控制模式,1同步速度2独立速度
*SpeedLeft	int	左轮速度
*SpeedRight	int	右轮速度
*Speed	int	速度
*Shield	int	要写入的屏蔽程序
*BranchTag	int	要写入的分支地标ID
*BranchDirection	int	要写入的分支方向
*/
void serverConnect::handReceMessage(QByteArray recByteMessage)
{
    QJsonParseError jsonError;
    QJsonDocument recDoucment = QJsonDocument::fromJson(recByteMessage, &jsonError);
    QJsonObject obj = recDoucment.object();
    QJsonValue CycleID = obj.value("CycleID");//命令循环号
    /*
            GCwStatus;//DBW300 PC 状态字节
            GCiManControl;//DBW302 PC 手动控制启停
            GCiManDirection;//DBW304 PC 手动控制分支方向
            GCiControl;//DBW306 PC 交互控制字
            GCiDestStation;//DBW308 PC 目标交通站
            GCiDestDirection;//DBW310 PC 目标方向
            GCiSpeed;//DBW312 当前速度
            GCiBranStation;//DBW314 PC 分支交通站
            GCiBranDirection;//DBW316 PC 分支方向
            GCiPartRelease;//DBW318 PC 释放料车
            GCiSpeedMode;//DBW320 速度控制模式
            GCiSpeed_A;//DBW322 PC A电机速度
            GCiSpeed_B;//DBW324 PC B电机速度
            GCiChooseshield;//DBW326 光栅范围
            */
    //下发新任务三个条件：小车不在充电状态，上次的循环号一致，小车在线状态
    //bool sendTaskFlag = false;
    if(CycleID.toInt() >= cycleIDTemp){//循环号正常（大于或等于上次都正常）
        QJsonValue IsAutoStation = obj.value("IsAutoStation");//目标站点是否是精定位站点
        GIsAutoStation = IsAutoStation.toBool();
        QJsonValue FinishCharge = obj.value("FinishCharge");//结束充电
        GFinishCharge = FinishCharge.toBool();
        QJsonValue PrepareToCharge = obj.value("PrepareToCharge");//准备充电
        GPrepareToCharge = PrepareToCharge.toBool();
        QJsonValue ManualMode = obj.value("ManualMode");//手动模式
        GManualMode = ManualMode.toBool();
        //QJsonValue CrossMaterialRack = obj.value("CrossMaterialRack");//穿过料架
        //QJsonValue PcPulse = obj.value("PcPulse");//PC心跳
        QJsonValue AllowToRun = obj.value("AllowToRun");//允许运行
        GAllowToRun = AllowToRun.toBool();
        QJsonValue ManualControl = obj.value("ManualControl");//手动控制
        allTask.GCiManControl = ManualControl.toInt();//DBW302 PC 手动控制启停
        QJsonValue ManualBranch = obj.value("ManualBranch");//手动控制分支方向
        allTask.GCiManDirection = ManualBranch.toInt();//DBW304 PC 手动控制分支方向
        //QJsonValue CiControl = obj.value("CiControl");//交互控制字
        QJsonValue TargetStationIDToWrite = obj.value("TargetStationIDToWrite");//要写入的目标交通站
        allTask.GCiDestStation = TargetStationIDToWrite.toInt();//DBW308 PC 目标交通站
        QJsonValue TargetDirectionToWrite = obj.value("TargetDirectionToWrite");//要写入的目标方向
        allTask.GCiDestDirection = TargetDirectionToWrite.toInt();//DBW310 PC 目标方向(目前没什么用)
        QJsonValue ReleaseMaterialCar = obj.value("ReleaseMaterialCar");//释放料车
        allTask.GCiPartRelease = ReleaseMaterialCar.toInt();//DBW318 PC 释放料车
        QJsonValue SpeedMode = obj.value("SpeedMode");//速度控制模式,1同步速度2独立速度
        allTask.GCiSpeedMode = SpeedMode.toInt();//DBW320 速度控制模式
        QJsonValue SpeedLeft = obj.value("SpeedLeft");//左轮速度
        allTask.GCiSpeed_A = SpeedLeft.toInt();//DBW322 PC A电机速度
        QJsonValue SpeedRight = obj.value("SpeedRight");//右轮速度
        allTask.GCiSpeed_B = SpeedRight.toInt();//DBW324 PC B电机速度
        QJsonValue SpeedToWrite = obj.value("Speed");//速度
        allTask.GCiSpeed = SpeedToWrite.toInt();//DBW312 当前速度
        QJsonValue Shield = obj.value("Shield");//要写入的屏蔽程序
        allTask.GCiChooseshield = Shield.toInt();//DBW326 光栅范围
        datetime = obj.value("Time").toString();//服务器发来的日期时间

        if(newTime){
            newTime = false;//开机同步一次服务器时间
            emit sendUpdateTime(datetime);
        }
        QJsonArray jsonArray = obj.value("LstBranch").toArray();
        //qDebug()<<"this is rev AllowToRun:"<<GAllowToRun<<GIsAutoStation;
        if(CycleID.toInt() > cycleIDTemp){//有新的任务下发
            if(recByteMessage.size() > 5){//查看会不会有收到一半数据的情况
                qDebug()<<"the cycleID change!AAAAAAAAAAAAAAAAAAAAAAAAaa"<<GCycleId<<CycleID.toInt();
                cycleIDTemp = CycleID.toInt();
                GCycleId = cycleIDTemp;
            }else{
                qDebug()<<"the cycleID change!BBBBBBBBBBBBBBBBBBBBBBBBaa"<<GCycleId<<CycleID.toInt();
            }
            allTask.GMessagePoint.clear();
            allTask.GMessagePoint.append(TargetStationIDToWrite.toInt());//目标交通站
            allTask.GMessagePoint.append(GViCurStation);//当前交通站

            allTask.GBranchTagList.clear();
            allTask.GBranchDirectionList.clear();
            for(int i = 0;i < jsonArray.size();i++){
                QJsonObject objec = jsonArray.at(i).toObject();
                int BranchTag = objec.value("BranchTag").toInt();//所有的转弯地标
                int BranchDirection = objec.value("BranchDirection").toInt();//所有的转弯方向
                allTask.GMessagePoint.append(BranchTag);//转弯点添加到所有地标列表中
                allTask.GBranchTagList.append(BranchTag);//转弯点添加到所有转弯列表中
                allTask.GBranchDirectionList.append(BranchDirection);//所有转弯方向添加到转弯方向列表中
            }
            GIsNewCycle = true;//新任务写PLC
        }

        //其他新任务与非新任务混合-------------------------------
        //手动模式下速度有变化就下发
        if(GManualMode){
            if(ciSpeed_ATemp != allTask.GCiSpeed_A || ciSpeed_BTemp != allTask.GCiSpeed_B){
                GIsNewCycle = true;
                qDebug()<<"this is ManualMode";
            }
            ciSpeed_ATemp = allTask.GCiSpeed_A;
            ciSpeed_BTemp = allTask.GCiSpeed_B;
        }

        //充电状态变化下发
        if(FinishChargeTemp != GFinishCharge){
            GIsNewCycle = true;
            FinishChargeTemp = GFinishCharge;
        }
        if(PrepareToChargeTemp != GPrepareToCharge){
            GIsNewCycle = true;
            PrepareToChargeTemp = GPrepareToCharge;
        }

        //使能变化下发
        if(AllowToRunTemp != GAllowToRun){
            GIsNewCycle = true;
            AllowToRunTemp = GAllowToRun;
        }

        //手动设置界面变化情况------------------------------------------start
        //1.任务地标变化
        if(TargetStationIDToWriteTemp != allTask.GCiDestStation){
            GIsNewCycle = true;
            TargetStationIDToWriteTemp = allTask.GCiDestStation;
        }
        if(allTask.GBranchDirectionList.size() > 0){
            //2.方向变化
            if(BranchDirectionTemp != allTask.GBranchDirectionList.at(0)){
                GIsNewCycle = true;
                BranchDirectionTemp = allTask.GBranchDirectionList.at(0);//手动下发任务修改方向直接赋值给PLC
            }
        }
        //3.光栅变化
        if(ShieldTemp != allTask.GCiChooseshield){
            GIsNewCycle = true;
            ShieldTemp = allTask.GCiChooseshield;
        }
        //4.速度变化
        if(SpeedTemp != allTask.GCiSpeed){
            GIsNewCycle = true;
            SpeedTemp = allTask.GCiSpeed;
        }
        //-----------------------------------------------------------end
        emit sendTask(allTask);
    }else{
        qDebug()<<"cycleError:"<<cycleIDTemp<<CycleID.toInt();
    }
}
