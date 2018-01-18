#ifndef FILES_H
#define FILES_H

#include <QObject>
#include <QFile>
#include <QHash>
#include <QPair>
#include "Flag.h"

typedef QHash<QString, QPair<quint32, quint32>> FileList;
typedef QPair<quint32, quint32> FileInfo;

class Config;

class Files : public QObject
{
    Q_OBJECT
public:
    explicit Files(Config *pConfig = 0, MainStruct *pMainStruct = 0, QObject *parent = 0);
    void LoadMesFile(QString filename, quint8 *buffer);
    void LoadDataFile(QString filename, char *buffer);
    QByteArray LoadBGMFile(char* filename);
    QByteArray LoadVoiceFile(QString filename);
    QImage LoadBG(char* filename, int posTop = -1, int posLeft = -1, int maskIndex = 0, quint32 *pal = 0);
    void ReadRawBG(char* filename, char* buffer);

signals:

public slots:

private:
    //begin
    Config *pConfig;
    void* pDecodeBuffer;
    quint32 haveCDROM;
    quint32 isArcMes;
    quint32 isArcBG;
    quint32 isArcBGM;
    quint32 isArcVoice;
    quint32 isArcData;
    quint32 nMesType;
    quint32 nDataType;
    MainStruct *pMainStruct;
    quint32 hFile_ArcMes;
    quint32 hFile_ArcBG;
    quint32 hFile_ArcBGM;
    quint32 hFile_ArcVoice;
    quint32 hFile_ArcData;
    quint32 handleCount;
    quint8 field_44_0;
    char filenameArcMes[256];
    char field_145[768];
    char filenameArcBg[256];
    char field_545[768];
    char filenameArcBGM[256];
    char field_945[768];
    char filenameArcVoice[256];
    char field_D45[768];
    char filenameArcData[256];
    char field_1145[768];
    char CDDRV[256];
    char field_1545[768];
    quint8 field_1845;
    quint8 field_1846;
    quint8 field_1847;
    char* p_filenamesBuffer_ArcMes;
    char* p_filenamesBuffer_ArcBG;
    char* p_filenamesBuffer_ArcBGM;
    char* p_filenamesBuffer_ArcVoice;
    char* p_filenamesBuffer_ArcData;
    qint32 filesCount_ArcMes;
    qint32 filesCount_ArcBG;
    qint32 filesCount_ArcBGM;
    qint32 filesCount_ArcVoice;
    qint32 filesCount_ArcData;
    qint32 field_1870_0;
    //end

    QFile mesFile;
    QFile bgFile;
    QFile bgmFile;
    QFile dataFile;
    QFile boyFile;
    QFile girlFile;
    FileList mesFileList;
    FileList bgFileList;
    FileList bgmFileList;
    FileList dataFileList;
    FileList boyFileList;
    FileList girlFileList;

    void ReadFileList(FileList &fileList, QFile &file);
    QByteArray ReadContent(QString filename, FileList &filelist, QFile &file);
    qint64 readRaw(char* filename, char *buffer, FileList &filelist, QFile &file);
};

#endif // FILES_H
