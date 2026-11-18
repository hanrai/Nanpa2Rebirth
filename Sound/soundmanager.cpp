#include "soundmanager.h"
#include "Sound/WaveFileHeader.h"
#include <QDebug>

SoundManager::SoundManager(QObject *parent) :
    QObject(parent)
{
}

void SoundManager::playBgm(QByteArray data, bool loop)
{
    startPlayback(bgm, std::move(data), loop);
}

void SoundManager::playVoice(QByteArray data)
{
    startPlayback(voice, std::move(data), false);
}

void SoundManager::startPlayback(Channel &channel, QByteArray data, bool loop)
{
    if (channel.player)
        channel.player->stop();

    delete channel.player;
    channel.player = nullptr;
    delete channel.buffer;
    channel.buffer = nullptr;

    if (data.size() < 44)
        return;

    channel.data = std::move(data);

    const auto *header =
        reinterpret_cast<const WaveFileHeader *>(channel.data.constData());

    QAudioFormat format;
    format.setSampleRate(header->sampleRate);
    format.setChannelCount(header->channelCount);

    if (header->bits_per_sample != 16) {
        qWarning() << "Unsupported PCM bit depth:"
                   << header->bits_per_sample;
        return;
    }

    format.setSampleFormat(QAudioFormat::Int16);

    const qsizetype available = channel.data.size() - 44;
    const qsizetype pcmSize = header->bytes_in_data;

    if (pcmSize < 0 || pcmSize > available) {
        qWarning() << "Invalid WAV payload length";
        return;
    }

    channel.buffer = new QBuffer(this);
    channel.buffer->setData(channel.data.mid(44, pcmSize));

    if (!channel.buffer->open(QIODevice::ReadOnly))
        return;

    channel.player = new QAudioSink(format, this);

    if (loop) {
        connect(channel.player, &QAudioSink::stateChanged,
                this, [this, &channel](QAudio::State state) {
            if (state == QAudio::IdleState &&
                channel.player && channel.buffer) {
                channel.buffer->seek(0);
                channel.player->start(channel.buffer);
            }
        });
    }

    channel.player->start(channel.buffer);
}
