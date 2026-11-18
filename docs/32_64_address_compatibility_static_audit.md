# Nanpa2 Rebirth：32/64 位地址兼容性静态审计

- 审计日期：2026-11-08
- 审计对象：当前工作树（含现有未提交修改）
- 审计方式：只读源码检索、调用链追踪、编译器记录布局输出；未运行游戏、未改动源代码、未执行重构、未提交 Git
- 数据限制：仓库中未发现 `*.ARC`、`SAVE*`、`*.MES` 或 `*.GP8` 样本，因此涉及脚本实际取值、旧存档字节内容和资源边界的结论按证据强度标记

## 1. 结论摘要

当前代码并不是简单地“用了若干 32 位整数”。它同时存在三套不同的地址语义：

1. 原游戏逻辑地址/偏移：MES 游标、动画/地图/移动数据偏移、缓存游标、书签和文件偏移。这些字段应继续保持固定宽度，通常仍应为 16 或 32 位。
2. 宿主机真实指针：VRAM 数据、Qt 对象、解码缓冲区、动画命令指针等。这些字段必须按宿主指针宽度保存，且不得进入旧存档或原始二进制格式。
3. 原 32 位进程地址或“地址槽”：`REG32BIT::addresses`、脚本选择器和若干旧 Win32 句柄字段。这类值不能直接扩宽后继续解引用，必须先确认其逻辑地址空间并经过显式翻译。

已证实的最高风险问题有：

- `FLAG` 在原 32 位布局中恰为 4096 字节；当前 x86_64 布局为 4224 字节。`sizeof(FLAG)` 读存档、`FlagSnapshot[4096]`、`MainStruct::mesBuffer` 的固定 `0x1000` 基址已经彼此失配。
- `REG32BIT` 在 32 位下是 26 个 4 字节地址槽，在 64 位下变为 26 个 8 字节指针；代码仍通过 `quint32*` 逐槽读写，发生半指针写入、步长错误和字段偏移漂移。
- `Engine::pMainStruct` 的临时伪基址只修正了部分 MES 访问；CPS、缓存写入、`FLAG` 内平面寻址和存档仍使用其他基址，当前内存模型不一致。
- 存档直接把宿主布局读入 `FLAG`，之后只修复部分指针；`REG32BIT` 未重建，且两种加载模式把书签和地图指针修到不同的存储区。
- `Animation` 仍用固定 `memset(..., 28)` 清理原 32 位前缀。x86_64 下 `unknown` 和若干计数器不在这 28 字节内，初始化范围已改变。

总体建议不是把全部 32 位字段改成 64 位，而是先把“固定格式状态”“逻辑地址空间”和“宿主运行时指针”拆开，再为旧格式建立显式的解析与地址翻译边界。

## 2. 已确认的布局差异

以下 64 位布局来自当前源码的 Clang record-layout 输出；32 位布局按相同字段顺序、4 字节指针和自然对齐静态推导。`FLAG` 的 32 位结果还被 `FlagSnapshot[4096]`、`cpsBase = 4096` 和 `mesBuffer` 旧基址共同印证。

| 字段 | 原 32 位偏移 | 当前 x86_64 偏移 | 差异 |
|---|---:|---:|---:|
| `FLAG::pDim16Bit` | 2176 | 2176 | 字段起点相同，字段宽度 4 → 8 |
| `FLAG::reg16Bit` | 2180 | 2184 | +4 |
| `FLAG::dim16Bit` | 2232 | 2236 | +4 |
| `FLAG::bookmarkIndex` | 2284 | 2288 | +4 |
| `FLAG::reg32Bit` | 2288 | 2296 | +8；结构大小 104 → 208 |
| `FLAG::dim32Bit` | 2392 | 2504 | +112；结构大小 36 → 48 |
| `FLAG::pMap` | 2428 | 2552 | +124；字段宽度 4 → 8 |
| `FLAG::charTable` | 2432 | 2560 | +128 |
| `FLAG::AnimationCommandBasePoint` | 3777 | 3905 | +128 |
| `FLAG::EF8` | 3832 | 3960 | +128 |
| `FLAG::field_F1C` | 3868 | 3996 | +128 |
| `FLAG::playerName` | 3932 | 4060 | +128 |
| `sizeof(FLAG)` / `MainStruct::mesBuffer` | 4096 | 4224 | +128 |

这说明从 `pDim16Bit` 之后，旧存档的字节位置和当前成员位置不再一致；从 `charTable` 开始形成稳定的 128 字节偏移差。单独把 MES 游标的基址调整为 `mesBuffer - 0x1000` 不能修复 `FLAG` 自身、CPS 或存档布局。

## 3. 详细发现

### A64-01：`FLAG` 同时作为旧 4096 字节存档块和宿主指针容器

- 位置：`Flag.h:61-99`；`Flag.h:108-125`；`engine.cpp:109-120`；`GameCommand/Command0x4.cpp:17-29, 39-55`
- 相关结构/函数：`REG32BIT`、`DIM32BIT`、`FLAG`、`MainStruct`、`Engine::LoadSave()`
- 地址实际语义：`FLAG` 中混合了游戏标志、固定宽度偏移、宿主指针和可能的旧进程地址槽。
- 来源/存储/使用：初始化时把宿主地址写入 `pDim16Bit`、`pMesCursorBookmarks`、`field_20`、`pMap`；加载时又把整个文件直接覆盖到 `FLAG`。
- 32/64 位差异：32 位 `FLAG` 为 4096 字节，当前 64 位为 4224 字节；成员偏移和 `MainStruct::mesBuffer` 起点改变。
- 风险：**严重（Critical）**。
- 兼容策略：定义独立的、固定 4096 字节的 legacy save DTO/字节视图；逐字段解析固定宽度数据；宿主指针完全排除在存档 DTO 外，并在加载完成后由运行时对象重建。不要对运行时 `FLAG` 使用 `sizeof(FLAG)` 作为文件格式大小。
- 状态：**已证实**。旧存档是否恰含所有 4096 字节、各未知尾字段的真实语义仍需样本验证。

### A64-02：`REG32BIT` 的 4 字节地址槽在 64 位下被错误建模为 8 字节指针数组

