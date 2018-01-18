#include "engine.h"
#include "DrawingUtil/drawingutil.h"
#include "MainWindow.h"
#include "Files/files.h"
#include "cps.h"

void Engine::gameCommand()
{
    QString cstr;
    QMutex m;
    int command = Calculate(cstr);

    prepareSubCommands();
    sendMessage(QString("GameCommand(%0);").arg(cstr));
    depth++;
    switch(command)
    {
    case 0:
        sendMessage(QString("MakeFont;"));
        //Todo:设置drawing对象的font相关属性；
        break;
    case 2:
        cursorCommand();
        break;
    case 3:
        AnimationCommand();
        break;
    case 4:
        LoadSave();
        break;
    case 5:
        SoundOperations(subCommands);
        break;
    case 6:
        sendMessage(QString("Unknown not finished:PlayVoice:%0").arg(QString((char*)subCommands[1].buffer)));
    {
        QByteArray voice = pFiles->LoadVoiceFile(QString((char*)subCommands[1].buffer).right(9));
        //emit voiceStarted(voice);
    }
        break;
    case 7:
        loadDataMes();
        strcpy(filenameData, (char*)subCommands[0].buffer);
        //还要redraw个什么dialog?大概不需要实现吧，以后再说
        break;
    case 8:
        DrawBG();
        break;
    case 9:
        paletteOperation();
        break;
    case 0xA:
        painting();
        break;
    case 0xB:
        sendMessage(QString("SkipableWait(timer:%0)").arg(subCommands[0].command>>2));
        SkipableWait();
        break;
    case 0xC:
        sendMessage(QString("%2SetBgTxtColor(%0,%1);")
                    .arg((subCommands[0].command&0xF0)>>4)
                .arg(subCommands[0].command&0xF)
                .arg(subCommands[0].command));
        Flag->dim16Bit.colorIndex = (subCommands[0].command&0xF)|
                16*(subCommands[0].command&0xF0);
        pDrawing->setTxtColor(Flag->dim16Bit.colorIndex);
        break;
    case 0xD:
        runMes();
        break;
    case 0xE://raw
    {
        sendMessage("GameCommand0E...Raw");
        char *i;
        for ( i = &gMainStruct.mesBuffer[Flag->dim32Bit.field_14 + 6]; *((quint16 *)i - 3) != 0xFFFFu; i += 10 )
          {
            if ( *((quint16 *)i - 2) <= subCommands[0].command && *(quint16 *)i >= subCommands[0].command )
            {
              if ( *((quint16 *)i - 1) <= subCommands[1].command && *((quint16 *)i + 1) >= subCommands[1].command )
                break;
            }
          }
          Flag->reg16Bit.moveResult = *((quint16 *)i - 3);
    }
        break;
    case 0xF:
    {
        sendMessage("setBookmarkIndex");
        quint16 i=0;
        do
        {
            if(mesBookmarkIndexs[i]==(subCommands[0].command&0xF))
                break;
            i++;
        }while(i<0x32);
        *((quint16*)&Flag->bookmarkIndex+1)=i;
    }
        break;
    case 0x11:
qDebug()<<QString("cpsMove:%0,%1,%2,%3")
          .arg(subCommands[0].command)
          .arg(subCommands[1].command)
          .arg(subCommands[2].command)
          .arg(subCommands[3].command);
        sendMessage(QString("cpsMove:%0,%1,%2,%3")
                .arg(subCommands[0].command)
                .arg(subCommands[1].command)
                .arg(subCommands[2].command)
                .arg(subCommands[3].command));
        pCps->cpsMove(
                subCommands[0].command,
                subCommands[1].command,
                subCommands[2].command,
                subCommands[3].command);
        break;
    case 0x12:
        MesCacheOperations(subCommands[0].command);
        break;
    case 0x13:
        command_0x13();
        break;
    case 0x14:
    {
        forever{
            m.lock();
            pMainWindow->userInput.wait(&m);
            m.unlock();
            if(pMainWindow->checkInput(0) || pMainWindow->checkInput(1))
                break;
        }
        forever{
            m.lock();
            pMainWindow->userInput.wait(&m);
            m.unlock();
            if(!pMainWindow->checkInput(0) || !pMainWindow->checkInput(1))
                break;
        }
    }
        break;
    default:
        sendMessage(QString("Unknown Game Command:%0").arg(command));
        qDebug()<<"Unknown command:"<<(void*)command;
        break;
    }
    depth--;
}
