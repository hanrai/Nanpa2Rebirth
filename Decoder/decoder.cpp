#include "decoder.h"

Decoder::Decoder(QObject *parent) : QObject(parent)
{

}

void Decoder::decodeGP8(quint8 *src, VramInfo *vram, quint16 top, quint16 left, quint16 width, quint16 height)
{
    p_sourceBuffer = src;
    index = 0;
    memset(dictionary, 0, 4096);
    qint32 dictionaryIndex = 4078;
    dictionary[4076] = 0;
    ByteWorker key, section, wordSize;
    key.dword = 0;
    auto rightEdge = left + width;
    auto bottomEdge = top + height;
    auto vw = vram->width;
    auto vh = vram->height;
    auto dest = vram->data;

    quint8 data;

    for(qint32 y=top; y<bottomEdge; y++)
    {
        for(qint32 x=left; x<rightEdge;)
        {
            key.dword >>= 1;
            if(!(key.byte[1] & 1))
            {
                key.byte[0] = p_sourceBuffer[index++];
                key.byte[1] = 0xFF;
            }
            if(key.byte[0] & 1)
            {
                data = p_sourceBuffer[index++];
                dictionary[dictionaryIndex++] = data;
                dictionaryIndex &= 0xFFF;
                dest[x+vw*(vh-bottomEdge+y)] = data;
                x++;
            }
            else
            {
                auto offset = p_sourceBuffer[index++];
                wordSize.dword = 0;
                section.dword = 0;

                wordSize.byte[0] = p_sourceBuffer[index++];

                section.dword = wordSize.dword;
                section.dword &= 0xF0;
                section.dword <<= 4;
                section.dword |= offset;

                wordSize.dword &= 0xF;
                wordSize.dword += 2;

                for (unsigned int i=0; i<=wordSize.dword; i++)
                {
                    data = dictionary[(section.dword + i) & 0xFFF];
                    dictionary[dictionaryIndex++] = data;
                    dictionaryIndex &= 0xFFF;
                    dest[x+vw*(vh-bottomEdge+y)] = data;
                    if(++x >= rightEdge)
                    {
                        if(++y >= bottomEdge)
                            break;
                        x = left;
                    }
                }
            }
        }
    }
}
