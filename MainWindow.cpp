#include "MainWindow.h"
#include <QPainter>
#include <QtDebug>
#include <QGuiApplication>

MainWindow::MainWindow(QWindow *parent) :
    QRasterWindow(parent),
    showCursor(true)
{
    //Init begin

    //Init end

    setMinimumHeight(400);
    setMinimumWidth(640);

    framebuffer=QImage(640, 400, QImage::Format_Indexed8);
    framebuffer.fill(Qt::black);

    cursorIndex = 0;
    inputFlag = 0;
    setTitle("Nanap2 rebirth engine WIP version. By hanrai@gmail.com");
}

bool MainWindow::checkInput(qint32 index)
{
    switch(index)
    {
    case 0:
        if(inputFlag&0x0001 || inputFlag&0x0002)
            return true;
        return inputFlag&0x2000;
    case 1:
        if(inputFlag&0x0004)
            return true;
        return inputFlag&0x4000;
    case 2:
        if(inputFlag&0x0008)
            return true;
        return inputFlag&0x0010;
    case 3:
        if(inputFlag&0x0020)
            return true;
        return inputFlag&0x0040;
    case 4:
        if(inputFlag&0x0080)
            return true;
        return inputFlag&0x0100;
    case 5:
        if(inputFlag&0x0200)
            return true;
        return inputFlag&0x0400;
    case 6:
        return QGuiApplication::queryKeyboardModifiers()&Qt::ShiftModifier;
    case 7:
        return inputFlag&0x1000;
    case 8:
        return inputFlag&0x2000;
    default:
        break;
    }
    return false;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if(geometry().width()<1 || geometry().height()<1
            || framebuffer.width()<1 || framebuffer.height()<1)
        return;
    if(!event->rect().intersects(viewportRect.toRect()))
        return;

    QRectF updateRect = event->rect().intersected(viewportRect.toRect());
    QRectF sourceRect = transback.mapRect(updateRect);

    painter.setClipRect(updateRect);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter.drawImage(updateRect, framebuffer.convertToFormat(QImage::Format_ARGB32_Premultiplied), sourceRect);
}

void MainWindow::mountCursor()
{
    if(!showCursor)
        return;
    if(cursor.isNull())
        return;
//    QRectF cursorRect = transformer.mapRect(cursor.rect());
//    QImage cursorImage(cursorRect.size().toSize(), QImage::Format_ARGB32_Premultiplied);
//    QPainter painter(&cursorImage);
//    painter.drawImage(cursorImage.rect(), cursor, cursor.rect());
//    QPixmap cursorPixmap = QPixmap::fromImage(cursorImage);
    //cur = QCursor(cursorPixmap);
    cur = QCursor(QPixmap::fromImage(cursor));
    setCursor(cur);
}

