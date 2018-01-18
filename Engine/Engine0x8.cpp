#include "engine.h"

void Engine::SetDim32Bits()
{
    QString cstr;
    quint32 index = Calculate(cstr);
    quint8 selector = getMes();
    quint32* pBase32=0;

    QString strBase;
    if(selector)
    {
        pBase32 = Flag->reg32Bit.addresses[selector-1];
        strBase = QString("reg32Bit[%0-1]").arg(selector);
    }
    else
    {
        strBase = "dim32Bit";
        pBase32 = pDim32Bit;
    }

    int i=0;//for debug
    do
    {
        QString cstr2;
        pBase32[index]=Calculate(cstr2);
        sendMessage(QString("%2[%0+%3]=%1;").arg(cstr).arg(cstr2).arg(strBase).arg(i));
        index++;
        i++;//for debug
    }while(getMes());
}