- 位置：`Flag.h:61-64`；`engine.cpp:157-158, 339-344`；`EngineCalculator.cpp:231-257`；`Engine/Engine0x8.cpp:3-30`
- 相关结构/函数：`REG32BIT::addresses[26]`、`pReg32Bit`、`Engine::Calculate()`、`Engine::SetDim32Bits()`
- 地址实际语义：脚本可写的 26 个 32 位地址槽；选择器非零时，其内容又被当作 `quint32*` 基址使用。它更接近“旧逻辑地址/旧进程地址表”，不是普通宿主指针数组。
- 来源/存储/使用：MES opcode `0x14` 经 `Calculate()` 产生 32 位值并写入；表达式 opcode `0xF6` 读取；`0xF5` 和 `SetDim32Bits()` 将槽值恢复为指针并索引。
- 32/64 位差异：`pReg32Bit` 仍按 4 字节递增，但底层 `addresses[]` 元素已为 8 字节。连续脚本写入会交替覆盖同一个指针的低/高半部；直接解引用通常得到无效地址。`EngineCalculator.cpp:240` 还从 `quint32` 槽恢复指针，不能表达一般 64 位宿主地址。
- 风险：**严重（Critical）**。
- 兼容策略：先把 26 个脚本槽定义为 `quint32 legacyAddress[26]`，保留原格式；所有解引用统一经过 `resolveLegacyAddress(quint32)`，返回经过范围验证的宿主 span/pointer。若后续证实槽内存的是区域 ID 或偏移，应按对应语义解析，不能直接 `reinterpret_cast`。
- 状态：**已证实**其布局和读写已损坏；槽值究竟是绝对旧地址还是逻辑偏移为**语义待确认**。

### A64-03：16/32 位脚本选择器依赖原 `FLAG` 平面偏移

- 位置：`EngineCalculator.cpp:32-51, 231-249`；`Engine/Engine0x7.cpp:35-67`；`Engine/Engine0x8.cpp:3-30`
- 相关结构/函数：`Engine::Calculate()` opcode `0xA0/0xF5`、`setDim16Bits()`、`SetDim32Bits()`
- 地址实际语义：`pReg16Bit[selector-1]` 是从 `FLAG` 起点计算的 16 位字节偏移；`&Flag->bookmarkIndex + selector` 是按原 4 字节槽位跨成员寻址。
- 来源/存储/使用：选择器和索引来自 MES；偏移值来自 `REG16BIT` 或 `REG32BIT` 槽；最终用于读写 `quint16`/`quint32` 属性。
- 32/64 位差异：旧偏移在当前 `FLAG` 中可能落到不同成员；`&bookmarkIndex + selector` 不再与指针数组元素一一对应；跨成员指针算术本身也不是受 C++ 对象模型保证的操作。
- 风险：**严重（Critical）**。
- 兼容策略：建立显式的 legacy FLAG byte view 或字段映射表。脚本选择器先解析为逻辑字段/逻辑区间，再访问运行时状态；不要依赖 C++ 结构体自然布局模拟旧进程内存。
- 状态：**已证实**存在布局依赖；哪些 selector 在实际游戏脚本中出现为**语义待确认**。

### A64-04：MES 临时伪基址只覆盖部分路径，形成多个不一致地址空间

- 位置：`engine.cpp:163-165, 372-389`；`GameCommand/Command0x12.cpp:23-33`；`GameCommand/Command0x7.cpp:11`
- 相关结构/函数：`Engine::pMainStruct`、`getMes()`、`MesCacheOperations()`、`loadDataMes()`
- 地址实际语义：`gMesCursor`、`mesStartPoint`、`mesCacheCursor` 是原逻辑地址/字节偏移；`Engine::pMainStruct` 是为使逻辑 `0x1000` 映射到当前 `mesBuffer` 而构造的宿主基址。
- 来源/存储/使用：MES 游标由初始化、跳转指令、锚点和缓存命令设置；`getMes()` 用伪基址读取，缓存整理也用伪基址复制。
- 32/64 位差异：当前伪基址实际指向 `gMainStruct` 内偏移 128，而不是结构起点。它能让 `0x1000` 映射到 `mesBuffer`，但 `getMes()` 的缓存写入却使用 `gMainStruct.Flag.mesFileName[cursor]`，CPS 又使用真实 `MainStruct*`。同一个逻辑值在不同路径落到不同宿主地址。
- 风险：**严重（Critical）**。
- 兼容策略：废除“伪指针 + 任意整数”的隐式模型，改为单一 `LegacyMemoryMap`：按逻辑区间映射 FLAG-save image、MES buffer、cache/data/map 区域；每次访问验证区间和长度。逻辑地址仍保持 `quint32`。
- 状态：**已证实**。实际缓存游标是否会落在受偏移漂移影响的区间需资源样本验证。

### A64-05：CPS 使用真实 `MainStruct* + cpsBase`，与当前 `mesBuffer` 起点相差 128 字节

- 位置：`cps.cpp:6-9, 95-121, 342-375`；`Flag.h:68-74`；`engine.cpp:117`
- 相关结构/函数：`pMoveInfoBase/pMoveInfos/pDoorInfos/pMyInfos` 宏、`CPS::init()`、`CPS::setMoveInfo()`
- 地址实际语义：`cpsBase = 4096` 是原 `MainStruct` 中 MES 区域的逻辑基址；`mapDataOffset`、`moveInfoOffset` 和四个 16 位子偏移是资源内部偏移。
- 来源/存储/使用：32 位 DIM 字段由初始化或 MES 设置；最终重解释为 `moveInfoBase`、`moveInfo`、`doorInfo`、`mapInfo` 和 `quint16` 表。
- 32/64 位差异：当前 `MainStruct::mesBuffer` 在 4224，而代码仍以真实结构起点加 4096；CPS 的首地址比对应 `mesBuffer + offset` 早 128 字节。
- 风险：**严重（Critical）**。
- 兼容策略：把 `cpsBase` 保留为 32 位逻辑基址，但通过统一 memory map 或明确的 `mesBuffer + relativeOffset` 访问；资源子结构按固定字节格式解析，不依赖宿主 `MainStruct` 布局。
- 状态：**已证实**地址差；实际命令是否触发及各偏移范围为**语义待确认**。

### A64-06：存档加载把旧指针值读入当前指针成员，且修复不完整、不一致

