# REG32BIT::addresses[26] 专项数据流审计

- TASK_ID：`N2R-REG32-ADDRESS-FLOW-AUDIT-001`
- 审计日期：2026-11-09
- 审计对象：当前工作树 `feat/qt6-migration`（含未提交修改），以现有源码为准
- 审计方式：只读静态数据流追踪；未修改 `.cpp`/`.h`/CMake/运行时配置；未运行游戏或测试；未提交 Git
- 相关前置文档：`docs/32_64_address_compatibility_static_audit.md`（A64-02 / A64-03）
- 证据强度标记：
  - **CONFIRMED**：直接由源码证明
  - **INFERRED**：由数据流/布局推断
  - **UNKNOWN**：静态源码无法确认

---

## 总判定（先看这个）

`REG32BIT::addresses[26]` **不是**单纯的 MES 虚拟机独立整数寄存器文件；也 **尚未**被静态证明为“绑定到特定命名内存区的固定间接寻址表”。

当前源码显示它同时承担两类角色：

1. **32 位槽寄存器**：经 `pReg32Bit` 按 `quint32` 步长读写（opcode `0x14` / `0xF6`）。**CONFIRMED**
2. **间接基址槽**：非零 selector 时，槽值被直接解释为 `quint32*`，再按 index 读写目标内存（`SetDim32Bits` / `Calculate` `0xF5`）。**CONFIRMED**（解释方式）；槽内实际存放的是旧绝对 VA、逻辑偏移，还是从别处拷来的指针截断值 —— **UNKNOWN**（缺 MES/运行时样本）

与 `REG16BIT` 的对照很关键：16 位选择器用的是 `Flag` 相对字节偏移；32 位选择器 **不加任何基址**，直接把槽值当指针。**CONFIRMED**

---

## A. 字段语义

### A.1 结构定义

```61:64:Flag.h
typedef struct tag_REG32BIT
{
    quint32* addresses[26];
} REG32BIT;
```

嵌入位置：

```86:88:Flag.h
    quint32 bookmarkIndex;
    REG32BIT reg32Bit;
    DIM32BIT dim32Bit;
```

### A.2 已确认用途（聚合级，不是按索引命名）

| 槽范围 | 已确认用途 | 证据强度 |
|---|---|---|
| `addresses[0..25]` 作为整体 | MES 可写的 26 个 32 位值槽（经由 `pReg32Bit`） | CONFIRMED |
| 同一批槽 | 非零 selector 时作为 `quint32*` 间接基址 | CONFIRMED |
| 与 `bookmarkIndex` 的邻接关系 | `0xF5` 用 `&Flag->bookmarkIndex + selector` 的 `quint32` 步长跨成员取槽 | CONFIRMED |
| 单个 `addresses[i]` 的游戏语义名 | 无源码命名、无注释、无初始化绑定表 | UNKNOWN |

对比：`DIM16BIT` / `REG16BIT` / `DIM32BIT` 都有命名字段；`REG32BIT` **只有** `addresses[26]`，没有任何按索引的语义标签。

### A.3 不得凭索引猜测的结论

以下内容 **全部标记 UNKNOWN**，本审计不猜测：

- 槽 0–25 各自对应哪一类游戏对象（角色、地图、物品、窗口……）
- 脚本是否总是把“指向 `dim32Bit` 同类块”的地址写入槽中
- 槽值在原版中是否恒等于 `MainStruct` 相对偏移
- 是否存在引擎侧“启动时把 26 个区基址填入 addresses”的遗失代码路径（当前树中 **未见**）

### A.4 与 `DIM32BIT` 的关系（避免混淆）

`DIM32BIT` 是另一组命名 32 位字段/指针；`SetDim32Bits` / `0xF5` 在 **selector==0** 时直接访问 `dim32Bit`，不经过 `addresses[]`。**CONFIRMED**

`addresses[]` 提供的是“额外基址选择器”，用于指向 **dim32Bit 以外**（或任意）的 `quint32` 数组视图。目标究竟是什么 —— **UNKNOWN**。

---

## B. 读写数据流

