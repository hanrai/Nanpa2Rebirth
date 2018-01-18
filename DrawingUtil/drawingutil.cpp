#include <QPainter>
#include <QDebug>
#include <QFont>
#include <QGuiApplication>
#include <QMutex>
#include "drawingutil.h"
//#include "MainWindow.h"
//#include "Files/DecodeArcMes.h"
//#include "Decoder/decoder.h"

DrawingUtil::DrawingUtil(MainWindow *pMainWindow, Config *pConfig, QObject *parent) :
    QObject(parent),
    colors(256, 0)
{
    //init begin
    this->pMainWindow = pMainWindow;
    this->pConfig = pConfig;
    pFont = 0;
    pBitmap = 0;
    pPalette = 0;
    //init end

    //init graphics
    pBitmap = new Bitmap;
    pBitmap->pDecodeHelper = 0;

    pPalette = new PaletteInfo;
    memset(pPalette, 0, sizeof(PaletteInfo));
    pPalette->pLogPalette = 0;

    pFont = new FontInfo;
    pFont->pFont = 0;
    pFont->pLogFont = 0;

    pBitmap->pDecodeHelper = new DecodeHelper;

    pPalette->pLogPalette = new LogPalette;
    memset (pPalette->pLogPalette, 0, sizeof(LogPalette));
    //中间很多事情做不了，不过都是清零以及标记动画调色盘条目而已，没什么用了
    pPalette->pLogPalette->logPalette.palVersion = 0x300;//没用了
    pPalette->pLogPalette->logPalette.palNumEntries = 256;

    pFont->pLogFont = new LOGFONT;
    pFont->pLogFont->lfHeight = 16;
    pFont->pLogFont->lfWidth = 8;
    pFont->pLogFont->lfEscapement = 0;
    pFont->pLogFont->lfOrientation = 0;
    pFont->pLogFont->lfWeight = 0;
    pFont->pLogFont->lfItalic = 0;
    pFont->pLogFont->lfUnderline = 0;
    pFont->pLogFont->lfStrikeOut = 0;
    pFont->pLogFont->lfCharSet = -128;
    pFont->pLogFont->lfOutPrecision = 3;
    pFont->pLogFont->lfClipPrecision = 0;
    pFont->pLogFont->lfQuality = 0;
    pFont->pLogFont->lfPitchAndFamily = 49;
    //strcpy(pFont->pLogFont->lfFaceName, "微软雅黑");

    bNeedMixPalettes = 0;
    config_BGType = 1;//需要改成从配置文件取值
    //init end

    //init drawing part begin
    quint16* p = (quint16*)pBitmap->bitmapInfo.bmiColors;
    for(int i=0; i<256; i++)
        p[i] = i;
    setTxtColor(7);
    //init end

    //init textPad
    textPad = QImage(32, 32, QImage::Format_ARGB32_Premultiplied);
    //init textPad end

    //为了便利
    logpe = this->pPalette->pLogPalette->logPalette.palPalEntry;

    //update signal
    connect(this, &DrawingUtil::framebufferUpdated, pMainWindow, &MainWindow::on_framebufferUpdated);
    connect(this, &DrawingUtil::paletteUpdated, pMainWindow, &MainWindow::on_paletteUpdated);
}

DrawingUtil::~DrawingUtil()
{
    delete pFont->pFont;
    delete pFont->pLogFont;
    delete pPalette->pLogPalette;
    delete pBitmap->pDecodeHelper;
    delete pFont;
    delete pPalette;
    delete pBitmap;
}

void DrawingUtil::setTargetPalette(quint32 *buffer)
{
    memcpy(pPalette->targetPalette, buffer, 236*4);
}

void DrawingUtil::makeStepPalette(quint8 step)
{
    PALETTEENTRY *steppe = pPalette->stepPalette;
    PALETTEENTRY *targpe = pPalette->targetPalette;
    for(quint8 i=0; i<236; i++)
    {
        steppe[i].peRed = calculateStep(logpe[i].peRed, targpe[i].peRed, step);
        steppe[i].peGreen = calculateStep(logpe[i].peGreen, targpe[i].peGreen, step);
        steppe[i].peBlue = calculateStep(logpe[i].peBlue, targpe[i].peBlue, step);
    }
}

