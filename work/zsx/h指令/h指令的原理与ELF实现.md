# 选项- h的原理和具体实现



## 一、选项- h 的介绍

选项-h ： 显示ELF的文件头信息

成员函数及其含义如下：

| 成员        | 含义                                                         |
| :---------- | :----------------------------------------------------------- |
| e_ident     | Magic: 7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00       |
|             | Class: ELF64                                                 |
|             | Data: 2's complement, little endian                          |
|             | Version: 1(current)                                          |
|             | OS/ABI: UNIX System V ABI                                    |
|             | ABI Version: 0                                               |
| e_type      | Type: None (None)                                            |
|             | ELF文件类型                                                  |
| e_machine   | Machine: None                                                |
|             | ELF文件的CPI平台属性                                         |
| e_version   | Version: 0x0                                                 |
|             | ELF版本号。一般为常数1                                       |
| e_entry     | Entry point address: 0x0                                     |
|             | **入口地址，规定ELF程序的入口虚拟地址，操作系统在加载完该程序后从这个地址开始执行进程的指令。可重定位指令一般没有入口地址，则该值为0** |
| e_phoff     | Start of program headers: 0(bytes into file)                 |
| e_shoff     | Start of section headers: 0 (bytes into file)                |
|             | Section Header Table 在文件中的偏移                          |
| e_word      | Flags: 0x0                                                   |
|             | ELF标志位，用来标识一些ELF文件平台相关的属性。               |
| e_ehsize    | Size of this header: 0 (bytes)                               |
|             | ELF Header本身的大小                                         |
| e_phentsize | Size of program headers: 0 (bytes)                           |
| e_phnum     | Number of program headers: 0                                 |
| e_shentsize | Size of section headers: 0 (bytes)                           |
|             | 单个Section Header大小                                       |
| e_shnum     | Number of section headers: 0                                 |
|             | Section Header的数量                                         |
| e_shstrndx  | Section header string table index: 0                         |
|             | Section Header字符串表在Section Header Table中的索引         |

## 二、选项- h的作用

`-h` 命令可以显示 文件的 ELF 文件头信息，包括文件类型、机器架构、入口地址、程序头表和节头表的偏移和数量等信息。

## 三、选项- h显示的信息解释

#### 3.1 Magic魔数

每种可执行文件的格式的开头几个字节都是很特殊的，特别是开头4个字节，通常被称为**魔数（Magic Number）**。通过对魔数的判断可以确定文件的格式和类型。如：ELF的可执行文件格式的头4个字节为`0x7F`、`e`、`l`、`f`；Java的可执行文件格式的头4个字节为`c`、`a`、`f`、`e`；如果被执行的是Shell脚本或perl、python等解释型语言的脚本，那么它的第一行往往是`#!/bin/sh`或`#!/usr/bin/perl`或`#!/usr/bin/python`，此时前两个字节`#`和`!`就构成了魔数，系统一旦判断到这两个字节，就对后面的字符串进行解析，以确定具体的解释程序路径。

#### 3.2 ELF文件类型

ELF文件主要有三种类型，可以通过ELF Header中的`e_type`成员进行区分。

- **可重定位文件（Relocatable File）**：`ETL_REL`。一般为`.o`文件。可以被链接成可执行文件或共享目标文件。静态链接库属于可重定位文件。
- **可执行文件（Executable File）**：`ET_EXEC`。可以直接执行的程序。
- **共享目标文件（Shared Object File）**：`ET_DYN`，一般为`.so`文件，有两种情况可以使用。
  - 链接器将其与其他可重定位文件、共享目标文件链接成新的目标文件；
  - 动态链接器将其与其他共享目标文件、结合一个可执行文件，创建进程映像。

#### 3.3 机器类型

ELF文件格式被设计成可以在多个平台下使用，但这并不是表示同一个ELF文件可以在不同的平台下使用，而是表示不同平台下的ELF文件都遵循同一套ELF标准。`e_machine`成员就表示该ELF文件的平台属性，

- `EM_M32`：1 「AT&T WE 32100」
- `EM_SPARC`：2 「SPARC」
- `EM_386`: 3 「Intel x86」
- `EM_68K`：4 「Motorola 680000」
- `EM_88K`：5 「Motorola 88000」
- `EM_860`：6 「Intel 80860」

