#include "engine.h"
void Engine::command_0x13()
{
    if(subCommands[0].command)
    {
        if(subCommands[0].command == 1);
        {
            //TODO:Maybe Fade out sound;

        }
    }
    else
        Flag->reg16Bit.moveResult = 1;
}
