#include <QTimeLine>
#include <QPainter>
#include <QColor>
#include "animationmanager.h"
#include "../DrawingUtil/drawingutil.h"
#include "../MainWindow.h"
#include "engine.h"

AnimationManager::AnimationManager(Config *pConfig, Engine *pEngine, MainStruct *pMainStruct, QObject *parent) :
    QObject(parent),
    animationTimer(this)
{
    //init begin
    this->pConfig = pConfig;
    threadGpEngine = pEngine;
    this->pMainStruct = pMainStruct;
    this->cursorIDShift = 0;
    //init end
    this->pDrawing = pEngine->pDrawing;
    pMainWindow = pEngine->pMainWindow;
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    for(Animation &a:animations)
        memset(&a, 0, 28);
    animationTimer.setInterval(15);
    connect(&animationTimer, SIGNAL(timeout()),
            this, SLOT(AnimationRoutin()));
}

void AnimationManager::PrepareAnimation(int index)
{
    char* cursor = gMainStruct.mesBuffer;
    cursor += gMainStruct.Flag.dim32Bit.animationOffset;
    cursor += 2*index+1;
    quint16 offset = *(quint16*)cursor;
    animations[index].bAnimation = 0;
    animations[index].counter = 0;
    animations[index].pCommands = gMainStruct.mesBuffer+offset+gMainStruct.Flag.dim32Bit.animationOffset;
    animations[index].commandIndex = 0;
    animations[index].itemIndexBookmark = 0;
    animations[index].itemIndexBookmark2 = 0;
    animations[index].DecCounter = 0;
    animations[index].DecCounter2 = 0;
    animations[index].field_1A = 0;
    animations[index].unknown = 0;
}

void AnimationManager::stopAllAnimagions()
{
    for(int i=0; i<10; i++)
        animations[i].bAnimation = 0;
}

void AnimationManager::PlayAnimations()
{
    if(!animationTimer.isActive())
        animationTimer.start();
}

void AnimationManager::AnimationRoutin()
{
    pMainWindow->vSync.wakeAll();
    if(gNotReady)
        return;
    animateCursor();
    pDrawing->MixTwoPalette();
    if(gMainStruct.Flag.dim16Bit.mainFlag & 4)
    {
        for(Animation &animation:animations)
        {
            if(animation.bAnimation)
                animationRunner(animation);
        }
    }
}

void AnimationManager::animateCursor()
{
    qint8 aniCursorFrameID[]{1, -1, 0, -1};
    static quint8 gAnimateCursorTimeoutCounter = 0;

    if ( threadGpEngine->isAnimateCursor )
    {
      if ( gAnimateCursorTimeoutCounter )
      {
        --gAnimateCursorTimeoutCounter;
      }
      else
      {
          qint8 id = aniCursorFrameID[cursorIDShift];
        if ( id == -1 )
        {
          gAnimateCursorTimeoutCounter = 32;
        }
        else
        {
            emit cursorAnimated(id);
        }
        cursorIDShift = (cursorIDShift + 1) & 3;
      }
    }
}

