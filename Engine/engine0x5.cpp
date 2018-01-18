#include "engine.h"

void Engine::setFlags2()
{
    QString cstr;
    quint16 index=0;
    index=Calculate(cstr);
    do
    {
        Flag->flag[index]=Calculate(cstr);
        sendMessage(QString("Flags[%0] = %1;").arg(index).arg(cstr));
        index++;
    }while(getMes());
}

