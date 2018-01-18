#ifndef IMGWINDOW_H
#define IMGWINDOW_H

#include <QList>
#include <QRectF>
#include <QPointF>
#include <QMouseEvent>
#include <QTimer>
#include <QImage>
#include <QWaitCondition>
#include <QRasterWindow>
#include <QOpenGLWindow>
#include <QAudioOutput>
#include <QBuffer>
struct WaveFileHeader
{
    char        id[4];          // should always contain "RIFF"
    int         totallength;    // total file length minus 8
    char        wavefmt[8];     // should be "WAVEfmt "
    int         format;         // 16 for PCM format
    short       pcm;            // 1 for PCM format
    short       channelCount;       // channels
    int         sampleRate;      // sampling frequency
    int         bytes_per_second;
    short       bytes_by_capture;
    short       bits_per_sample;
    char        data[4];        // should always contain "data"
    int         bytes_in_data;
};
class Config;

class MainWindow : public QRasterWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWindow *parent = 0);
    void SetCursorList(QList<QImage> cursorList)
        {cursors = cursorList;}
    bool checkInput(qint32 index);
    QPoint cursorPos;

    QWaitCondition userInput;
    QWaitCondition vSync;
    quint16 inputFlag;

signals:

public slots:
    void on_Engine_hideCursor();
    void on_Engine_showCursor();
    void on_Engine_BGMStarted(QByteArray data, bool isLoop = false);
    void on_AnimationEngine_cursorAnimated(qint8 id);
    void on_paletteUpdated(QVector<QRgb> palette);
    void on_framebufferUpdated(QRect rect, QByteArray data);
    void on_Engine_voiceStarted(QByteArray data);

protected:
    void mouseMoveEvent(QMouseEvent* ev);
    void keyPressEvent(QKeyEvent* ev);
    void keyReleaseEvent(QKeyEvent* ev);
    void mousePressEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent* ev);
    void resizeEvent(QResizeEvent *ev);
    void paintEvent(QPaintEvent *event);

private:
    void mountCursor();
    //Init begin
public:
    Config *pConfig;
private:
    quint32 hDesktop;//没用了？
    quint32 menuID;
    char* title[128];//没用了？
    quint32 gameState;
    //gameState
    //0=正常？
    //1=进行游戏暂停操作？
    //2=暂停？
    //3=恢复游戏
    //255=结束游戏
    quint16 inputFlags;
    quint16 field_92;
    quint32 hInstance;//没用了？
    char field_98[8];//宽和高？
    quint32 top;
    quint32 left;
    //Init end

    QImage framebuffer;
    QTransform transformer;
    QTransform transback;
    QPointF cursorPosition;

    QImage cursor;
    QList<QImage> cursors;
    QRectF viewportRect;
    qreal scale;
    quint32 cursorIndex;
    bool showCursor;
    QCursor cur;

    //sounds
    QByteArray BGMData;
    QSharedPointer<QBuffer> BGMBuffer;
    QSharedPointer<QAudioOutput> BGMPlayer;
    QByteArray voiceData;
    QSharedPointer<QBuffer> voiceBuffer;
    QSharedPointer<QAudioOutput> voicePlayer;
};

#endif // IMGWINDOW_H
