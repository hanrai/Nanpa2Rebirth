#ifndef DRAWINGUTIL_H
#define DRAWINGUTIL_H

#include <QObject>
#include <QImage>
#include <windows.h>
#include "Flag.h"
#include "Engine.h"

class MainWindow;
class Config;
class QFont;
class Engine;

typedef struct
{
    BITMAPINFO bitmapInfo;
    RGBQUAD colors[127];
    DecodeHelper *pDecodeHelper;
    quint8 scanlineBuffer[640];
    qint16 top;
    qint16 left;
    qint16 width;
    qint16 height;
} Bitmap;

typedef struct
{
    LOGPALETTE logPalette;
    PALETTEENTRY paletteentry[256];
} LogPalette;

typedef struct
{
    PALETTEENTRY targetPalette[256];
    LogPalette* pLogPalette;
    quint32 hLogPalette;
    PALETTEENTRY stepPalette[256];
} PaletteInfo;

typedef struct
{
    QFont* pFont;
    LOGFONT* pLogFont;
} FontInfo;

class DrawingUtil : public QObject
{
    Q_OBJECT
public:
    explicit DrawingUtil(MainWindow* pMainWindow, Config *pConfig = 0, QObject *parent = 0);
    ~DrawingUtil();
    void setColor();
    void setTxtColor(quint16 colorIndex);

    void setDrawingUtilPalette(quint32* buffer);

    void MixTwoPalette();
    bool mixColor(quint8 &cur, qint16 target, qint16 step);
    void waitAnimating();
    void waitPaletteAnimating(quint8 start, quint8 count);
    void setTargetPalette(quint32* buffer);
    void makeStepPalette(quint8 step);
    void copyTargetPalette(qint32 start, qint32 count);
    void paintingImg(quint8 command,
                     qint32 left,
                     qint32 top,
                     qint32 width,
                     qint32 height,
                     VramInfo* srcVram,
                     qint32 targetLeft,
                     qint32 targetTop,
                     VramInfo* destVram,
                     qint32 finalLeft,
                     qint32 finalTop,
                     VramInfo* finalVram,
                     qint32 maskIndex);
    void fillBackground(quint32 left, quint32 top, quint32 width, quint32 height, VramInfo *sourceVram, quint16 colorIndex);
    void paintImgToTarget(quint32 left,
                          quint32 top,
                          quint32 width,
                          quint32 height,
                          VramInfo *sourceVram,
                          quint32 targetLeft,
                          quint32 targetTop,
                          VramInfo *destVram);
    void paintImgToTargetWithMask(quint32 left,
                                  quint32 top,
                                  quint32 width,
                                  quint32 height,
                                  VramInfo *sourceVram,
                                  quint32 targetLeft,
                                  quint32 targetTop,
                                  VramInfo *destVram,
                                  quint8 mask);
    void updateWindow(quint16 left, quint16 top, quint16 width, quint16 height);
    void decodeGP8(char* srcBuffer, quint32* palette, VramInfo* pVram, qint16 top, qint16 left);
    void drawChar(Engine* pEngine, VramInfo* pVram, quint16 txt, quint16 winLeft, quint16 winTop, int a7);
    void drawHighlight(quint16 left, quint16 top, quint16 width, quint16 height, VramInfo* vRam, quint16 mask);
    void slideEffect(quint32 left,
                     quint32 top,
                     quint32 width,
                     quint32 height,
                     VramInfo *sourceVram,
                     quint32 targetLeft,
                     quint32 targetTop,
                     VramInfo *targetVRam);
    void switchImgs(quint32 left,
                    quint32 top,
                    quint32 width,
                    quint32 height,
                    VramInfo *sourceVram,
                    quint32 targetLeft,
                    quint32 targetTop,
                    VramInfo *destVram);

    quint32 bNeedMixPalettes;

signals:
    void paletteUpdated(QVector<QRgb> colorTable);
    void framebufferUpdated(QRect rect, QByteArray data);

public slots:

private:
    //begin
public:
    Bitmap* pBitmap;
    PaletteInfo* pPalette;
private:
    MainWindow* pMainWindow;
    FontInfo* pFont;
    Config *pConfig;
    quint32 field_14;
    quint32 startPoint;
    quint32 newEntryCount;
    quint32 config_BGType;
    quint32 field_28;
    void* hdcCompatible;
    void* font_hGdiObj;
    void* hBitmap;
    void* hBitmapGdiObj;
    void *ppbBits;
    quint32 font_double_width;
    quint32 font_height;
    quint16 colorIndexCombo;
    quint16 bkColorIndex;
    quint16 txtColorIndex;
    QVector<QRgb> colors;
    quint16 field_44A;
    quint8 field_44C;
    //end

    //hdcTextPad sim
    QImage textPad;
    QRgb backgroundColor;
    QRgb textColor;
    //hdcTextPad sim end

    //为了便利
    void sendLogPalette();
    quint8 calculateStep(quint8 a, quint8 b, quint8 step);
public:
    PALETTEENTRY* logpe;
private:
    void paintFramebuffer(quint32 left,
                          quint32 top,
                          quint32 width,
                          quint32 height,
                          VramInfo *sourceVram,
                          quint32 targetLeft,
                          quint32 targetTop,
                          VramInfo *destVram);
};
#endif // DRAWINGUTIL_H