## 四、代码实现

### 4.1算法思路

下面以32为ELF字长为例进行解析

- 数据结构定义「32位」

```cpp
typedef struct {
    unsigned char    e_ident[16];        /* ELF "magic数" */
    unsigned char    e_type[2];        /* 标识对象文件类型 */
    unsigned char    e_machine[2];        /* 指定所需的体系结构 */
    unsigned char    e_version[4];        /* 标识目标文件版本 */
    unsigned char    e_entry[4];        /* 入口点虚拟地址*/
    unsigned char    e_phoff[4];        /* 程序头表文件偏移量 */
    unsigned char    e_shoff[4];        /* 节头表文件偏移量 */
    unsigned char    e_flags[4];        /* 特定于处理器的标志 */
    unsigned char    e_ehsize[2];        /* ELF头大小（以字节为单位） */
    unsigned char    e_phentsize[2];        /* 程序头表条目大小 */
    unsigned char    e_phnum[2];        /* 程序头表条目计数 */
    unsigned char    e_shentsize[2];        /* 节头表条目大小 */
    unsigned char    e_shnum[2];        /* 节头表项计数 */
    unsigned char    e_shstrndx[2];        /* 节头字符串表索引 */
} Elf32_External_Ehdr;
```

- Magic数的16个字节对应关系
  - e_ident
    - 第1个字节：0x7f 表示ELF标记
    - 第2个字节：‘E’
    - 第3个字节：‘L’
    - 第4个字节：‘F’
    - 第5个字节：ELF文件类型，也就是class
    - 第6个字节：数据编码信息，字节序信息，大端序、小端序
    - 第7个字节：ELF版本号：标识ELF Version, 该值等于EV_CURRENT 
    - 第8个字节：OS信息
    - 第9个字节：ABI 版本信息
    - 10~16个字节：填充位
  - e_type：2个字节，指示文件类型 
  - e_machine：2个字节，机器类型
  - e_version：文件版本
  - e_entry：4个字节/8个字节-进程开始的虚拟地址
  - e_phoff：4个字节/8个字节-程序头部表的开始
  - e_shoff：4个字节/8个字节-节头部表的开始
  - e_flags：4个字节-取决于目标架构
  - e_ehsize：2个字节-文件头部的大小
  - e_phentsize：2个字节-程序头部的大小
  - e_phnum：2个字节-程序头部的条目数
  - e_shentsize：2个字节，节头部表的大小
  - e_shnum：2个字节-节头部表的条目数
  - e_shstrndx：2个字节，节头部条目和其位置的对应关系

### 4.3代码详细解释

#### 4.3.1判断magic数是否能正确表示ELF文件

​	magic数的第1～第4个字节表示ELF版本号，固定为`7f 45 4c 46`

​	直接读取即可，代码如下：

```cpp
/*判断是否Magic是否存在问题*/
    if (   elf_header.e_ident[EI_MAG0] != ELFMAG0
            || elf_header.e_ident[EI_MAG1] != ELFMAG1
            || elf_header.e_ident[EI_MAG2] != ELFMAG2
            || elf_header.e_ident[EI_MAG3] != ELFMAG3)
    {
        printf("Not an ELF file - it has the wrong magic bytes at the start\n");
        return 0;
    }
```

#### 4.3.2提取ELF文件的class

​	magic数的第5个字节表示class，class一共有3种类型，否则返回\<unknown\>

实现代码如下：

```cpp
/*从magic数中，提取elf_class信息*/
const char *ELF_process::get_elf_class (unsigned int elf_class)
{
    static char buff[32];
    /*判断elf_class处的字节信息*/
    switch (elf_class)
    {
    case ELFCLASSNONE:
        return ("none");
    case ELFCLASS32:
        return "ELF32";
    case ELFCLASS64:
        return "ELF64";
    default:
        snprintf (buff, sizeof (buff), ("<unknown: %x>"), elf_class);
        return buff;
    }
}
```

#### 4.3.3提取ELF文件的data数据编码信息

