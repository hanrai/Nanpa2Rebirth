#include "engine.h"
#include <QFile>
#include "Files/files.h"

void Engine::CallMes()
{
depth++;
    char filename[128];
    gStruct.mesStartPoint = 4096;
    prepareSubCommands();
    auto backupCursor = gStruct.gMesCursor;
    sendMessage(QString("CallMes(\"%0\")").arg(QString((char*)subCommands[0].buffer)));
    strcpy(filename, Flag->mesFileName);
    strcpy(Flag->mesFileName,(char*)subCommands[0].buffer);
    strcpy(filenameMes, (char*)subCommands[0].buffer);
    gStruct.gMesCursor = 4096;
    pFiles->LoadMesFile(QString((char*)subCommands[0].buffer), (quint8*)gMainStruct.mesBuffer);
    StartEngine();
    if(!(Flag->dim16Bit.mainFlag & 0x10))
    {
        strcpy(Flag->mesFileName,filename);
        strcpy(filenameMes, filename);
        gStruct.gMesCursor = backupCursor;
        pFiles->LoadMesFile(QString(filename), (quint8*)gMainStruct.mesBuffer);
    }
depth--;
}