void DrawingUtil::copyTargetPalette(qint32 start, qint32 count)
{
    if(start < count+start)
    {
        quint32* pSrcStart = (quint32*)&this->pPalette->targetPalette[start];
        quint32* pDesStart = (quint32*)&logpe[start];
        memcpy(pDesStart, pSrcStart, count*4);
    }
    sendLogPalette();
    setColor();
}

void DrawingUtil::paintingImg(quint8 command, qint32 left, qint32 top, qint32 width, qint32 height, VramInfo *srcVram, qint32 targetLeft, qint32 targetTop, VramInfo *destVram, qint32 finalLeft, qint32 finalTop, VramInfo *finalVram, qint32 maskIndex)
{
    switch(command)
    {
    case 0:
        paintImgToTarget(left, top, width, height, srcVram, targetLeft, targetTop, destVram);
        break;
    case 1:
        paintImgToTargetWithMask(left, top, width, height, srcVram, targetLeft, targetTop, destVram, maskIndex);
        break;
    case 2:
        fillBackground(left, top, width, height, srcVram, maskIndex);
        break;
    case 3:
        switchImgs(left, top, width, height, srcVram, targetLeft, targetTop, destVram);
        break;
    case 4:
        drawHighlight(left, top, width, height, srcVram, maskIndex);
        break;
    case 5:
        slideEffect(left, top, width, height, srcVram, targetLeft, targetTop, destVram);
        break;
    default:
        return;
    }
}

void DrawingUtil::updateWindow(quint16 left, quint16 top, quint16 width, quint16 height)
{
    QByteArray data;
    data.resize(width*height);
    VramInfo v;
    v.left = 0;
    v.top = 0;
    v.width = width;
    v.height = height;
    v.data = (quint8*)data.data();
    paintFramebuffer(left, top, width, height, &gpEngine->vrams[0], 0, 0, &v);
    QRect rect(left, top, width, height);
    emit framebufferUpdated(rect, data);
}

void DrawingUtil::paintFramebuffer(quint32 left,
                                   quint32 top,
                                   quint32 width,
                                   quint32 height,
                                   VramInfo *sourceVram,
                                   quint32 targetLeft,
                                   quint32 targetTop,
                                   VramInfo *destVram)
{
    quint32 h = qMin(height, (quint32)destVram->height);
    quint32 w = qMin(width, (quint32)destVram->width);
    if(!h)
        return;
    for(quint32 i=0; i<h; i++)
    {
        quint8* s = &sourceVram->data[sourceVram->width*(sourceVram->height-(top+i)-1)+left];
        quint8* d = &destVram->data[destVram->width*(targetTop+i)+targetLeft];
        memcpy(d, s, w);
    }
}

void DrawingUtil::decodeGP8(char *srcBuffer, quint32 *palette, VramInfo *pVram, qint16 top, qint16 left)
{
    typedef struct
    {
        quint16 top;
        quint16 left;
        quint16 width;
        quint16 height;
        quint8 palette[1024];
        quint8 data[1];
    } GP8Header;

    Decoder decoder;

    GP8Header* header = (GP8Header*)srcBuffer;
    quint8* srcPalette = header->palette;
    quint8* destPalette = (quint8*)palette;
    for(int i=0; i<256; i++)
    {
        destPalette[0] = srcPalette[2];
        destPalette[1] = srcPalette[1];
        destPalette[2] = srcPalette[0];
        destPalette += 4;
        srcPalette += 4;
    }

    auto t = top;
    if (top == -1)
        t = header->top;

    auto l = left;
    if (left == -1)
        l = header->left;

    auto w = header->width;
    if(w%4)
        w = 4*(w/4)+4;

    auto h = header->height;

    decoder.decodeGP8(header->data, pVram, t, l, w, h);
    pBitmap->top = t;
    pBitmap->left = l;
    pBitmap->width = w;
    pBitmap->height = h;
}

