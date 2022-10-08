#include "plcconnect.h"
#include <QTimer>
#include <QDebug>
#include <QMutex>
#include "mymsleep.h"

/*
 * 交互数据长度设定：
    第一次握手：
        发：22
        回：22
    第二次握手：
        发：25
        回：27
    写数据：
        发：35+写长度(28)DBW300~DBW326
        回：22
    读数据：
        发：31
        回：25+读长度(48)DBW200~DBW246
 */

PLCConnect::PLCConnect(QObject *parent) : QObject(parent)
{
    this->moveToThread(&plcThread);
    plcThread.start();
}

/*******************************************************/
//进程开始
/*******************************************************/
void PLCConnect::plcStart()
{
    sendCmd = 0;//通讯序号
    DBWLen = 28;//写入多少(B)长度的DBW
    DBWRLen = 200;//读取多少(B)长度的DBW
    connectCount = 0;//连接PLC次数
    ctrlCycle = 0;//主控制周期
    PLCIsConnectTCP = false;//第二次连接成功
    allowReadFlag = false;//允许发读的指令（在发完写的指令后一断时间）
    //isWriteToPLC = false;//本次读指令是不是读上次写入PLC的数据
//    isFirstON = true;//刚开机
    isFistReceCycle = false;//开机收到新循环号
    isViControl = false;//状态控制中
    //directlyWrite = false;//直接写入PLC
    allowDebugReadCmd = true;//允许打印读指令
    dataIsChange = true;//plc数据有变化，用于去重复打印
    readPLCTemp = "";//记录本次读到的内容
    ViCurStationTemp = 0;//记录当前站点
    sendWriteDataTemp = "00";//记录本次写给PLC的内容
    curSendPLCCycle = 1;//1.发读DBW200 2.发写DBW300
    readOrWriteTemp = 0;//读1 写2
    //curServerCycleID = 0;//记录本次服务器的发来的循环号
    startCmdBuf1 = "\x03**\x16\x11\xE0***\x01*\xC1\x02\x10*\xC2\x02\x03\x01\xC0\x01\x0A";
    startCmdBuf2 = "\x03**\x19\x02\xF0\x80\x32\x01**\xCC\xC1*\x08**\xF0**\x01*\x01\x03\xC0";
    liveCount = 0;//记录PLC线程在跑，10s一次打印
    firstTaskFlag = false;//开机后第一次收到任务

    plcSocket = new QTcpSocket(this);
    QObject::connect(plcSocket,SIGNAL(disconnected()),this,SLOT(disPLCSocket()));

    for(int i = 0;i<100;i++){
        writeBuf[i] = 0;
    }
    writeBuf[8] = -1;//初始默认分支方向为-1
//    writeBuf[14] = 1;//初始默认网络状态为正常在线
    CwStatus = 0;
    plcConnect();//开机连接PLC
    ctrlPLC();
}

/*******************************************************/
//PLC断开后的逻辑处理
/*******************************************************/
void PLCConnect::disPLCSocket()
{
    qDebug()<<"plc is disConnect";
    PLCIsConnectTCP = false;
    GPLCIsConnectSuccess = false;
    plcConnect();//断线重连PLC
}


