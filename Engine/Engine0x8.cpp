#include "engine.h"
#include "Reg32Audit.h"

void Engine::SetDim32Bits()
{
#if N2R_REG32_AUDIT
    /* Outer loop already consumed engine opcode 0x08. */
    const quint32 reg32InsnStart = gStruct.gMesCursor - 1;
#endif
    QString cstr;
    quint32 index = Calculate(cstr);
    quint8 selector = getMes();
    quint32* pBase32=0;

    QString strBase;
    if(selector)
    {
        pBase32 = Flag->reg32Bit.addresses[selector-1];
        strBase = QString("reg32Bit[%0-1]").arg(selector);
    }
    else
    {
        strBase = "dim32Bit";
        pBase32 = pDim32Bit;
    }

    int i=0;//for debug
    do
    {
        QString cstr2;
        /* Keep Calculate() call count identical; split assignment for forensic gate. */
        const quint32 value = static_cast<quint32>(Calculate(cstr2));
#if N2R_REG32_AUDIT
        {
            const int slotIndex = selector ? (selector - 1) : -1;
            const quint32 slotU32 = selector
                    ? Reg32Audit_SlotU32ViaBookmark(Flag, selector)
                    : 0;
            const quint32 pregU32 = (slotIndex >= 0)
                    ? Reg32Audit_SlotU32ViaPReg(pReg32Bit, slotIndex)
                    : 0;
            const quintptr structAddr = (slotIndex >= 0 && slotIndex < 26)
                    ? reinterpret_cast<quintptr>(Flag->reg32Bit.addresses[slotIndex])
                    : reinterpret_cast<quintptr>(pBase32);
            const bool willDeref = true;
            const quint32 rawSlotU32 = slotU32 ? slotU32 : pregU32;
            Reg32Audit_Log(Reg32Audit_IndirectWrite,
                           Flag->mesFileName,
                           reg32InsnStart,
                           selector,
                           slotIndex,
                           rawSlotU32,
                           static_cast<int>(index),
                           pregU32,
                           structAddr,
                           willDeref,
                           value,
                           true);
            if (Reg32Audit_ShouldForensicStop(selector))
            {
                sendMessage(QString("%2[%0+%3]=%1; /* REG32 forensic stop, write skipped */")
                            .arg(cstr).arg(cstr2).arg(strBase).arg(i));
                isStop = true;
                break;
            }
        }
#endif
        pBase32[index]=value;
        sendMessage(QString("%2[%0+%3]=%1;").arg(cstr).arg(cstr2).arg(strBase).arg(i));
        index++;
        i++;//for debug
    }while(getMes());
}
