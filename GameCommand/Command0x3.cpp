#include "engine.h"
#include "Animation/animationmanager.h"

void Engine::AnimationCommand()
{
    sendMessage(QString("ThreadCommand(%0);").arg(subCommands[0].command));
    depth++;
    QMutex m;
    switch(subCommands[0].command)
    {
    case 0:
        sendMessage("SetJobItemFromDatabase();");
        depth++;
        sendMessage(QString("JobItemID=%0").arg(subCommands[1].command));
        sendMessage(QString("Command2=%0").arg(subCommands[0].command));
        sendMessage(QString("databaseOffset=%0").arg(Flag->dim32Bit.animationOffset));
        depth--;
        pAnimations->PrepareAnimation(subCommands[1].command);
        break;
    case 1:
        sendMessage(QString("LoopAnimationJob:%0").arg(subCommands[1].command));
        pAnimations->animations[subCommands[1].command].bAnimation = -1;
        break;
    case 4:
        sendMessage("PlayAnimationWait();");
        sendMessage(QString("JobItemIndex=%0").arg(subCommands[1].command));
        pAnimations->animations[subCommands[1].command].bAnimation = 2;
        m.lock();
        pAnimations->animations[subCommands[1].command].playing.wait(&m);
        m.unlock();
        break;
    case 6:
        sendMessage("StopAllAnimations;");
        for(Animation &a:pAnimations->animations)
            a.bAnimation = 0;
        break;
    case 7:
        sendMessage("PlayAllAnimations");
    {
        quint16 *offset = (quint16*)&gMainStruct.mesBuffer[Flag->dim32Bit.animationOffset+1];
        for(Animation &a:pAnimations->animations)
        {
            if(a.bAnimation)
                if(a.commandIndex)
                {
                    a.bAnimation = -1;
                    a.counter = 0;
                    a.pCommands = &gMainStruct.mesBuffer[Flag->dim32Bit.animationOffset+*offset];
                    a.commandIndex = 0;
                    a.itemIndexBookmark = 0;
                    a.itemIndexBookmark2 = 0;
                    a.DecCounter = 0;
                    a.DecCounter2 = 0;
                    a.field_1A = 0;
                    a.unknown = 0;
                    pAnimations->animationRunner(a);
                    a.bAnimation = -1;
                    a.counter = 0;
                    a.pCommands = &gMainStruct.mesBuffer[Flag->dim32Bit.animationOffset+*offset];
                    a.commandIndex = 0;
                    a.itemIndexBookmark = 0;
                    a.itemIndexBookmark2 = 0;
                    a.DecCounter = 0;
                    a.DecCounter2 = 0;
                    a.field_1A = 0;
                    a.unknown = 0;
                }
            offset++;
        }
    }
        break;
    default:
        sendMessage(QString("Unknown animation command:%0").arg(subCommands[0].command));
        break;
    }
    depth--;
}
