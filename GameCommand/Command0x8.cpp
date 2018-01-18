#include "engine.h"
#include "../DrawingUtil/drawingutil.h"
#include "../MainWindow.h"
#include "Files/files.h"
#include "Animation/animationmanager.h"

void Engine::DrawBG()
{
    sendMessage(QString("DrawBackground(\"%0\");").arg((char*)subCommands[0].buffer));
    depth++;
    sendMessage(QString("vRam=%0").arg(Flag->dim16Bit.vRamIndex));
    depth--;
    quint32 palette[256];

    this->pAnimations->stopAllAnimagions();
    strcpy(filenamePic, (char*)subCommands[0].buffer);
    auto offset = Flag->dim32Bit.defmainOffset;
    auto pBuffer = gMainStruct.mesBuffer + offset;
    pFiles->ReadRawBG(filenamePic, pBuffer);
    VramInfo* pVram = &vrams[Flag->dim16Bit.vRamIndex];
    auto left = Flag->dim16Bit.posLeft;
    auto top = Flag->dim16Bit.posTop;
    pDrawing->decodeGP8(pBuffer, palette, pVram, top, left);
    Flag->dim16Bit.left = pDrawing->pBitmap->left;
    Flag->dim16Bit.top = pDrawing->pBitmap->top;
    Flag->dim16Bit.width = pDrawing->pBitmap->width;
    Flag->dim16Bit.height = pDrawing->pBitmap->height;
    if(Flag->dim16Bit.mainFlag & 1)
        memcpy(paletteBuffer, palette, 256*4);
    if(!Flag->dim16Bit.vRamIndex)
        pDrawing->updateWindow(Flag->dim16Bit.left,
                               Flag->dim16Bit.top,
                               Flag->dim16Bit.width,
                               Flag->dim16Bit.height);
}
