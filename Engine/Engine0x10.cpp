#include "engine.h"
#include "w.h"
#include "MainWindow.h"
#include "DrawingUtil/drawingutil.h"
#include "palettes.h"
void Engine::command_0x10()
{
    QMutex m;
    prepareSubCommands();
    sendMessage(QString("Command0x10(%0);").arg(subCommands[0].command));
    depth++;
    switch(subCommands[0].command)
    {
    case 0x5E:
        sendMessage(QString("BackupPalette(index:%0, count:%1);")
                    .arg(subCommands[1].command)
                .arg(subCommands[2].command));
        if(subCommands[2].command)
            memcpy(anotherPaletteBuffer+subCommands[1].command,
                    paletteBuffer+subCommands[1].command,
                    subCommands[2].command*4);
        break;
    case 0x5F:
        sendMessage("RestorePalette();");
        memcpy(paletteBuffer, anotherPaletteBuffer, sizeof(paletteBuffer));
        break;
    case 0x60:
        sendMessage("Get stored color;");
    {

        quint8 count = gMainStruct.Flag.flag[2024] != 1 ? 16 : 10;
        PALETTEENTRY* p = (PALETTEENTRY*)paletteBuffer;
        for(quint8 i=0; i<count; i++)
        {
            p[i].peRed = storedColors[i*4+2];
            p[i].peGreen = storedColors[i*4+1];
            p[i].peBlue = storedColors[i*4];
        }
    }
        break;
    case 0x61:
        Flag->reg16Bit.moveResult = pMainWindow->checkInput(7);
        break;
    case 0x63:
        Flag->reg16Bit.moveResult = pMainWindow->checkInput(0);
        break;
    case 0x64:
        Flag->flag[2035] = 0;
        if ( pMainWindow->checkInput(0) == 1 )
        {
            do{
                m.lock();
                pMainWindow->userInput.wait(&m);
                m.unlock();
            }while ( pMainWindow->checkInput(0) == 1 );
        }
        if ( pMainWindow->checkInput(1) == 1 )
        {
            do{
                m.lock();
                pMainWindow->userInput.wait(&m);
                m.unlock();
            }while ( pMainWindow->checkInput(1) == 1 );
        }
        break;
    case 0x65:
        sendMessage("ResetTimer(); No use");
        timer = 0;
        break;
    case 0x66:
        sendMessage("pEngine->field_EB0=16");
        field_EB0 = 16;
        break;
    case 0x67:
        if(!moveSpeed)
            break;
        forever
        {
            if(pMainWindow->checkInput(6))
                break;
            m.lock();
            pMainWindow->userInput.wait(&m,moveSpeed);
            m.unlock();
        }
        break;
    case 0x6C:
        sendMessage("setMBRightPressedMark");
        Flag->reg16Bit.moveResult = pMainWindow->checkInput(1) == 1;
        break;
    case 0x6E:
        sendMessage(QString("loadStoredPalette6E:%0,%1").arg(subCommands[2].command).arg(subCommands[1].command));
    {
//        PALETTEENTRY* p = (PALETTEENTRY*)paletteBuffer;
//        auto palette = off_43E260[subCommands[2].command + 4*subCommands[1].command];
//        for(quint8 i=16; i<188; i++)
//        {
//            p[i].peRed = palette[i*4+2];
//            p[i].peGreen = palette[i*4+1];
//            p[i].peBlue = palette[i*4];
//        }
    }
        break;
    case 0x70:
        sendMessage(QString("loadStoredPalette70:%0,%1").arg(subCommands[2].command).arg(subCommands[1].command));
    {
        PALETTEENTRY* p = (PALETTEENTRY*)paletteBuffer;
        auto palette = off_45D3E0[subCommands[2].command + 4*subCommands[1].command];
        for(quint8 i=16; i<236; i++)
        {
            p[i].peRed = palette[i*4+2];
            p[i].peGreen = palette[i*4+1];
            p[i].peBlue = palette[i*4];
        }
    }
        break;
    case 0x71:
        sendMessage(QString("loadStoredPalette71:%0,%1").arg(subCommands[2].command).arg(subCommands[1].command));
    {
        PALETTEENTRY* p = (PALETTEENTRY*)paletteBuffer;
        auto palette = off_45D2E0[subCommands[2].command + 4*subCommands[1].command];
        for(quint8 i=16; i<236; i++)
        {
            p[i].peRed = palette[i*4+2];
            p[i].peGreen = palette[i*4+1];
            p[i].peBlue = palette[i*4];
        }
    }
        break;
    case 0x73:
        sendMessage("Black the palette.");
        memset(&paletteBuffer[16], 0, 220*4);
        break;
    case 0x78:
        sendMessage("78 Palette Animation");
    {
        quint32 buffer[256];
        memcpy(buffer, pDrawing->pPalette->targetPalette, 236*4);
        memcpy(paletteBuffer, anotherPaletteBuffer, 236*4);
        bool same = true;
        PALETTEENTRY *s, *d;
        s = (PALETTEENTRY*)buffer;
        d = (PALETTEENTRY*)anotherPaletteBuffer;
        for(auto i=128; i<236; i++)
        {
            if(s[i].peRed!=d[i].peRed || s[i].peGreen!=d[i].peGreen || s[i].peBlue!=d[i].peBlue)
            {
                same = false;
                break;
            }
        }
        if(same)
        {
            Flag->reg16Bit.moveResult = 0;
            break;
        }
        memset(&paletteBuffer[16], 0, 220*4);
        pDrawing->setTargetPalette(paletteBuffer);
        pDrawing->makeStepPalette(16);
        pDrawing->waitPaletteAnimating(0, 236);
        Flag->reg16Bit.moveResult = 1;
    }
        break;
    case 0x7A:
        sendMessage("7A SetPalettes");
    {
        memcpy(paletteBuffer, pDrawing->logpe, 236*4);
        memcpy(&paletteBuffer[16], &anotherPaletteBuffer[16], 112*4);
    }
        break;
    case 0x7B:
        sendMessage("7B Palette Animation");
    {
        quint32 buffer[256];
        memcpy(buffer, pDrawing->pPalette->targetPalette, 236*4);
        auto count = Flag->flag[2024]!=1?16:10;
        memcpy(anotherPaletteBuffer, storedColors, count*4);
        memcpy(paletteBuffer, anotherPaletteBuffer, 236*4);
        bool same = true;
        PALETTEENTRY *s, *d;
        s = (PALETTEENTRY*)buffer;
        d = (PALETTEENTRY*)anotherPaletteBuffer;
        for(auto i=10; i<236; i++)
        {
            if(s[i].peRed!=d[i].peRed || s[i].peGreen!=d[i].peGreen || s[i].peBlue!=d[i].peBlue)
            {
                same = false;
                break;
            }
        }
        if(same)
        {
            Flag->reg16Bit.moveResult = 0;
            break;
        }
        memset(&paletteBuffer[10], 0, 226*4);
        pDrawing->setTargetPalette(paletteBuffer);
        pDrawing->makeStepPalette(16);
        pDrawing->waitPaletteAnimating(0, 236);
        memset(vrams[0].data, Flag->dim16Bit.colorIndex>>8, vrams[0].width*vrams[0].height);
        pDrawing->updateWindow(0, 0, vrams[0].width, vrams[0].height);
        Flag->reg16Bit.moveResult = 1;
    }
        break;
    default:
        sendMessage("Unknown command:TODO:finish this");
        break;
    }
    depth--;
}