- 位置：`GameCommand/Command0x4.cpp:13-31, 33-56`
- 相关结构/函数：`Engine::LoadSave()` case 0/case 2
- 地址实际语义：文件中的对应 4 字节位置即使历史上保存过地址，也只能是旧进程值；加载后需要重建宿主引用。
- 来源/存储/使用：`QFile::read(pMain, sizeof(FLAG))` 直接覆盖运行时对象，然后手工修复部分字段。
- 32/64 位差异：当前请求读取 4224 字节；若旧文件为 4096 字节，返回值未检查且尾部 128 字节保留旧内存。更早的字节已经按错误成员边界解释。`REG32BIT::addresses` 完全未修复。
- 额外不一致：case 0 指向 `gMainStruct.mesCursorBookmarks/g_dword_562890/&gMainStruct.pMap`；case 2 指向 `Engine` 自身的重复数组和 `char pMap[52]`。两种模式的状态所有权不同。
- 风险：**严重（Critical）**。
- 兼容策略：文件读取到独立 4096 字节缓冲区；检查精确长度/版本；解析纯数据字段；明确两种加载模式需要保留/替换哪些逻辑状态；最后统一调用 runtime rebind。缺失或旧指针槽不得参与解析。
- 状态：**已证实**。旧文件是否包含原始指针、case 2 的预期所有权为**语义待确认**。

### A64-07：`FlagSnapshot[4096]` 只覆盖当前 `FLAG` 的前 4096 字节

- 位置：`engine.h:89`；`engine.cpp:145`；`GameCommand/Command0x4.cpp:39-49`
- 相关结构/函数：`Engine::FlagSnapshot`、`LoadSave()` case 2
- 地址实际语义：原设计显然把 FLAG 状态视为 4096 字节镜像。
- 来源/存储/使用：加载前把当前 FLAG 前 4096 字节复制到快照，读文件后只恢复前 128 字节文件名。
- 32/64 位差异：32 位下快照等于整个 FLAG；64 位下漏掉最后 128 字节，并包含宿主指针的原始字节。虽然当前只从快照恢复 128 字节，但名称和容量会误导后续维护，不能作为完整状态备份。
- 风险：**高（High）**。
- 兼容策略：将其重定义为明确的 `LegacyFlagImage[4096]` 或只保存实际需要保留的 `mesFileName[128]`；禁止复制运行时指针。
- 状态：**已证实**。

### A64-08：`Animation` 的固定 28 字节清零依赖 32 位布局

- 位置：`Animation/animationmanager.h:13-28`；`Animation/animationmanager.cpp:22-23, 29-45`
- 相关结构/函数：`Animation`、`AnimationManager` 构造函数、`PrepareAnimation()`
- 地址实际语义：`pCommands` 是指向 `mesBuffer` 内动画命令流的宿主指针；`unknown` 是未识别宿主指针；两个 bookmark 是命令流索引；`QWaitCondition` 是非平凡 Qt 对象。
- 来源/存储/使用：构造函数对每个元素执行 `memset(&a, 0, 28)`；随后 `PrepareAnimation()` 显式设置主要字段。
- 32/64 位差异：32 位布局中 28 字节恰好到 `QWaitCondition` 之前；当前 64 位布局为：`pCommands@8`、bookmarks `@16/@20/@24`、计数器 `@28/@29`、`field_1A@30`、`unknown@32`、`playing@40`。固定 28 字节不再清理计数器、`field_1A` 和 `unknown`。
- 风险：**高（High）**。
- 兼容策略：对每个平凡字段显式初始化，保留 `QWaitCondition` 的正常构造；不要对包含 Qt 对象的整个结构做原始 `memset`。
- 状态：**已证实**。

### A64-09：动画数据库存在固定步长、未对齐读取和基址混用

- 位置：`Animation/animationmanager.cpp:29-45, 103-178`；`Animation/animationmanager.h:29-40`；`GameCommand/Command0x3.cpp:40-68`
- 相关结构/函数：`PrepareAnimation()`、`animationRunner()`、`PaintingInstruction`
- 地址实际语义：`animationOffset` 是 MES/数据区逻辑偏移；表内 `quint16` 是相对偏移；`commandIndex`/bookmarks 是数组索引；`pCommands` 是已解析宿主指针。
- 来源/存储/使用：从 `mesBuffer + animationOffset` 读取 16 位表项，形成 `pCommands`；命令 150 读取调色板偏移；图像命令用 `17 * command` 定位 packed record。
- 32/64 位差异：索引和文件偏移本身无需扩宽；风险来自通过 `quint16*` 读取可能未对齐字节，以及 `&Flag.AnimationCommandBasePoint[animationOffset + ...]` 与其他路径使用 `mesBuffer + animationOffset` 的基址不一致。`PaintingInstruction` 实际大小为 13，但记录步长写死为 17，可能是合法文件格式，也可能遗漏 4 字节字段。
- 风险：**高（High）**。
- 兼容策略：把动画数据库作为 byte span 解析；用小端读取函数取得 16 位值；把 record stride 17 命名为格式常量并验证最小长度；确认图像命令表的真正基址后只保留一种路径。
- 状态：未对齐读取和两种基址是**已证实**；17 字节记录含义及 `AnimationCommandBasePoint` 语义为**语义待确认**。

### A64-10：`decodeGP8()` 调用链传递的是真实指针，未发现指针压入 32 位整数

- 位置：`GameCommand/Command0x8.cpp:13-29`；`Files/files.cpp:138-140, 206-221`；`DrawingUtil/drawingutil.cpp:185-225`；`Decoder/decoder.cpp:8-21`
- 相关结构/函数：`Engine::DrawBG()` → `Files::ReadRawBG()` → `DrawingUtil::decodeGP8()` → `Decoder::decodeGP8()`
- 地址实际语义：`defmainOffset` 是 `mesBuffer` 内逻辑偏移；`pBuffer`/`srcBuffer`/`header->data` 是宿主指针；`VramInfo::data` 是宿主像素缓冲区指针；宽高和坐标是数值。
- 来源/存储/使用：归档表提供文件 size/offset；原始 GP8 写到 `mesBuffer + defmainOffset`；头部被解析后，压缩数据指针传给 Decoder；目标是 `Engine::vrams[vRamIndex].data`。
- 32/64 位差异：本调用链没有把 `src` 或 VRAM 指针存入 8/16/32 位整数，指针参数在 64 位上传递宽度正确。`defmainOffset` 保持 32 位是合理的，只要它被限定为 `mesBuffer` 内偏移。
- 剩余风险：`ReadRawBG()` 不知道目标容量；`decodeGP8()` 没有源长度和目标 span，不能证明 offset、头部 1032 字节、压缩流或写入矩形在界内。
- 风险：**高（High）**（边界问题）；就“指针截断”本身为低风险。
- 兼容策略：保持 `defmainOffset` 为 32 位逻辑偏移；跨模块传递 `(span, length)`；验证 `offset + fileSize <= mesBufferCapacity`、头部长度、宽高、VRAM 选择器和输出矩形。
- 状态：指针来源和无截断结论为**已证实**；实际资源是否越界需样本验证。

