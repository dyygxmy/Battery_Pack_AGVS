#include "mymsleep.h"
#include <QTime>

myMsleep::myMsleep()
{

}

myMsleep myMsleep::mSleep(unsigned int msec)
{
    QTime outTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < outTime);
}