void DrawingUtil::drawChar(Engine *pEngine, VramInfo *pVram, quint16 txt, quint16 txtLeft, quint16 txtTop, int style)
{
    textPad.fill(QColor(0,0,0,0));
    ByteWorker c;
    c.dword = 0;
    c.word[0] = txt;
    ByteWorker t;
    t.dword = 0;
    if((c.byte[1]<0x81 || c.byte[1]>0xFE) && (c.byte[1]<0xE0 || c.byte[1]>0xEF))
        t.byte[0] = c.byte[0];
    else
    {
        t.byte[0] = c.byte[1];
        t.byte[1] = c.byte[0];
    }
    QString text = QString::fromLocal8Bit((char*)&t.dword);
    QPainter painter(&textPad);
    QFont font("微软雅黑");
    font.setPixelSize(pFont->pLogFont->lfHeight);
    font.setStyleStrategy((QFont::StyleStrategy)(QFont::PreferBitmap|
                          QFont::PreferDevice|
                          QFont::NoAntialias|
                          QFont::NoSubpixelAntialias|
                          QFont::PreferMatch));
    painter.setFont(font);
    painter.setPen(QColor(0xFF, 0xFF, 0xFF, 0xFF));
    painter.setBrush(QBrush(QColor(0xFF, 0xFF, 0xFF, 0xFF)));
    painter.setBackgroundMode(Qt::TransparentMode);
    auto fontHigh = pEngine->Flag->dim16Bit.font_height-3;
    painter.drawText(1, 0+fontHigh, text);
    painter.drawText(0, 1+fontHigh, text);
    for(int y=0; y<pEngine->Flag->dim16Bit.font_height; y++)
    {
        quint8* pBuffer = (quint8*)&pVram->data[pVram->width * (pVram->height - (y + txtTop) - 1)];
        for(int x=0; x<pEngine->Flag->dim16Bit.font_width; x++)
        {
            if(textPad.pixel(x,y))
                pBuffer[txtLeft+x] = 2;
        }
    }
    textPad.fill(QColor(0, 0, 0, 0));
    painter.drawText(0, 0+fontHigh, text);
    switch(style)
    {
    case 1:
        painter.drawText(-1, 0+fontHigh, text);
        break;
    case 2:
        painter.drawText(0, 1+fontHigh, text);
        break;
    case 3:
        painter.drawText(-1, 0+fontHigh, text);
        painter.drawText(0, 1+fontHigh, text);
        painter.drawText(-1, 1+fontHigh, text);
        break;
    default:
        break;
    }
    for(int y=0; y<pEngine->Flag->dim16Bit.font_height; y++)
    {
        quint8* pBuffer = &pVram->data[pVram->width * (pVram->height - (y + txtTop) - 1)];
        for(int x=0; x<pEngine->Flag->dim16Bit.font_width; x++)
        {
            if(textPad.pixel(x,y))
                pBuffer[txtLeft+x] = txtColorIndex;
        }
    }
}

void DrawingUtil::drawHighlight(quint16 left, quint16 top, quint16 width, quint16 height, VramInfo *vRam, quint16 mask)
{
    quint8 highlight = (quint8)((mask>>8)&0xFF);
    quint8 darklight = (quint8)(mask&0xFF);
    quint16 w = qMin(width, (quint16)vRam->width);
    quint16 h = qMin(height, (quint16)vRam->height);
    for(auto y=0; y<h; y++)
    {
        quint8* data = &vRam->data[vRam->width*(vRam->height-(y+top)-1)+left];
        for(auto x=0; x<w; x++)
        {
            if(data[x] == darklight)
                data[x] = highlight;
            else if(data[x] == highlight)
                    data[x] = darklight;
        }
    }
}