### B.1 别名建立（所有 `pReg32Bit` 路径的根）

| 项 | 内容 |
|---|---|
| 文件/行号 | `engine.cpp:157-158` |
| 函数 | `Engine` 构造函数 |
| 类型 | 别名建立（类型重解释） |
| 数据来源 | `&Flag->reg32Bit` / `&Flag->dim32Bit` |
| 最终目标 | `Engine::pReg32Bit`、`Engine::pDim32Bit` 视为 `quint32*` 平面数组 |
| 证据强度 | CONFIRMED |

```157:158:engine.cpp
    pDim32Bit=reinterpret_cast<quint32*>(&Flag->dim32Bit);
    pReg32Bit=reinterpret_cast<quint32*>(&Flag->reg32Bit);
```

含义：**脚本平面访问把 `REG32BIT` 当作 `quint32[]`，不是 `quint32*[]`。** 这与类型声明冲突，是 64 位布局损坏的根因之一。**CONFIRMED**

---

### B.2 写入：初始化 / 重置

| 项 | 内容 |
|---|---|
| 文件/行号 | `engine.cpp:109` |
| 函数 | `Engine` 构造函数 |
| 类型 | 整块清零写入 |
| 数据来源 | `memset(&gMainStruct, 0, sizeof(gMainStruct))` |
| 最终目标 | 全部 `addresses[]` → 0 / nullptr |
| 证据强度 | CONFIRMED |

初始化段（`engine.cpp:111-130`）显式绑定了 `pDim16Bit`、`dim32Bit.pMesCursorBookmarks`、`dim32Bit.field_20`、`pMap` 等，**没有任何**对 `reg32Bit.addresses[i]` 的单独赋值。**CONFIRMED**

结论：启动时地址槽 **未** 绑定到特定内存区域；全零。**CONFIRMED**

---

### B.3 写入：MES opcode `0x14` → `pReg32Bit[index]`

| 项 | 内容 |
|---|---|
| 文件/行号 | `engine.cpp:336-348` |
| 函数 | `Engine::StartEngine()` |
| 类型 | 直接槽写入（按 `quint32`） |
| 选择器/索引 | `index = getMes()`，随后每次循环 `index++`；以 `getMes()!=0` 为续写分隔 |
| 合法范围 | 源码 **无边界检查**；结构名义容量 26；越界会写穿后续 `dim32Bit` 等（INFERRED） |
| 数据来源 | `Calculate(cstr)` 的 `int` 返回值 |
| 最终目标 | `pReg32Bit[index]`（即 `reg32Bit` 的第 `index` 个 `quint32` 视图） |
| 证据强度 | CONFIRMED |

```336:348:engine.cpp
        case 0x14:
            depth++;
        {
            for(int index = getMes();;index++)
            {
                QString cstr;
                pReg32Bit[index]=Calculate(cstr);
                sendMessage(QString("reg32Bit[%0] = %1;").arg(index).arg(pReg32Bit[index]));
                if(!getMes())
                    break;
            }
        }
            depth--;
            break;
```

说明：`EngineGameCommand.cpp:122` 的 `case 0x14` 是游戏子命令（等待输入），与本寄存器写入无关。**CONFIRMED**

---

### B.4 写入值来源：`Calculate()` 可能产出的数据类型

| 项 | 内容 |
|---|---|
| 文件/行号 | `EngineCalculator.cpp:5-282` |
| 函数 | `Engine::Calculate` |
| 返回类型 | `int`（在 LP64 上仍为 32 位有符号）**CONFIRMED** |
| 赋给槽时 | 截断/转换进 `quint32` **CONFIRMED** |

`Calculate` 可压栈的来源类别：

