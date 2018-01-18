#include "cps.h"
#include "engine.h"
#include "DrawingUtil/drawingutil.h"
#include "Flag.h"

#define pMoveInfoBase ((moveInfoBase*)((char*)pMainStruct + pMainStruct->Flag.dim32Bit.moveInfoOffset + pMainStruct->Flag.dim32Bit.cpsBase))
#define pMoveInfos ((moveInfo*)((char*)pMoveInfoBase+pMoveInfoBase->moveInfosOffset))
#define pDoorInfos ((doorInfo*)((char*)pMoveInfoBase+pMoveInfoBase->doorsInfosOffset))
#define pMyInfos ((quint16*)((char*)pMoveInfoBase+pMoveInfoBase->myInfosOffset))

CPS::CPS(Engine *parent, MainStruct *pMainStruct) : QObject(parent)
{
    this->pEngine = parent;
    this->pMainStruct = pMainStruct;
    auto i = 0;
    for(auto y=0; y<400; y+=16)
        for(auto x=0; x<640; x+=16)
            offsets[i++] = x+640*(400-y-1);
    field_5154_0 = 0;
    moveHistoryIndex = 0;
    field_5156_0 = 0;
    pVram0 = &pEngine->vrams[0];
    pVram2 = &pEngine->vrams[2];
    pVram3 = &pEngine->vrams[3];
}

void CPS::cpsMove(qint32 command, qint32 index1, qint32 index2, qint32 a5)
{
    switch(command)
    {
    case 0:
        init();
        break;
    case 1:
        setBirthPos(index1, index2, a5);
        break;
    case 2:
        getMapData();
        break;
    case 3:
        resetBackupMapBuffer();
        break;
    case 4:
        setMoveInfos();
        break;
    case 5:
        setMoveInfo(index1, index2);
        break;
    case 6:
        setMyBlocks();
        break;
    case 7:
        selectMoveInfo(index1, index2);
        break;
    case 10:
        paintInitMap();
        break;
    case 17:
        setMoveResult();
        break;
    case 20:
        goBack(index1, index2);
        break;
    case 21:
        setMoveCommand(index1);
        break;
    case 22:
        setFlag(index1, index2);
        break;
    default:
qDebug()<<"Unknown move command:"<<command;
        break;
    }
}

void CPS::setBirthPos(qint32 doorIndex, qint32 moveInfoIndex, quint8 a4)
{
    doorInfo* pDoorInfo = pDoorInfos+doorIndex;
    pMoveInfo = pMoveInfos+moveInfoIndex;

    pMainStruct->mapLeft = pDoorInfo->mapLeft;
    pMainStruct->mapTop = pDoorInfo->mapTop;
    pMoveInfo->myPosX = pDoorInfo->myPosX;
    pMoveInfo->myPosY = pDoorInfo->myPosY;
    pMoveInfo->frameCommandCombo = a4<<2;

    for(auto i=0; i<32; i++)
    {
        moveHistory[i].myPosX = pMoveInfo->myPosX;
        moveHistory[i].myPosY = pMoveInfo->myPosY;
        moveHistory[i].frameCommandCombo = pMoveInfo->frameCommandCombo;
    }
}

void CPS::setMoveInfos()
{
    for(pMoveInfo=pMoveInfos; pMoveInfo->flag!=0xFF; pMoveInfo++)
    {
        auto moveCombIndex = pMoveInfoBase->moveCommandComboIndex;
        auto combos = (char*)pMoveInfoBase+2*moveCombIndex;
        auto pCommandCombo = (char*)pMoveInfoBase+combos[moveCombIndex];
        pMoveInfo->moveCommandComb = combos[moveCombIndex]+1;
        pMoveInfo->moveCommand = *pCommandCombo>>4;
        pMoveInfo->myFrameIndex = (*pCommandCombo&0xF)+1;
    }
}

void CPS::setMoveInfo(quint32 moveInfoIndex, quint32 moveCommandCombosOffset)
{
    if(field_5156_0)
       return;

    pMoveInfo = &pMoveInfos[moveInfoIndex];
    if(pMoveInfo->flag=0xFF)
        return;

    pMoveInfo->moveCommandCombosOffset = moveCommandCombosOffset;
    pMoveInfo->moveCommandComb = ((quint16*)((char*)pMoveInfoBase+pMoveInfoBase->moveCommandComboIndex))[moveCommandCombosOffset];
    auto pCommandCombo = (char*)pMoveInfoBase+pMoveInfo->moveCommandComb;
    pMoveInfo->moveCommand = *pCommandCombo>>4;
    pMoveInfo->myFrameIndex = (*pCommandCombo&0xF)+1;
}

