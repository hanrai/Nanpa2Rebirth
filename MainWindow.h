#ifndef IMGWINDOW_H
#define IMGWINDOW_H

#include <QList>
#include <QRectF>
#include <QPointF>
#include <QMouseEvent>
#include <QImage>
#include <QWaitCondition>
#include <QRasterWindow>
#include <QCursor>
#include <QTransform>

class Config;

enum InputBit : quint16 {
    InputMouseLeft   = 0x0001,
    InputMouseRight  = 0x0002,
    InputAltCancel   = 0x0004,
    InputUpKey       = 0x0008,
    InputAltUp       = 0x0010,
    InputDownKey     = 0x0020,
    InputAltDown     = 0x0040,
    InputLeftKey     = 0x0080,
    InputAltLeft     = 0x0100,
    InputRightKey    = 0x0200,
    InputAltRight    = 0x0400,
    InputControl     = 0x1000,
    InputConfirm     = 0x2000,
    InputCancel      = 0x4000,
};

class MainWindow : public QRasterWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWindow *parent = nullptr);
    void SetCursorList(QList<QImage> cursorList)
        {cursors = cursorList;}
    bool checkInput(qint32 index);
    QPoint cursorPos;

    QWaitCondition userInput;
    QWaitCondition vSync;
    quint16 inputFlag;

    Config *pConfig = nullptr;

public slots:
    void hideGameCursor();
    void showGameCursor();
    void setAnimatedCursor(qint8 id);
    void updatePalette(QVector<QRgb> palette);
    void updateFramebuffer(QRect rect, QByteArray data);

protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void mountCursor();
    void rebuildDisplay(const QRect &rect = QRect());

    QImage framebuffer;
    QImage displayBuffer;
    QTransform transformer;
    QTransform transback;
    QPointF cursorPosition;

    QImage cursor;
    QList<QImage> cursors;
    QRectF viewportRect;
    qreal scale = 1.0;
    bool cursorVisible = true;
    QCursor cur;
};

#endif // IMGWINDOW_H