| opcode | 产出 | 与地址槽关系 |
|---|---|---|
| `<0x80` 立即数 | 小整数 | 可写入槽 |
| `0xF1` | 16 位立即数 | 可写入槽 |
| `0xF2` | 32 位立即数（MES 四字节） | **可直接把任意 u32 常量写入槽** CONFIRMED |
| `0x80` | `reg16Bit[i]` | 通常小整数 |
| `0xA0` | Flag 相对 16 位属性 | 数值语义，不是 32 位基址 |
| `0xF3`/`0xF4` | `Flag->flag[]` | 字节 |
| `0xF5` | 经基址解引用的 `quint32` | 可把“别处读到的 32 位值”再写入槽 |
| `0xF6` | `pReg32Bit[i]` | 槽间拷贝 |
| `0xE0–0xEF` 等 | 算术/比较 | 派生值 |

**未发现**宿主 `void*` / `char*` 被直接赋给 `pReg32Bit` 的路径。**CONFIRMED**  
因此：若槽里曾出现“像指针的值”，只可能来自 (a) MES `0xF2` 常量、(b) 从其他 `quint32` 视图读回、(c) 算术结果。**(a)(b) CONFIRMED 可能；(c) 是否用于造址 UNKNOWN**

**不得**因为声明是 `quint32*` 就认定写入值是宿主指针。约束已遵守：类型声明 ≠ 数据语义。

---

### B.5 间接写入：`SetDim32Bits()`（MES opcode `8`）

| 项 | 内容 |
|---|---|
| 文件/行号 | `Engine/Engine0x8.cpp:3-31`；调度于 `engine.cpp:292-293` |
| 函数 | `Engine::SetDim32Bits` |
| 类型 | 读槽为基址 + 向目标写 `quint32` |
| 选择器 | `selector = getMes()`（`quint8`） |
| 选择器范围 | `0` → 使用 `pDim32Bit`；`!=0` → `addresses[selector-1]`；无上界检查 |
| 槽值解释 | **宿主 `quint32*` 绝对指针**（无 Flag/MainStruct 基址相加） |
| 最终访问 | `pBase32[index] = Calculate(...)`；`index` 来自 `Calculate`，每次成功后续写 `index++` |
| 元素类型 | `quint32` |
| 32/64 依赖 | 直接依赖指针宽度与 `addresses[]` 元素宽度 |
| 能否写到其他游戏内存 | **能**（只要槽值指向该内存）CONFIRMED |
| 证据强度 | CONFIRMED（机制）；目标区域集合 UNKNOWN |

```3:30:Engine/Engine0x8.cpp
void Engine::SetDim32Bits()
{
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
        pBase32[index]=Calculate(cstr2);
        ...
        index++;
        i++;
    }while(getMes());
}
```

对照 16 位路径 `setDim16Bits`（`Engine/Engine0x7.cpp:42-45`）：

```42:45:Engine/Engine0x7.cpp
    if(selector)
    {
strBase = QString("&Flag->mesFileName[pReg16Bit[%0-1]]").arg(selector);
        base=(quint16*)(&Flag->mesFileName[pReg16Bit[selector-1]]);
```

| 对比项 | REG16 选择器 | REG32 选择器 |
|---|---|---|
| 槽内容角色 | Flag 内字节偏移 | 直接当指针 |
| 是否加基址 | 加 `(char*)Flag` / `mesFileName` | **不加** |
| 证据强度 | CONFIRMED | CONFIRMED |

---

### B.6 读取（不解引用）：`Calculate` opcode `0xF6`

| 项 | 内容 |
|---|---|
| 文件/行号 | `EngineCalculator.cpp:252-257` |
| 函数 | `Engine::Calculate` |
| 类型 | 直接槽读取 |
| 选择器 | `index = getMes()`（`quint8`） |
| 合法范围 | 无检查；名义 0–25 |
| 槽值解释 | 普通 `quint32` 数值压栈 |
| 最终目标 | 表达式栈；可再被 `0x14` 写回或其他运算使用 |
| 间接写能力 | 无（本路径） |
| 证据强度 | CONFIRMED |

```252:257:EngineCalculator.cpp
        case 0xF6:
        {
            quint8 index=getMes();
            stack.push(pReg32Bit[index]);
            str.push(QString("(reg32Bit[%0]):%1").arg(index).arg(pDim32Bit[index]));
        }
```