/*******************************************************/
//处理接收PLC的数据
/*******************************************************/
void PLCConnect::readPLCData(QString receMessage)
{
    int revLen = receMessage.size()/2;//48 25+DBWRLen
    //qDebug()<<"receMessage:"<<receMessage;
    if(22 == revLen){//第一次握手回复的03 00 00 16 11 D0 00 01 00 08 00 C0 01 0A C1 02 10 00 C2 02 03 01
        if(receMessage.mid(8,10) == "11d0000100"){
            qDebug()<<"ack-first"<<receMessage;
            writeCmdToPLC(startCmdBuf2,true);//发送第二次握手指令
        }else{//写指令的返回
            if(receMessage.mid(21*2,2)=="ff"){//写入数据回复的“成功” 成功FF，失败05
                if(4 == sendWriteDataTemp.size()){//一个值（占4byte）是写网络状态的回复
                    GSendWifiStatusFlag = false;//状态发完就不再发了
                }else{//其他（地标任务）写指令回复
                    GIsNewCycle = false;
                }
                dataIsChange = true;
                qDebug()<<"ack-w:"<<sendWriteDataTemp<<endl<<receMessage;
            }else if(receMessage.mid(21*2,2)=="05"){
                qDebug()<<"write fail"<<receMessage.size()<<receMessage;
            }else{
                qDebug()<<"write error"<<receMessage.size()<<receMessage;
            }
        }
    }else if(27 == revLen){//第二次握手回复03 00 00 1B 02 F0 80 32 03 00 00 CC C1 00 08 00 00 00 00 F0 00 00 01 00 01 00 F0
        if(receMessage == "0300001b02f08032030000ccc1000800000000f0000001000100f0"){
            qDebug()<<"ack-second";
            PLCIsConnectTCP = true;
        }
    }else if(revLen == 25 + DBWRLen){//读指令的返回
        //curServerCycleID = GCycleId;//数据正常交互可以赋值新的循环号
        if(receMessage.mid(21*2,2)=="ff"){//"ff" is success
            QString readBuf = receMessage.right(DBWRLen*2);
            //写过一次再打印读取的数据
            if(dataIsChange){
                dataIsChange = false;
                qDebug()<<"read PLC new data:"<<readBuf<<DBWRLen<<revLen;
            }
            handReveMessage(receMessage,true);//解析PLC发来的数据
            readPLCTemp = readBuf;
            GEnableSendToHttp = true;
        }else if(receMessage.mid(21*2,2)=="05"){
            qDebug()<<"read fail"<<receMessage.size()<<receMessage;
        }else{
            qDebug()<<"read error"<<receMessage.size()<<receMessage;
        }
    }else{
        qDebug()<<"outher ask:"<<receMessage.size()<<receMessage;
    }
}

/*******************************************************/
//连接PLC
/*******************************************************/
void PLCConnect::plcConnect()
{
    plcSocket->connectToHost(GPLCIPAddress,GPLCPort);
    plcSocket->waitForConnected(2000);

    if(plcSocket->state() == QAbstractSocket::ConnectedState){
        qDebug()<<"plc connect success";
        connectCount = 0;
        writeCmdToPLC(startCmdBuf1,true);//发送第一次握手指令
    }else{
        qDebug()<<"plc connect fail";
        if(connectCount < 10){//再给10次连接机会
            plcConnect();
        }else{
            return;
        }
        connectCount++;
    }
}


/*******************************************************/
//写入指令到PLC
/*******************************************************/
void PLCConnect::writeCmdToPLC(QByteArray sendBuf, bool type)
{
    int Tx_len=0,i;
    Tx_len = sendBuf.size();
    if(type){//第一次或第二次的握手包 格式需要转换
        for(i=0;i<Tx_len;i++){
            if(sendBuf[i]=='*'){
                sendBuf[i] = 0;
            }
        }
    }
    QByteArray sendData = sendBuf.right(DBWLen).toHex();
    if(sendBuf.size() == 22){
        qDebug()<<"send first cmd";//第一次握手
    }else if(sendBuf.size() == 25){
        qDebug()<<"send second cmd";//第二次握手
    }else if(sendBuf.size() == 31){
        if(allowDebugReadCmd){
            qDebug()<<"send read cmd:"<<sendBuf.toHex();//读指令
        }
    }else if(sendBuf.size() == 35+DBWLen){
        //if(sendWriteDataTemp != sendData){
        qDebug()<<"send write cmd:"<<sendData<<DBWLen;//写指令
        //}
        sendWriteDataTemp = sendData;
    }else{
        if(GSendWifiStatusFlag){//写wifi状态的指令
            sendWriteDataTemp = sendBuf.right(1*2).toHex();//1个值占2个db，4个byte
            qDebug()<<"outher sendBuf:"<<sendWriteDataTemp<<endl<<sendBuf.toHex();
        }else{
            qDebug()<<"outher sendBuf:"<<sendBuf.size()<<sendBuf.toHex();
        }
    }
    plcSocket->write(sendBuf,Tx_len);
    if(!plcSocket->waitForBytesWritten(1000)){
        qDebug()<<"send plc cmd faile:"<<plcSocket->errorString();
    }else{
        if(plcSocket->waitForReadyRead(1000)){
            //qDebug()<<"read plc data success";
            QString allData = QString(plcSocket->readAll().toHex());
            readPLCData(allData);
            ctrlCycle = 30;//有一个正常的交互，主循环不需要延迟(改30ms延迟，不影响性能)
        }else{
            qDebug()<<"read plc data faile:"<<plcSocket->errorString();
        }
    }
}

