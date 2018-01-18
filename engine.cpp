#include "engine.h"
#include "DrawingUtil/drawingutil.h"
#include "Animation/animationmanager.h"
#include <QGuiApplication>
#include <QtDebug>
#include "config.h"
#include "Files/files.h"
#include "DrawingUtil/drawingutil.h"
#include "stack.h"
#include "Flag.h"
#include "MainWindow.h"
#include "cps.h"

inline QString GetNumber(int number)
{
    QString n = QString().setNum(number);
    n+= QString("(0x%0)").arg(QString::number(number, 16).toUpper());
    return n;
}

Engine::Engine(QObject *parent, MainWindow *pMainWindow) :
    QThread(parent),
    engineStartEvent(0),
    depth(0)
{
    //Engine init begin
    pConfig = 0;
    this->pMainWindow = 0;

    pFiles = 0;
    pDrawing = 0;
    pStack = 0;
    pObject7 = 0;
    pCps = 0;
    pSoundInfo = 0;
    directdraw = 0;
    field_1EC4_0 = 0;
    field_1EC6_0 = 0;
    field_1EC7_0 = 0;
    stopRepositionWindow = 0;
    screenModeOperationSuccessed = 0;
    //Engine init end

    //Engine initialization begin
    memset(filenameMes, 0, sizeof(filenameMes));
    memset(filenamePic, 0, sizeof(filenamePic));
    memset(filenameData, 0, sizeof(filenameData));
    memset(filenameMusic, 0, sizeof(filenameMusic));
    memset(filenameVoice, 0, sizeof(filenameVoice));
    pCursorID = 0;
    isCursorVisible = 1;

    this->pMainWindow = pMainWindow;
    this->pConfig = new Config(this, &gMainStruct, this);
    this->pFiles = new Files(pConfig, &gMainStruct, this);
    this->pDrawing = new DrawingUtil(pMainWindow, pConfig, this);
    this->pStack = new Stack(pConfig, this);
    //需要考察，是否需要换个线程创建

    this->pCps = new CPS(this, &gMainStruct);
    //Engine initialization end

    //Init MainStructure begin
    this->vrams[0].left = 0;
    this->vrams[0].top = 0;
    this->vrams[0].width = 640;
    this->vrams[0].height = 400;
    this->vrams[0].data = 0;
    this->vrams[1].left = 0;
    this->vrams[1].top = 0;
    this->vrams[1].width = 640;
    this->vrams[1].height = 1800;
    this->vrams[1].data = 0;
    this->vrams[2].left = 0;
    this->vrams[2].top = 0;
    this->vrams[2].width = 640;
    this->vrams[2].height = 400;
    this->vrams[2].data = 0;
    this->vrams[3].left = 0;
    this->vrams[3].top = 0;
    this->vrams[3].width = 640;
    this->vrams[3].height = 400;
    this->vrams[3].data = 0;
    this->vrams[4].left = 0;
    this->vrams[4].top = 0;
    this->vrams[4].width = 640;
    this->vrams[4].height = 400;
    this->vrams[4].data = 0;
    this->vrams[5].left = 0;
    this->vrams[5].top = 0;
    this->vrams[5].width = 640;
    this->vrams[5].height = 2800;
    this->vrams[5].data = 0;

    this->vrams[0].data = new quint8[this->vrams[0].height * this->vrams[0].width];
    this->vrams[1].data = new quint8[this->vrams[1].height * this->vrams[1].width];
    this->vrams[2].data = new quint8[this->vrams[2].height * this->vrams[2].width];
    this->vrams[3].data = new quint8[this->vrams[3].height * this->vrams[3].width];
    this->vrams[4].data = new quint8[this->vrams[4].height * this->vrams[4].width];
    this->vrams[5].data = new quint8[this->vrams[5].height * this->vrams[5].width];

    memset(this->vrams[0].data, 0, 256000u);
    memset(this->vrams[1].data, 0, 256000u);
    memset(this->vrams[2].data, 0, 256000u);
    memset(this->vrams[3].data, 0, 256000u);
    memset(this->vrams[4].data, 0, 256000u);
    memset(this->vrams[5].data, 0, 256000u);
    memset(gStruct.mesCaches, 0, sizeof(gStruct.mesCaches));
    memset(&gMainStruct, 0, sizeof(gMainStruct));

    gMainStruct.Flag.pDim16Bit = (quint16*)&gMainStruct.Flag.dim16Bit;
    gMainStruct.Flag.dim16Bit.font_width = 16;
    gMainStruct.Flag.dim16Bit.font_height = 16;
    gMainStruct.Flag.dim16Bit.txtWidth = 16;
    gMainStruct.Flag.dim16Bit.txtHeight = 16;
    gMainStruct.Flag.dim16Bit.B10offset = 2832;
    gMainStruct.Flag.dim32Bit.cpsBase = 4096;
    gMainStruct.Flag.dim32Bit.pMesCursorBookmarks = (quint32*)gMainStruct.mesCursorBookmarks;
    gMainStruct.Flag.dim32Bit.field_20 = gMainStruct.g_dword_562890;
    gMainStruct.Flag.pMap = &gMainStruct.pMap;
    gMainStruct.Flag.dim16Bit.mainFlag = 0x27;
    gMainStruct.Flag.dim32Bit.defmainOffset = 131072;
    gMainStruct.Flag.dim16Bit.winLeft = 0;
    gMainStruct.Flag.dim16Bit.winTop = 0;
    gMainStruct.Flag.dim16Bit.winWidth = 640;
    gMainStruct.Flag.dim16Bit.winHeight = 480;
    gMainStruct.Flag.dim16Bit.colorIndex = 7;
    gMainStruct.Flag.dim16Bit.posTop = -1;
    gMainStruct.Flag.dim16Bit.posLeft = -1;
    gMainStruct.Flag.dim16Bit.colorMaskIndex = 8;
    gStruct.pMainStruct = &gMainStruct;
    gStruct.gMesCursor = 0x1000;
    gStruct.mesStartPoint = 0x1000;
    this->bookmarksReady = 0;
    this->depthCounter = 0;
    this->isAnimateCursor = 0;
    this->field_EB0 = 16;
    this->field_EB1 = 0;
    this->field_EB4 = 0;
    this->moveSpeed = 8;
    //Init MainStructure end

    memset(mesAnchors,0,sizeof(mesAnchors));
    memset(mesCache,0,1024);
    memset(FlagSnapshot,0,4096);
    memset(paletteBuffer, 0, sizeof(paletteBuffer));
    memset(anotherPaletteBuffer, 0, sizeof(anotherPaletteBuffer));

    memset(mesCursorBookmarks,0,sizeof(mesCursorBookmarks));
    memset(g_dword_562890,0,sizeof(g_dword_562890));
    memset(pMap,0,sizeof(pMap));
    memset(mesBookmarkIndexs,0,sizeof(mesBookmarkIndexs));

    Flag = &gMainStruct.Flag;
    pDim16Bit=reinterpret_cast<quint16*>(&Flag->dim16Bit);
    pReg16Bit=reinterpret_cast<quint16*>(&Flag->reg16Bit);
    pDim32Bit=reinterpret_cast<quint32*>(&Flag->dim32Bit);
    pReg32Bit=reinterpret_cast<quint32*>(&Flag->reg32Bit);
    stack.reserve(0x400);

    isCursorVisible=1;

    pMainStruct = (char*)&gMainStruct;
    depthCounter=0;
    isStop=false;
    bookmarksReady=false;
    strcpy(Flag->mesFileName,"START.MES");

    pFiles->LoadMesFile(QString("START.MES"), (quint8*)gMainStruct.mesBuffer);

    cursors[0]<<QImage("131.png")<<QImage("132.png");
    cursors[1]<<QImage("133.png")<<QImage("134.png");
    cursors[2]<<QImage("135.png")<<QImage("136.png");
    cursors[3]<<QImage("137.png")<<QImage("138.png");
    cursors[4]<<QImage("139.png")<<QImage("140.png");
    cursors[5]<<QImage("141.png")<<QImage("142.png");
    cursors[6]<<QImage("143.png")<<QImage("144.png");
    cursors[7]<<QImage("145.png")<<QImage("146.png");
    cursors[8]<<QImage("147.png")<<QImage("148.png");
    cursors[9]<<QImage("149.png")<<QImage("150.png");
    cursors[10]<<QImage("151.png")<<QImage("152.png");
    cursors[11]<<QImage("153.png")<<QImage("154.png");
    cursors[12]<<QImage("155.png")<<QImage("156.png");
    cursors[13]<<QImage("157.png")<<QImage("158.png");
    cursors[14]<<QImage("159.png")<<QImage("160.png");
    gNotReady = 0;
    isMusicOn = 1;
}