​	数据编码信息有

- none；
- 二进制补码，小端序；
- 二进制补码，大端序；

```cpp
/*获取data的编码信息*/
const char *ELF_process::get_data_encoding (unsigned int encoding)
{
    static char buff[32];
    /*表示数据存储的方式，大端、小端*/
    switch (encoding)
    {
    case ELFDATANONE:
        return ("none");
    case ELFDATA2LSB:
        return ("2's complement, little endian");
    case ELFDATA2MSB:
        return ("2's complement, big endian");
    default:
        snprintf (buff, sizeof (buff), ("<unknown: %x>"), encoding);
        return buff;
    }
}
```

#### 4.3.4提取ELF的版本信息

直接比对进行输出即可

```cpp
 const char *str=elf_header.e_ident[EI_VERSION] == EV_CURRENT ? 
	"(current)" : (elf_header.e_ident[EI_VERSION] != EV_NONE ? "<unknown>" : "");
```

#### 4.3.5提取ELF的操作系统信息以及ABI版本信息

获取操作系统信息

```cpp
const char *ELF_process::get_osabi_name (unsigned int osabi)
{

    static    char buff[32];
    switch (osabi)
    {
    case  ELFOSABI_NONE:
        return "UNIX System V ABI";
    case  ELFOSABI_HPUX:
        return "HP-UX";
    case  ELFOSABI_NETBSD:
        return "NetBSD";
    case  ELFOSABI_GNU:
        return "Object uses GNU ELF extensions";
    case  ELFOSABI_SOLARIS:
        return "Sun Solaris";
    case  ELFOSABI_AIX:
        return "IBM AIX";
    case  ELFOSABI_IRIX:
        return "SGI Irix";
    case  ELFOSABI_FREEBSD:
        return "FreeBSD";
    case  ELFOSABI_TRU64:
        return "Compaq TRU64 UNIX";
    case  ELFOSABI_MODESTO:
        return "Novell Modesto";
    case  ELFOSABI_OPENBSD:
        return "OpenBSD";
    case  ELFOSABI_ARM_AEABI:
        return "ARM EABI";
    case  ELFOSABI_ARM:
        return "ARM";
    case  ELFOSABI_STANDALONE:
        return "Standalone (embedded) application";
    default:
        break;
    }

    snprintf (buff, sizeof (buff), ("<unknown: %x>"), osabi);
    return buff;
}
```

ABI版本信息直接进行输出即可

```cpp
elf_header.e_ident[EI_ABIVERSION]);
```

#### 4.3.6 文件类型e_type

```cpp
/*返回文件类型*/

const char *ELF_process::get_file_type(unsigned e_type)
{

    static char buff[32];

    switch (e_type)
    {
    case ET_NONE:
        return "NONE (None)";
    case ET_REL:
        return "REL (Relocatable file)"; //重定向文件
    case ET_EXEC:
        return "EXEC (Executable file)"; //执行文件
    case ET_DYN:
        return "DYN (Shared object file)"; //共享文件
    case ET_CORE:
        return "CORE (Core file)"; //核心文件

    default: //如果不是如上三种文件，那么返回具体的处理器信息或操作系统信息
        if ((e_type >= ET_LOPROC) && (e_type <= ET_HIPROC))
            snprintf(buff, sizeof(buff), ("Processor Specific: (%x)"), e_type);
        else if ((e_type >= ET_LOOS) && (e_type <= ET_HIOS))
            snprintf(buff, sizeof(buff), ("OS Specific: (%x)"), e_type);
        else
            snprintf(buff, sizeof(buff), ("<unknown>: %x"), e_type);
        return buff;

    }
}
```

#### 4.3.7 机器类型e_machine

分别对机器类型进行编码然后进行判断

