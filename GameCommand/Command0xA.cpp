#include "engine.h"
#include "../DrawingUtil/drawingutil.h"
#include <QMutex>
#include "MainWindow.h"

void Engine::painting()
{
    sendMessage(QString("Painting(%0);").arg(subCommands[0].command));
    depth++;

    VramInfo* pVram;
    quint32 targetVramIndes = 0;

    switch(subCommands[0].command)
    {
    case 0:
        sendMessage(QString("PaintingImgToTarget();"));
        depth++;
        sendMessage(QString("SourceVRam=%0").arg(subCommands[5].command));
        sendMessage(QString("TargetVRam=%0").arg(subCommands[8].command));
        sendMessage(QString("Width=%0")
                    .arg(subCommands[3].command-subCommands[1].command+1));
        sendMessage(QString("Left=%0").arg(subCommands[1].command));
        sendMessage(QString("Top=%0").arg(subCommands[2].command));
        sendMessage(QString("Height=%0")
                    .arg(subCommands[4].command-subCommands[2].command+1));
        sendMessage(QString("TargetTop=%0").arg(subCommands[7].command));
        sendMessage(QString("TargetLeft=%0").arg(subCommands[6].command));
        depth--;
        pDrawing->paintingImg(0,
                subCommands[1].command,
                subCommands[2].command,
                subCommands[3].command-subCommands[1].command+1,
                subCommands[4].command-subCommands[2].command+1,
                &vrams[subCommands[5].command],
                subCommands[6].command,
                subCommands[7].command,
                &vrams[subCommands[8].command],
                0,0,0,0);
        targetVramIndes = subCommands[8].command;
        break;
    case 1:
        sendMessage(QString("PaintingImgToTargetWithMask();"));
        depth++;
        sendMessage(QString("SourceVRam=%0").arg(subCommands[5].command));
        sendMessage(QString("TargetVRam=%0").arg(subCommands[8].command));
        sendMessage(QString("Width=%0")
                    .arg(subCommands[3].command-subCommands[1].command+1));
        sendMessage(QString("Left=%0").arg(subCommands[1].command));
        sendMessage(QString("Top=%0").arg(subCommands[2].command));
        sendMessage(QString("Height=%0")
                    .arg(subCommands[4].command-subCommands[2].command+1));
        sendMessage(QString("TargetTop=%0").arg(subCommands[7].command));
        sendMessage(QString("TargetLeft=%0").arg(subCommands[6].command));
        depth--;
        pDrawing->paintingImg(1,
                subCommands[1].command,
                subCommands[2].command,
                subCommands[3].command-subCommands[1].command+1,
                subCommands[4].command-subCommands[2].command+1,
                &vrams[subCommands[5].command],
                subCommands[6].command,
                subCommands[7].command,
                &vrams[subCommands[8].command],
                0,0,0,
                Flag->dim16Bit.colorMaskIndex);
        targetVramIndes = subCommands[8].command;
        break;
    case 2:
        sendMessage(QString("FillBackground();"));
        depth++;
        sendMessage(QString("vRamIndex=%0;").arg(Flag->dim16Bit.vRamIndex));
        sendMessage(QString("left=%0;").arg(subCommands[1].command));
        sendMessage(QString("top=%0;").arg(subCommands[2].command));
        sendMessage(QString("width=%0;").arg(subCommands[3].command-subCommands[1].command));
        sendMessage(QString("height=%0;").arg(subCommands[4].command-subCommands[2].command));
        depth--;
        pVram = &vrams[Flag->dim16Bit.vRamIndex];
        pDrawing->paintingImg(2,
                subCommands[1].command,
                subCommands[2].command,
                subCommands[3].command-subCommands[1].command+1,
                subCommands[4].command-subCommands[2].command+1,
                pVram,
                0,0,0,0,0,0,
                Flag->dim16Bit.colorIndex);
        targetVramIndes = Flag->dim16Bit.vRamIndex;
        break;
    case 4:
        sendMessage(QString("drawHighlight();"));
        depth++;
        sendMessage(QString("vRamIndex=%0;").arg(Flag->dim16Bit.vRamIndex));
        sendMessage(QString("left=%0;").arg(subCommands[1].command));
        sendMessage(QString("top=%0;").arg(subCommands[2].command));
        sendMessage(QString("width=%0;").arg(subCommands[3].command-subCommands[1].command));
        sendMessage(QString("height=%0;").arg(subCommands[4].command-subCommands[2].command));
        depth--;
        pVram = &vrams[Flag->dim16Bit.vRamIndex];
        pDrawing->paintingImg(4,
                subCommands[1].command,
                subCommands[2].command,
                subCommands[3].command-subCommands[1].command+1,
                subCommands[4].command-subCommands[2].command+1,
                pVram,
                0,0,0,0,0,0,
                Flag->dim16Bit.colorIndex);
        subCommands[6].command = subCommands[1].command;
        subCommands[7].command = subCommands[2].command;
        targetVramIndes = Flag->dim16Bit.vRamIndex;
        break;
    case 5:
        sendMessage(QString("slideEffect();"));
        depth++;
        sendMessage(QString("SourceVRam=%0").arg(subCommands[5].command));
        sendMessage(QString("TargetVRam=%0").arg(subCommands[8].command));
        sendMessage(QString("Width=%0")
                    .arg(subCommands[3].command-subCommands[1].command+1));
        sendMessage(QString("Left=%0").arg(subCommands[1].command));
        sendMessage(QString("Top=%0").arg(subCommands[2].command));
        sendMessage(QString("Height=%0")
                    .arg(subCommands[4].command-subCommands[2].command+1));
        sendMessage(QString("TargetTop=%0").arg(subCommands[7].command));
        sendMessage(QString("TargetLeft=%0").arg(subCommands[6].command));
        depth--;
        pDrawing->paintingImg(5,
                subCommands[1].command,
                subCommands[2].command,
                subCommands[3].command-subCommands[1].command+1,
                subCommands[4].command-subCommands[2].command+1,
                &vrams[subCommands[5].command],
                subCommands[6].command,
                subCommands[7].command,
                &vrams[subCommands[8].command],
                0,0,0,0);
        targetVramIndes = subCommands[8].command;
        break;
    default:
        sendMessage(QString("Unknown painting command:%0").arg(subCommands[0].command));
        break;
    }
        depth--;
    if ( Flag->dim16Bit.mainFlag & 0x20 )
    {
        if ( targetVramIndes )
            return;
        pDrawing->updateWindow(
            subCommands[6].command,
            subCommands[7].command,
            subCommands[3].command - subCommands[1].command + 1,
            subCommands[4].command - subCommands[2].command + 1);
        QMutex m;
        m.lock();
        pMainWindow->vSync.wait(&m,2);
        m.unlock();
    }

}
