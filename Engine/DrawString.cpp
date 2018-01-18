#include "engine.h"
#include "DrawingUtil/drawingutil.h"

void Engine::DrawString()
{
    VramInfo* pVram = &vrams[Flag->dim16Bit.vRamIndex];
    auto width = Flag->dim16Bit.winWidth;
    auto height = Flag->dim16Bit.winHeight;
    auto left = Flag->dim16Bit.winLeft;
    auto top = Flag->dim16Bit.winTop;
    while(auto data = getMes())
    {
        quint16 c = data;
        c <<= 8;
        c |= getMes();

        pDrawing->drawChar(this, pVram, c,
                           Flag->dim16Bit.txtLeft,
                           Flag->dim16Bit.txtTop,
                           Flag->dim16Bit.drawTxtStyle);
        Flag->dim16Bit.txtLeft += Flag->dim16Bit.txtWidth;
        if(Flag->dim16Bit.txtLeft+Flag->dim16Bit.txtWidth > Flag->dim16Bit.winWidth)
        {
            Flag->dim16Bit.txtLeft = Flag->dim16Bit.winLeft;
            Flag->dim16Bit.txtTop += Flag->dim16Bit.txtHeight;
        }
        if(Flag->dim16Bit.txtHeight + Flag->dim16Bit.txtTop > Flag->dim16Bit.winHeight)
        {
            Flag->dim16Bit.txtLeft = Flag->dim16Bit.winLeft;
            Flag->dim16Bit.txtTop = Flag->dim16Bit.winTop;
        }
    }
    sendMessage("OutputText1");
    if(!Flag->dim16Bit.vRamIndex)
        pDrawing->updateWindow(left,
                               top,
                               width-left+1,
                               height-top+1);
}
