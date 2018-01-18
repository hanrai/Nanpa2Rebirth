#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QBuffer>
#include <QAudioOutput>
#include <QSharedPointer>
#include <QByteArray>
#include "global.h"

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

class SoundManager : public QObject
{
    Q_OBJECT
public:
    explicit SoundManager(QObject *parent = 0);
signals:

public:
    void Play(QByteArray data, bool bLoop = false);

private:
    QByteArray wavData;
    QSharedPointer<QBuffer> buffer;
    QSharedPointer<QAudioOutput> player;
};

#endif // SOUNDMANAGER_H