### A64-11：缓存和书签保存的是逻辑游标，不是宿主指针

- 位置：`Flag.h:140-156`；`engine.h:27-31, 130-132`；`engine.cpp:132-133, 372-389, 455-471`；`Engine/Engine0xE.cpp:28-44`；`GameCommand/Command0x12.cpp:3-42`
- 相关结构/函数：`MesCacheState`、`GStruct`、`MesAnchor`、`mesCursorBookmarks`、`MesCacheOperations()`
- 地址实际语义：`gMesCursor`、`mesStartPoint`、`mesCacheCursor`、`MesAnchor::*`、`mesCursorBookmarks[]` 都是旧逻辑地址/偏移；不是宿主机地址。
- 来源/存储/使用：MES 跳转字节、锚点定义、缓存命令和用户输入书签设置；随后交给 legacy memory base 进行读取/复制。
- 32/64 位差异：这些值的逻辑空间远小于 4 GiB，改成 64 位不会修复映射问题，反而会破坏存档/脚本格式。真正风险是把它们与不一致的宿主基址相加。
- 风险：**高（High）**，原因是映射不一致而非字段宽度。
- 兼容策略：保留 `quint32`；为类型增加语义包装（如 `MesOffset`/`LegacyAddress`）；所有转换集中到一个带边界检查的 resolver。
- 状态：**已证实**。

### A64-12：二进制结构通过结构体重解释读取，宽度大多稳定但存在对齐/别名/端序假设

- 位置：`Files/files.cpp:93-105, 143-184`；`DrawingUtil/drawingutil.cpp:185-225`；`MainWindow.cpp:275-304`；`Sound/soundmanager.cpp:19-48`；`Sound/WaveFileHeader.h:5-19`
- 相关结构/函数：两个局部 `GP8Header`、`FilenameListStructure`、`WaveFileHeader`
- 地址实际语义：文件头字段、归档文件偏移和大小；均不是宿主指针。
- 来源/存储/使用：`QByteArray::data()`/`constData()` 被直接转为结构体指针；字段驱动调色板复制、文件 seek、图像尺寸和音频格式。
- 32/64 位差异：固定宽度整数使字段大小在常见 x86_64 ABI 下保持稳定；`FilenameListStructure` 有 `sizeof == 20` 断言，Win32 兼容图形结构也有断言。但任意 `char*` 不保证目标类型对齐，结构体别名访问和本机端序仍是隐含前提；`WaveFileHeader` 还把数据区固定在 44 字节，未验证块 ID/扩展 chunk。
- 风险：**中（Medium）**。
- 兼容策略：用小端字节读取函数或 `memcpy` 到已对齐的本地固定格式对象；对每个格式增加 `static_assert` 和最小长度检查；把 1032、44、20 命名为格式常量，不与宿主 `sizeof` 混用。
- 状态：**已证实**存在重解释访问；当前 x86_64 是否因具体分配对齐而实际失败为**待验证**。

### A64-13：联合体 type-punning 和原生 `unsigned int/short` 依赖实现与小端序

- 位置：`Decoder/decoder.h:7-12`；`Decoder/decoder.cpp:15-64`；`EngineCalculator.cpp:7-12, 201-218`；`Engine/Engine0x9.cpp:5-30`；`Engine/Engine0xA.cpp:5-17`；`Engine/Engine0x12.cpp:5-30`；`Files/DecodeArcMes.cpp:5-61`
- 相关结构/函数：各 `ByteWorker` union、MES 数值解码、LZ 解码
- 地址实际语义：4 字节小端整数拆装和压缩码字，不是指针。
- 来源/存储/使用：逐字节从文件/MES 填入 union，再读 `dword/word`；或反向读字节。
- 32/64 位差异：主流 LP64 平台的 `unsigned int` 仍为 32 位、`unsigned short` 为 16 位，所以不是直接的 64 位指针问题；但标准可移植性、active union member 和端序依赖仍存在。
- 风险：**中（Medium）**。
- 兼容策略：统一使用 `quint32/quint16/quint8` 和显式 little-endian 组合函数；不要把这类固定格式数值改成 `size_t` 或 64 位。
- 状态：**已证实**。

### A64-14：运行时缓冲区结构中的原生指针应保持宿主宽度，但不得整体复制/序列化

- 位置：`engine.h:33-40, 94-108, 138-143`；`Flag.h:158-166`；`Decoder/decoder.h:24-30`；`DrawingUtil/drawingutil.h:15-45, 125-164`；`cps.h:75-104`；`Files/files.h:31-76`
- 相关结构/函数：`VramInfo`、`DecodeHelper`、`Decoder`、`Bitmap`、`PaletteInfo`、`FontInfo`、`Engine`、`CPS`、`Files`
- 地址实际语义：堆缓冲区、Qt/引擎对象和内存区域的宿主指针。
- 来源/存储/使用：`new[]`、`QByteArray::data()`、对象构造注入或指向其他成员；用于直接解引用和跨模块参数传递。
- 32/64 位差异：指针自然扩宽，包含它们的结构体大小、对齐和后续成员偏移改变。当前大多数结构只在运行时按成员访问，因此本身可兼容；一旦使用固定长度 `memset/memcpy` 或持久化就会失效。
- 风险：**中（Medium）**，`Animation` 和 `FLAG` 已在其他发现中升级。
- 兼容策略：运行时结构使用正常指针/智能指针；禁止把整对象当 legacy blob；固定格式另建 DTO；对拥有关系和生命周期做明确标注。
- 状态：**已证实**。

