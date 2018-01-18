#include "engine.h"

void Engine::RunFromSavedAnchorPoint()
{
    sendMessage("AnchorRun()");
    depth++;
    if(Flag->dim16Bit.mainFlag&0x40)
        Flag->dim16Bit.mainFlag|=0x80;
    prepareSubCommands();

    if ( Flag->dim16Bit.mainFlag & 0x80 )
              Flag->dim16Bit.mainFlag &= 0xFF7Fu;
    RunFromSavedAnchorPoint(subCommands[0].command);

    depth--;
}