/*******************************************************/
//写入PLC的数据逻辑处理
/*******************************************************/
void PLCConnect::writeDataCtrl()
{
    if(ViCurStationTemp != GViCurStation && !GManualMode){//在自动模式下且地标有变化
        //读到非法地标急停及解除急停
        if(!GMessagePoint.contains(GViCurStation) && GMessagePoint.size() > 0){//当前地标不在地标列表中
            //当前地标非特殊地标就急停
            if(GViCurStation != 130 && GViCurStation != 131 && GViCurStation != 132 &&//光栅大小  数字越大光栅越小
                    GViCurStation != 110 && GViCurStation != 112 && GViCurStation != 122 ){//速度大小 数字越大速度越小
                qDebug()<<"this is message point error"<<GMessagePoint;
                GIsStop = true;
                GIsNewCycle = true;
                ViCurStationTemp = GViCurStation;
            }
        }

        //读到转弯点了将转弯信息发给PLC
        for(int i = 0;i < GBranchTagList.size();i++){
            if(GBranchTagList.at(i) == GViCurStation){
                writeBuf[7] = GBranchTagList.at(i);//DBW314 PC 分支交通站
                writeBuf[8] = GBranchDirectionList.at(i);//DBW316 PC 分支方向
                //directlyWrite = true;
                GIsNewCycle = true;
                ViCurStationTemp = GViCurStation;
                break;
            }
        }
    }
    if(GIsStop && GMessagePoint.contains(GViCurStation)){//重新给过地标，列表中有当前地标了，解锁放行
        GIsStop = false;
        GIsNewCycle = true;
    }

    bool thisFinishCharge = GFinishCharge;
    bool thisPrepareToCharge = GPrepareToCharge;
    //交互控制字赋值逻辑********************************************************
    qDebug()<<"GIsNewCycle:"<<GIsNewCycle<<GSendWifiStatusFlag<<GCiWifiStatus;
    if(firstTaskFlag){
        if(GSendWifiStatusFlag){//如果网络状态变化，这次直接写
            curSendPLCCycle = 2;
        }else{
            if(GIsNewCycle){//如果这次不是发送WIFI状态那才是正常发送任务的时候
                isViControl = false;//默认上次没有任务
                curSendPLCCycle = 2;
                if(GIsStop){//急停情况
                    writeBuf[3] = 2;//交互控制字 0:NULL 1:下发任务 2:立即停车
                }else{
                    if(1 == GViControl){//如果GViControl证明有任务进行中，发0结束任务后下次还要发一次1
                        writeBuf[3] = 0;//DBW306 PC 交互控制字
                        isViControl = true;
                    }else{
                        writeBuf[3] = 1;//DBW306 PC 交互控制字
                        isViControl = false;
                    }
                }
            }else{//本次不是新任务
                if(isViControl){//上次还有任务
                    if(1 == GViControl){//如果GViControl证明有任务进行中，发0结束任务后下次还要发一次1
                        writeBuf[3] = 0;//DBW306 PC 交互控制字
                    }else{
                        writeBuf[3] = 1;//DBW306 PC 交互控制字
                        curSendPLCCycle = 2;
                        isViControl = false;
                        //directlyWrite = true;
                    }
                }
            }
        }
    }



    //状态字赋值逻辑********************************************************
    /*DBW300 AGVPLC状态字
     * bit15:备用
     * bit14:备用
     * bit13:备用
     * bit12:备用
     * bit11:备用
     * bit10:备用
     * bit9:备用
     * bit8:备用
     * bit7:备用 （后加的站属性）
     * bit6:备用
     * bit5:充电结束
     * bit4:准备充电
     * bit3:手动模式
     * bit2:穿过料架；
     * bit1:心跳；周期2S，脉宽1S
     * bit0:允许运行。小车的运动使能
     */
    //根据服务器发来的信息来修改写入PLC的状态字数据
    if(thisFinishCharge){//bit5:充电结束
        CwStatus = (CwStatus | (0x01<<5));
    }else{
        CwStatus = (CwStatus & (~(0x01<<5)));
    }
    if(thisPrepareToCharge){//bit4:准备充电
        CwStatus = (CwStatus | (0x01<<4));
    }else{
        CwStatus = (CwStatus & (~(0x01<<4)));
    }
    if (GManualMode){//bit3:手动模式
        CwStatus = (CwStatus | (0x01<<3));
        CwStatus = (CwStatus | (0x01<<6));
    }else{
        CwStatus = (CwStatus & (~(0x01<<3)));
        CwStatus = (CwStatus & (~(0x01<<6)));
    }
    if(GAllowToRun){//bit0:允许运行 发给PLC，另站属性要任务结束后才能发
        CwStatus = CwStatus | 0x01;//bit0:允许运行 无条件为1
        if(!GIsInTask || GIsNewCycle){//bit2:执行任务中(正在前往目标站) || 来新任务
            if(GIsAutoStation){//bit7:站属性
                CwStatus = (CwStatus | (0x01<<7));
            }else{
                CwStatus = (CwStatus & (~(0x01<<7)));
            }
        }
    }else{//不允许运行发给PLC，站属性可以发
        CwStatus = CwStatus & (~0x01);//bit0:允许运行 无条件为0
        if(GIsAutoStation){//bit7:站属性
            CwStatus = (CwStatus | (0x01<<7));
        }else{
            CwStatus = (CwStatus & (~(0x01<<7)));
        }
    }
}