### A64-15：若干 32 位 `h*`/`p*` 整数字段疑似旧 Win32 句柄或指针占位

- 位置：`config.h:7-11, 24-32`；`MainWindow.h:53-74`；`DrawingUtil/drawingutil.h:33-39, 134-143`；`Files/files.h:33-49`
- 相关字段：`DialogNowFile::hwndDialogNowFile`、`Config::hFileLog/pDialogNowFile`、`MainWindow::hDesktop/hInstance`、`PaletteInfo::hLogPalette`、`Files::hFile_Arc*`
- 地址实际语义：当前代码大多只清零或未使用；命名来自旧 Win32 反编译，可能是句柄、ID 或未迁移占位。
- 来源/存储/使用：目前未发现从宿主指针赋值或恢复并解引用；`Config::pDialogNowFile` 仅设为 0，`Files::hFile_Arc*` 仅设为 0。
- 32/64 位差异：若未来重新接入本机句柄，这些 32 位字段不能承载 64 位句柄/指针；若只是游戏 ID，则应继续保持 32 位。
- 风险：**中（Medium，潜伏）**。
- 兼容策略：在重新启用前逐字段确认；宿主句柄使用平台句柄类型或 `quintptr`，逻辑 ID 保持 `quint32`。不要仅因名字含 `p`/`h` 就批量扩宽。
- 状态：全部为**语义待确认**；当前未发现已发生的指针截断。

### A64-16：`MainWindow::title` 是 128 个指针，不是 128 字节字符缓冲区

- 位置：`MainWindow.h:59-62`
- 相关结构/函数：`MainWindow::title`
- 地址实际语义：按声明是 `char* [128]` 指针数组，但注释表示已经不用；命名更像旧标题字符缓冲区。
- 来源/存储/使用：全仓库未发现使用。
- 32/64 位差异：占用从 512 字节变为 1024 字节，会改变 `MainWindow` 私有布局，但当前没有外部二进制布局依赖。
- 风险：**低（Low）**。
- 兼容策略：保持停用状态；若恢复功能，先确认原意是 `char title[128]`、字符串指针数组还是 Qt 字符串，再选择类型。
- 状态：声明事实**已证实**；设计语义为**语义待确认**。

### A64-17：静态调色板指针数组在 64 位下会扩宽，但当前按元素访问，不依赖字节布局

- 位置：`palettes.h:127693-127759`；`Engine/Engine0x10.cpp:103-126`
- 相关结构/函数：`off_45D2E0[]`、`off_45D3E0[]`、`command_0x10()`
- 地址实际语义：指向编译期调色板字节数组的宿主指针表。
- 来源/存储/使用：静态初始化；按 `off_... [index]` 取元素并读取颜色字节。
- 32/64 位差异：表本身大小翻倍，但 C++ 元素索引自动采用正确指针步长；没有序列化或固定字节偏移使用。
- 风险：**低（Low）**。
- 兼容策略：保持指针宽度；增加索引范围验证；不要把反编译符号名中的旧地址当作当前地址。
- 状态：**已证实**为低风险宿主指针表。

### A64-18：仍有固定字节数/步长依赖，需要按“格式常量”与“宿主布局”分治

- 位置：`engine.cpp:418`；`Animation/animationmanager.cpp:23, 176`；`Files/files.cpp:131, 149-165`；`MainWindow.cpp:275-304`；`DrawingUtil/drawingutil.cpp:95, 116`；`Engine/Engine0x10.cpp:19-25, 131-199`
- 相关常量：`0x348`、`28`、`17`、`1032`、`20`、`44`、`4` 字节 palette entry、`236*4`
- 地址实际语义：其中 `0x348 = 30 * sizeof(SubCommand)` 在当前字段宽度下稳定；20/44/1032/4/17 多数是文件或游戏格式步长；28 是已失效的宿主结构前缀大小。
- 32/64 位差异：固定格式常量应不变；由宿主结构布局推导的常量必须消除。混在一起会导致错误的批量“64 位化”。
- 风险：**中（Medium）**；`Animation` 的 28 已单独评为高风险。
- 兼容策略：为每个常量命名并注明单位/格式来源；宿主数组清零使用字段初始化或 `sizeof(array)`；文件 stride 使用显式解析器和 `static_assert`。
- 状态：**已证实**。

### A64-19：当前直接的整数/指针转换点需要分类处理

- 位置：`EngineCalculator.cpp:240`；`EngineGameCommand.cpp:142`；大量 C 风格结构指针转换见 A64-09、A64-12、A64-13
- 相关函数：`Engine::Calculate()`、`Engine::gameCommand()`
- 地址实际语义：`EngineCalculator.cpp:240` 把 32 位槽值恢复为 `quint32*`，属于真实地址转换；`EngineGameCommand.cpp:142` 把命令整数转为 `void*` 仅用于日志输出。
- 来源/存储/使用：前者随后解引用；后者只传给 `QDebug`。
- 32/64 位差异：前者在 64 位下不能恢复任意宿主指针且可能得到无效低地址；后者不会截断有效指针，但输出语义误导。
- 风险：前者 **严重（Critical）**（已计入 A64-02），后者 **低（Low）**。
- 兼容策略：前者改用 legacy address resolver；后者按整数十六进制输出。全仓库当前未发现“把一个已存在的宿主指针显式窄化保存到 8/16/32 位整数”的其他活动代码。
- 状态：**已证实**。历史版本中 `AnimationManager::FillBackground()` 曾出现 `(qint32)&instruction->height`，但当前工作树已不再如此；本报告不把历史代码算作当前缺陷。

## 4. 指针包含结构清单

下表只列项目源码直接声明的指针字段；Qt 类和值类型内部的实现指针不逐一展开，但它们同样禁止原始序列化。