void MainWindow::mouseMoveEvent(QMouseEvent *ev)
{
    if(!viewportRect.contains(ev->pos()))
    {
        unsetCursor();
        return;
    }
    setCursor(cur);

    cursorPosition = ev->pos();
    cursorPos = transback.map(cursorPosition).toPoint();
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    switch(ev->key())
    {
    case Qt::Key_2:
    case Qt::Key_Down:
        inputFlag |= 0x0020;
        break;
    case Qt::Key_8:
    case Qt::Key_Up:
        inputFlag |= 0x0008;
        break;
    case Qt::Key_6:
    case Qt::Key_Right:
        inputFlag |= 0x0200;
        break;
    case Qt::Key_4:
    case Qt::Key_Left:
        inputFlag |= 0x0080;
        break;
    case Qt::Key_Control:
        inputFlag |= 0x1000;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        inputFlag |= 0x2000;
        break;
    case Qt::Key_Escape:
        inputFlag |= 0x4000;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    switch(ev->key())
    {
    case Qt::Key_2:
    case Qt::Key_Down:
        inputFlag &= 0xFFDF;
        break;
    case Qt::Key_8:
    case Qt::Key_Up:
        inputFlag &= 0xFFF7;
        break;
    case Qt::Key_6:
    case Qt::Key_Right:
        inputFlag &= 0xFDFF;
        break;
    case Qt::Key_4:
    case Qt::Key_Left:
        inputFlag &= 0xFF7F;
        break;
    case Qt::Key_Control:
        inputFlag &= 0xEFFF;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        inputFlag &= 0xDFFF;
        break;
    case Qt::Key_Escape:
        inputFlag &= 0xBFFF;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    switch(ev->button())
    {
    case Qt::LeftButton:
        inputFlag |= 0x2000;
        break;
    case Qt::RightButton:
        inputFlag |= 0x4000;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    switch(ev->button())
    {
    case Qt::LeftButton:
        inputFlag &= 0xDFFF;
        break;
    case Qt::RightButton:
        inputFlag &= 0xBFFF;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    qreal ratio = (qreal)framebuffer.width()/(qreal)framebuffer.height();
    qreal winWidth = geometry().width();
    qreal winHeight = geometry().height();
    qreal top,left,width,height;

    if(winWidth/winHeight >= ratio)
    {
        height = winHeight;
        width = height*ratio;
    }
    else
    {
        width = winWidth;
        height = width/ratio;
    }

    top = (winHeight-height)/2;
    left = (winWidth-width)/2;

    scale = width/framebuffer.width();
    viewportRect.setRect(left, top, width, height);
    transformer = QTransform(scale, 0, 0, scale, left, top);
    transback = transformer.inverted();
    mountCursor();
    update();
}

void MainWindow::on_Engine_hideCursor()
{
    if(!showCursor)
        return;
    setCursor(QCursor(Qt::BlankCursor));
    showCursor = false;
}

void MainWindow::on_Engine_showCursor()
{
    if(showCursor)
        return;
    showCursor = true;
    mountCursor();
}

void MainWindow::on_Engine_BGMStarted(QByteArray data, bool isLoop)
{
    BGMData = data;
    WaveFileHeader *header = (WaveFileHeader*)BGMData.data();

    QAudioFormat format;
    format.setSampleRate(header->sampleRate);
    format.setChannelCount(header->channelCount);
    format.setSampleSize(header->bits_per_sample);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    char* audioRawData = BGMData.data() + 44;

    BGMBuffer = decltype(BGMBuffer)(new QBuffer(this));
    BGMBuffer->setData(audioRawData, header->bytes_in_data);
    BGMBuffer->open(QIODevice::ReadOnly);

    BGMPlayer = decltype(BGMPlayer)(new QAudioOutput(format, this));
    if(isLoop)
        connect(BGMPlayer.data(), &QAudioOutput::stateChanged,
                [&](QAudio::State state)
        {
            if(state == QAudio::IdleState)
            {
                BGMBuffer->seek(0);
                BGMPlayer->start(BGMBuffer.data());
            }
        });

    BGMPlayer->start(BGMBuffer.data());
}

void MainWindow::on_AnimationEngine_cursorAnimated(qint8 id)
{
    if(cursors.isEmpty())
        return;
    if(!showCursor)
        return;
    cursor = cursors[id];
    mountCursor();
}

void MainWindow::on_paletteUpdated(QVector<QRgb> palette)
{
    framebuffer.setColorTable(palette);
    update();
}

void MainWindow::on_framebufferUpdated(QRect rect, QByteArray data)
{
    quint8* fbuffer = framebuffer.bits();
    for(int y=rect.top(); y<rect.bottom()+1; y++)
        memcpy(fbuffer+y*framebuffer.width()+rect.left(),
               data.data()+(y-rect.top())*rect.width(),
               rect.width());
    QRect targetRect = transformer.mapRect(rect);
    targetRect.adjust(-1,-1,1,1);
    update(targetRect.united(viewportRect.toRect()));
}

void MainWindow::on_Engine_voiceStarted(QByteArray data)
{
        voiceData = data;
        WaveFileHeader *header = (WaveFileHeader*)voiceData.data();

        QAudioFormat format;
        format.setSampleRate(22050);
        format.setChannelCount(1);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);

        char* audioRawData = voiceData.data()+44;

        voiceBuffer = decltype(voiceBuffer)(new QBuffer(this));
        voiceBuffer->setData(audioRawData, voiceData.size()-44);
        voiceBuffer->open(QIODevice::ReadOnly);

        voicePlayer = decltype(voicePlayer)(new QAudioOutput(format, this));

        voicePlayer->start(voiceBuffer.data());
}

