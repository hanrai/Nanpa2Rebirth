#include "engine.h"

void Engine::runMes()
{
    sendMessage(QString("RunMes(at:%0);").arg(subCommands[0].command));
    depth++;

    int backupMesCursor = gStruct.gMesCursor;
    sendMessage(QString("backupMesCursor = %0;").arg(backupMesCursor));
    int backupStartPoint = gStruct.mesStartPoint;
    sendMessage(QString("backupStartPoint = %0;").arg(backupStartPoint));

    gStruct.gMesCursor=subCommands[0].command;
    gStruct.mesStartPoint=subCommands[0].command;

    StartEngine();

    sendMessage(QString("MainFlag=%0").arg(Flag->dim16Bit.mainFlag));
    if(!(Flag->dim16Bit.mainFlag&0x10))
    {
        sendMessage("RestoreCursor();");
        gStruct.gMesCursor = backupMesCursor;
        gStruct.mesStartPoint = backupStartPoint;
    }

    depth--;
}
