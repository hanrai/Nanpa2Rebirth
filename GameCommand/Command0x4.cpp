#include "engine.h"
#include <QFile>
#include "Files/files.h"

void Engine::LoadSave()
{
    quint32 command=subCommands[0].command;
    sendMessage(QString("LoadSave(%0);").arg(command));
    depth++;

    switch(command)
    {
    case 0:
        sendMessage(QString("ReadSavedFile:%0").arg(subCommands[1].command));
    {
        QString filename=QString("SAVE0%0").arg(subCommands[1].command);
        char* pMain=reinterpret_cast<char*>(Flag);
        QFile savefile(filename);
        savefile.open(QIODevice::ReadOnly);
        savefile.read(pMain,sizeof(FLAG));
        savefile.close();
        strcpy(filenameMes, Flag->mesFileName);
        pFiles->LoadMesFile(QString(filenameMes), (quint8*)gMainStruct.mesBuffer);
        gMainStruct.Flag.pDim16Bit = pDim16Bit;
        Flag->dim16Bit.B10offset = 0xB10;
        Flag->dim32Bit.cpsBase = 4096;
        Flag->dim32Bit.pMesCursorBookmarks = (quint32*)gMainStruct.mesCursorBookmarks;
        Flag->dim32Bit.field_20 = gMainStruct.g_dword_562890;
        Flag->pMap = &gMainStruct.pMap;
        Flag->dim16Bit.mainFlag |= 0x10;
    }
        break;
    case 2:
    {
        sendMessage(QString("ReadWithoutScene();"));
        QString filename=QString("SAVE0%0").arg(subCommands[1].command);
        char *pMain;
        char *psnap;
        pMain=reinterpret_cast<char*>(Flag);
        psnap=(char*)FlagSnapshot;
        memcpy(psnap,pMain,sizeof(FlagSnapshot));

        sendMessage(QString("ReadFile(\"%0\");").arg(filename));
        QFile savefile(filename);
        savefile.open(QIODevice::ReadOnly);
        savefile.read(pMain,sizeof(FLAG));
        savefile.close();

        memcpy(pMain,psnap,128);
        Flag->pDim16Bit=pDim16Bit;
        Flag->dim16Bit.B10offset=2832;
        Flag->dim32Bit.cpsBase = 4096;
        Flag->dim32Bit.pMesCursorBookmarks=(quint32*)&mesCursorBookmarks;
        Flag->dim32Bit.field_20=(qint32*)&g_dword_562890;
        Flag->pMap=(quint32*)&pMap;
    }
        break;
    default:
        sendMessage(QString("Unknown LoadSave Command:%0").arg(command));
        break;
    }

    depth--;
}