/*******************************************************/
//控制读写指令的逻辑循环
/*******************************************************/
void PLCConnect::ctrlPLC()
{
    ctrlCycle = 500;
    curSendPLCCycle = 1;//默认1（读）
    writeDataCtrl();
    if(readOrWriteTemp != curSendPLCCycle){
        allowDebugReadCmd = true;
        qDebug()<<"curSendPLCCycle:"<<curSendPLCCycle;
        readOrWriteTemp = curSendPLCCycle;
    }else{
        allowDebugReadCmd = false;
    }
    //qDebug()<<"curSendPLCCycle:"<<curSendPLCCycle<<PLCIsConnectTCP;
    if(PLCIsConnectTCP){//PLC连接正常
        if(2 == curSendPLCCycle){//发写指令
            writeBuf[0] = CwStatus<<8;//DBW300 PC 状态字节
            if(GSendWifiStatusFlag){//写给PLC网络状态
                writePLCFunc(GPLCDB,GPLCDBWWRITE+DBWLen,2,&GCiWifiStatus);
            }else{//写给PLC控制地标或急停
                writePLCFunc(GPLCDB,GPLCDBWWRITE,DBWLen,writeBuf);//DB号：2   写入DBW首地址:300 写入数据长度：28 写入数据内容
            }
        }else if(1 == curSendPLCCycle){//发读指令
            readPLCFunc(GPLCDB,GPLCDBWREAD,DBWRLen);//DB号：2   读取DBW首地址:200 读取长度：200
        }
    }
    QTimer::singleShot(ctrlCycle,this,SLOT(ctrlPLC()));
}

