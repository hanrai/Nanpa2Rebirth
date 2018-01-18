#include "engine.h"

void Engine::setDim16Bits()
{
//debug
    QStringList dbg;
    dbg << "pgMainFlags"
        << "vRamIndex"
        << "field_4"
        << "mainFlag"
        << "cursorLeft"
        << "cursorTop"
        << "g_left"
        << "g_top"
        << "g_width"
        << "g_height"
        << "g_winLeft"
        << "g_winTop"
        << "colorIndex"
        << "byte_46209A"
        << "font_double_width"
        << "font_height"
        << "unknown_4620A0"
        << "winWidth"
        << "winHeight"
        << "posTop"
        << "posLeft"
        << "left"
        << "top"
        << "width"
        << "height"
        << "colorMaskIndex";
//debug end

    quint16 *base=0;

QString cstr;
    int index=Calculate(cstr);
    int selector = getMes();

QString strBase;
    if(selector)
    {
strBase = QString("&Flag->mesFileName[pReg16Bit[%0-1]]").arg(selector);
        base=(quint16*)(&Flag->mesFileName[pReg16Bit[selector-1]]);
    }
    else
    {
strBase = "dim16Bit";
        base=pDim16Bit;
    }

int i=0;
    do
    {
QString cstr2;
        base[index]=Calculate(cstr2);
if(base == pDim16Bit)
    if(index>dbg.size()-1)
        sendMessage(QString("Unknown dim16Bit index:%0").arg(index));
    else
        sendMessage(QString("dim16Bit:") + dbg[index] + "=" + cstr2);
else
    sendMessage(QString("%2[%0+%3] = %1;").arg(cstr).arg(cstr2).arg(strBase).arg(i));
        index++;
i++;
    }while(getMes());
}
