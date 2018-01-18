#include "engine.h"

void Engine::mesJump()
{
    union ByteWorker
    {
        quint8 byte[4];
        quint16 word[2];
        quint32 dword;
    };

    QString cstr;
    bool selector=Calculate(cstr)==1;

    sendMessage(QString("if(!%0):%1;").arg(cstr).arg(selector));

    if(selector)
    {
        getMes(4);
        sendMessage("SkipDWORD();");
    }
    else
    {
        ByteWorker offset;
        offset.byte[0]=getMes();
        offset.byte[1]=getMes();
        offset.byte[2]=getMes();
        offset.byte[3]=getMes();

        gStruct.gMesCursor = gStruct.mesStartPoint + offset.dword;
        sendMessage(QString("   goto pos_%0;")
                    .arg(gStruct.gMesCursor));
    }
}
