#include "engine.h"
#include "global.h"
#include "Files/files.h"

void Engine::SoundOperations(SubCommand *)
{
    sendMessage(QString("SoundCommand(%0);").arg(subCommands[0].command));
    depth++;

    strcpy(filenameMusic, (char*)subCommands[1].buffer);
    if(!isMusicOn)
        return;
    switch(subCommands[0].command)
    {
    case 0:
        sendMessage(QString("PlayLoopSound(%0)").arg((char*)subCommands[1].buffer));
        emit BGMStarted(pFiles->LoadBGMFile((char*)subCommands[1].buffer), true);
        break;
    case 3:
        sendMessage(QString("PlaySoundFile(\"%0\");").arg((char*)subCommands[1].buffer));
        emit BGMStarted(pFiles->LoadBGMFile((char*)subCommands[1].buffer));
        break;
    default:
        sendMessage(QString("Unknown sound command:%0").arg(subCommands[0].command));
        break;
    }

    depth--;
}
