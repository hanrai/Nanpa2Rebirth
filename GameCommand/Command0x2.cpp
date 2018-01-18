#include "engine.h"
#include "MainWindow.h"
#include "Animation/animationmanager.h"

void Engine::cursorCommand()
{
    switch(subCommands[0].command)
    {
    case 0:
        sendMessage("ShowCursor();");
        emit showCursor();
        break;
    case 1:
        sendMessage("HideCursor();");
        emit hideCursor();
        isCursorVisible=0;
        break;
    case 3:
        sendMessage(QString("GetCursorPos(%0,%1);").arg(pMainWindow->cursorPos.x())
                    .arg(pMainWindow->cursorPos.y()));
        Flag->dim16Bit.cursorLeft = pMainWindow->cursorPos.x();
        Flag->dim16Bit.cursorTop = pMainWindow->cursorPos.y();
        break;
    case 4:
        sendMessage(QString("SetCursorPosotion(%0, %1); No use")
                    .arg(subCommands[1].command)
                .arg(subCommands[2].command));
        break;
    case 5:
        sendMessage(QString("SetCursorID(%0);")
                    .arg(subCommands[1].command));
        isAnimateCursor = 0;
        pAnimations->cursorIDShift = subCommands[1].command;
        pCursorID = subCommands[1].command;
        pMainWindow->SetCursorList(cursors[subCommands[1].command]);
        isAnimateCursor = 1;
        break;
    default:
        sendMessage("UnknownCursorCommand...");
    }
}
