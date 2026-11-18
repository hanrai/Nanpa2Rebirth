#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QBuffer>
#include <QAudioSink>
#include <QAudioFormat>
#include <QByteArray>

class SoundManager : public QObject
{
    Q_OBJECT
public:
    explicit SoundManager(QObject *parent = nullptr);

public slots:
    void playBgm(QByteArray data, bool loop = false);
    void playVoice(QByteArray data);

private:
    struct Channel {
        QByteArray data;
        QBuffer *buffer = nullptr;
        QAudioSink *player = nullptr;
    };

    void startPlayback(Channel &channel, QByteArray data, bool loop);

    Channel bgm;
    Channel voice;
};

#endif // SOUNDMANAGER_H
