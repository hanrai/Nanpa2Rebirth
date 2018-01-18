#include "engine.h"
void Engine::setReg16Bits()
{
    int index = getMes();

    do
    {
        QString cstr;
        pReg16Bit[index++]=Calculate(cstr);
        sendMessage(QString("reg16Bit[%0] = %1;").arg(index-1).arg(cstr));
    }while(getMes());
}