void PLCConnect::allowedToRead()
{
    allowReadFlag = true;
}

/*******************************************************/
//发读指令的数据整理
/*******************************************************/
void PLCConnect::readPLCFunc(int station,int addr,int len)
{
    QByteArray tempBuf = "\x03**\x1F\x02\xF0\x80\x32\x01***\x01*\x0E**\x04\x01\x12\x0A\x10\x02*\x30*\x02\x84**\x50";
    for(int i=0;i<tempBuf.size();i++){
        if(tempBuf[i]=='*'){
            tempBuf[i] = 0;
        }
    }

    tempBuf[12] = sendCmd;    //SN 通讯序号
    tempBuf[24] = len;        //read data length 长度
    tempBuf[25] = station>>8; //DB station high byte
    tempBuf[26] = station;    //DB station low byte DB号
    tempBuf[27] = 132;        //wr area  DB(132) M(133) I(129) Q(130) 区域
    tempBuf[29] = (addr<<3)>>8;//read high address
    tempBuf[30] = addr<<3;    //read low address 首地址 DBW*8

    tempBuf[3]  = tempBuf.size(); //cmd length 整个报文长度
    writeCmdToPLC(tempBuf,false);
    sendCmd ++;
}

/*******************************************************/
//发写指令的数据整理
/*******************************************************/
void PLCConnect::writePLCFunc(int station,int addr,int len,int *buf)
{
    QByteArray tempBuf = "\x03**\x25\x02\xF0\x80\x32\x01***\x06*\x0E*\x06\x05\x01\x12\x0A\x10\x04*\x01*\x02\x84**\x50*\x04*\x10*\x38";
    for(int i=0;i<tempBuf.size();i++){
        if(tempBuf[i]=='*'){
            tempBuf[i] = 0;
        }
    }

    tempBuf[12] = sendCmd; //SN 通讯序号
    tempBuf[16] = len + 4; //write data length 写入数据长度
    tempBuf[22] = 2;       //data type 数据类型
    tempBuf[23] = 0;       //data len high byte
    tempBuf[24] = len;     //data len low byte 固定
    tempBuf[25] = station>>8;  //DB station high byte
    tempBuf[26] = station;     //DB station low byte DB号
    tempBuf[27] = 132;     //wr area  DB(132) M(133) I(129) Q(130) 区域
    tempBuf[29] = (addr<<3)>>8;//write high address(addr*8)
    tempBuf[30] = addr<<3; //write low address(addr*8) 地址
    tempBuf[33] = (len<<3)>>8;//data len high byte(len*8)
    tempBuf[34] = len<<3;  //data len low byte(len*8) 数据类型
    for(int i=0;i<len/2;i++){
        tempBuf[35+2*i] = buf[i]>>8;
        tempBuf[35+2*i+1] = buf[i];
    }
    tempBuf[3]  = tempBuf.size(); //cmd length 整个报文长度
    qDebug()<<"writePLCFunc tempBuf:"
           <<buf[0]<<buf[1]<<buf[2]<<buf[3]<<buf[4]<<buf[5]<<buf[6]
          <<buf[7]<<buf[8]<<buf[9]<<buf[10]<<buf[11]<<buf[12]<<buf[13];
    writeCmdToPLC(tempBuf,false);
    sendCmd ++;
}

