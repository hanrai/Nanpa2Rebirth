#include "stack.h"
#include <QDateTime>

Stack::Stack(Config *pConfig, QObject *parent) : QObject(parent)
{
    //init begin
    this->pConfig = pConfig;
    this->stackPointer = 0;
    QDateTime timestamp;
    this->random = timestamp.time().msec()
            + timestamp.time().second()
            + timestamp.time().minute()
            + timestamp.time().hour()
            + timestamp.date().day()
            + timestamp.date().dayOfWeek()
            + timestamp.date().month()
            + timestamp.date().year();
    //init end
}