| 结构/类 | 指针字段（含嵌套） | 用途 | 是否进入原始复制/持久化 | 64 位判断 |
|---|---|---|---|---|
| `REG32BIT` (`Flag.h:61`) | `quint32* addresses[26]` | 旧地址槽/目标基址 | 嵌入 `FLAG` 存档 | **严重；类型建模错误** |
| `DIM32BIT` (`Flag.h:66`) | `pMesCursorBookmarks`, `field_20` | 运行时数组指针 | 嵌入 `FLAG` 存档 | **严重；应从 DTO 排除并重建** |
| `FLAG` (`Flag.h:79`) | `pDim16Bit`, nested `REG32BIT`, nested `DIM32BIT`, `pMap` | 混合状态 | `sizeof(FLAG)` 文件读取、快照 | **严重** |
| `MainStruct` (`Flag.h:108`) | nested `FLAG`, `mapInfo* pMap` | 全局内存状态 | 整体 `memset`，不直接存档 | 布局影响逻辑内存映射 |
| `GStruct` (`Flag.h:147`) | `MainStruct* pMainStruct` | 运行时全局引用 | 仅 `mesCaches` 被清零 | 宿主指针，勿持久化 |
| `DecodeHelper` (`Flag.h:158`) | `p_sourceBuffer`, `p_destBuffer` | 解码缓冲区 | 所属 `Bitmap` 堆对象 | 宿主指针，当前未序列化 |
| `VramInfo` (`engine.h:33`) | `quint8* data` | VRAM 像素缓冲区 | `VramInfo[6]` 不做原始复制 | 宿主指针，当前安全传递 |
| `Animation` (`animationmanager.h:13`) | `pCommands`, `unknown`, nested `QWaitCondition` | 命令流和同步 | 固定 28 字节 `memset` | **高风险布局依赖** |
| `Bitmap` (`drawingutil.h:15`) | `pDecodeHelper` | 解码状态 | 分配后按成员使用 | 宿主指针；结构大小改变 |
| `PaletteInfo` (`drawingutil.h:33`) | `pLogPalette`; `hLogPalette` 待确认 | 调色板 | 整体 `memset(sizeof)` 后重建指针 | 当前 runtime-only；句柄待确认 |
| `FontInfo` (`drawingutil.h:41`) | `pFont`, `pLogFont` | Qt/字体对象 | 按成员初始化 | 宿主指针 |
| `Engine` (`engine.h:42`) | 多个对象指针、`void*`、nested `VramInfo`、FLAG 视图指针 | 核心运行时 | `FlagSnapshot` 仅复制 FLAG 区 | 不应作为旧结构镜像 |
| `CPS` (`cps.h:49`) | `pEngine`, `pMainStruct`, `pVram*`, `pMoveInfo`, map-data pointers | 运行时缓存视图 | 不序列化 | 指针宽度本身安全；基址计算高风险 |
| `Files` (`files.h:15`) | `pConfig`, `pDecodeBuffer`, `pMainStruct`, 5 个 filename buffer 指针 | 文件系统运行时 | 不序列化 | 宿主指针；旧 hFile 字段待确认 |
| `Decoder` (`decoder.h:14`) | `p_sourceBuffer`, `p_destBuffer` | 临时解码游标 | 栈对象，不序列化 | 宿主指针；`p_destBuffer` 当前未使用 |
| `Stack` (`stack.h:9`) | `pConfig`, `pMainStruct1` | 运行时引用 | 不序列化 | `pMainStruct1` 未初始化/未使用，需清理确认 |
| `Config` (`config.h:13`) | `MainStruct* pMainStruct`; `pDialogNowFile` 为 32 位待确认 | 配置运行时 | 不序列化 | 真指针安全；整数占位待确认 |
| `MainWindow` (`MainWindow.h:19`) | `pConfig`, `char* title[128]`, Qt smart pointers | UI/音频 | 不序列化 | title 语义可疑；Qt 成员 runtime-only |
| `DrawingUtil` (`drawingutil.h:47`) | 多个对象指针和 `void*` GDI 占位 | 绘图运行时 | 不序列化 | 宿主指针；GDI 占位待确认 |
| `SoundManager` (`soundmanager.h:13`) | Qt `QSharedPointer` 内部指针 | 音频运行时 | 不序列化 | 按 Qt ABI 管理 |
| `OpenGLWindow` (`openglwindow.h:51`) | `m_context`, `m_device` | Qt OpenGL 对象 | 不序列化 | 按成员使用，低风险 |
| `off_45D2E0/off_45D3E0` (`palettes.h:127693`) | `quint8*` 静态指针数组 | 调色板表 | 不序列化 | 元素访问安全，表大小改变无外部依赖 |
| 全局根指针 (`main.cpp:13-17`; `global.h:15`) | `gpEngine`, `threadGpEngine`, `files` | 引擎/文件对象入口 | 不序列化 | 宿主指针；`files` 当前仅声明，未见本仓库定义/使用 |

## 5. 字段级地址存储清单

### 5.1 组 A：宿主机真实指针——必须保持宿主宽度

| 字段/参数 | 定义/来源 | 存储位置 | 使用位置 | 结论 |
|---|---|---|---|---|
| `VramInfo::data` | `engine.h:35`；`new[]` 于 `engine.cpp:95-100` | `Engine::vrams[6]` | Decoder、DrawingUtil、CPS | 真指针；不得放入旧格式 |
| `Animation::pCommands` | `animationmanager.h:19`；由 `mesBuffer + offsets` 生成 | `animations[10]` | `animationRunner()` | 真指针；可由逻辑 offset 重建 |
| `Decoder::p_sourceBuffer` | `decoder.h:26`；参数赋值于 `decoder.cpp:10` | Decoder 栈对象 | 解码循环 | 真指针；调用链无窄化 |
| `DecodeHelper::p_sourceBuffer/p_destBuffer` | `Flag.h:161-162` | `Bitmap::pDecodeHelper` 指向对象内 | 当前未见活动使用 | 真指针；语义部分待确认 |
| `DIM32BIT::pMesCursorBookmarks/field_20` | `Flag.h:75-76`；初始化/加载后绑定 | `FLAG` 内 | 脚本平面访问 | 真指针；必须移出持久 DTO |
| `FLAG::pDim16Bit/pMap` | `Flag.h:83,89`；初始化/加载后绑定 | `FLAG` 内 | 旧结构兼容访问 | 真指针；必须重建 |
| `MainStruct::pMap` | `Flag.h:125`；CPS init 生成 | 全局状态 | `CPS::getMapData()` | 真指针；由 map offset 重建 |
| `REG32BIT::addresses[]` | `Flag.h:63` | `FLAG` 内 | SetDim32/Calculate | 当前声明为真指针但实际更像 legacy 地址槽；需重建模型 |

