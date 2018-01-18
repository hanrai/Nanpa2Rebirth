#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QStack>
#include <QColor>
#include "Flag.h"
#include <functional>

class Config;
class MainWindow;
class Files;
class DrawingUtil;
class Stack;
class AnimationManager;
class CPS;

typedef struct tag_SubCommand
{
    quint8 type;
    quint8 buffer[23];
    quint32 command;
} SubCommand;

typedef struct tag_MesAnchor
{
    quint32 mesCursor;
    quint32 mesStartPoint;
} MesAnchor;

struct VramInfo
{
    quint8* data;
    qint16 left;//left?
    qint16 top;//top?
    qint16 width;
    qint16 height;
};

class Engine : public QThread
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0, MainWindow *pMainWindow = 0);
    ~Engine();
    char* pMainStruct;

    void StartEngine();
    bool isStop;
    int engineStartEvent;
    bool event(QEvent *e);

signals:
    void StartAnimation();
    //void PlaySoundStream(QByteArray data, bool isLoop = false);
    //new
    void hideCursor();
    void showCursor();
    void BGMStarted(QByteArray data, bool isLoop = false);
    void voiceStarted(QByteArray data);
public slots:

private:
    //Engine Struct begin
    SubCommand subCommands[30];
    quint32 subCommandIndex;
    char mesBookmarkIndexs[48];
    qint16 field_37C;
    qint16 field_37E;
    bool bookmarksReady;
    MesAnchor mesAnchors[100];
    quint32 paletteBuffer[256];
    quint32 anotherPaletteBuffer[256];
    unsigned int depthCounter;
    qint16 field_EA6;
    qint32 isMusicOn;
    qint32 configVoice;
    qint8 field_EB0;
    qint8 field_EB1;
    qint8 field_EB2;
    qint8 field_EB3;
    qint32 field_EB4;
    qint16 moveSpeed;
    qint16 field_EBA;
    qint32 configMoveSpeed=16;
    qint32 colorBits;
    quint8 FlagSnapshot[4096];
    qint16 field_1EC4_0;
    qint8 field_1EC6_0;
    qint8 field_1EC7_0;
public:
    MainWindow* pMainWindow;
    Config *pConfig;
    DrawingUtil* pDrawing;
private:
    Files* pFiles;
    Stack* pStack;
public:
    AnimationManager* pAnimations;
private:
    void* pSoundInfo;
    void* pObject7;
    CPS* pCps;
    void* directdraw;
public:
    VramInfo vrams[6];
private:
    char filenameMes[128];
    char filenamePic[128];
    char filenameData[128];
    char filenameMusic[128];
    char filenameVoice[128];
public:
    qint32 isAnimateCursor;
private:
    qint32 cursorID;
    qint32 pCursorID;//For LoadCursor
public:
    qint32 configMiniMapMoveSpeed;
private:
    qint32 stopRepositionWindow;
    qint32 screenModeOperationSuccessed;
    qint32 isCursorVisible;
    //Engine Struct end

    unsigned int depth;

    quint8 mesCache[1024];
    int mesCursorBookmarks[50];//FLAG
    int g_dword_562890[50];//FLAG
    char pMap[52];//FLAG?

    QList<QImage> cursors[15];

public:
    FLAG *Flag;
private:
    quint16 *pDim16Bit;
    quint16 *pReg16Bit;
    quint32 *pDim32Bit;
    quint32 *pReg32Bit;

    void sendMessage(QString message);
    unsigned char getMes(quint32 step = 1);
    int Calculate(QString &calstr);
    void setDim16Bits();
    void gameCommand();
    void outputText2();
    void prepareSubCommands();
    void cursorCommand();
    void SetDim32Bits();
    void command_0x10();
    void nakedJump();
    void setReg16Bits();
    void setFlags();
    void paletteOperation();
    void painting();
    void MesCacheOperations(int command);
    void loadDataMes();
    void runMes();
    void mesJump();
    void DefineAnchors();
    void ComChange();
    void CallMes();
    void RunFromSavedAnchorPoint();
    void AnimationCommand();
    void SoundOperations(SubCommand*);
    void DrawBG();
    void DrawString();
    void LoadSave();
    void unknown0xe();
    void SkipableWait();
    void UserInput();
    void RunFromSavedAnchorPoint(quint32 index);
    void setFlags2();
    void command_0x13();
    void miniMapCommand();
    QStack<int> stack;

    // QThread interface
protected:
    void run();
};

#endif // ENGINE_H
