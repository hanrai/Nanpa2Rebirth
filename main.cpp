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
#include "Flag.h"

Engine *gpEngine;
Engine *threadGpEngine;
qint32 gNotReady;
MainStruct gMainStruct;
GStruct gStruct;
quint16 gMesCachesIndex;
QWaitCondition animating;
quint32 timer;

void myMessageOutput(QtMsgType type, const QMessageLogContext& , const QString &msg)
{
    QFile debug("Debug.log");
    debug.open(QIODevice::WriteOnly|QIODevice::Append);
    Q_ASSERT(debug.error()==QFileDevice::NoError);

    switch (type) {
    case QtDebugMsg:
        debug.write(QString("%0\r\n").arg(QString(msg)).toLocal8Bit());
        break;
    case QtWarningMsg:
        debug.write(QString("Warning:%0\r\n").arg(QString(msg)).toLocal8Bit());
        break;
    case QtCriticalMsg:
        debug.write(QString("Critical:%0\r\n").arg(QString(msg)).toLocal8Bit());
        break;
    case QtFatalMsg:
        debug.write(QString("Fatal:%0\r\n").arg(QString(msg)).toLocal8Bit());
        abort();
    }
    debug.close();
}

int main(int argc, char *argv[])
{
    //qInstallMessageHandler(myMessageOutput);
    QGuiApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB18030-0"));
    qRegisterMetaType<QVector<QRgb> >("QVector<QRgb>");

    MainWindow mainWindow;
    Engine *engine = new Engine(&mainWindow, &mainWindow);
    engine->setObjectName("Engine");
    mainWindow.pConfig = engine->pConfig;
    engine->pAnimations = new AnimationManager(engine->pConfig, engine, &gMainStruct, &mainWindow);
    engine->pAnimations->setObjectName("AnimationEngine");
    QMetaObject::connectSlotsByName(&mainWindow);

    engine->engineStartEvent = QEvent::registerEventType();

    mainWindow.show();
    engine->pAnimations->PlayAnimations();
    engine->start();
    return a.exec();
}
