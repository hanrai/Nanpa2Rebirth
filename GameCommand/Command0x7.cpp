#include "engine.h"
#include "Files/files.h"

void Engine::loadDataMes()
{
        depth++;
    sendMessage(QString("LoadDataMesFile(%0)").arg((char*)subCommands[0].buffer));
    sendMessage(QString("   Offset=%0")
            .arg(subCommands[1].command));

    pFiles->LoadDataFile(QString((char*)subCommands[0].buffer),(char*)&gMainStruct.mesBuffer[subCommands[1].command]);

    depth--;
}
