#include "engine.h"
#include "MainWindow.h"
#include <QGuiApplication>
#include <QMutex>

void Engine::UserInput()
{
    sendMessage("UserInput()");
    depth++;
    QMutex mute;
    quint32 index = 0;
    for(index = 0; index<50; index++)
        if(!mesCursorBookmarks[index])
            break;
    Flag->bookmarkIndex = index;

    auto flag = Flag->dim16Bit.mainFlag;

    Flag->dim16Bit.mainFlag &= 0xFBFF;

    RunFromSavedAnchorPoint(38);

    while(!(Flag->dim16Bit.mainFlag & 0x8))
    {
        RunFromSavedAnchorPoint(39);
        if(Flag->dim16Bit.mainFlag & 0x8)
            break;
        if(pMainWindow->checkInput(0))
        {
            RunFromSavedAnchorPoint(32);
            if(!Flag->flag[2035] && pMainWindow->checkInput(0))
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute);
                mute.unlock();
            }while(pMainWindow->checkInput(0));
            continue;
        }
        if(pMainWindow->checkInput(1))
        {
            if(pMainWindow->checkInput(1))
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute);
                mute.unlock();
            }while(pMainWindow->checkInput(1));
            RunFromSavedAnchorPoint(33);
            if(pMainWindow->checkInput(1))
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute);
                mute.unlock();
            }while(pMainWindow->checkInput(1));
            continue;
        }
        if(pMainWindow->checkInput(2))
        {
            RunFromSavedAnchorPoint(34);
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute,10*16);
                mute.unlock();
            }while(pMainWindow->checkInput(2));
            continue;
        }
        if(pMainWindow->checkInput(3))
        {
            RunFromSavedAnchorPoint(35);
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute,10*16);
                mute.unlock();
            }while(pMainWindow->checkInput(3));
            continue;
        }
        if(pMainWindow->checkInput(4))
        {
            RunFromSavedAnchorPoint(36);
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute,10*16);
                mute.unlock();
            }while(pMainWindow->checkInput(4));
        }
        if(pMainWindow->checkInput(5))
        {
            RunFromSavedAnchorPoint(37);
            do
            {
                mute.lock();
                pMainWindow->userInput.wait(&mute,10*16);
                mute.unlock();
            }while(pMainWindow->checkInput(5));
            continue;
        }
    }
    Flag->dim16Bit.mainFlag = flag;
    bookmarksReady = 0;
    depth--;
}

