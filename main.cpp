#include <QGuiApplication>
#include <QFile>
#include <QtDebug>
#include <QTextCodec>
#include <QMetaObject>
#include <QWaitCondition>
#include "engine.h"
#include "MainWindow.h"
#include "Animation/animationmanager.h"
#include "DrawingUtil/drawingutil.h"
#include "Sound/soundmanager.h"
#include "Flag.h"

Engine *gpEngine = nullptr;
Engine *threadGpEngine = nullptr;
qint32 gNotReady = 0;
MainStruct gMainStruct{};
GStruct gStruct{};
quint16 gMesCachesIndex = 0;
QWaitCondition animating;
quint32 timer = 0;

namespace {
    void messageHandler(QtMsgType type, const QMessageLogContext& , const QString &msg)
    {
        QFile file("Debug.log");
        if (!file.open(QIODevice::WriteOnly|QIODevice::Append)) 
            return;

        QByteArray line;
        switch (type) {
        case QtDebugMsg:    break;
        case QtInfoMsg:     line += "Info:";     break;
        case QtWarningMsg:  line += "Warning:";  break;
        case QtCriticalMsg: line += "Critical:"; break;
        case QtFatalMsg:    line += "Fatal:";    break;
        }
        line += msg.toUtf8();
        line += "\n";
        file.write(line);
        
        if (type == QtFatalMsg)
            abort();
    }

    void wireEngine(Engine *engine, MainWindow *window, SoundManager *sound)
    {
        QObject::connect(engine, &Engine::hideCursor,
                         window, &MainWindow::hideGameCursor);
        QObject::connect(engine, &Engine::showCursor,
                         window, &MainWindow::showGameCursor);
        QObject::connect(engine, &Engine::BGMStarted,
                         sound, &SoundManager::playBgm);
        QObject::connect(engine, &Engine::voiceStarted,
                         sound, &SoundManager::playVoice);

        QObject::connect(engine->pAnimations, &AnimationManager::cursorAnimated,
                         window, &MainWindow::setAnimatedCursor);
    }
} // namespace

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QGuiApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB18030-0"));
    
    qRegisterMetaType<QVector<QRgb>>();

    MainWindow mainWindow;
    auto *sound = new SoundManager(&mainWindow);
    auto engine = new Engine(&mainWindow, &mainWindow);
    mainWindow.pConfig = engine->pConfig;

    engine->pAnimations = new AnimationManager(
        engine->pConfig, engine, &gMainStruct, &mainWindow);

    wireEngine(engine, &mainWindow, sound);

    engine->engineStartEvent = QEvent::registerEventType();

    mainWindow.show();
    engine->pAnimations->PlayAnimations();
    engine->start();

    return a.exec();
}