void DrawingUtil::slideEffect(quint32 left, quint32 top, quint32 width, quint32 height, VramInfo *sourceVRam, quint32 targetLeft, quint32 targetTop, VramInfo *targetVRam)
{
    QMutex m;
    VramInfo *pSourceVRam; // ebp@1
    VramInfo *pTargetVRam; // edi@1
    int sourceScanline; // esi@1
    int scanlineOffset; // edx@3
    int v13; // ebx@3
    int v14; // ecx@5
    int v15; // esi@5
    unsigned int v16; // edi@6
    int v17; // edx@7
    char *v18; // edx@7
    char *v19; // ebx@7
    int result; // eax@12
    quint8 y; // [sp+10h] [bp-24h]@1
    unsigned int v22; // [sp+14h] [bp-20h]@4
    signed int j; // [sp+18h] [bp-1Ch]@2
    signed int i; // [sp+1Ch] [bp-18h]@1
    int targetStartAddress; // [sp+20h] [bp-14h]@1
    int v26; // [sp+28h] [bp-Ch]@5
    int v27; // [sp+2Ch] [bp-8h]@5
    int sourceScanline_; // [sp+30h] [bp-4h]@1
    quint8 step2; // [sp+3Ch] [bp+8h]@2
    quint8 x; // [sp+4Ch] [bp+18h]@3
    quint8 step1; // [sp+50h] [bp+1Ch]@1

    pSourceVRam = sourceVRam;
    pTargetVRam = targetVRam;
    sourceScanline = top + sourceVRam->width * (sourceVRam->height - top - 1);
    sourceScanline_ = top + sourceVRam->width * (sourceVRam->height - top - 1);
    y = 0;
    targetStartAddress = targetLeft + targetVRam->width * (targetVRam->height - targetTop - 1);
    step1 = 0;
    i = 8;
    do
    {
      j = 8;
      step2 = step1;
      do
      {
        x = step2;
        scanlineOffset = sourceScanline - pSourceVRam->width * y;
        v13 = targetStartAddress - pTargetVRam->width * y;
        if ( y < (signed int)height )
        {
          v22 = (height - (unsigned int)y + 7) >> 3;
          do
          {
            v14 = scanlineOffset;
            v15 = v13;
            scanlineOffset += -8 * pSourceVRam->width;
            v26 = scanlineOffset;
            v13 += -8 * pTargetVRam->width;
            v27 = v13;
            if ( x < (signed int)width )
            {
              v16 = (width - (unsigned int)x + 7) >> 3;
              do
              {
                v17 = v14;
                v14 += 8;
                v18 = (char*)&pSourceVRam->data[v17];
                v19 = (char*)&targetVRam->data[v15];
                pSourceVRam = sourceVRam;
                v15 += 8;
                --v16;
                v19[x] = v18[x];
              }
              while ( v16 );
              scanlineOffset = v26;
              v13 = v27;
            }
            pTargetVRam = targetVRam;
            x = (x - 3) & 7;
            --v22;
          }
          while ( v22 );
          sourceScanline = sourceScanline_;
        }
        y = (y + 3) & 7;
        step2 = (step2 + 1) & 7;
        m.lock();
        pMainWindow->vSync.wait(&m);
        m.unlock();
        updateWindow(left, top, width, height);
        --j;
      }
      while ( j );
      step1 = (step1 - 3) & 7;
      result = i-- - 1;
    }
    while ( i );
}

void DrawingUtil::switchImgs(quint32 left, quint32 top, quint32 width, quint32 height, VramInfo *sourceVram, quint32 targetLeft, quint32 targetTop, VramInfo *destVram)
{
    qint32 h = qMin(height, (quint32)destVram->height);
    qint32 w = qMin(width, (quint32)destVram->width);
    if(!h)
        return;
    for(auto y=0; y<h; y++)
    {
        char* sData = (char*)&sourceVram->data[sourceVram->width*(sourceVram->height-(top+y)-1)+left];
        char* dData = (char*)&destVram->data[destVram->width*(destVram->height-(targetTop+y)-1)+targetLeft];
        memcpy(pBitmap->scanlineBuffer, sData, w);
        memcpy(sData, dData, w);
        memcpy(dData, pBitmap->scanlineBuffer, w);
    }
}

void DrawingUtil::sendLogPalette()
{
    QVector<QRgb> colorTable;
    for(int i=0; i<256; i++)
        colorTable << qRgb(logpe[i].peRed, logpe[i].peGreen, logpe[i].peBlue);
    emit paletteUpdated(colorTable);
}

quint8 DrawingUtil::calculateStep(quint8 a, quint8 b, quint8 step)
{
    if(a == b)
        return 0;
    quint8 diff = qMax(a, b)-qMin(a, b);
    return diff/step+1;
}

void DrawingUtil::setColor()
{
    PALETTEENTRY* p = pPalette->pLogPalette->logPalette.palPalEntry;
    for(int i=0; i<256; i++)
        colors[i] = qRgb(p->peRed, p->peGreen, p->peBlue);
    colors[1] = qRgb(0xFF, 0xFF, 0xFF);
    colors[2] = qRgb(0, 0, 0);
    backgroundColor = colors[bkColorIndex];
    textColor = colors[txtColorIndex];
}

void DrawingUtil::setTxtColor(quint16 colorIndex)
{
    bkColorIndex = (colorIndex&0xFF00)>>8;
    txtColorIndex = colorIndex&0xFF;
    colorIndexCombo = colorIndex;
    backgroundColor = colors[bkColorIndex];
    textColor = colors[txtColorIndex];
}

