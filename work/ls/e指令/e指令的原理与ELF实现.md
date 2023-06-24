# 选项- e的原理和具体实现



## 1选项- e 的介绍

> `-e`选项显示全部头信息，包括文件头信息和程序头信息。

### 1.1 显示ELF的文件头信息

`process_file_header()`

成员函数及其含义如下：

| 成员        | 含义                                                         |
| ----------- | ------------------------------------------------------------ |
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

### 1.2 显示ELF的程序头信息



## 2 选项- e的作用

`-e` 命令可以显示 文件的 ELF 文件头信息（包括文件类型、机器架构、入口地址、程序头表和节头表的偏移和数量等信息）。



## 3 选项- e显示的信息解释

### 3.1 文件头信息

#### 3.1.1 Magic魔数

每种可执行文件的格式的开头几个字节都是很特殊的，特别是开头4个字节，通常被称为**魔数（Magic Number）**。通过对魔数的判断可以确定文件的格式和类型。如：ELF的可执行文件格式的头4个字节为`0x7F`、`e`、`l`、`f`；Java的可执行文件格式的头4个字节为`c`、`a`、`f`、`e`；如果被执行的是Shell脚本或perl、python等解释型语言的脚本，那么它的第一行往往是`#!/bin/sh`或`#!/usr/bin/perl`或`#!/usr/bin/python`，此时前两个字节`#`和`!`就构成了魔数，系统一旦判断到这两个字节，就对后面的字符串进行解析，以确定具体的解释程序路径。

#### 3.1.2 ELF文件类型

ELF文件主要有三种类型，可以通过ELF Header中的`e_type`成员进行区分。

- **可重定位文件（Relocatable File）**：`ETL_REL`。一般为`.o`文件。可以被链接成可执行文件或共享目标文件。静态链接库属于可重定位文件。

- **可执行文件（Executable File）**：`ET_EXEC`。可以直接执行的程序。

- 共享目标文件（Shared Object File）

  ：

  ```
  ET_DYN
  ```

  ，一般为

  ```
  .so
  ```

  文件，有两种情况可以使用。

  - 链接器将其与其他可重定位文件、共享目标文件链接成新的目标文件；
  - 动态链接器将其与其他共享目标文件、结合一个可执行文件，创建进程映像。

#### 3.1.3 机器类型

ELF文件格式被设计成可以在多个平台下使用，但这并不是表示同一个ELF文件可以在不同的平台下使用，而是表示不同平台下的ELF文件都遵循同一套ELF标准。`e_machine`成员就表示该ELF文件的平台属性，

- `EM_M32`：1 「AT&T WE 32100」
- `EM_SPARC`：2 「SPARC」
- `EM_386`: 3 「Intel x86」
- `EM_68K`：4 「Motorola 680000」
- `EM_88K`：5 「Motorola 88000」
- `EM_860`：6 「Intel 80860」

### 3.2文件头信息



## 4 代码实现

### 4.1 算法思路

#### 4.1.1 显示ELF的文件头信息

下面以32为ELF字长为例进行解析

- 数据结构定义「32位」

```c++
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

#### 4.1.2 显示ELF的程序头信息

### 4.2 流程图

#### 4.2.1 显示ELF的文件头信息

![image-20230623233438918](e%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230623233438918.png)

#### 4.2.2 显示ELF的程序头信息



### 4.3 代码详细解释

---

> **显示ELF的文件头信息**

#### 4.3.1 显示文件头信息-总控判断

若此`option`表示为`-e`，则执行`process_file_header(file)`打印文件头信息，若无文件头，则直接退出。

```c++
if(option & (1<<0) || option & (1<<6) ) // -h -e 
    {    
	    if (!process_file_header())
    	{
            return 0;
        }
    }