void AnimationManager::animationRunner(Animation &animation)
{
    if(!animation.bAnimation)
        return;

    if(animation.counter)
    {
        animation.counter--;
        return;
    }

    quint8 command = animation.pCommands[animation.commandIndex++];

    switch(command)
    {
    case 0:
        return;
    case 1:
        if(animation.bAnimation == 1)
            animation.bAnimation = 0;
        return;
    case 2:
        animation.counter = animation.pCommands[animation.commandIndex++];
        return;
    case 3:
        animation.commandIndex = 0;
        return;
    case 4:
        animation.bAnimation = 0;
        animation.playing.wakeAll();
        return;
    case 5:
        animation.itemIndexBookmark = animation.commandIndex;
        animation.DecCounter = animation.pCommands[animation.commandIndex++];
        return;
    case 6:
        animation.DecCounter--;
        if(animation.DecCounter)
            animation.commandIndex = animation.itemIndexBookmark;
        return;
    case 7:
        animation.itemIndexBookmark2 = animation.commandIndex;
        animation.DecCounter2 = animation.pCommands[animation.commandIndex++];
        return;
    case 8:
        animation.DecCounter2--;
        if(animation.DecCounter2)
            animation.commandIndex = animation.itemIndexBookmark2;
        return;
    case 150:
    {
        quint16 paletteOffset = *(quint16*)&animation.pCommands[animation.commandIndex];
        animation.commandIndex+=2;
        quint8* paletteCursor = (quint8*)pMainStruct->mesBuffer+pMainStruct->Flag.dim32Bit.animationOffset+paletteOffset;
        PALETTEENTRY p[236];
        for(int i=0;i<236;i++)
        {
            p[i].peBlue = *paletteCursor++;
            p[i].peRed = *paletteCursor++;
            p[i].peGreen = *paletteCursor++;
        }
        pDrawing->setTargetPalette((quint32*)p);
        pDrawing->copyTargetPalette(0, 236);
    }
        return;
    case 255:
        animation.bAnimation = 0;
        animation.playing.wakeAll();
        return;
    default:
        break;
    }

    quint32 imgCommandOffset = 17*command+pMainStruct->Flag.dim32Bit.animationOffset;
    PaintingInstruction* paintingInstruction =
            (PaintingInstruction*)(&pMainStruct->Flag.AnimationCommandBasePoint[imgCommandOffset]);
    quint8 imgCommand = paintingInstruction->commandVRamSelectorCombo;

    switch(imgCommand&0xF0)
    {
    case 0x10:
        PaintingToTarget(paintingInstruction);
        PaintImgToTargetWithMask(paintingInstruction);
        //PaintImgToTargetWithMask_2(paintingInstruction);原程序中没实现，同PaintImgToTargetWithMask
        break;
    case 0x20:
        PaintImgToTargetWithMask(paintingInstruction);
        //PaintImgToTargetWithMask_2(paintingInstruction);
        break;
    case 0x30:
        switchImgs(paintingInstruction);
        break;
    case 0x50:
        PaintImgToTargetWithMask_2(paintingInstruction);
        break;
    case 0x60:
        FillBackground(paintingInstruction);
        break;
    default:
        return;
    }
}

void AnimationManager::PaintingToTarget(PaintingInstruction *instruction)
{
    auto left = instruction->left;
    auto top = instruction->top;
    auto width = instruction->width;
    auto height = instruction->height;
    auto tLeft = instruction->targetLeft;
    auto tTop = instruction->targetTop;
    quint8 srcVramID = (instruction->commandVRamSelectorCombo>>1)&1;
    quint8 dstVramID = instruction->commandVRamSelectorCombo&1;
    auto srcVram = &threadGpEngine->vrams[srcVramID];
    auto dstVram = &threadGpEngine->vrams[dstVramID];
    pDrawing->paintingImg(0, left, top, width, height, srcVram, tLeft, tTop, dstVram, 0, 0, 0, 0);
    if(!dstVramID)
        pDrawing->updateWindow(tLeft, tTop, width, height);
}

void AnimationManager::PaintImgToTargetWithMask(PaintingInstruction *instruction)
{
    pDrawing->paintingImg(1,instruction->left,
            instruction->top,
            instruction->width,
            instruction->height,
            &threadGpEngine->vrams[(instruction->commandVRamSelectorCombo>>1)&1],
            instruction->targetLeft,
            instruction->targetTop,
            &threadGpEngine->vrams[(instruction->commandVRamSelectorCombo&1)],
            0,0,0,
            threadGpEngine->Flag->dim16Bit.colorMaskIndex);
    if(!(instruction->commandVRamSelectorCombo&1))
        pDrawing->updateWindow(instruction->targetLeft,
                               instruction->targetTop,
                               instruction->width,
                               instruction->height);
}

void AnimationManager::PaintImgToTargetWithMask_2(PaintingInstruction *instruction)
{
    PaintImgToTargetWithMask(instruction);
}

void AnimationManager::switchImgs(PaintingInstruction *instruction)
{
    pDrawing->paintingImg(3,
                          instruction->left,
                          instruction->top,
                          instruction->width,
                          instruction->height,
                          &threadGpEngine->vrams[(instruction->commandVRamSelectorCombo>>1)&1],
                          instruction->targetLeft,
                          instruction->targetTop,
                          &threadGpEngine->vrams[(instruction->commandVRamSelectorCombo&1)],
                          0,0,0,0);
    pDrawing->updateWindow(instruction->targetLeft,
                           instruction->targetTop,
                           instruction->width,
                           instruction->height);
}

void AnimationManager::FillBackground(PaintingInstruction *instruction)
{
    pDrawing->paintingImg(2,
                          instruction->left,
                          instruction->top,
                          instruction->width,
                          instruction->height,
                          &threadGpEngine->vrams[(instruction->commandVRamSelectorCombo>>1)&1],
                          0, 0, 0, 0, 0, 0, (qint32)&instruction->height);
    if(!(instruction->commandVRamSelectorCombo&1))
    pDrawing->updateWindow(instruction->targetLeft,
                           instruction->targetTop,
                           instruction->width,
                           instruction->height);
}