Engine::~Engine()
{
    delete[] this->vrams[0].data;
    delete[] this->vrams[1].data;
    delete[] this->vrams[2].data;
    delete[] this->vrams[3].data;
    delete[] this->vrams[4].data;
    delete[] this->vrams[5].data;
}

bool Engine::event(QEvent *e)
{
    if(e->type() == engineStartEvent)
    {
        e->accept();
        StartEngine();
        return true;
    }
    else
    {
        return QObject::event(e);
    }
}
void Engine::run()
{
    StartEngine();
}

void Engine::StartEngine()
{
    depth++;
    depthCounter++;
    sendMessage(QString("Engine(%0){;").arg(depthCounter));

    while(!isStop)
    {
        sendMessage("");
        if(Flag->dim16Bit.mainFlag & 16)
        {
            Flag->dim16Bit.mainFlag &= 0xFFEF;
            gStruct.gMesCursor = 4096;
            gStruct.mesStartPoint = 4096;
        }
        sendMessage(QString("pos_%0:%1").arg(gStruct.gMesCursor).arg(gStruct.gMesCursor-gStruct.mesStartPoint));
        quint8 command = getMes();
        sendMessage("mes:"+GetNumber(command));
        switch(command)
        {
        case 0:
            sendMessage(QString("return(%0);").arg(depthCounter));
            --depthCounter;
            isStop=true;
            break;
        case 1:
        {
            if(!(Flag->dim16Bit.mainFlag&0xC0))
            {
                MesCacheOperations(1);
            }
            if(Flag->dim16Bit.mainFlag&0x40)
                Flag->dim16Bit.mainFlag &= 0x80;
            DrawString();
            if(Flag->dim16Bit.mainFlag&0x80)
                Flag->dim16Bit.mainFlag &= 0xFF7F;
        }
            break;
        case 2:
            depth++;
            sendMessage("mes:%0");
            if(Flag->dim16Bit.mainFlag&0x40)
            {
                auto flag = Flag->dim16Bit.mainFlag;
                Flag->dim16Bit.mainFlag|=0x80;
                sendMessage(QString("(mainFlag:%0 |= 0x80):%1")
                            .arg(GetNumber(flag))
                            .arg(GetNumber(Flag->dim16Bit.mainFlag)));
            }
            outputText2();
            if(Flag->dim16Bit.mainFlag&0x80)
            {
                auto flag = Flag->dim16Bit.mainFlag;
                Flag->dim16Bit.mainFlag&=0xFF7F;
                sendMessage(QString("(mainFlag:%0 &= 0xFF7F):%1")
                            .arg(GetNumber(flag))
                            .arg(GetNumber(Flag->dim16Bit.mainFlag)));
            }
            depth--;
            break;
        case 3:
            setFlags();
            break;
        case 4:
            setReg16Bits();
            break;
        case 5:
            setFlags2();
            break;
        case 7:
            setDim16Bits();
            break;
        case 8:
            SetDim32Bits();
            break;
        case 0x9:
            mesJump();
            break;
        case 0xA:
            nakedJump();
            break;
        case 0xB:
            if(Flag->dim16Bit.mainFlag&0x40)
            {
                auto flag = Flag->dim16Bit.mainFlag;
                Flag->dim16Bit.mainFlag|=0x80;
                sendMessage(QString("(mainFlag:%0 |= 0x80):%1")
                            .arg(GetNumber(flag))
                            .arg(GetNumber(Flag->dim16Bit.mainFlag)));
            }
            gameCommand();
            break;
        case 0xC:
            ComChange();
            break;
        case 0xD:
            CallMes();
            break;
        case 0xE:
            unknown0xe();
            break;
        case 0xF:
            RunFromSavedAnchorPoint();
            break;
        case 0x10:
            depth++;
            sendMessage("Command 0x10");
            command_0x10();
            depth--;
            break;
        case 0x12:
            DefineAnchors();
            break;
        case 0x13:
            UserInput();
            break;
        case 0x14:
            depth++;
        {
            for(int index = getMes();;index++)
            {
                QString cstr;
                pReg32Bit[index]=Calculate(cstr);
                sendMessage(QString("reg32Bit[%0] = %1;").arg(index).arg(pReg32Bit[index]));
                if(!getMes())
                    break;
            }
        }
            depth--;
            break;
        default:
            if ((command >= 0x81 && command <= 0xFE)
              ||(command >= 0xE0 && command <= 0xEF))
                DrawString();
            else
                sendMessage(QString("Unknown command:%0").arg(command));
            break;
        }
    }
    depth--;
    isStop=false;
    sendMessage("}");
}

