#include "files.h"
#include "DecodeArcMes.h"
#include <QtDebug>
#include <QImage>

Files::Files(Config *pConfig, MainStruct *pMainStruct, QObject *parent) :
    QObject(parent),
    mesFile("CHS.ARC"),
    bgFile("BG.ARC"),
    bgmFile("BGM.ARC"),
    dataFile("HDATA.ARC")
//    boyFile("BOY.ARC"),
//    girlFile("GIRL.ARC")
{
    //init begin
    this->pConfig = pConfig;
    this->pDecodeBuffer = 0;
    this->pMainStruct = pMainStruct;
    this->hFile_ArcData = 0;
    this->hFile_ArcVoice = 0;
    this->hFile_ArcBGM = 0;
    this->hFile_ArcBG = 0;
    this->hFile_ArcMes = 0;
    this->p_filenamesBuffer_ArcData = 0;
    this->p_filenamesBuffer_ArcVoice = 0;
    this->p_filenamesBuffer_ArcBGM = 0;
    this->p_filenamesBuffer_ArcBG = 0;
    this->p_filenamesBuffer_ArcMes = 0;
    this->field_44_0 = 0;
    this->field_1870_0 = 0;
    //init end

    ReadFileList(mesFileList, mesFile);
    ReadFileList(bgFileList, bgFile);
    ReadFileList(bgmFileList, bgmFile);
    ReadFileList(dataFileList, dataFile);
//    ReadFileList(boyFileList, boyFile);
//    ReadFileList(girlFileList, girlFile);
}

void Files::LoadMesFile(QString filename, quint8 *buffer)
{
    auto rawData = ReadContent(filename, mesFileList, mesFile);
    DecodeArcMes(rawData.data(), (char*)buffer, rawData.size());
}

void Files::LoadDataFile(QString filename, char* buffer)
{
    auto rawData = ReadContent(filename, dataFileList, dataFile);
    DecodeArcMes(rawData.data(), buffer, rawData.size());
}

QByteArray Files::LoadBGMFile(char* filename)
{
    return ReadContent(QString(filename), bgmFileList, bgmFile);
}

QByteArray Files::LoadVoiceFile(QString filename)
{
    filename = filename.toUpper();
    FileList *list;
    QFile *file;
    if(boyFileList.contains(filename))
    {
        list = &boyFileList;
        file = &boyFile;
    }
    else if(girlFileList.contains(filename))
    {
        list = &girlFileList;
        file = &girlFile;
    }
    else
        return QByteArray();

    auto info = (*list)[filename];
    auto size = info.first;
    auto offset = info.second;

    file->open(QIODevice::ReadOnly);
    Q_ASSERT(file->error() == QFileDevice::NoError);
    file->seek(offset);
    auto tempBuffer = file->read(size);
    file->close();

    return tempBuffer;
}

QImage Files::LoadBG(char *filename, int posTop, int posLeft, int maskIndex, quint32* pal)
{
    QByteArray data = ReadContent(QString(filename), bgFileList, bgFile);

    typedef struct
    {
        quint16 top;
        quint16 left;
        quint16 width;
        quint16 height;
        quint32 palette[256];
    } GP8Header;

    GP8Header* header = (GP8Header*)data.data();
    if(pal)
    {
        memcpy(pal, header->palette, 256*4);
        for(int i=0; i<256; i++)
            pal[i]<<=8;
    }

    char* sourceBuffer = data.data();

    quint32 targetLeft, targetTop;
    targetLeft = posLeft==-1 ? header->left : posLeft;
    targetTop = posTop==-1 ? header->top : posTop;

    QImage decodedImage(header->width, header->height, QImage::Format_Indexed8);

    QVector<QRgb> colorTable(256, 0);
    memcpy(colorTable.data(), header->palette, 256*4);
    for(auto &color:colorTable)
        color |= 0xFF000000;

    maskIndex &= 0xFF;
    colorTable[maskIndex] &= 0xFFFFFF;

    QPoint offset(targetLeft, targetTop);

    decodedImage.setOffset(offset);
    decodedImage.setColorTable(colorTable);

    DecodeArcMes(sourceBuffer+1032, (char*)decodedImage.bits(), data.size()-1032);

    //decodedImage = decodedImage.mirrored();

    return decodedImage;
}

void Files::ReadRawBG(char *filename, char *buffer)
{
    readRaw(filename, buffer, bgFileList, bgFile);
}

void Files::ReadFileList(FileList &fileList, QFile &file)
{
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.error() == QFileDevice::NoError);

    quint32 fileCount = 0;
    file.read((char*)&fileCount, 4);

    QByteArray fileListBuffer;
    fileListBuffer = file.read(fileCount*20);
    file.close();
    Q_ASSERT((quint32)fileListBuffer.size() == fileCount*20);

    typedef struct
    {
        quint32 filename[3];
        quint32 size;
        quint32 offset;
    } FilenameListStructure;
    FilenameListStructure *pFilenameList;

    Q_ASSERT ( sizeof(FilenameListStructure)==20 );
    pFilenameList = (FilenameListStructure*) fileListBuffer.data();

    for(quint32 i=0; i<fileCount; i++)
    {
        pFilenameList[i].filename[0] ^= 0x55555555;
        pFilenameList[i].filename[1] ^= 0x55555555;
        pFilenameList[i].filename[2] ^= 0x55555555;
        pFilenameList[i].size ^= 0xAA55AA55;
        pFilenameList[i].offset ^= 0x55AA55AA;
    }

    for(quint32 i=0; i<fileCount; i++)
    {
        auto strFilename = QString((const char*)pFilenameList[i].filename);
        strFilename.truncate(12);

        auto size = pFilenameList[i].size;
        auto offset = pFilenameList[i].offset;

        fileList[strFilename] = qMakePair(size, offset);
    }
}

QByteArray Files::ReadContent(QString filename, FileList &filelist, QFile &file)
{
    filename = filename.toUpper();
    Q_ASSERT(filelist.contains(filename));

    auto info = filelist[filename];
    auto size = info.first;
    auto offset = info.second;

    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.error() == QFileDevice::NoError);
    file.seek(offset);
    auto tempBuffer = file.read(size);
    file.close();

    return tempBuffer;
}

qint64 Files::readRaw(char *filename, char* buffer, FileList &filelist, QFile &file)
{
    QString fn(filename);
    fn = fn.toUpper();
    Q_ASSERT(filelist.contains(fn));

    auto info = filelist[fn];
    auto size = info.first;
    auto offset = info.second;

    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.error() == QFileDevice::NoError);
    file.seek(offset);
    auto sz = file.read(buffer, size);
    file.close();
    return sz;
}