附注：调试字符串误用 `pDim32Bit[index]`，与真实压栈值不一致；不影响语义结论，但说明维护时易混淆两套平面。**CONFIRMED**

---

### B.7 读取 + 解引用：`Calculate` opcode `0xF5`

| 项 | 内容 |
|---|---|
| 文件/行号 | `EngineCalculator.cpp:231-250` |
| 函数 | `Engine::Calculate` |
| 类型 | 跨成员取槽 → 转为指针 → 索引读 `quint32` |
| 选择器 | `selector = getMes()`；`index` 来自表达式栈 |
| selector==0 | `base32bit = (quint32*)&Flag->dim32Bit` |
| selector!=0 | `base32bit = (quint32*)*(&Flag->bookmarkIndex + selector)` |
| 槽值解释 | 先按 `quint32` 读出，再强制转换为 `quint32*` |
| 最终目标 | 压栈该指针指向的 `base32bit[index]` |
| 合法范围 | 无检查；原 32 位布局下 selector 1..26 对齐 `addresses[0..25]`（见 C 节） |
| 跨成员指针运算 | **是**（`bookmarkIndex` 上做 `quint32*` 算术）CONFIRMED |
| 间接写能力 | 本路径只读；但可读到任意被指向的 `quint32` |
| 证据强度 | CONFIRMED |

```231:250:EngineCalculator.cpp
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
```

与 `SetDim32Bits` 的一致性（原 32 位）：

- `SetDim32Bits`：`addresses[selector-1]`
- `0xF5`：`*(bookmarkIndex + selector)`，因 `addresses` 紧跟 `bookmarkIndex`，等价于读同一槽再解引用

**CONFIRMED（32 位布局下等价）**；**当前 64 位布局下二者已错位（见 C）CONFIRMED**

---

### B.8 间接写入路径汇总（别名 / 原始复制 / 存档）

| 路径 | 行为 | 是否触及 addresses | 证据 |
|---|---|---|---|
| `memset(&gMainStruct,0,…)` | 清零 | 是 | CONFIRMED `engine.cpp:109` |
| opcode `0x14` | 按槽写 u32 | 是 | CONFIRMED |
| `LoadSave` case 0/2 | `read(pMain, sizeof(FLAG))` 整块覆盖 | 是，且加载后 **不重建** addresses | CONFIRMED `GameCommand/Command0x4.cpp:17-29,39-55` |
| `FlagSnapshot` | 加载前拷贝前 4096 字节；之后只恢复前 128 字节文件名 | 快照含旧 addresses 字节，但恢复后 addresses 来自存档 | CONFIRMED |
| `ComChange` / `CallMes` | 重载 MES，不重置 Flag 主体 | **不**清 addresses | CONFIRMED |
| Animation / CPS / Files | 无 `reg32Bit`/`pReg32Bit`/`addresses` 引用 | 否 | CONFIRMED（全库检索） |
| `memcpy` 专拷 `reg32Bit` | 无 | 否 | CONFIRMED |
| 引擎写 SAVE 文件 | 本仓库未见 `SAVE*` 写出实现 | 存档中是否含历史槽值 UNKNOWN | UNKNOWN |

`LoadSave` 修复清单对比：

| 字段 | case 0 修复 | case 2 修复 | addresses 修复 |
|---|---|---|---|
| `pDim16Bit` | 有 | 有 | — |
| `dim32Bit.cpsBase` | 有 | 有 | — |
| `pMesCursorBookmarks` | 有 | 有（指向 Engine 副本） | — |
| `field_20` | 有 | 有 | — |
| `pMap` | 有 | 有 | — |
| `reg32Bit.addresses[]` | **无** | **无** | CONFIRMED 缺失 |

---

### B.9 其他可能间接碰到槽字节的路径

| 路径 | 说明 | 证据 |
|---|---|---|
| `EngineGameCommand.cpp:96` `*((quint16*)&Flag->bookmarkIndex+1)=i` | 写的是 `bookmarkIndex` 的高 16 位，不是 `addresses[0]`（LE 下仍在 bookmarkIndex 四字节内） | CONFIRMED |
| `sizeof(FLAG)` 存档读写 | 把整块 FLAG（含 reg32Bit）当 blob | CONFIRMED |
| `pReg32Bit` 越界写 | 无边界时可能写穿 `dim32Bit` | INFERRED |

