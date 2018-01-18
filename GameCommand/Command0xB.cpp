#include "engine.h"
#include "../MainWindow.h"
#include <QMutex>

void Engine::SkipableWait()
{
    QMutex mutex;
    if(subCommandIndex)
    {
        while(!pMainWindow->checkInput(6))
        {
            auto timeout = subCommands[0].command>>2;
            mutex.lock();
            pMainWindow->userInput.wait(&mutex,timeout*20);
            mutex.unlock();
            break;
        }
    }
    else if(pMainWindow->checkInput(7))
    {
        //not ready MaybeFadeOutSound
    }
    else if(pMainWindow->checkInput(0))
    {
        while(pMainWindow->checkInput(0))
        {
            if(pMainWindow->checkInput(7))
                break;
            mutex.lock();
            pMainWindow->userInput.wait(&mutex);
            mutex.unlock();
        }
    }
    else
    {
        while(!pMainWindow->checkInput(7))
        {
            if(pMainWindow->checkInput(0))
                break;
            mutex.lock();
            pMainWindow->userInput.wait(&mutex);
            mutex.unlock();
        }
        while(pMainWindow->checkInput(0))
        {
            if(pMainWindow->checkInput(7))
                break;
            mutex.lock();
            pMainWindow->userInput.wait(&mutex);
            mutex.unlock();
        }
    }
}
