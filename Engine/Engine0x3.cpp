#include "engine.h"

void Engine::setFlags()
{
    quint16 index = 0;
    index = getMes();
    index |= getMes()<<8;

    do
    {
        QString cstr;
        Flag->flag[index] = Calculate(cstr);

        if(Flag->flag[index] > 0xF)
            Flag->flag[index] = 0xF;

        sendMessage(QString("Flags[%0] = %1;").arg(index).arg(cstr));
        index++;
    }while(getMes());
}