```cpp
/*获取机器类型*/
const char *ELF_process::get_machine_name(unsigned e_machine)
{

    static    char buff[64];

    switch (e_machine)
    {
    case EM_NONE:
        return ("None");
    case EM_AARCH64:
        return "AArch64";
    case EM_M32:
        return "WE32100";
    case EM_SPARC:
        return "Sparc";
    case EM_386:
        return "Intel 80386";
    case EM_68K:
        return "MC68000";
    case EM_88K:
        return "MC88000";
    case EM_860:
        return "Intel 80860";
    case EM_MIPS:
        return "MIPS R3000";
    case EM_S370:
        return "IBM System/370";
    case EM_MIPS_RS3_LE:
        return "MIPS R4000 big-endian";
    case EM_PARISC:
        return "HPPA";
    case EM_SPARC32PLUS:
        return "Sparc v8+" ;
    case EM_960:
        return "Intel 90860";
    case EM_PPC:
        return "PowerPC";
    case EM_PPC64:
        return "PowerPC64";
    case EM_FR20:
        return "Fujitsu FR20";
    case EM_RH32:
        return "TRW RH32";
    case EM_ARM:
        return "ARM";
    case EM_SH:
        return "Renesas / SuperH SH";
    case EM_SPARCV9:
        return "Sparc v9";
    case EM_TRICORE:
        return "Siemens Tricore";
    case EM_ARC:
        return "ARC";
    case EM_H8_300:
        return "Renesas H8/300";
    case EM_H8_300H:
        return "Renesas H8/300H";
    case EM_H8S:
        return "Renesas H8S";
    case EM_H8_500:
        return "Renesas H8/500";
    case EM_IA_64:
        return "Intel IA-64";
    case EM_MIPS_X:
        return "Stanford MIPS-X";
    case EM_COLDFIRE:
        return "Motorola Coldfire";
    case EM_ALPHA:
        return "Alpha";
    case EM_D10V:
        return "d10v";
    case EM_D30V:
        return "d30v";
    case EM_M32R:
        return "Renesas M32R (formerly Mitsubishi M32r)";
    case EM_V800:
        return "Renesas V850 (using RH850 ABI)";
    case EM_V850:
        return "Renesas V850";
    case EM_MN10300:
        return "mn10300";
    case EM_MN10200:
        return "mn10200";
    case EM_FR30:
        return "Fujitsu FR30";
    case EM_PJ:
        return "picoJava";
    case EM_MMA:
        return "Fujitsu Multimedia Accelerator";
    case EM_PCP:
        return "Siemens PCP";
    case EM_NCPU:
        return "Sony nCPU embedded RISC processor";
    case EM_NDR1:
        return "Denso NDR1 microprocesspr";
    case EM_STARCORE:
        return "Motorola Star*Core processor";
    case EM_ME16:
        return "Toyota ME16 processor";
    case EM_ST100:
        return "STMicroelectronics ST100 processor";
    case EM_TINYJ:
        return "Advanced Logic Corp. TinyJ embedded processor";
    case EM_PDSP:
        return "Sony DSP processor";
    case EM_FX66:
        return "Siemens FX66 microcontroller";
    case EM_ST9PLUS:
        return "STMicroelectronics ST9+ 8/16 bit microcontroller";
    case EM_ST7:
        return "STMicroelectronics ST7 8-bit microcontroller";
    case EM_68HC16:
        return "Motorola MC68HC16 Microcontroller";
    case EM_68HC12:
        return "Motorola MC68HC12 Microcontroller";
    case EM_68HC11:
        return "Motorola MC68HC11 Microcontroller";
    case EM_68HC08:
        return "Motorola MC68HC08 Microcontroller";
    case EM_68HC05:
        return "Motorola MC68HC05 Microcontroller";
    case EM_SVX:
        return "Silicon Graphics SVx";
    case EM_ST19:
        return "STMicroelectronics ST19 8-bit microcontroller";
    case EM_VAX:
        return "Digital VAX";
    case EM_AVR:
        return "Atmel AVR 8-bit microcontroller";
    case EM_CRIS:
        return "Axis Communications 32-bit embedded processor";
    case EM_JAVELIN:
        return "Infineon Technologies 32-bit embedded cpu";
    case EM_FIREPATH:
        return "Element 14 64-bit DSP processor";
    case EM_ZSP:
        return "LSI Logic's 16-bit DSP processor";
    case EM_MMIX:
        return "Donald Knuth's educational 64-bit processor";
    case EM_HUANY:
        return "Harvard Universitys's machine-independent object format";
    case EM_PRISM:
        return "Vitesse Prism";
    case EM_X86_64:
        return "Advanced Micro Devices X86-64";
    case EM_S390:
        return "IBM S/390";
    case EM_OPENRISC:
    case EM_ARC_A5:
        return "ARC International ARCompact processor";
    case EM_XTENSA:
        return "Tensilica Xtensa Processor";
    case EM_MICROBLAZE:
    case EM_TILEPRO:
        return "Tilera TILEPro multicore architecture family";
    case EM_TILEGX:
        return "Tilera TILE-Gx multicore architecture family";
    default:
        snprintf (buff, sizeof (buff), ("<unknown>: 0x%x"), e_machine);
    }

    return buff;

}
```