```

#### 4.3.2 `process_file_header(file)`：获取文件头信息

```c++
int ELF_process::process_file_header(void)
{

    if (   elf_header.e_ident[EI_MAG0] != ELFMAG0
            || elf_header.e_ident[EI_MAG1] != ELFMAG1
            || elf_header.e_ident[EI_MAG2] != ELFMAG2
            || elf_header.e_ident[EI_MAG3] != ELFMAG3)
    {
        printf("Not an ELF file - it has the wrong magic bytes at the start\n");
        return 0;
    }

    printf("ELF Header:\n");
    printf("  Magic:     ");
    for (int i = 0; i <EI_NIDENT ; ++i)
        printf ("%2.2x ", elf_header.e_ident[i]);
    printf("\n");
    printf("  Class:                             %s\n",
           get_elf_class(elf_header.e_ident[EI_CLASS]));

    printf ("  Data:                              %s\n",
            get_data_encoding (elf_header.e_ident[EI_DATA]));
    const char *str=elf_header.e_ident[EI_VERSION] == EV_CURRENT ? 
	"(current)" : (elf_header.e_ident[EI_VERSION] != EV_NONE ? "<unknown>" : "");
    printf ("  Version:                           %d %s\n",
            elf_header.e_ident[EI_VERSION],str);
    printf ("  OS/ABI:                            %s\n",
            get_osabi_name (elf_header.e_ident[EI_OSABI]));

    printf ("  ABI Version:                       %d\n",
            elf_header.e_ident[EI_ABIVERSION]);

    printf ("  Type:                              %s\n",
            get_file_type (elf_header.e_type));

    printf ("  Machine:                           %s\n",
            get_machine_name (elf_header.e_machine));

    printf ("  Version:                           0x%lx\n",
            (unsigned long) elf_header.e_version);

    printf ("  Entry point address:               0x%x",elf_header.e_entry);

    printf ("\n  Start of program headers:          %d",elf_header.e_phoff);

    printf (" (bytes into file)\n  Start of section headers:          %d",elf_header.e_shoff);
    printf (" (bytes into file)\n");

    printf ("  Flags:                             0x%lx\n",(unsigned  long)elf_header.e_flags);

    printf ("  Size of this header:               %ld (bytes)\n",(long)elf_header.e_ehsize);

    printf ("  Size of program headers:           %ld (bytes)\n",(long)elf_header.e_phentsize);

    printf ("  Number of program headers:         %ld\n",(long)elf_header.e_phnum);

    printf ("  Size of section headers:           %ld (bytes)\n",
            (long) elf_header.e_shentsize);

    printf ("  Number of section headers:         %ld\n",
            (long) elf_header.e_shnum);

    if (section_headers != NULL && elf_header.e_shnum == SHN_UNDEF)
        printf (" (%ld)", (long) section_headers[0].sh_size);

    printf ("  Section header string table index: %ld\n",
            (long) elf_header.e_shstrndx);

    return 1;

}
```

##### 4.3.2.1判断magic数是否能正确表示ELF文件

 magic数的第1～第4个字节表示ELF版本号，固定为`7f 45 4c 46`

 直接读取即可，代码如下：

```c++
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



##### 4.3.2.2提取ELF文件的class

 magic数的第5个字节表示class，class一共有3种类型，否则返回<unknown>

实现代码如下：

```c++
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



##### 4.3.2.3提取ELF文件的data数据编码信息

 数据编码信息有

- none；
- 二进制补码，小端序；
- 二进制补码，大端序；

```c++
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



##### 4.3.2.4提取ELF的版本信息

直接比对进行输出即可

```c++
 const char *str=elf_header.e_ident[EI_VERSION] == EV_CURRENT ? 
	"(current)" : (elf_header.e_ident[EI_VERSION] != EV_NONE ? "<unknown>" : "");
```



##### 4.3.2.5提取ELF的操作系统信息以及ABI版本信息

获取操作系统信息

```c++
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

```c++
elf_header.e_ident[EI_ABIVERSION]);
```



##### 4.3.2.6 文件类型e_type

```c++
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



##### 4.3.2.7 机器类型e_machine

分别对机器类型进行编码然后进行判断

