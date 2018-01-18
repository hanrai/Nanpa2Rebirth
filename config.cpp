#include "config.h"
#include "Flag.h"

Config::Config(Engine *pEngine, MainStruct *pMainStruct, QObject *parent) : QObject(parent)
{
    gpEngine = pEngine;
    this->pMainStruct = pMainStruct;
    this->pDialogNowFile = 0;
    memset(&dialogNowFile, 0, sizeof(DialogNowFile));
    gNotReady = 1;
}

bool Config::canRedraw()
{
    return dialogNowFile.bCanRedraw;
}
