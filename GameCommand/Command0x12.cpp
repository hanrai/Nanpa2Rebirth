#include "engine.h"

void Engine::MesCacheOperations(int command)
{
    switch(command)
    {
    case 0:
        sendMessage(QString("ResetCacheCursor(gStruct.mesCacheCursor:%0);").arg(subCommands[1].command));
        memset(gStruct.mesCaches, 0, sizeof(gStruct.mesCaches));
        gStruct.mesCacheCursor=subCommands[1].command;
        break;
    case 1:
        sendMessage(QString("MesCachePushBack"));
        if(!(Flag->dim16Bit.mainFlag&0xC0) && (Flag->dim16Bit.mainFlag&0x0400))
        {
            for(gMesCachesIndex = 0; gMesCachesIndex<32; gMesCachesIndex++)
                if(!gStruct.mesCaches[gMesCachesIndex].bActive)
                    break;
        }
        if(gMesCachesIndex == 32)
        {
            char buffer[1024];
            gStruct.mesCacheCursor = gStruct.mesCaches[0].mesCacheCursor;
            for(auto i=1; i<32; i++)
            {
                auto sz = gStruct.mesCaches[i].size;
                Q_ASSERT(sz<0x400);
                memcpy(buffer, pMainStruct+gStruct.mesCaches[i].mesCacheCursor, sz);
                memcpy(pMainStruct+gStruct.mesCacheCursor, buffer, sz);
                gStruct.mesCaches[i-1].bActive = 1;
                gStruct.mesCaches[i-1].mesCacheCursor = gStruct.mesCacheCursor;
                gStruct.mesCaches[i-1].size = sz;
                gStruct.mesCacheCursor += sz;
            }
            gMesCachesIndex = 31;
            gStruct.mesCaches[31].bActive = 0;
            gStruct.mesCaches[31].mesCacheCursor = 0;
            gStruct.mesCaches[31].size = 0;
        }
        gStruct.mesCaches[gMesCachesIndex].bActive = 1;
        gStruct.mesCaches[gMesCachesIndex].mesCacheCursor = gStruct.mesCacheCursor;
        gStruct.mesCaches[gMesCachesIndex].size = 0;
        break;
    default:
        sendMessage(QString("Unknown mode:%0").arg(command));
        break;
    }
}
