#include "engine.h"

void Engine::DefineAnchors()
{
    union ByteWorker
    {
        quint8 byte[4];
        quint16 word[2];
        quint32 dword;
    };

    QString cstr;
    int anchorIndex = Calculate(cstr);

    ByteWorker offset;
    offset.byte[0]=getMes();
    offset.byte[1]=getMes();
    offset.byte[2]=getMes();
    offset.byte[3]=getMes();

    sendMessage(QString("mesAnchors[%0].mesCursor = %1;")
                .arg(cstr).arg(gStruct.gMesCursor));
    mesAnchors[anchorIndex].mesCursor=gStruct.gMesCursor;

    sendMessage(QString("mesAnchors[%0].mesStartPoint = %1;")
                .arg(cstr).arg(gStruct.mesStartPoint));
    mesAnchors[anchorIndex].mesStartPoint=gStruct.mesStartPoint;

    sendMessage(QString("gStruct.gMesCursor = %0;").arg(gStruct.mesStartPoint+offset.dword));
    gStruct.gMesCursor=gStruct.mesStartPoint+offset.dword;
}