void Engine::sendMessage(QString message)
{
//    QString msg;
//    for(unsigned int i=0;i<depth;i++)
//        msg+="   ";
//    qDebug()<<msg+message;
}

unsigned char Engine::getMes(quint32 step)
{
auto bd = depth;
depth=0;
    quint8 data = pMainStruct[gStruct.gMesCursor];
    gStruct.gMesCursor += step;

    if((Flag->dim16Bit.mainFlag & 0x400) &&
       (Flag->dim16Bit.mainFlag & 0x80)  &&
        step)
    {
        gMainStruct.Flag.mesFileName[gStruct.mesCacheCursor++] = data;
        gStruct.mesCaches[gMesCachesIndex].size++;
        Q_ASSERT_X(gStruct.mesCaches[gMesCachesIndex].size<0x400,"Push into mes cache","Out of memory");
        sendMessage(QString("Mes command 0x%0: push into cache#%1").arg(data).arg(gMesCachesIndex));
    }
depth=bd;
    return data;
}

void Engine::outputText2()
{
    depth++;
//    sendMessage("===Output Text2 Start===");
//    sendMessage("Select vramBuffer depend on MainFlags[1]");

    while(1)
    {
        char c=getMes();
        if(!c)
            break;
        sendMessage(QString("Unknown OutputText2(\"%0\")").arg(QString(c,1)));
    }
//    if(!Flag.dim16Bit.vRamIndex)
//    {
//        sendMessage("MainFlags[1] is zero, redraw window...");
//    }
//    sendMessage("===Output Text2 End===");
    depth--;
    return;
}

