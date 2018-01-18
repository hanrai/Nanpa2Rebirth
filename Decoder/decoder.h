#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include "engine.h"

union ByteWorker
{
    unsigned int dword;
    unsigned char byte[4];
    unsigned short int word[2];
};

class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(QObject *parent = 0);
    void decodeGP8(quint8 *src, VramInfo *vram, quint16 top, quint16 left, quint16 width, quint16 height);

signals:

public slots:
private:
    quint8 dictionary[4096];
    quint8* p_sourceBuffer;
    quint8* p_destBuffer;
    qint32 field_1008;
    qint32 index;
    qint32 index_destBuffer;
};

#endif // DECODER_H
