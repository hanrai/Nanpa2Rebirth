#include "engine.h"
#include <QFile>
#include "Files/files.h"

void Engine::ComChange()
{
depth++;
    gStruct.mesStartPoint = 4096;
    prepareSubCommands();
    sendMessage(QString("ComChange(\"%0\")").arg(QString((char*)subCommands[0].buffer)));
    strcpy(Flag->mesFileName,(char*)subCommands[0].buffer);
    strcpy(filenameMes, (char*)subCommands[0].buffer);
    pFiles->LoadMesFile(QString((char*)subCommands[0].buffer), (quint8*)gMainStruct.mesBuffer);
    Flag->dim16Bit.mainFlag|=0x10;
depth--;
}