### 5.2 组 B：原游戏逻辑地址/偏移——应保留固定宽度

| 字段 | 宽度 | 实际语义 | 生成/来源 | 使用 | 结论 |
|---|---:|---|---|---|---|
| `DIM32BIT::defmainOffset` | 32 | `mesBuffer` 内 GP8 临时区偏移 | 默认 131072/脚本 | `DrawBG()` | 保持 32 位；验证容量 |
| `animationOffset` | 32 | MES/数据区动画数据库偏移 | 脚本 | AnimationManager | 保持 32 位；统一基址 |
| `mapDataOffset` | 32 | map 数据相对 MES 区偏移 | 脚本 | CPS init | 保持 32 位；resolver 转换 |
| `moveInfoOffset` | 32 | move 数据相对 MES 区偏移 | 脚本 | CPS 宏 | 保持 32 位；resolver 转换 |
| `cpsBase` | 32 | legacy MES 区基址，当前固定 4096 | 初始化/加载修复 | CPS | 保持逻辑常量；勿等同 `offsetof` |
| `REG16BIT` 选择器值 | 16 | 从 legacy FLAG 基址计算的字节偏移 | MES | Calculate/setDim16 | 保持 16 位格式；映射到字段视图 |
| `GStruct::gMesCursor/mesStartPoint` | 32 | MES 逻辑游标/基点 | 跳转、调用、锚点 | `getMes()` | 保持 32 位；统一解析 |
| `MesCacheState::mesCacheCursor` | 32 | legacy memory/cache 偏移 | cache command | cache compaction | 保持 32 位；统一解析 |
| `MesAnchor::mesCursor/mesStartPoint` | 32 | MES 逻辑游标快照 | DefineAnchors | RunFromSavedAnchorPoint | 保持 32 位 |
| `mesCursorBookmarks[]` | 32 each | MES 逻辑游标 | `unknown0xe()` | 用户输入/脚本 | 保持 32 位，不是指针 |
| `Animation::commandIndex/itemIndexBookmark*` | 32 | `pCommands` 内数组索引 | animation runner | animation runner | 保持 32 位；验证长度 |
| `moveInfoBase::*Offset` | 16 | CPS 资源块内相对偏移 | 数据文件 | CPS 宏/函数 | 保持 16 位；小端解析 |
| `FileList` pair second / record `offset` | 32 | 归档文件偏移 | ARC 索引 | `QFile::seek()` | 保持 32 位格式；转换为 `qint64` seek 前验证 |
| `CPS::offsets[]` | 32 | VRAM 像素数组索引 | 构造计算 | block painting | 数组索引，不是地址 |

### 5.3 组 C：可能承载旧地址/句柄——语义待确认

| 字段 | 证据 | 当前状态 | 修复前要求 |
|---|---|---|---|
| `REG32BIT::addresses[26]` 的槽值 | 被脚本写入并恢复为 `quint32*` | 活动且已损坏 | 用实际 MES 追踪值域，判定绝对旧地址/逻辑偏移/区域表 |
| `DIM32BIT::field_10/field_14` | `field_14` 用作 `mesBuffer` 表偏移 | 部分活动 | 分字段命名，确认 `field_10` |
| `REG16BIT::OffsetToMainstructField_EF8` | 名称和 A0 平面访问暗示偏移 | 未见直接字段引用 | 追踪对应脚本 selector |
| `Config::pDialogNowFile` | `quint32` 且名称为 p | 仅清零 | 确认是地址、句柄还是状态值 |
| `DialogNowFile::hwndDialogNowFile` | 旧 HWND 命名，类型 `int` | 当前未使用 | 若恢复 UI，使用宿主窗口句柄类型 |
| `MainWindow::hDesktop/hInstance` | 旧 Win32 命名 | 当前未使用 | 确认是否彻底废弃 |
| `PaletteInfo::hLogPalette` | 旧 GDI handle 命名 | 当前未使用 | 不得用 32 位承载新宿主句柄 |
| `Files::hFile_Arc*` | 旧 file handle 命名 | 仅清零 | 当前 QFile 已替代，建议保持停用并文档化 |
| `FLAG::playerName/unknown_*` | 仅为 32 位字段，缺少地址行为证据 | 未确认 | **不得仅因 32 位就认定为指针** |
| `CPS::field_415C/field_514D` 等 | 仅为 32 位未知字段 | 未确认 | **语义待确认**，不批量扩宽 |

### 5.4 组 D：明确不是地址

- `SubCommand::command`：表达式结果/命令参数。
- `MesCacheState::bActive/size`：状态与长度。
- `DIM16BIT` 坐标、尺寸、VRAM index、颜色值：游戏数值。
- `FilenameListStructure::size`：文件长度；`offset` 才是文件偏移，但两者都不是宿主指针。
- 调色板 `quint32`、`PALETTEENTRY` 和颜色表：像素/颜色数据。
- `WaveFileHeader` 的 32 位字段：文件格式数值。
- `Animation` 的 bookmarks 和 `commandIndex`：命令流索引。
- `MainStruct` 的 map/cps left/top/width/height：游戏坐标和尺寸。

## 6. 保存、恢复、复制和序列化机制

| 机制 | 位置 | 保存内容 | 地址语义 | 64 位结论 |
|---|---|---|---|---|
| SAVE 加载 | `GameCommand/Command0x4.cpp:17-29, 44-55` | 原始 `FLAG` blob | 混合纯数据和旧/宿主指针 | **不兼容；必须改为 legacy DTO 解析** |
| FLAG 快照 | `engine.h:89`; `Command0x4.cpp:41,49` | 4096 字节内存镜像 | 包含指针字节 | 仅能作为旧格式 image，不能镜像当前结构 |
| MES cache 元数据 | `Flag.h:140-152`; `Command0x12.cpp` | offset/size/active | 逻辑偏移 | 宽度正确，mapper 不一致 |
| MES cache 内容 | `engine.cpp:383-385`; `Command0x12.cpp:28-33` | 命令字节 | 通过逻辑地址定位 | 写入和整理使用不同基址 |
| MES 书签 | `Engine0xE.cpp:28-44` | 50 个逻辑游标及关联值 | 逻辑偏移/索引 | 保持 32 位；不应视为指针 |
| MES anchors | `engine.h:27-31`; `Engine0x12.cpp` | 游标和 start point | 逻辑偏移 | 保持 32 位 |
| 动画状态 | `animationmanager.h/.cpp` | runtime `Animation[10]` | 指针 + 索引 + Qt 同步对象 | 未持久化；固定 28 字节初始化不兼容 |
| 图像缓冲区 | `VramInfo`, `QImage`, `Bitmap` | 像素数组 | 宿主指针 + 数组索引 | 不序列化；需长度传播 |
| ARC 文件表 | `Files/files.cpp:143-185` | 20 字节记录 | 文件 offset/size | 固定格式，保持 32 位并显式小端解析 |
| WAV/GP8 头 | `WaveFileHeader.h`; `drawingutil.cpp`; `files.cpp` | 固定格式字段 | 文件数值，不含指针 | 不应随宿主位数变化 |