void CPS::selectMoveInfo(quint32 index, quint32 flag)
{
    pMoveInfo = &pMoveInfos[index];
    pMoveInfo->flag = flag;
}

void CPS::setFlag(qint32 index1, qint32 index2)
{
    moveInfo* info = &pMoveInfos[index1];
    info->flag = index2<<2;
}

void CPS::setMoveResult()
{
    pMainStruct->Flag.reg16Bit.moveResult = field_5156_0<<8|field_5156_0;
}

void CPS::setMoveCommand(qint32 index)
{
    if(!pEngine->configMiniMapMoveSpeed)
        return;
    moveInfo* info = &pMoveInfos[index];
    if(count20 >= 20)
    {
        count20 -= 20;
        info->moveCommand = 0xF;
    }
}

void CPS::goBack(qint32 index1, qint32 index2)
{
    pMoveInfo = pMoveInfos+index1;
    auto i = moveHistoryIndex-1;
    if(i < 0)
        i = 31;
    i = i-2*index2+((i-2*index2)<0?32:0);
    pMoveInfo->myPosX = moveHistory[i].myPosX;
    pMoveInfo->myPosY = moveHistory[i].myPosY;
    pMoveInfo->frameCommandCombo = moveHistory[i].frameCommandCombo;
}

void CPS::setMyBlocks()
{
    auto i=0;
    for(pMoveInfo = pMoveInfos;
        pMoveInfo->flag != 0xFF;
        pMoveInfo++)
    {
        if(pMoveInfo->flag&1)
            setMyBlocks(i, pMoveInfo);
        i++;
    }
}

void CPS::setMyBlocks(qint32 index, moveInfo* pMoveInfo)
{
    index <<= 2;
    quint16 bufferIndex = pMoveInfo->myPosX-pMainStruct->mapLeft+pMoveInfo->myPosY*40-pMainStruct->mapTop*40;
    quint16* pMyInfo = pMyInfos+4*(pMoveInfo->frameCommandCombo+16*pMoveInfo->myInfoOffset);
    quint16 topGap = pMoveInfo->myPosY-pMainStruct->mapTop;
    quint16 leftGap = pMoveInfo->myPosX-pMainStruct->mapLeft;

    for(auto y=0; y<myHeight; y++)
    {
        for(auto x=0; x<myWidth; x++)
        {
            if(leftGap+x>pMainStruct->cpsWidth || topGap+y>pMainStruct->cpsHeight)
                continue;
            auto i = y*myWidth+x;
            quint16 mapBufferIndex = 4*(bufferIndex+x+y*40);
            quint16 offset = (index<<8)|pMyInfo[i];
            if(flag&4)
            {
                offset |= 0x400;
                mapBuffer[mapBufferIndex+1] = offset;
            }
            else
            {
                quint16 layer2 = mapBuffer[mapBufferIndex+2];
                if(layer2 == 0xFFFF)
                    mapBuffer[mapBufferIndex+2] = offset;
                else if(mapBuffer[mapBufferIndex+3] == 0xFFFF)
                {
                    if(pMoveInfo[(layer2>>10)].myPosY < pMoveInfo[(offset>>10)].myPosY)
                        mapBuffer[mapBufferIndex+3] = offset;
                    else
                    {
                        mapBuffer[mapBufferIndex+3] = layer2;
                        mapBuffer[mapBufferIndex+2] = offset;
                    }
                }
            }
        }
    }
}