```c++
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

##### 4.3.2.8 获取其他头文件信息

```c++
//获取文件头信息的其他部分
//下面的代码为32位信息显示
//只支持32位的信息显示目前
int ELF_process::get_file_header(FILE *file)
{

    /* 读取ELF信息，前16个字节，只有当读取成功了才往后面走  */
    if (fread (elf_header.e_ident, EI_NIDENT, 1, file) != 1)
        return 0;

    /* 目前只是实现了32位环境下的表示  */
    is_32bit_elf = (elf_header.e_ident[EI_CLASS] != ELFCLASS64);

    /* 读取头部信息的其他部分  */
    if (is_32bit_elf)
    {

        Elf32_External_Ehdr ehdr32; //文件头信息的其他部分
        //读取信息除了前16个字节，从后一个字节开始读取
        if (fread (ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1, file) != 1)
            return 0;
        //读取对应部分的字节信息
        elf_header.e_type      = BYTE_GET (ehdr32.e_type);
        elf_header.e_machine   = BYTE_GET (ehdr32.e_machine);
        elf_header.e_version   = BYTE_GET (ehdr32.e_version);
        elf_header.e_entry     = BYTE_GET (ehdr32.e_entry);
        elf_header.e_phoff     = BYTE_GET (ehdr32.e_phoff);
        elf_header.e_shoff     = BYTE_GET (ehdr32.e_shoff);
        elf_header.e_flags     = BYTE_GET (ehdr32.e_flags);
        elf_header.e_ehsize    = BYTE_GET (ehdr32.e_ehsize);
        elf_header.e_phentsize = BYTE_GET (ehdr32.e_phentsize);
        elf_header.e_phnum     = BYTE_GET (ehdr32.e_phnum);
        elf_header.e_shentsize = BYTE_GET (ehdr32.e_shentsize);
        elf_header.e_shnum     = BYTE_GET (ehdr32.e_shnum);
        elf_header.e_shstrndx  = BYTE_GET (ehdr32.e_shstrndx);


        {
            if (is_32bit_elf)
                get_32bit_section_headers(file,1);
            else
            {
                //64λ ...
            }
        }

    }
    return 1;
}
```

- 文件版本e_version

直接存储即可，不需要进行判断输出

```c++
(unsigned long) elf_header.e_version)
```



- 进程开始的虚拟地址e_entry

直接存储即可，不需要进行判断输出

```c++
elf_header.e_entry
```



- 程序头部表的开始e_phoff

直接存储即可，不需要进行判断输出

```c++
elf_header.e_phoff
```



- 节点头部表的开始e_shoff

直接存储即可，不需要进行判断输出

```c++
elf_header.e_shoff
```



- 取决于目标架构e_flags

直接存储即可，不需要进行判断输出

```c++
(unsigned  long)elf_header.e_flags
```



- 文件头部的大小e_ehsize

直接存储即可，不需要进行判断输出

```c++
(long)elf_header.e_ehsize
```



- 程序头部的大小e_phentsize

直接存储即可，不需要进行判断输出

```c++
(long)elf_header.e_phentsize
```



- 程序头部的条目数e_phnum

直接存储即可，不需要进行判断输出

```c++
(long)elf_header.e_phnum
```



- 节头部表的大小e_shentsize

直接存储即可，不需要进行判断输出

```c++
(long) elf_header.e_shentsize
```



- 节头部表的条目数e_shnum

直接存储即可，不需要进行判断输出

```c++
(long) elf_header.e_shnum
  
//注意需要判断节点表是否存在，否则需要进行输出
if (section_headers != NULL && elf_header.e_shnum == SHN_UNDEF)
        printf (" (%ld)", (long) section_headers[0].sh_size);
```



- 节头部表的条目和其位置的对应关系e_shstrndx

直接存储即可，不需要进行判断输出

```
(long) elf_header.e_shstrndx
```

---

> **显示ELF的文件头信息**

#### 4.3.3 显示程序头信息-总控判断

若此`option`表示为`-l`或`e`，则执行`process_program_headers(file)`打印程序头信息。

```c++
if(option & (1<<1) || option & (1<<6))  //-l   -e
    { 
        process_program_headers(file);
    }