#### 4.3.7 文件版本e_version

直接存储即可，不需要进行判断输出

```cpp
(unsigned long) elf_header.e_version)
```

#### 4.3.8 进程开始的虚拟地址e_entry

直接存储即可，不需要进行判断输出

```cpp
elf_header.e_entry
```

#### 4.3.9 程序头部表的开始e_phoff

直接存储即可，不需要进行判断输出

```cpp
elf_header.e_phoff
```

#### 4.3.10 节点头部表的开始e_shoff

直接存储即可，不需要进行判断输出

```cpp
elf_header.e_shoff
```

#### 4.3.11 取决于目标架构e_flags

直接存储即可，不需要进行判断输出

```cpp
(unsigned  long)elf_header.e_flags
```

#### 4.3.12 文件头部的大小e_ehsize

直接存储即可，不需要进行判断输出

```cpp
(long)elf_header.e_ehsize
```

#### 4.3.13程序头部的大小e_phentsize

直接存储即可，不需要进行判断输出

```cpp
(long)elf_header.e_phentsize
```

#### 4.3.13程序头部的条目数e_phnum

直接存储即可，不需要进行判断输出

```cpp
(long)elf_header.e_phnum
```

#### 4.3.14节头部表的大小e_shentsize

直接存储即可，不需要进行判断输出

```cpp
(long) elf_header.e_shentsize
```

#### 4.3.15节头部表的条目数e_shnum

直接存储即可，不需要进行判断输出

```cpp
(long) elf_header.e_shnum
  
//注意需要判断节点表是否存在，否则需要进行输出
if (section_headers != NULL && elf_header.e_shnum == SHN_UNDEF)
        printf (" (%ld)", (long) section_headers[0].sh_size);
```

#### 4.3.16节头部表的条目和其位置的对应关系e_shstrndx

直接存储即可，不需要进行判断输出

```cpp
(long) elf_header.e_shstrndx
```

## 五、测试用例

下面我们尝试编写代码并进行编译：

```cpp
#include <iostream>
using namespace std;
int main(){
    int a = 1;
    cout << "Hello Linux!!HZAU\n";
    return 0;
}
```

编译产生.so文件

```cpp
g++ -o test-h.so test-h.cpp
```

使用`./main test-h.so -h` 查看头文件信息

```
ELF Header:
  Magic:     7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX System V ABI
  ABI Version:                       0
  Type:                              NONE (None)
  Machine:                           None
  Version:                           0x0
  Entry point address:               0x0
  Start of program headers:          0 (bytes into file)
  Start of section headers:          0 (bytes into file)
  Flags:                             0x0
  Size of this header:               0 (bytes)
  Size of program headers:           0 (bytes)
  Number of program headers:         0
  Size of section headers:           0 (bytes)
  Number of section headers:         0
  Section header string table index: 0

There are no sections in this file.
```

使用readelf -h test-h 的输出结果如下：

```
ELF 头：
  Magic：  7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  版本:                              1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              EXEC (可执行文件)
  系统架构:                          Advanced Micro Devices X86-64
  版本:                              0x1
  入口点地址：              0x400670
  程序头起点：              64 (bytes into file)
  Start of section headers:          16672 (bytes into file)
  标志：             0x0
  本头的大小：       64 (字节)
  程序头大小：       56 (字节)
  Number of program headers:         9
  节头大小：         64 (字节)
  节头数量：         30
  字符串表索引节头： 29
```