/*******************************************************/
//解析PLC发来的数据
/*******************************************************/
void PLCConnect::handReveMessage(QString recMessage,bool type)
{
    //qDebug()<<"recMessage from plc"<<type;
//    if(isFirstON){//是开机获取上次写入PLC的数据
//        qDebug()<<"recMessage from plc isFirstON";
//        GCwStatusTemp = recMessage.mid(25*2+2*(300-DBWRLen),2).toInt(0,16);//DBW300 上次发小车的状态字节
//        isFirstON = false;
//    }else{
        /*
     * DBW200 小车状态字 bit0手自动 bit1心跳 bit2执行任务中 bit3
     * DBW202 小车编号
     * DBW204 小车当前地标
     * DBW206 小车运动状态 -1后退 0未运行 1 前进
     * DBW208 小车电压
     * DBW212 小车故障代码
     * DBW214 小车交互控制字
     * DBW216 小车电流
     * DBW220 小车电量
    */
        //读取PLC数据内容为db25+，即byte50开始
        GVwStatus = qint16(recMessage.mid(25*2+2*(200-DBWRLen),2).toInt(0,16));//DBW200 小车 状态字节
        GViNum = qint16(recMessage.mid(25*2+2*(202-DBWRLen),4).toInt(0,16));//DBW202 小车 编号
        GViCurStation = qint16(recMessage.mid(25*2+2*(204-DBWRLen),4).toInt(0,16));//DBW204 小车 当前所在交通站编号
        GViRunStatus = qint16(recMessage.mid(25*2+2*(206-DBWRLen),4).toInt(0,16));//DBW206 小车 运动状态及方向
        GVrBatteryVol;//DBD208 小车 当前电池电压
        GViErrorCode = qint16(recMessage.mid(25*2+2*(212-DBWRLen),4).toInt(0,16));//DBW212 小车 故障代码
        GViControl = qint16(recMessage.mid(25*2+2*(214-DBWRLen),4).toInt(0,16));//DBW214 小车 交互控制字
        GVrBatteryCul = 0;//DBD216 小车 当前电池电流
        GVrBatteryQ = 0;//DBD220 小车 当前电池电量
        GViCurDestination = qint16(recMessage.mid(25*2+2*(224-DBWRLen),4).toInt(0,16));//DBW224 小车当前目标交通站编号
        GViDestDirection = qint16(recMessage.mid(25*2+2*(226-DBWRLen),4).toInt(0,16));//DBW226 目标方向 -1后退，0:NULL，1前进
        GViSpeed = qint16(recMessage.mid(25*2+2*(228-DBWRLen),4).toInt(0,16));//DBW228 当前速度
        GViBranStation = qint16(recMessage.mid(25*2+2*(230-DBWRLen),4).toInt(0,16));//DBW230 当前分支交通站
        GViBranDirection = qint16(recMessage.mid(25*2+2*(232-DBWRLen),4).toInt(0,16));//DBW232 当前分支方向。(面向运动方向来定义左右) -2左弯道，-1左分支，0直行，1右分支，2右弯道，3穿过
        GViSpeedMode = 0;//DBW234 速度控制模式
        GViSpeed_A = 0;//DBW236 表示当前A轮实时转速
        GViSpeed_B = 0;//DBW238 表示当前B轮实时转速
        GViChooseshield = qint16(recMessage.mid(25*2+2*(240-DBWRLen),4).toInt(0,16));//DBW240 区域选择屏蔽
        GViBatteryVol = qint16(recMessage.mid(25*2+2*(242-DBWRLen),4).toInt(0,16));//DBW242 小车当前电池电压
        GViBatteryCul = qint16(recMessage.mid(25*2+2*(244-DBWRLen),4).toInt(0,16));// DBW244 小车当前电池电流
        GViBatteryQ = qint16(recMessage.mid(25*2+2*(246-DBWRLen),4).toInt(0,16));//DBW246 小车当前电池电量
        GCwStatusTemp = recMessage.mid(25*2+2*(300-DBWRLen),2).toInt(0,16);//DBW300 上次发小车的状态字节

        /*
     * 小车状态字
     * bit15:备用
     * bit14:备用
     * bit13:备用
     * bit12:备用
     * bit11:备用
     * bit10:备用
     * bit9:备用
     * bit8:备用
     * bit7:运动中（电机在转动）
     * bit6:充电中
     * bit5:携带料车
     * bit4:故障
     * bit3:已就绪（可执行任务）
     * bit2:执行任务中(正在前往目标站)
     * bit1:心跳
     * bit0:操作模式:0手动，1自动
     */
        if(((GVwStatus>>7) & 0x0001))//bit7:运动中（电机在转动）
        {
            GIsRunning = true;
        }
        else
        {
            GIsRunning = false;
        }
        if(((GVwStatus>>6) & 0x0001))//bit6:充电中
        {
            GIsCharging = true;
        }
        else
        {
            GIsCharging = false;
        }
        if(((GVwStatus>>5) & 0x0001))//bit5:携带料车
        {
            GIsEmpty = true;
        }
        else
        {
            GIsEmpty = false;
        }
        if(((GVwStatus>>4) & 0x0001))//bit4:故障
        {
            GIsHaveErr = true;
        }
        else
        {
            GIsHaveErr = false;
        }
        if(((GVwStatus>>3) & 0x0001))//bit3:已就绪（可执行任务）
        {
            GIsReady = true;
        }
        else
        {
            GIsReady = false;
        }
        if(((GVwStatus>>2) & 0x0001))//bit2:执行任务中(正在前往目标站)
        {
            GIsInTask = true;
        }
        else
        {
            GIsInTask = false;
        }
        if(((GVwStatus>>1) & 0x0001))//bit1:心跳
        {
            GAgvPulse = true;
        }
        else
        {
            GAgvPulse = false;
        }
        if(GVwStatus & 0x0001)//bit0:操作模式:0手动，1自动
        {
            GOperateMode = true;
        }
        else
        {
            GOperateMode = false;
        }
        GPLCIsConnectSuccess = true;
//    }
}