仓库未发现写出 SAVE 文件的代码，因此当前只能审计加载端。旧存档的准确生产格式、版本差异和指针槽内容需要真实样本或原程序写档路径验证。

## 7. 按修复依赖关系分组

### 依赖组 0：冻结兼容契约（所有后续工作的前置）

1. 明确 legacy FLAG image 为 4096 字节，并记录 32 位字段偏移。
2. 明确逻辑地址空间的区间：FLAG、MES、data/cache/map 等各自的基址和容量。
3. 收集至少一份 ARC/MES/GP8 和不同模式的 SAVE 样本，记录 `REG32BIT`、DIM offsets 和 selector 值域。
4. 增加只读布局/格式断言；不要修改存档或脚本字段宽度。

### 依赖组 1：建立逻辑地址解析层

依赖组 0。实现单一的 `LegacyMemoryMap`/resolver，输入固定宽度逻辑地址和访问长度，输出经过验证的宿主 byte span。先替换：

- `Engine::pMainStruct[gMesCursor]`
- cache 读写/整理
- `REG16BIT` 平面偏移访问
- `REG32BIT` 地址槽解引用
- CPS 的 `cpsBase + offset`

### 依赖组 2：拆分存档 DTO 与运行时状态

依赖组 0，最好依赖组 1。读取固定 4096 字节 legacy image，逐字段转换到 runtime state；运行时指针集中 rebind。处理 case 0/case 2 的状态所有权差异，并检查读入长度。不要序列化 `FLAG` C++ 对象本身。

### 依赖组 3：修复 `REG32BIT`/selector 模型

依赖组 0 和 1。先确认槽值语义，再将 26 个槽改为固定 32 位 legacy value；所有读写仍保持脚本兼容，只有 resolver 结果使用宿主指针。

### 依赖组 4：统一数据区消费者

依赖组 1。让动画、CPS、`DrawBG/decodeGP8` 和 `loadDataMes` 使用同一套 `(region, offset, length)` API；禁止一个路径基于 `MainStruct*`、另一个基于 `mesBuffer`、第三个基于 `Flag.mesFileName`。

### 依赖组 5：二进制格式安全解析

可与组 2-4 并行，但应在端到端验证前完成。替换未对齐结构指针读取，传播源/目标长度；为 ARC 20 字节记录、GP8 1032 字节头、WAV 基本头、动画 17 字节记录建立格式常量和边界检查。

### 依赖组 6：纯运行时结构清理

依赖较少。修复 `Animation` 固定 28 字节初始化；确认 `MainWindow::title`、旧 h/p 字段、未初始化未使用指针；保留运行时真实指针宽度。此组不能替代前面的地址模型修复。

### 依赖组 7：兼容性验证

依赖组 1-6。建议至少覆盖：

- 32 位布局 golden offsets 与 4096 字节 save fixture。
- 旧存档 case 0/case 2 加载后，所有 runtime pointer 都指向当前对象且未使用文件中的地址字节。
- MES 跳转、锚点、书签、缓存滚动在统一逻辑地址空间内保持相同游标。
- `REG32BIT` selector 的有效/无效地址解析。
- CPS map/move 数据基址与资源内部 16 位偏移。
- `decodeGP8` 从 archive entry 到 VRAM 的源/目标边界。
- 动画 command table、17 字节 record 和 palette offset。

## 8. 不应采取的修复

- 不应把所有 `quint32`、文件 offset、MES cursor、bookmark 或数组 index 批量改为 64 位。
- 不应通过 `#pragma pack(4)` 强行让包含宿主指针的 runtime `FLAG` 恢复旧布局；这仍会把 8 字节指针塞进旧槽位，并污染存档格式。
- 不应假设把旧 32 位数值 cast 为 `quintptr` 就能恢复地址；必须先解析其逻辑空间。
- 不应让保存格式依赖 `sizeof(FLAG)`、`sizeof(MainStruct)` 或 Qt/编译器 ABI。
- 不应把反编译符号名中的十六进制数字视为当前宿主地址。
- 不应在缺乏读写证据时把 `playerName`、`unknown_*`、CPS unknown fields 等所有 32 位字段认定为指针。

## 9. 待验证问题

1. `REG32BIT` 26 个槽的实际 MES 值域：旧绝对 VA、相对 FLAG/MES offset，还是区域基址表？
2. 旧 SAVE 文件长度、版本和指针槽内容；case 0/case 2 的预期差异。
3. `animationOffset` 的真正基准，以及 `AnimationCommandBasePoint + offset` 是否为反编译基址补偿。
4. 动画记录为何使用 17 字节步长，而 `PaintingInstruction` 只声明 13 字节。
5. `DIM32BIT::field_10/field_14`、`REG16BIT::OffsetToMainstructField_EF8` 和未知 32 位字段的语义。
6. MES cache cursor 的实际范围，以及 `gMainStruct.Flag.mesFileName[cursor]` 是否试图模拟全局旧地址空间。
7. `ReadRawBG()` 目标区间是否总能容纳 archive entry；当前接口无法证明。
8. 旧 Win32 `h*`/`p*` 整数字段是废弃占位、逻辑 ID 还是将来需恢复的宿主句柄。

在上述问题得到样本证据前，应保持“语义待确认”，不得通过批量扩宽或结构体 packing 猜测修复。
