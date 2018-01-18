#include "engine.h"

void Engine::nakedJump()
{
    union ByteWorker
    {
        quint8 byte[4];
        quint16 word[2];
        quint32 dword;
    };
    ByteWorker offset;
    offset.byte[0]=getMes();
    offset.byte[1]=getMes();
    offset.byte[2]=getMes();
    offset.byte[3]=getMes();

    gStruct.gMesCursor = gStruct.mesStartPoint + offset.dword;
    sendMessage(QString("goto pos_%0;")
                    .arg(gStruct.gMesCursor));
}