---

## C. 别名与内存布局

### C.1 原 32 位 vs 当前 64 位（围绕 bookmarkIndex / reg32Bit）

布局与既有 Clang record-layout 审计一致；本处聚焦地址槽别名。

| 字段 | 原 32 位偏移 | 当前 x86_64 偏移 | 大小 32→64 |
|---|---:|---:|---|
| `bookmarkIndex` | 2284 | 2288 | 4→4 |
| （对齐填充） | 无 | 2292–2295（4 字节） | 0→4 |
| `reg32Bit` | 2288 | 2296 | 104→208 |
| `dim32Bit` | 2392 | 2504 | 36→48（`sizeof`） |
| `sizeof(FLAG)` | 4096 | 4224 | +128 |

`REG32BIT` 元素：

| ABI | 声明类型 | 元素宽 | 26 槽总宽 | `pReg32Bit` 步长 |
|---|---|---:|---:|---:|
| 原 32 位 | `quint32*` | 4 | 104 | 4（一致） |
| 当前 64 位 | `quint32*` | 8 | 208 | 仍按 4（**不一致**） |

### C.2 三条访问路径在两种 ABI 上的落点

记 `BM` = `&Flag->bookmarkIndex`。

#### 原 32 位（一致）

| 访问 | 公式 | 落点 |
|---|---|---|
| `pReg32Bit[i]` | `reg32Bit + 4*i` | `addresses` 的第 i 个 u32 槽 |
| `addresses[s-1]` | `reg32Bit + 4*(s-1)` | 同槽，作为指针 |
| `0xF5` `*(BM+s)` | `BM + 4*s` | 同槽（因 `reg32Bit == BM+4`） |

**CONFIRMED**：三路径在 32 位下对齐到同一组 26×4 字节槽。

#### 当前 64 位（分裂）

| 访问 | 公式 | 落点 |
|---|---|---|
| `pReg32Bit[i]` | `reg32Bit + 4*i` | 每个 8 字节指针的半字；连续写撕裂指针 |
| `addresses[s-1]` | `reg32Bit + 8*(s-1)` | 完整 8 字节指针元素 |
| `0xF5` `*(BM+s)` | `BM + 4*s` | s=1 落在 **padding**；其后与 `pReg32Bit`/`addresses[]` 均不对齐 |

**CONFIRMED**：当前树中三条路径已不再指向同一逻辑槽。

示意（64 位，FLAG 局部）：

```
offset 2288: bookmarkIndex (4)
offset 2292: padding (4)          <--- 0xF5 selector=1 读这里
offset 2296: addresses[0] bytes0-7 <--- SetDim32Bits selector=1；pReg32Bit[0] 只写前 4 字节
offset 2300:                       <--- pReg32Bit[1]；0xF5 selector=3 等
offset 2304: addresses[1] ...
```

### C.3 通过 `quint32*` 或固定偏移访问该结构的全部路径

| # | 路径 | 方式 | 文件:行 |
|---|---|---|---|
| 1 | `pReg32Bit = reinterpret_cast<quint32*>(&Flag->reg32Bit)` | 平面 `quint32*` | `engine.cpp:158` |
| 2 | `pReg32Bit[index] = …` / 读取 | `quint32` 步长 | `engine.cpp:342-343`；`EngineCalculator.cpp:255` |
| 3 | `Flag->reg32Bit.addresses[selector-1]` | 声明指针数组元素 | `Engine/Engine0x8.cpp:13` |
| 4 | `(quint32*)*(&Flag->bookmarkIndex+selector)` | 固定 `quint32` 跨成员偏移 + 解引用 | `EngineCalculator.cpp:240` |
| 5 | `read/memcpy` 整块 `FLAG` | 原始字节覆盖 | `GameCommand/Command0x4.cpp` |

