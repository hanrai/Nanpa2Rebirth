#ifndef CPSMANAGER_H
#define CPSMANAGER_H

#include <QObject>
class Engine;
struct MainStruct;
struct VramInfo;

struct moveInfoBase
{
    quint16 moveInfosOffset;
    quint16 moveCommandComboIndex;
    quint16 doorsInfosOffset;
    quint16 myInfosOffset;
    quint8 data[1];
};

struct doorInfo
{
    quint8 mapLeft;
    quint8 mapTop;
    quint8 myPosX;
    quint8 myPosY;
};

struct moveInfo
{
    quint8  flag;
    quint8  myInfoOffset;
    quint8  myWidth;
    quint8  myHeight;
    quint16 myPosX;
    quint16 myPosY;
    quint8  frameCommandCombo;
    quint8  moveCommandCombosOffset;
    quint8  moveCommand;
    quint8  myFrameIndex;
    quint16 moveCommandComb;
};

struct moveHistory
{
    quint16 myPosX;
    quint16 myPosY;
    quint8 frameCommandCombo;
    quint8 field_5;
};

class CPS : public QObject
{
    Q_OBJECT
public:
    explicit CPS(Engine* parent = 0, MainStruct* pMainStruct = 0);
    void cpsMove(qint32 command, qint32 index1, qint32 index2, qint32 a5);

signals:

public slots:
private:
    void init();
    void getMapData();
    void resetBackupMapBuffer();
    void setBirthPos(qint32 doorIndex, qint32 moveInfoIndex, quint8 a4);
    void setMoveInfos();
    void setMoveInfo(quint32 moveInfoIndex, quint32 moveCommandCombosOffset);
    void selectMoveInfo(quint32 index, quint32 flag);
    void setFlag(qint32 index1, qint32 index2);
    void setMoveResult();
    void setMoveCommand(qint32 index);
    void goBack(qint32 index1, qint32 index2);
    void setMyBlocks();
    void setMyBlocks(qint32 index, moveInfo *pMoveInfo);
    void placeBlock(quint16* pBlock, quint16 left, quint16 top);
    void paintInitMap();
private:
    Engine* pEngine;
    MainStruct* pMainStruct;
    quint8 blockBuffer[256];
    quint8 field_108[256];
    quint16 mapBuffer[4000];
    quint16 backupMapBuffer[4000];
    qint16 moveHistoryIndex;
    moveHistory moveHistory[32];
    quint8 flag;
    quint8 myWidth;
    quint8 myHeight;
    quint8 field_414D;
    qint16 moveResult;
    quint8 field_4150;
    quint8 field_4151;
    quint8 field_4152;
    quint8 field_4153;
    quint16 field_4154;
    quint16 map2DataOffset;
    quint8* field_4158;
    quint32 field_415C;
    quint32 offsets[1000];
    VramInfo* pVram2;
    VramInfo* pVram3;
    VramInfo* pVram0;
    quint8 field_510C[52];
    moveInfo* pMoveInfo;
    quint8* pRightSideMapData;
    quint8* pLeftSideMapData;
    quint8 field_514C;
    quint32 field_514D;
    quint16 field_5151;
    quint8 field_5153;
    quint8 field_5154_0;
    quint8 paintingCommand;
    quint8 field_5156_0;
    quint8 field_5157;
    quint8 field_5158;
    quint8 field_5159;
    quint8 field_515A;
    quint8 field_515B;
    quint16 count20;
    quint16 field_515E[9984];
    quint8 field_9F5E[19970];
};

#endif // CPSMANAGER_H
