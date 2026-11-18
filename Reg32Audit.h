#ifndef REG32_AUDIT_H
#define REG32_AUDIT_H

/*
 * TASK: N2R-REG32-DYNAMIC-EVIDENCE-001
 * Temporary REG32BIT slot / indirect-access audit.
 *
 * Compile-time switches:
 *   N2R_REG32_AUDIT          - 1: emit [REG32-AUDIT] logs (default: on in QT_DEBUG)
 *   N2R_REG32_FORENSIC_STOP  - 1: after logging, stop before dereferencing
 *                              REG32 slot pointers (selector != 0)
 *
 * Disable entirely: -DN2R_REG32_AUDIT=0
 */

#ifndef N2R_REG32_AUDIT
#  ifdef QT_DEBUG
#    define N2R_REG32_AUDIT 1
#  else
#    define N2R_REG32_AUDIT 0
#  endif
#endif

#ifndef N2R_REG32_FORENSIC_STOP
#  define N2R_REG32_FORENSIC_STOP 0
#endif

#if N2R_REG32_AUDIT

#include <QtGlobal>
#include <QDebug>
#include <QString>
#include "Flag.h"

enum Reg32AuditEvent
{
    Reg32Audit_WriteSlot = 0,
    Reg32Audit_IndirectRead,
    Reg32Audit_IndirectWrite
};

inline const char *Reg32Audit_EventName(Reg32AuditEvent event)
{
    switch (event)
    {
    case Reg32Audit_WriteSlot:     return "WRITE_SLOT";
    case Reg32Audit_IndirectRead:  return "INDIRECT_READ";
    case Reg32Audit_IndirectWrite: return "INDIRECT_WRITE";
    }
    return "UNKNOWN";
}

/* MES plane view: same quint32-step used by opcode 0xF5. */
inline quint32 Reg32Audit_SlotU32ViaBookmark(FLAG *Flag, int selector)
{
    if (selector <= 0)
        return 0;
    return *(&Flag->bookmarkIndex + selector);
}

/* Script plane view via pReg32Bit (quint32[] over reg32Bit). */
inline quint32 Reg32Audit_SlotU32ViaPReg(const quint32 *pReg32Bit, int slotIndex)
{
    if (!pReg32Bit || slotIndex < 0)
        return 0;
    return pReg32Bit[slotIndex];
}

inline void Reg32Audit_Log(Reg32AuditEvent event,
                           const char *mesFileName,
                           quint32 insnStartOffset,
                           int selector,
                           int slotIndex,
                           quint32 rawU32,
                           int elemIndex,
                           quint32 structSlotU32,
                           quintptr structAddrAsPtr,
                           bool willDeref,
                           quint32 writeU32 = 0,
                           bool hasWriteU32 = false)
{
    QString line = QString("[REG32-AUDIT] event=%1 mes=%2 insn=0x%3 selector=%4 slot=%5 "
                           "raw_u32=0x%6 elem=%7 struct_slot_u32=0x%8 struct_addr=%9 "
                           "will_deref=%10 forensic=%11")
                       .arg(Reg32Audit_EventName(event))
                       .arg(mesFileName ? mesFileName : "(null)")
                       .arg(insnStartOffset, 0, 16)
                       .arg(selector)
                       .arg(slotIndex)
                       .arg(rawU32, 8, 16, QChar('0'))
                       .arg(elemIndex)
                       .arg(structSlotU32, 8, 16, QChar('0'))
                       .arg(QString("0x%1").arg(structAddrAsPtr, 0, 16))
                       .arg(willDeref ? 1 : 0)
                       .arg(N2R_REG32_FORENSIC_STOP);
    if (hasWriteU32)
        line += QString(" write_u32=0x%1").arg(writeU32, 8, 16, QChar('0'));
    qDebug().noquote() << line;
}

/* true => caller must skip the real slot-pointer dereference. */
inline bool Reg32Audit_ShouldForensicStop(int selector)
{
#if N2R_REG32_FORENSIC_STOP
    return selector != 0;
#else
    Q_UNUSED(selector);
    return false;
#endif
}

#endif /* N2R_REG32_AUDIT */

#endif /* REG32_AUDIT_H */
