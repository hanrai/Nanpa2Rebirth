#include "engine.h"
#include "../DrawingUtil/drawingutil.h"
#include "MainWindow.h"
#include <QMutex>
#include <QWaitCondition>
#include <QColor>

void Engine::paletteOperation()
{
    sendMessage(QString("PaletteCommand(%0);").arg(subCommands[0].command));
    depth++;

    pDrawing->waitAnimating();

    if(Flag->dim16Bit.mainFlag&2)
        memcpy(anotherPaletteBuffer,paletteBuffer,sizeof(anotherPaletteBuffer));
    quint8 resetPaletteMark = 1;
    switch(subCommands[0].command)
    {
    case 1:
    {
        if(subCommandIndex > 1)
        {
            sendMessage("PaletteAnimation()");
            sendMessage(QString("bgColor=%0,%0,%0,%0").arg(subCommands[1].command));
            quint8 c = subCommands[1].command;
            memset(paletteBuffer, c, 236*4);
        }
        pDrawing->setTargetPalette(paletteBuffer);
        pDrawing->makeStepPalette(16);
        pDrawing->waitPaletteAnimating(0, 236);
    }
        break;
    case 2:
        sendMessage("PaletteAnimation2();");
        pDrawing->setTargetPalette(paletteBuffer);
        pDrawing->makeStepPalette(16);
        pDrawing->waitPaletteAnimating(
                subCommands[1].command,
                subCommands[2].command);
        break;
    case 5:
        sendMessage(QString("copyTargetPalette(start:%0,count:%1);")
                    .arg(subCommands[1].command)
                .arg(subCommands[2].command));
        {
            pDrawing->setTargetPalette(paletteBuffer);
            pDrawing->copyTargetPalette(subCommands[1].command,
                    subCommands[2].command);
        }
        break;
    case 7:
        sendMessage(QString("PaletteAnimation7"));
    {
        if(subCommandIndex > 2)
            memset(paletteBuffer, subCommands[2].command, 236*4);
        pDrawing->setTargetPalette(paletteBuffer);
        pDrawing->makeStepPalette(subCommands[1].command);
        pDrawing->waitPaletteAnimating(0, 236);

    }
        break;
    default:
        sendMessage(QString("Unknown Graphics Command:%0").arg(subCommands[0].command));
        break;
    }

    if(resetPaletteMark & Flag->dim16Bit.mainFlag)
        memcpy(paletteBuffer,anotherPaletteBuffer,sizeof(anotherPaletteBuffer));
    depth--;
}
