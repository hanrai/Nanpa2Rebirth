#ifndef STACK_H
#define STACK_H

#include <QObject>
#include "Flag.h"

class Config;

class Stack : public QObject
{
    Q_OBJECT
public:
    explicit Stack(Config* pConfig, QObject *parent = 0);

signals:

public slots:

private:
    //begin
    Config *pConfig;
    qint32 stack[1024];
    qint16 stackPointer;
    qint16 field_1006;
    MainStruct *pMainStruct1;
    quint32 random;
    //end
};

#endif // STACK_H
