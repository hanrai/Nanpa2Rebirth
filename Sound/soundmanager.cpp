#include "soundmanager.h"
#include <QDebug>

SoundManager::SoundManager(QObject *parent) :
    QObject(parent)
{

}

void SoundManager::Play(QByteArray data, bool bLoop)
{
    WaveFileHeader *header = (WaveFileHeader*)data.data();

    QAudioFormat format;
    format.setSampleRate(header->sampleRate);
    format.setChannelCount(header->channelCount);
    format.setSampleSize(header->bits_per_sample);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    char* audioRawData = data.data() + 44;

    buffer = decltype(buffer)(new QBuffer(this));
    buffer->setData(audioRawData, header->bytes_in_data);
    buffer->open(QIODevice::ReadOnly);

    player = decltype(player)(new QAudioOutput(format, this));
qDebug()<<"Error:"<<player->error();
    if(bLoop)
        connect(player.data(), &QAudioOutput::stateChanged,
                [&](QAudio::State state)
        {
            if(state == QAudio::IdleState)
            {
                buffer->seek(0);
                player->start(buffer.data());
            }
        });

    player->start(buffer.data());
qDebug()<<"PlayError:"<<player->error();
}