void CPS::placeBlock(quint16 *pBlock, quint16 left, quint16 top)
{
    qint32 offset = offsets[pBlock[0]&0x3FF];
    qint32 v0Offset = left+(pVram0->width*(pVram0->height-top-1));
    quint8* pv0data;
    quint8* pv2data;
    quint8* pv3data;
    quint8* pBufferData;
    if(pBlock[1]!=0x3FF || pBlock[2]!=0xFFFF || pBlock[3]!=0xFFFF)
    {
        pv2data = pVram2->data+offset;
        pBufferData = blockBuffer+240;
        for(auto i=0; i<16; i++)
        {
            memcpy(pBufferData, pv2data, 16);
            pv2data -= pVram2->width;
            pBufferData -= 16;
        }

        if((pBlock[2]&0x3FF) != 0x3FF)
        {
            pv3data = pVram3->data+offsets[pBlock[2]&0x3FF];
            pBufferData = blockBuffer+240;
            for(auto y=0; y<16; y++)
            {
                for(auto x=0; x<16; x++)
                {
                    if(pv3data[x] != pMainStruct->Flag.dim16Bit.colorMaskIndex)
                        pBufferData[x] = pv3data[x];
                }
                pBufferData -= 16;
                pv3data -= pVram3->width;
            }
            if((pBlock[3]&0x3FF) != 0x3FF)
            {
                pv3data = pVram3->data+offsets[pBlock[3]&0x3FF];
                pBufferData = blockBuffer+240;
                for(auto y=0; y<16; y++)
                {
                    for(auto x=0; x<16; x++)
                    {
                        if(pv3data[x] != pMainStruct->Flag.dim16Bit.colorMaskIndex)
                            pBufferData[x] = pv3data[x];
                    }
                    pBufferData -= 16;
                    pv3data -= pVram3->width;
                }
            }
        }
        if(pBlock[1] & 0x400)
        {
            pv3data = pVram3->data+offsets[pBlock[1]&0x3FF];
            pBufferData = blockBuffer+240;
            for(auto y=0; y<16; y++)
            {
                for(auto x=0; x<16; x++)
                {
                    if(pv3data[x] != pMainStruct->Flag.dim16Bit.colorMaskIndex)
                        pBufferData[x] = pv3data[x];
                }
                pBufferData -= 16;
                pv3data -= pVram3->width;
            }
        }
        else if(pBlock[1] != 0x3FF)
        {
            pv2data = pVram2->data+offsets[pBlock[1]&0x3FF];
            pBufferData = blockBuffer+240;
            for(auto y=0; y<16; y++)
            {
                for(auto x=0; x<16; x++)
                {
                    if(pv2data[x] != pMainStruct->Flag.dim16Bit.colorMaskIndex)
                        pBufferData[x] = pv2data[x];
                }
                pBufferData -= 16;
                pv2data -= pVram2->width;
            }
        }

        pv0data = pVram0->data+v0Offset;
        pBufferData = blockBuffer+240;
        for(auto i=0; i<16; i++)
        {
            memcpy(pv0data, pBufferData, 16);
            pv0data -= pVram0->width;
            pBufferData -= 16;
        }
    }
    else
    {
        quint8* v2Data = pVram2->data+offset;
        quint8* v0Data = pVram0->data+v0Offset;
        for(auto i=0; i<16; i++)
        {
            memcpy(v0Data, v2Data, 16);
            v0Data -= pVram0->width;
            v2Data -= pVram2->width;
        }
    }
}

void CPS::paintInitMap()
{
    qint32 top = 16*pMainStruct->cpsTop;
    qint32 left = 16*pMainStruct->cpsLeft;
    for(auto y=0; y<pMainStruct->cpsHeight; y++)
    {
        for(auto x=0; x<pMainStruct->cpsWidth; x++)
        {
            qint32 i = 4*(x+y*40);
            quint16* pBlock = mapBuffer+i;
            placeBlock(pBlock, left+x*16, top+y*16);
            backupMapBuffer[i] = mapBuffer[i];
            backupMapBuffer[i+1] = mapBuffer[i+1];
            backupMapBuffer[i+2] = mapBuffer[i+2];
            backupMapBuffer[i+3] = mapBuffer[i+3];
        }
    }
    pEngine->pDrawing->updateWindow(0, 0, 640, 400);
}

void CPS::init()
{
    pMainStruct->pMap = (mapInfo*)((char*)pMainStruct + pMainStruct->Flag.dim32Bit.mapDataOffset + pMainStruct->Flag.dim32Bit.cpsBase);
    pMainStruct->mapWidth = pMainStruct->pMap->mapWidth;
    pMainStruct->mapHeight = pMainStruct->pMap->mapHeight;
    map2DataOffset = 3*(pMainStruct->mapWidth*pMainStruct->mapHeight+2);
    resetBackupMapBuffer();
    field_4154 = -1;
    field_4153 = 0;
    field_4152 = 0;
}

void CPS::getMapData()
{
    if(!pMainStruct->cpsHeight || !pMainStruct->cpsWidth)
        return;

    auto pMap = pMainStruct->pMap->mapData;
    auto pBlockID = mapBuffer;

    for(auto y=0; y<pMainStruct->cpsHeight; y++)
    {
        auto row = pMap + 3*y*pMainStruct->cpsWidth;
        for(auto x=0; x<pMainStruct->cpsWidth; x++)
        {
            quint16 data = row[0]|(row[1]<<8);
            pBlockID[0] = data&0x3FF;
            pBlockID[1] = ((row[2]&0xF)<<6)|(data>>10);
            pBlockID[2] = 0xFFFF;
            pBlockID[3] = 0xFFFF;
            pBlockID += 4;
            row += 3;
        }
    }
}

void CPS::resetBackupMapBuffer()
{
    for(auto i=0; i<4000; i+=4)
    {
        backupMapBuffer[i] = 0x3FF;
        backupMapBuffer[i+1] = 0x3FF;
        backupMapBuffer[i+2] = 0xFFFF;
        backupMapBuffer[i+3] = 0xFFFF;
    }
}