void DrawingUtil::fillBackground(quint32 left,
                                 quint32 top,
                                 quint32 width,
                                 quint32 height,
                                 VramInfo* sourceVram,
                                 quint16 colorIndex)
{
    quint8 color = (colorIndex>>8) & 0xFF;
    quint32 ramWidth = sourceVram->width;
    quint32 ramHeight = sourceVram->height;
    quint32 paintHeight = qMin(ramHeight, height);
    quint32 paintWidth = qMin(ramWidth, width);
    if(!paintHeight)
        return;

    for(quint32 y=0; y<height; y++)
    {
        quint8* buffer = &sourceVram->data[ramWidth*(ramHeight-(y+top)-1)]+left;
        memset(buffer, color, paintWidth);
    }
}

void DrawingUtil::paintImgToTarget(quint32 left,
                                   quint32 top,
                                   quint32 width,
                                   quint32 height,
                                   VramInfo *sourceVram,
                                   quint32 targetLeft,
                                   quint32 targetTop,
                                   VramInfo *destVram)
{
    quint32 h = qMin(height, (quint32)destVram->height);
    quint32 w = qMin(width, (quint32)destVram->width);
    if(!h)
        return;
    for(int i=0; i<h; i++)
    {

        quint8* s = &sourceVram->data[sourceVram->width*(sourceVram->height-(top+i)-1)+left];
        quint8* d = &destVram->data[destVram->width*(destVram->height-(targetTop+i)-1)+targetLeft];
        memcpy(d, s, w);
    }
}

void DrawingUtil::paintImgToTargetWithMask(quint32 left, quint32 top, quint32 width, quint32 height, VramInfo *sourceVram, quint32 targetLeft, quint32 targetTop, VramInfo *destVram, quint8 mask)
{
    quint32 h = qMin(height, (quint32)destVram->height);
    quint32 w = qMin(width, (quint32)destVram->width);
    if(!h)
        return;
    for(int i=0; i<h; i++)
    {

        quint8* s = &sourceVram->data[sourceVram->width*(sourceVram->height-(top+i)-1)+left];
        quint8* d = &destVram->data[destVram->width*(destVram->height-(targetTop+i)-1)+targetLeft];
        for(int j=0; j<w; j++)
            if(s[j] != mask)
                d[j] = s[j];
    }
}

void DrawingUtil::MixTwoPalette()
{
    if(!bNeedMixPalettes)
        return;

    bool flag = true;
    PALETTEENTRY* targetpe = this->pPalette->targetPalette;
    PALETTEENTRY* steppe = this->pPalette->stepPalette;

    for(int i=this->startPoint; i<this->newEntryCount; i++)
    {
        flag &= mixColor(logpe[i].peRed, targetpe[i].peRed, steppe[i].peRed);
        flag &= mixColor(logpe[i].peGreen, targetpe[i].peGreen, steppe[i].peGreen);
        flag &= mixColor(logpe[i].peBlue, targetpe[i].peBlue, steppe[i].peBlue);
    }

    QVector<QRgb> colorTable;
    for(int i=0; i<256; i++)
        colorTable << qRgb(logpe[i].peRed, logpe[i].peGreen, logpe[i].peBlue);
    emit paletteUpdated(colorTable);

    if(flag)
    {
        bNeedMixPalettes = 0;
        animating.wakeAll();
    }
}

bool DrawingUtil::mixColor(quint8 &cur, qint16 target, qint16 step)
{
    bool flag = true;
    qint16 res;
    if(cur == target)
        return true;
    flag = false;
    if(cur<=target)
    {
        res = cur+step;
        cur = qMin(res, target);
    }
    else
    {
        res = cur-step;
        cur = qMax(res, target);
    }
    return flag;
}

void DrawingUtil::waitAnimating()
{
    //之后要考虑是否需要增加game state判断和bNeexMixPalettes判断
    QMutex mute;
    mute.lock();
    if(bNeedMixPalettes)
        animating.wait(&mute);
    mute.unlock();

    setColor();
}

void DrawingUtil::waitPaletteAnimating(quint8 start, quint8 count)
{
    startPoint = start;
    newEntryCount = count;
    bNeedMixPalettes = 1;
    waitAnimating();
}
