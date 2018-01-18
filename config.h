#ifndef CONFIG_H
#define CONFIG_H
#include "Flag.h"
class QObject;
class Engine;

typedef struct tag_DialogNowFile
{
    int hwndDialogNowFile;
    int bCanRedraw;
} DialogNowFile;

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(Engine *pEngine = 0, MainStruct* pMainStruct = 0, QObject *parent = 0);
    bool canRedraw();

signals:

public slots:

private:
    char filepathConfig[1024];
    char workDir[1024];
    quint32 isDebug;                    // 103=Debug
                                        // 104=Normal
    quint32 hFileLog;
    MainStruct *pMainStruct;
    quint32 pDialogNowFile;
    DialogNowFile dialogNowFile;
};

#endif // CONFIG_H
