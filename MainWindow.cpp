#include "MainWindow.h"
#include <QPainter>
#include <QtDebug>
#include <QGuiApplication>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <cstring>
#include <iterator>

MainWindow::MainWindow(QWindow *parent) :
    QRasterWindow(parent)
{
    setMinimumHeight(400);
    setMinimumWidth(640);

    framebuffer = QImage(640, 400, QImage::Format_Indexed8);
    framebuffer.fill(Qt::black);
    displayBuffer = QImage(640, 400, QImage::Format_ARGB32_Premultiplied);
    displayBuffer.fill(Qt::black);

    inputFlag = 0;
    setTitle("Nanap2 rebirth engine WIP version. By hanrai@gmail.com");
}

bool MainWindow::checkInput(qint32 index)
{
    static const quint16 masks[] = {
        InputMouseLeft | InputMouseRight | InputConfirm, // 0 confirm
        InputCancel | InputAltCancel,                    // 1 cancel
        InputUpKey | InputAltUp,                         // 2 up
        InputDownKey | InputAltDown,                     // 3 down
        InputLeftKey | InputAltLeft,                     // 4 left
        InputRightKey | InputAltRight,                   // 5 right
        0,                                               // 6 shift
        InputControl,                                    // 7 control
        InputConfirm,                                    // 8 confirm
    };

    if (index == 6)
        return QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier;

    if (index < 0 || index >= qint32(std::size(masks)))
        return false;

    return (inputFlag & masks[index]) != 0;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    if (geometry().width() < 1 || geometry().height() < 1
            || displayBuffer.width() < 1 || displayBuffer.height() < 1)
        return;

    QPainter painter(this);
    painter.fillRect(event->rect(), Qt::black);

    if (!event->rect().intersects(viewportRect.toRect()))
        return;

    QRectF updateRect = event->rect().intersected(viewportRect.toRect());
    QRectF sourceRect = transback.mapRect(updateRect);

    painter.setClipRect(updateRect);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawImage(updateRect, displayBuffer, sourceRect);
}

void MainWindow::mountCursor()
{
    if (!cursorVisible)
        return;
    if (cursor.isNull())
        return;
    cur = QCursor(QPixmap::fromImage(cursor));
    setCursor(cur);
}

void MainWindow::rebuildDisplay(const QRect &rect)
{
    if (rect.isEmpty()) {
        displayBuffer = framebuffer.convertToFormat(
            QImage::Format_ARGB32_Premultiplied);
        return;
    }

    const QRect clipped = rect.intersected(framebuffer.rect());
    if (clipped.isEmpty())
        return;

    const QImage dirty = framebuffer.copy(clipped).convertToFormat(
        QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&displayBuffer);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(clipped.topLeft(), dirty);
}

void MainWindow::mouseMoveEvent(QMouseEvent *ev)
{
    const QPointF pos = ev->position();
    if (!viewportRect.contains(pos)) {
        unsetCursor();
        return;
    }
    setCursor(cur);

    cursorPosition = pos;
    cursorPos = transback.map(cursorPosition).toPoint();
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_2:
    case Qt::Key_Down:
        inputFlag |= InputDownKey;
        break;
    case Qt::Key_8:
    case Qt::Key_Up:
        inputFlag |= InputUpKey;
        break;
    case Qt::Key_6:
    case Qt::Key_Right:
        inputFlag |= InputRightKey;
        break;
    case Qt::Key_4:
    case Qt::Key_Left:
        inputFlag |= InputLeftKey;
        break;
    case Qt::Key_Control:
        inputFlag |= InputControl;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        inputFlag |= InputConfirm;
        break;
    case Qt::Key_Escape:
        inputFlag |= InputCancel;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_2:
    case Qt::Key_Down:
        inputFlag &= ~InputDownKey;
        break;
    case Qt::Key_8:
    case Qt::Key_Up:
        inputFlag &= ~InputUpKey;
        break;
    case Qt::Key_6:
    case Qt::Key_Right:
        inputFlag &= ~InputRightKey;
        break;
    case Qt::Key_4:
    case Qt::Key_Left:
        inputFlag &= ~InputLeftKey;
        break;
    case Qt::Key_Control:
        inputFlag &= ~InputControl;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        inputFlag &= ~InputConfirm;
        break;
    case Qt::Key_Escape:
        inputFlag &= ~InputCancel;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    switch (ev->button()) {
    case Qt::LeftButton:
        inputFlag |= InputConfirm;
        break;
    case Qt::RightButton:
        inputFlag |= InputCancel;
        break;
    default:
        ev->ignore();
        break;
    }
    userInput.wakeAll();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    switch (ev->button()) {
    case Qt::LeftButton:
        inputFlag &= ~InputConfirm;
        break;
    case Qt::RightButton:
        inputFlag &= ~InputCancel;
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
    const qreal ratio = qreal(framebuffer.width()) / qreal(framebuffer.height());
    const qreal winWidth = geometry().width();
    const qreal winHeight = geometry().height();
    qreal top, left, width, height;

    if (winWidth / winHeight >= ratio) {
        height = winHeight;
        width = height * ratio;
    } else {
        width = winWidth;
        height = width / ratio;
    }

    top = (winHeight - height) / 2;
    left = (winWidth - width) / 2;

    scale = width / framebuffer.width();
    viewportRect.setRect(left, top, width, height);
    transformer = QTransform(scale, 0, 0, scale, left, top);
    transback = transformer.inverted();
    mountCursor();
    update();
}

void MainWindow::hideGameCursor()
{
    if (!cursorVisible)
        return;
    setCursor(QCursor(Qt::BlankCursor));
    cursorVisible = false;
}

void MainWindow::showGameCursor()
{
    if (cursorVisible)
        return;
    cursorVisible = true;
    mountCursor();
}

void MainWindow::setAnimatedCursor(qint8 id)
{
    if (cursors.isEmpty())
        return;
    if (!cursorVisible)
        return;
    cursor = cursors[id];
    mountCursor();
}

void MainWindow::updatePalette(QVector<QRgb> palette)
{
    framebuffer.setColorTable(palette);
    rebuildDisplay();
    update();
}

void MainWindow::updateFramebuffer(QRect rect, QByteArray data)
{
    quint8 *fbuffer = framebuffer.bits();
    for (int y = rect.top(); y < rect.bottom() + 1; y++)
        memcpy(fbuffer + y * framebuffer.width() + rect.left(),
               data.data() + (y - rect.top()) * rect.width(),
               size_t(rect.width()));

    rebuildDisplay(rect);

    QRect targetRect = transformer.mapRect(rect);
    targetRect.adjust(-1, -1, 1, 1);
    update(targetRect.united(viewportRect.toRect()));
}