除此之外，源码中 **没有** 其他 `addresses` / `pReg32Bit` / `reg32Bit` 引用。**CONFIRMED**

---

## D. 架构判定

### D.1 能否简单替换为 `std::array<quint32,26>`？

| 结论 | 证据强度 | 说明 |
|---|---|---|
| **作为存储模型：应当改回 26×`quint32`，而不是 `quint32*`** | CONFIRMED | `0x14`/`0xF6`/`0xF5`（按 u32 取槽）与原 32 位布局一致；当前 `quint32*` 是错误建模 |
| **仅替换类型、不改解引用语义：不足** | CONFIRMED | `SetDim32Bits` / `0xF5` 仍会把槽值当指针解引用 |
| **“简单替换后即可在 64 位正确运行”：否** | INFERRED | 还缺地址翻译；否则解引用的仍是旧 32 位数值 |

### D.2 是否需要为地址槽绑定独立的内存空间？

| 结论 | 证据强度 | 说明 |
|---|---|---|
| 槽在启动时 **未** 绑定独立区 | CONFIRMED | 仅 memset 0 |
| 运行期 **可以** 指向独立区（机制上） | CONFIRMED | 间接读写任意 `quint32*` |
| 游戏是否 **实际** 依赖多块独立目标区 | UNKNOWN | 需 MES/GDB 观察 selector 与槽值 |
| 与 `dim32Bit`（selector 0）是并列基址机制 | CONFIRMED | 源码分支明确 |

### D.3 是否必须经由全局 `LegacyMemoryMap`？

| 结论 | 证据强度 | 说明 |
|---|---|---|
| 若槽值是旧进程绝对 VA 或跨 FLAG/MES/cache 的逻辑地址，则需要统一翻译层 | INFERRED | 与前置审计 A64-04 一致；本对象是最直接的解引用点之一 |
| 源码中已存在名为 `LegacyMemoryMap` 的实现 | 否 — 仅出现在审计建议中 | CONFIRMED（当前树无该类型） |
| “必须且只能”用全局 map，而非局部 resolver | UNKNOWN | 架构选项；静态码不能唯一决定 |
| 至少不能继续 `reinterpret_cast` 裸解引用 | CONFIRMED | 64 位下数值无法表达宿主指针 |

### D.4 是否存在无需保留的历史指针机制？

| 机制 | 是否保留 | 证据强度 |
|---|---|---|
| 26 个脚本可写的 32 位槽 | **必须保留**（脚本 ABI） | CONFIRMED |
| 非零 selector 的间接 `quint32` 数组访问 | **必须保留语义** | CONFIRMED |
| 把槽声明/存储成宿主 `quint32*` | **不应保留** | CONFIRMED（64 位已损坏；与 `pReg32Bit` 冲突） |
| 无翻译地 `*(quint32*)slot` | **不应保留** | CONFIRMED |
| 假设槽值 == `MainStruct` 偏移 | **不得作为前提** | 约束；当前也无源码证明 |

---

## E. 动态验证计划（最小 GDB）

静态分析无法确认槽内数值的“游戏语义”。建议在 **修复布局之前或并行** 用下列最小探针；重点是区分“逻辑值”与“当前损坏指针”，避免把 64 位错误布局下的垃圾解引用误当成原版语义。

### E.1 推荐断点

| 断点位置 | 目的 |
|---|---|
| `Engine::StartEngine` 内 `case 0x14`：`pReg32Bit[index]=Calculate(...)` 之后 | 记录每次槽写入的 `index` 与 32 位值 |
| `Engine::SetDim32Bits`：取出 `selector` 之后、解引用之前 | 记录 selector、index、原始槽比特 |
| `Engine::Calculate` `case 0xF5`：计算 `base32bit` 之后、`base32bit[index]` 之前 | 对比 `*(BM+selector)` 与 `addresses[selector-1]` 原始字节 |
| `Engine::Calculate` `case 0xF6` | 记录纯寄存器读 |
| `Engine::LoadSave`：`read` 完成后 | 观察存档覆盖后的 26 槽原始字节（加载后无重建） |

### E.2 每次命中应读取的量