void Engine::prepareSubCommands()
{
    depth++;
    subCommandIndex = 0;
    memset(subCommands,0,0x348);

    forever
    {
        char command = getMes();
        if(!command)
            break;
        if(command==1)
        {
            int i=0;
            subCommands[subCommandIndex].type=1;
            forever
            {
                char c=getMes();
                if(!c)
                    break;
                subCommands[subCommandIndex].buffer[i++]=c;
            }
            sendMessage(QString("subCommands[%0].buffer=%1").arg(subCommandIndex)
                        .arg(QString((char*)subCommands[subCommandIndex].buffer)));
        }
        else
        {
            if(command==2)
            {
                subCommands[subCommandIndex].type=2;
                QString cstr;
                subCommands[subCommandIndex].command=Calculate(cstr);
                sendMessage(QString("subCommands[%0].command=%1").arg(subCommandIndex)
                            .arg(cstr));
            }
        }
        subCommandIndex++;
    }
    depth--;
}

void Engine::RunFromSavedAnchorPoint(quint32 index)
{
    sendMessage(QString("RunFromSavedAnchorPoint(%0);").arg(index));
    depth++;

    quint32 anchorIndex = index;
    quint32 backMesCursor = gStruct.gMesCursor;
    quint32 backMesStartPoint = gStruct.mesStartPoint;

    gStruct.gMesCursor = mesAnchors[anchorIndex].mesCursor;
    gStruct.mesStartPoint = mesAnchors[anchorIndex].mesStartPoint;
    StartEngine();

    if(!(Flag->dim16Bit.mainFlag&0x10))
    {
        gStruct.gMesCursor = backMesCursor;
        gStruct.mesStartPoint = backMesStartPoint;
    }

    depth--;
}
