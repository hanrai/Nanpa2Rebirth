#include "engine.h"
#include <QStack>
#include <QString>

int Engine::Calculate(QString &calstr)
{
    union ByteWorker
    {
        quint8 byte[4];
        quint16 word[2];
        quint32 dword;
    };

    int result;
    int command;
    QStack<QString> str;
    calstr="(";
    bool notFinished = true;
    do
    {
        switch(command=getMes())
        {
        case 0x80:
        {
            quint8 index = getMes();
            stack.push(pReg16Bit[index]);
            str.push(QString("(reg16Bit[%0]):%1")
                     .arg(index)
                     .arg(pReg16Bit[index]));
        }
            break;
        case 0xA0:
        {
            quint8 index=stack.pop();
            auto strIndex = str.pop();
            quint8 selector = getMes();
            quint16* pProperties=0;
            QString strBaseAddress;
            if(selector)
            {
                strBaseAddress=QString("((quint16*)(((char*)&Flag)+reg16Bit[%0-1]:%1)").arg(selector)
                        .arg(pReg16Bit[selector-1]);
                pProperties=(quint16*)(((char*)Flag)+pReg16Bit[selector-1]);
            }
            else
            {
                strBaseAddress="(dim16Bit";
                pProperties=pDim16Bit;
            }
            str.push(strBaseAddress+QString("[%0]):%1").arg(strIndex).arg(pProperties[index]));
            stack.push(pProperties[index]);
        }
            break;
        case 0xE0:
        {
            auto strL = str.pop();
            int l=stack.pop();
            auto strR = str.pop();
            int r=stack.pop();
            int res = l+r;
            stack.push(res);
            str.push(QString("(%0 + %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xE1:
        {
            auto strR = str.pop();
            int r=stack.pop();
            auto strL = str.pop();
            int l=stack.pop();
            int res = l-r;
            stack.push(res);
            str.push(QString("(%0 - %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xE2:
        {
            auto strL = str.pop();
            int l=stack.pop();
            auto strR = str.pop();
            int r=stack.pop();
            int res = l*r;
            stack.push(res);
            str.push(QString("(%0 * %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xE3:
        {
            auto r=stack.pop();
            auto strR = str.pop();
            auto l=stack.pop();
            auto strL = str.pop();
            int res = l/r;
            stack.push(res);
            str.push(QString("(%0 / %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xE4:
        {
            auto r=stack.pop();
            auto strR = str.pop();
            auto l=stack.pop();
            auto strL = str.pop();
            int res = l%r;
            stack.push(res);
            str.push(QString("(%0 %% %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xE6:
        {
            auto l=stack.pop();
            auto strL = str.pop();
            auto r=stack.pop();
            auto strR = str.pop();
            stack.push(l==1 && r==1);
            str.push(QString("(%0 && %1):%2").arg(strL).arg(strR).arg(l==1 && r==1));
        }
            break;
        case 0xE7:
        {
            auto l=stack.pop();
            auto r=stack.pop();
            stack.push(l==1 || r==1);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 || %1):%2").arg(strL).arg(strR).arg(l==1 || r==1));
        }
            break;
        case 0xE8:
        {
            quint32 l=stack.pop();
            quint32 r=stack.pop();
            quint32 res=l&r;
            stack.push(res);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 & %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xE9:
        {
            quint32 l=stack.pop();
            quint32 r=stack.pop();
            quint32 res=l|r;
            stack.push(res);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 | %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xEB:
        {
            quint32 l=stack.pop();
            quint32 r=stack.pop();
            quint32 res=r<l;
            stack.push(res);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 > %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xEC:
        {
            quint32 l=stack.pop();
            quint32 r=stack.pop();
            quint32 res = l<r;
            stack.push(res);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 < %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xEF:
        {
            int res=stack.pop()==stack.pop();
            stack.push(res);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 == %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xF0:
        {
            int res=stack.pop()!=stack.pop();
            stack.push(res);
            auto strL = str.pop();
            auto strR = str.pop();
            str.push(QString("(%0 != %1):%2").arg(strL).arg(strR).arg(res));
        }
            break;
        case 0xF1:
        {
            quint32 l=getMes();
            quint32 r=getMes();
            quint32 res=l|r<<8;
            stack.push(res);
            str.push(QString("(%0)").arg(res));
        }

            break;
        case 0xF2:
        {
            ByteWorker bw;
            bw.byte[0]=getMes();
            bw.byte[1]=getMes();
            bw.byte[2]=getMes();
            bw.byte[3]=getMes();
            stack.push(bw.dword);
            str.push(QString("(%0)").arg(bw.dword));
        }
            break;
        case 0xF3:
        {
            ByteWorker bw;
            bw.dword=0;
            bw.byte[0]=getMes();
            bw.byte[1]=getMes();
            stack.push(Flag->flag[bw.dword]);
            str.push(QString("(Flags[%0]):%1").arg(bw.dword).arg(Flag->flag[bw.dword]));
        }
            break;
        case 0xF4:
        {
            quint32 index;
            Q_ASSERT(stack.size()>=1);
            index=stack.pop();
            stack.push(Flag->flag[index]);
            str.push(QString("(Flags[%0]):%1").arg(str.pop()).arg(Flag->flag[index]));
        }
            break;
        case 0xF5:
        {
            int index = stack.pop();
            auto strIndex = str.pop();
            quint8 selector = getMes();
            quint32* base32bit=0;
            QString strBase;
            if(selector)
            {
                base32bit=(quint32*)*(&Flag->bookmarkIndex+selector);
                strBase=QString("((quint32*)*(&Flag.bookmarkIndex+%0)").arg(selector);
            }
            else
            {
                base32bit=(quint32*)&Flag->dim32Bit;
                strBase=QString("(dim32Bit");
            }
            stack.push(base32bit[index]);
            str.push(strBase+QString("[%0]):%1").arg(strIndex).arg(base32bit[index]));
        }
            break;
        case 0xF6:
        {
            quint8 index=getMes();
            stack.push(pReg32Bit[index]);
            str.push(QString("(reg32Bit[%0]):%1").arg(index).arg(pDim32Bit[index]));
        }
            break;
        case 0xFF:
            calstr+=str.pop();
            notFinished = false;
            break;
        default:
            if(command>=0x80)
            {
                sendMessage(QString("Error Command not implemented:%0")
                            .arg(QString::number((unsigned int)command,16).toUpper().rightJustified ( 2, '0' )));
                isStop=true;
            }
            str.push(QString("(%0)").arg(command));
            stack.push(command);
            break;
        }
    }while(notFinished);
    result = stack.pop();
    if(!stack.isEmpty())
    {
        sendMessage(QString("Error! stack is not empty:%0").arg(stack.size()));
        isStop=true;
    }
    calstr+=")";
    return result;
}