```



- 与`process_file_header`时的判断不同，读取文件头为：若无，则直接跳出程序。

- `process_program_headers(file)`则为直接执行。（可以执行）

  ```c++
  if(option & (1<<0) || option & (1<<6) ) // -h -e 
      {    
  	    if (!process_file_header())
      	{
              return 0;
          }
      }
  ```

##### 4.3.3 `process_program_headers(file)`

>  `ELF_process::process_program_headers` 函数的详细功能描述：
>
>  1. 检查 ELF 文件的程序头数量（`e_phnum`）是否为零：
>
>    - 如果程序头数量为零，进一步检查程序头偏移量（`e_phoff`）是否为零：
>      - 如果程序头偏移量不为零，打印出可能损坏的 ELF 头的错误信息。
>      - 如果程序头偏移量为零，说明该文件中没有程序头，打印相应提示信息，并返回 0 表示处理失败。
>
>  2. 如果程序头数量不为零，根据程序头数量的多少打印相应的表头信息。
>
>  3. 根据 ELF 文件的位数（32 位或非 32 位），打印程序头的格式。
>
>  4. > 调用 `get_program_headers` 函数从文件中获取程序头信息。
>
>  5. 使用循环遍历每个程序头，并打印以下信息：
>
>    - > 调用`get_segment_type`打印程序头的类型（`p_type`）
>
>    - 如果是 32 位 ELF 文件，打印以下信息：
>
>      - 偏移量（`p_offset`）
>      - 虚拟地址（`p_vaddr`）
>      - 物理地址（`p_paddr`）
>      - 文件大小（`p_filesz`）
>      - 内存大小（`p_memsz`）
>      - 标志（`p_flags`）的读取（R）、写入（W）、执行（E）权限
>      - 对齐信息（`p_align`）
>
>    - 打印完当前程序头的信息后，换行进行下一个程序头的打印。
>
>  6. 返回 0 表示处理完成。
>
>  该函数的功能是打印出 ELF 文件的程序头信息，包括各个程序头的类型、偏移量、虚拟地址、物理地址、文件大小、内存大小、标志和对齐信息。它还负责处理特殊情况，如程序头数量为零或程序头偏移量非零。

#### 4.3.4 `process_program_headers(FILE *file)`:获取程序头信息。

```c++
// 功能：处理 ELF 文件的程序头。
int ELF_process::process_program_headers(FILE *file)
{

    Elf32_Phdr* segment;
    unsigned long dynamic_addr;
    // 检查程序头数量
    if(elf_header.e_phnum == 0)
    {
				// 程序头开始:0(字节到文件)
        if(elf_header.e_phoff!=0)
        {
            printf ("possibly corrupt ELF header - it has a non-zero program"
                    " header offset, but no program headers");  
          //可能损坏的 ELF 头 - 具有非零程序头偏移量，但没有程序头
        }
        else
        {
            printf ("\nThere are no program headers in this file.\n");  
            //此文件中没有程序头
            return 0;
        }

    }
    else
    {
        if(elf_header.e_phnum>1)
            printf ("\nProgram Headers:\n");    //多个程序头如下：
        else
            printf ("\nProgram Header:\n");     //程序头如下：

        if(is_32bit_elf)
            printf("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n");
            //        类型            偏移量    虚拟地址    物理地址    文件大小  内存大小 标志 对齐
        else
            printf("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n");
            //        类型            偏移量    虚拟地址             物理地址            文件大小  内存大小  标志  对齐

    }

    // 获取程序头信息，并检查是否成功获取，若失败则返回 0
    if (! get_program_headers (file))
        return 0;


    unsigned int i;
    for (i = 0, segment = program_headers;
            i < elf_header.e_phnum;
            i++, segment++)
    {
        printf ("  %-14.14s ", get_segment_type (segment->p_type));
         // 打印段类型

        if(is_32bit_elf)
        {
             // 打印偏移量、虚拟地址、物理地址、文件大小、内存大小
            printf ("0x%6.6x ", (unsigned int) segment->p_offset);
            printf ("0x%8.8x ", (unsigned int) segment->p_vaddr);
            printf ("0x%8.8x ", (unsigned int) segment->p_paddr);
            printf ("0x%5.5x ", (unsigned int) segment->p_filesz);
            printf ("0x%5.5x ", (unsigned int) segment->p_memsz);

            // 打印标志信息，R 表示可读，W 表示可写，E 表示可执行
            printf ("%c%c%c ",
                    (segment->p_flags & PF_R ? 'R' : ' '),
                    (segment->p_flags & PF_W ? 'W' : ' '),
                    (segment->p_flags & PF_X ? 'E' : ' '));
            printf ("%#x", (unsigned int) segment->p_align);
        }

        printf("\n");   //打印换行

    }

    return 0;
}
```

------

##### 4.3.4.1 `get_program_headers()`:获取程序头信息。

> `get_program_headers()`函数的详细功能描述：
>
> 1. 声明了两个指针变量`phdrs`和`phdrs64`，用于指向32位和64位ELF程序头的数组。
> 2. 检查是否已经存在先前读取的程序头缓存，如果存在则返回1，表示已成功获取程序头。
> 3. 使用`cmalloc`函数为32位ELF程序头数组分配内存，并分配相应数量的`Elf32_Phdr`结构体空间。
> 4. 使用`cmalloc`函数为64位ELF程序头数组分配内存，并分配相应数量的`Elf64_Phdr`结构体空间。
> 5. 如果32位ELF程序头数组分配内存失败，输出"内存不足"的错误提示，返回0表示未成功获取程序头。
> 6. 根据ELF文件的位数，分别调用`get_32bit_program_headers`或`get_64bit_program_headers`函数来获取程序头的内容。
> 7. 如果获取程序头成功，将程序头数组指针赋值给`program_headers`变量，并返回1表示成功获取程序头。
> 8. 如果获取程序头失败，释放之前分配的32位和64位程序头数组的内存，并返回0表示未成功获取程序头。

```c++
int ELF_process::get_program_headers(FILE *file)
{

    Elf32_Phdr* phdrs;      // 32位ELF程序头数组指针
    Elf64_Phdr* phdrs64;    // 64位ELF程序头数组指针

    /* 检查之前读取的缓存 */
    if (program_headers != NULL)
        return 1;

    // 为32位ELF程序头数组分配内存
    phdrs = (Elf32_Phdr *) cmalloc (elf_header.e_phnum,
                                    sizeof (Elf32_Phdr));
    
    // 为64位ELF程序头数组分配内存
    phdrs64 = (Elf64_Phdr *) cmalloc (elf_header.e_phnum,
                                    sizeof (Elf64_Phdr));
    if (phdrs == NULL)
    {
        printf("Out of memory\n");
        return 0;
    }

    if (is_32bit_elf
            // 获取32位ELF程序头信息
            ? get_32bit_program_headers (file, phdrs)
            // 获取64位ELF程序头信息
            : get_64bit_program_headers (file, phdrs64))
    {
         // 将程序头数组指针指向获取到的32位ELF程序头数组
        program_headers = phdrs;
        return 1;
    }

    free (phdrs);        // 释放32位ELF程序头数组内存
    free (phdrs64);     // 释放64位ELF程序头数组内存
    return 0;



}
```

###### `get_32bit_program_headers()`

> `get_32bit_program_headers()`函数的详细功能描述：
>
> 1. 通过调用`get_data`函数从文件中获取32位外部程序头信息，并将结果存储在`phdrs`指针中。
> 2. 如果获取外部程序头信息失败（`phdrs`为NULL），则返回0，表示失败。
> 3. 使用循环将外部程序头信息复制到内部程序头结构中。
> 4. 在每次迭代中，将外部程序头的各个字段（p_type、p_offset等）从大端字节序转换为系统字节序，并将其赋值给相应的内部程序头字段。
> 5. 循环完成后，释放外部程序头信息的内存。
> 6. 返回1，表示成功获取并复制程序头信息。

```c++
int ELF_process::get_32bit_program_headers(FILE *file, Elf32_Phdr *pheaders)
{

    Elf32_External_Phdr* phdrs;     // 32位外部程序头指针
    Elf32_External_Phdr* external;  // 外部程序头指针，遍历外部程序头数组
    Elf32_Phdr* internal;           // 内部程序头指针，遍历内部程序头数组

    unsigned int i;

    // 获取32位外部程序头信息
    phdrs = (Elf32_External_Phdr *) get_data (NULL, file, elf_header.e_phoff,
            elf_header.e_phentsize,
            elf_header.e_phnum,
            ("program headers"));

    if (!phdrs)
        return 0;
    
    // 将外部程序头信息复制到内部程序头结构中
    for (i = 0, internal = pheaders, external = phdrs;
            i < elf_header.e_phnum;
            i++, internal++, external++)
    {

        internal->p_type   = BYTE_GET (external->p_type);       //程序头
        internal->p_offset = BYTE_GET (external->p_offset);     //偏移量
        internal->p_vaddr  = BYTE_GET (external->p_vaddr);      //虚拟地址
        internal->p_paddr  = BYTE_GET (external->p_paddr);      //物理地址
        internal->p_filesz = BYTE_GET (external->p_filesz);     //文件大小
        internal->p_memsz  = BYTE_GET (external->p_memsz);      //内存
        internal->p_flags  = BYTE_GET (external->p_flags);      //标志
        internal->p_align  = BYTE_GET (external->p_align);      //对齐
    }

    // 释放外部程序头信息的内存
    free (phdrs);

    return 1;
}
```

#### 4.3.4.2 `get_segment_type()`



```c++
const char *ELF_process::get_segment_type(unsigned int p_type)
{
    static char buff[32]; // 静态字符数组用于存储结果

  	// 根据给定的p_type值进行判断
    switch (p_type) 
    {
    case PT_NULL: // 如果p_type等于PT_NULL
        return "NULL"; // 返回字符串"NULL"
    case PT_LOAD: // 如果p_type等于PT_LOAD
        return "LOAD"; // 返回字符串"LOAD"
    case PT_DYNAMIC: // 如果p_type等于PT_DYNAMIC
        return "DYNAMIC"; // 返回字符串"DYNAMIC"
    case PT_INTERP: // 如果p_type等于PT_INTERP
        return "INTERP"; // 返回字符串"INTERP"
    case PT_NOTE: // 如果p_type等于PT_NOTE
        return "NOTE"; // 返回字符串"NOTE"
    case PT_SHLIB: // 如果p_type等于PT_SHLIB
        return "SHLIB"; // 返回字符串"SHLIB"
    case PT_PHDR: // 如果p_type等于PT_PHDR
        return "PHDR"; // 返回字符串"PHDR"
    case PT_TLS: // 如果p_type等于PT_TLS
        return "TLS"; // 返回字符串"TLS"
    case PT_GNU_EH_FRAME: // 如果p_type等于PT_GNU_EH_FRAME
        return "GNU_EH_FRAME"; // 返回字符串"GNU_EH_FRAME"
    case PT_GNU_STACK: // 如果p_type等于PT_GNU_STACK
        return "GNU_STACK"; // 返回字符串"GNU_STACK"
    case PT_GNU_RELRO: // 如果p_type等于PT_GNU_RELRO
        return "GNU_RELRO"; // 返回字符串"GNU_RELRO"

    default: // 如果p_type不匹配以上任何值
        if ((p_type >= PT_LOPROC) && (p_type <= PT_HIPROC)) // 如果p_type的值在PT_LOPROC和PT_HIPROC之间
        {
            const char * result; // 声明一个指向字符的指针result

            switch (elf_header.e_machine) // 根据elf_header.e_machine的值进行判断
            {
            case EM_AARCH64: // 如果elf_header.e_machine等于EM_AARCH64
                result = get_aarch64_segment_type (p_type); // 调用get_aarch64_segment_type函数，将返回值赋给result
                break;
            case EM_ARM: // 如果elf_header.e_machine等于EM_ARM
                result = get_arm_segment_type (p_type); // 调用get_arm_segment_type函数，将返回值赋给result
                break;
            case EM_MIPS: // 如果elf_header.e_machine等于EM_MIPS
            case EM_MIPS_RS3_LE: // 或者elf_header.e_machine等于EM_MIPS_RS3_LE
                result = get_mips_segment_type (p_type); // 调用get_mips_segment_type函数，将返回值赋给result
                break;
            case EM_PARISC: // 如果elf_header.e_machine等于EM_PARISC
                result = get_parisc_segment_type (p_type); // 调用get_parisc_segment_type函数，将返回值赋给result
                break;
            case EM_IA_64: // 如果elf_header.e_machine等于EM_IA_64
                result = get_ia64_segment_type (p_type); // 调用get_ia64_segment_type函数，将返回值赋给result
                break;
            case EM_TI_C6000: // 如果elf_header.e_machine等于EM_TI_C6000
                result = get_tic6x_segment_type (p_type); // 调用get_tic6x_segment_type函数，将返回值赋给result
                break;
            default:
                result = NULL; // 如果不匹配以上任何值，将result设为NULL
                break;
            }

            if (result != NULL) // 如果result不为NULL
                return result; // 返回result

            sprintf (buff, "LOPROC+%x", p_type - PT_LOPROC); // 将格式化的字符串写入buff，格式为"LOPROC+十六进制的(p_type - PT_LOPROC)"

        }
        else if ((p_type >= PT_LOOS) && (p_type <= PT_HIOS)) // 如果p_type的值在PT_LOOS和PT_HIOS之间
        {
            const char * result; // 声明一个指向字符的指针result

            switch (elf_header.e_machine) // 根据elf_header.e_machine的值进行判断
            {
            case EM_PARISC: // 如果elf_header.e_machine等于EM_PARISC
                result = get_parisc_segment_type (p_type); // 调用get_parisc_segment_type函数，将返回值赋给result
                break;
            case EM_IA_64: // 如果elf_header.e_machine等于EM_IA_64
                result = get_ia64_segment_type (p_type); // 调用get_ia64_segment_type函数，将返回值赋给result
                break;
            default:
                result = NULL; // 如果不匹配以上任何值，将result设为NULL
                break;
            }

            if (result != NULL) // 如果result不为NULL
                return result; // 返回result

            sprintf (buff, "LOOS+%x", p_type - PT_LOOS); // 将格式化的字符串写入buff，格式为"LOOS+十六进制的(p_type - PT_LOOS)"
        }
        else
            snprintf (buff, sizeof (buff), ("<unknown>: %x"), p_type); // 将格式化的字符串写入buff，格式为"<unknown>: 十六进制的p_type"

        return buff; // 返回buff
    }
}

```





## 测试样例

