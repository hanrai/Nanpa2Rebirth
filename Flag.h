#ifndef FLAG_H
#define FLAG_H
#include <QtCore>
typedef struct tag_DIM16BIT
{
    quint16 B10offset;
    quint16 vRamIndex;
    quint16 field_4;
    quint16 mainFlag;
    quint16 cursorLeft;
    quint16 cursorTop;
    quint16 winLeft;
    quint16 winTop;
    quint16 winWidth;
    quint16 winHeight;
    quint16 txtLeft;
    quint16 txtTop;
    quint16 colorIndex;
    quint16 flagDigPosComb;//command0x10:case 0x6f:
    quint16 font_width;
    quint16 font_height;
    quint16 drawTxtStyle;
    quint16 txtWidth;
    quint16 txtHeight;
    qint16 posTop;
    qint16 posLeft;
    quint16 left;
    quint16 top;
    quint16 width;
    quint16 height;
    quint16 colorMaskIndex;
} DIM16BIT;

typedef struct tag_REG16BIT
{
    quint16 tempReg;
    quint16 system_Month;
    quint16 system_DayOfWeek;
    quint16 system_Day;
    quint16 dayOfWeek;
    quint16 system_Minute;
    quint16 system_Second;
    quint16 date;
    quint16 forLoopIndex;
    quint16 OffsetToMainstructField_EF8;
    quint16 field_14;
    quint16 field_16;
    quint16 money;
    quint16 field_1A;
    quint16 field_1C;
    quint16 field_1E;
    quint16 field_20;
    quint16 field_22;
    quint16 moveResult;
    quint16 time;
    quint16 field_28;
    quint16 field_2A;
    quint16 systemData[4];
} REG16BIT;

typedef struct tag_REG32BIT
{
    quint32* addresses[26];
} REG32BIT;

typedef struct tag_DIM32BIT
{
    quint32 defmainOffset;
    quint32 animationOffset;
    quint32 mapDataOffset;
    quint32 moveInfoOffset;
    quint32 field_10;
    quint32 field_14;
    quint32 cpsBase;
    quint32 *pMesCursorBookmarks;
    qint32 *field_20;
} DIM32BIT;

typedef struct tag_FLAG
{
    char mesFileName[128];
    quint8 flag[2048];
    quint16 *pDim16Bit;
    REG16BIT reg16Bit;
    DIM16BIT dim16Bit;
    quint32 bookmarkIndex;
    REG32BIT reg32Bit;
    DIM32BIT dim32Bit;
    void* pMap;
    quint8 charTable[1345];
    quint8 AnimationCommandBasePoint[55];
    quint16 EF8[18];
    quint8 field_F1C[64];
    quint32 playerName;
    quint32 unknown_462728;
    quint32 unknown_46272C;
    quint32 unknown_462730;
    quint8 unknown_462734[148];
} FLAG;

struct mapInfo
{
    quint16 mapWidth;
    quint16 mapHeight;
    quint8 mapData[1];
};

struct MainStruct
{
    FLAG Flag;
    char mesBuffer[204946];
    char unknown_49485A[1515];
    char unknown_494E45[65536];
    char unknown_4A4E45[131072];
    char field_6367D[108435];
    char mesCache[1024];
    char field_7E210[1576];
    char unknown_4E0000[345421];
    char unknown_53454D[758];
    char unknown_534843[65536];
    char unknown_544843[67074];
    char unknown_554E45[55683];
    int mesCursorBookmarks[50];
    int g_dword_562890[50];
    mapInfo* pMap;
    qint32 mapLeft;
    qint32 mapTop;
    qint32 mapWidth;
    qint32 mapHeight;
    qint32 cpsLeft;
    qint32 cpsTop;
    qint32 cpsWidth;
    qint32 cpsHeight;
    quint32 halfWindowWidth_Maybe;
    quint32 halfWindowHeight_Maybe;
    quint8 field_C0D4B[7];
    char field_1011C3;
};

typedef struct tag_MesCacheState
{
    quint32 bActive;
    quint32 mesCacheCursor;
    quint32 size;
} MesCacheState;

typedef struct tag_GStruct
{
    quint32 mesStartPoint;
    MainStruct* pMainStruct;
    quint32 mesCacheCursor;
    MesCacheState mesCaches[32];
    quint32 gMesCursor;
    quint16 field_190;

} GStruct;

typedef struct tag_DecodeHelper
{
    quint8 workingBuffer[4096];
    quint8* p_sourceBuffer;
    quint8* p_destBuffer;
    quint32 field_1008;
    qint32 index;
    qint32 index_destBuffer;
} DecodeHelper;

class Engine;
class QWaitCondition;

extern MainStruct gMainStruct;
extern GStruct gStruct;
extern Engine *gpEngine;
extern qint32 gNotReady;
extern Engine *threadGpEngine;
extern quint16 gMesCachesIndex;
extern QWaitCondition animating;
extern quint32 timer;

#endif // FLAG_H