1. `index` / `selector`
2. 槽原始 8 字节（即便逻辑宽是 4，也要看清 64 位撕裂）
3. `pReg32Bit[i]` 读到的 u32
4. `Flag->reg32Bit.addresses[i]` 作为指针打印的值
5. `&gMainStruct`、`&gMainStruct.Flag`、`gMainStruct.mesBuffer`、`Engine::pMainStruct` 的宿主地址
6. 若即将解引用：先 **不要** `x/w base32bit`；先算 `slot_u32` 与上述基址的差

### E.3 如何判定槽值类别

对每次写入的 `value`（按 u32）：

| 检验 | 若成立则倾向 |
|---|---|
| `value == 0` | 空槽 / 未初始化 |
| `value < 0x1000` 或像小整数 | 逻辑索引/小常量，**不是**可用宿主指针 |
| `value - 0x1000` 落在 `mesBuffer` 逻辑范围内 | MES 逻辑地址（类似游标） |
| `value` ≈ 某次运行的 `&Flag + K` 且 K 像 FLAG 内偏移 | Flag 平面绝对 VA（原 32 位风格） |
| `value` ≈ `&gMainStruct + K` | MainStruct 绝对 VA |
| `value` 等于当时 `dim32Bit` 某指针字段的低 32 位 | 从 DIM 指针拷贝/截断而来 |
| MES 回溯显示前置 `0xF2` 立即数 | 脚本硬编码常量（对照原版加载基址） |

### E.4 避免把损坏指针误判为原语义

1. **优先在“写槽”断点采数**，不要在“已经崩溃的解引用”后倒推。
2. 同时记录三条视图：`pReg32Bit[i]`、`addresses[i]` 原始字节、`*(uint32_t*)((char*)&bookmarkIndex + 4*selector)`；三者不一致时，说明当前是布局伪影，不是脚本语义。
3. 若只能在 64 位构建调试：把槽值始终当 **u32 比特模式** 打印（`p/x *(uint32_t*)...`），禁止先转 `void*` 再判断“像不像指针”。
4. 理想对照：同一 MES 路径在原 32 位构建或带 legacy FLAG 镜像的探测器上复跑，比较写入的 u32 是否稳定（与 ASLR 无关则更像逻辑偏移；随加载基址变则像绝对 VA）。
5. 仓库若缺少 `*.MES`/`SAVE*` 样本，动态结论应标为实验室数据，并归档原始 `index/selector/value` 日志。

### E.5 最小日志字段（建议）

```
ts, site(0x14|0xF5|0xF6|SetDim32|LoadSave), index, selector,
slot_u32, slot_raw8, flag_base, mes_buf, pmain_pseudo,
diff_flag, diff_mes, diff_pmain
```

---

## 结论摘要（≤10 行）

1. `addresses[26]` 是 MES 可写的 32 位槽，并在非零 selector 下被当作 `quint32*` 间接基址；不是纯封闭寄存器文件。**CONFIRMED**
2. 唯一常态写入源是 opcode `0x14`←`Calculate()`；启动仅清零，无预绑定表。**CONFIRMED**
3. 读取有两路：`0xF6` 读槽值；`0xF5`/`SetDim32Bits` 解引用后访问 `quint32` 数组，具备碰触其他游戏内存的能力。**CONFIRMED**
4. 与 `REG16BIT`（Flag 相对偏移）不同，REG32 选择器 **不加基址**。**CONFIRMED**
5. 当前 `quint32* addresses[26]` + `pReg32Bit` 的混用已在 64 位撕裂布局；存储应视为 `quint32[26]`。**CONFIRMED**
6. 存档 `sizeof(FLAG)` 覆盖槽且不重建；MES 重载不重置槽。**CONFIRMED**
7. 26 槽的逐项游戏含义、槽值是旧 VA / 逻辑偏移 / 截断指针 —— **UNKNOWN**。
8. 尚缺运行时证据：真实 MES 的 `0x14`/`0xF5`/`SetDim32` 值域日志，以及（可选）原 32 位对照与 SAVE 样本中的槽字节。