void PLCConnect::pulseStop(bool isStop)
{
    if(isStop){
        qDebug()<<"plc pulse is stop";
        PLCIsConnectTCP = false;
        GPLCIsConnectSuccess = false;
        plcSocket->abort();//断开连接
    }else{
        PLCIsConnectTCP = true;
    }
}


void PLCConnect::getTask(taskStruct allTask)
{
    writeBuf[1] = allTask.GCiManControl;//DBW302 PC 手动控制启停
    writeBuf[2] = allTask.GCiManDirection;//DBW304 PC 手动控制分支方向
    //writeBuf[3] = allTask.GCiControl;//DBW306 PC 交互控制字
    writeBuf[4] = allTask.GCiDestStation;//DBW308 PC 目标交通站
    writeBuf[5] = allTask.GCiDestDirection;//DBW310 PC 目标方向
    writeBuf[6] = allTask.GCiSpeed;//DBW312 当前速度
    //writeBuf[7] = allTask.GCiBranStation;//DBW314 PC 分支交通站
    //writeBuf[8] = allTask.GCiBranDirection;//DBW316 PC 分支方向
    writeBuf[9] = allTask.GCiPartRelease;//DBW318 PC 释放料车
    writeBuf[10] = allTask.GCiSpeedMode;//DBW320 速度控制模式
    writeBuf[11] = allTask.GCiSpeed_A;//DBW322 PC A电机速度
    writeBuf[12] = allTask.GCiSpeed_B;//DBW324 PC B电机速度
    writeBuf[13] = allTask.GCiChooseshield;//DBW326 光栅范围

    GMessagePoint.clear();
    GMessagePoint.append(allTask.GMessagePoint);
    GBranchTagList.clear();
    GBranchTagList.append(allTask.GBranchTagList);
    GBranchDirectionList.clear();
    GBranchDirectionList.append(allTask.GBranchDirectionList);

    firstTaskFlag = true;//开机后第一次收到任务
}
