#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <functional>
#include <QWaitCondition>
#include "Flag.h"
class DrawingUtil;
class Config;
class MainWindow;

typedef struct
{
    qint8 bAnimation;
    qint8 counter;
    qint8 unknown1;
    qint8 unknown2;
    char* pCommands;
    quint32 commandIndex;
    quint32 itemIndexBookmark;
    quint32 itemIndexBookmark2;
    qint8 DecCounter;
    qint8 DecCounter2;
    qint16 field_1A;
    void* unknown;
    QWaitCondition playing;
} Animation;
#pragma pack(1)
typedef struct
{
    quint8 commandVRamSelectorCombo;
    quint16 left;
    quint16 top;
    quint16 width;
    quint16 height;
    quint16 targetLeft;
    quint16 targetTop;
} PaintingInstruction;
#pragma pack()

class AnimationManager : public QObject
{
    Q_OBJECT
public:
    explicit AnimationManager(Config *pConfig = 0,
                              Engine *pEngine = 0,
                              MainStruct *pMainStruct = 0,
                              QObject *parent = 0);

    //QByteArray buffer;
    Animation animations[10];

    void PrepareAnimation(int index);
    void stopAllAnimagions();
signals:
    void cursorAnimated(qint8 id);

public slots:
    void PlayAnimations();
    void AnimationRoutin();
private:
    //begin
    qint32 field_118;
    MainStruct *pMainStruct;
public:

    qint8 cursorIDShift;
private:
    //end
    Config* pConfig;
    DrawingUtil *pDrawing;
    MainWindow *pMainWindow;
    QTimer animationTimer;
    void animateCursor();
public:
    void animationRunner(Animation& animation);
private:
    void PaintingToTarget(PaintingInstruction* instruction);
    void PaintImgToTargetWithMask(PaintingInstruction* instruction);
    void PaintImgToTargetWithMask_2(PaintingInstruction* instruction);
    void switchImgs(PaintingInstruction* instruction);
    void FillBackground(PaintingInstruction* instruction);
};

#endif // ANIMATIONMANAGER_H
