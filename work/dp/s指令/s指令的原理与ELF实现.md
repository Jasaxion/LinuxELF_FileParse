# 选项-s 的原理和具体实现

## 1.选项-s 的介绍

### 1.1.重定位

重定位操作是连接符号引用（symbolic references）和符号定义（symbolic definitions）的过程。例如，程序中调用一个（外部）函数，代码中我们只需要指定函数名（符号引用）即可，但是当程序实际运行的时候，相关的CALL指令必须能够正确无误地跳转到函数实际地址处（符号定义）去执行函数代码。可是在链接阶段之前，符号的虚拟地址（亦可称运行时地址）并没有分配，只有在链接阶段的符号解析过程中链接器才会为符号分配虚拟地址。在符号地址确认后，链接器这才会修改机器指令（即重定位操作是在符号解析之后），可是链接器并不会聪明到可以自动找到可重定位文件中引用外部符号的地方（即需要修改的地方），所以可重定位文件必须提供相应的信息来帮助链接器，换句话说，可重定位文件中必须包含相关的信息来告诉链接器如何去修改节的内容，只有这样，最后生成的可执行文件或者共享库才会包含正确的信息来构建最终的进程映像。可重定位项就是帮助链接器进行重定位操作的信息。

重定位就是把符号引用与符号定义链接起来的过程。

当程序中调用一个函数时，相关的 call 指令必须在执行期将控制流转到正确的目标地址。所以，so 文件中必须包含一些重定位相关的信息，linker 据此完成重定位的工作。

例如两种基本的重定位类型R_386_32和R386_PC32：

| 宏定义     | 值 | 重定位修正方法    |
| ---------- | -- | ----------------- |
| R_386_32   | 1  | 绝对寻址修正S+A   |
| R_386_PC32 | 2  | 相对寻址修正S+A-P |

* A=保存正在修正位置的值；
* P=被修正的位置（相对于节区开始的偏移量或虚拟地址）；
* S=符号的实际地址，即由r_info的高24位指定的符号的实际地址。

### 1.2.链接时重定位

在.o文件链接时将发生重定位
这些重定位信息保存在一系列的重定位项中（.rel.dyn等表），重定位项的结构如下：

```c
typedef struct
{
    Elf32_Addr  r_offset;       /* Address */
    Elf32_Word  r_info;         /* Relocation type and symbol index */
} Elf32_Rel;

typedef struct
{
    Elf32_Addr  r_offset;       /* Address */
    Elf32_Word  r_info;         /* Relocation type and symbol index */
    Elf32_Sword r_addend;       /* Addend */
} Elf32_Rela;
```

#### 1.2.1.r_offset

本数据成员给出重定位所作用的位置。对于重定位文件来说,此值是受重定位作用的存储单元在节中的字节偏移量;

对于可执行文件或共享ELF文件来说,此值是受重定位作用的存储单元的虚拟地址。

#### 1.2.2.r_info

本数据成员既给出了重定位所作用的符号表索引,也给出了重定位的类型。以下是应用于 r_info 的宏定义。

```c
#define ELF32_R_SYM(val)        ((val) >> 8)  //得到符号表的索引
#define ELF32_R_TYPE(val)       ((val) & 0xff)  //得到type
#define ELF32_R_INFO(sym, type)     (((sym) << 8) + ((type) & 0xff))
```

#### 1.2.3.r_addend

本成员指定了一个加数,这个加数用于计算需要重定位的域的值。

Elf32_Rela 与 Elf32_Rel 在结构上只有一处不同,就是前者有 r_addend。Elf32_Rela 中是用r_addend 显式地指出加数;而对 Elf32_Rel来说,加数是隐含在被修改的位置里的。Elf32_Rel中加数的形式这里并不定义,它可以依处理器架构(ELF32_R_TYPE(info))的不同而自行决定。

#### 1.2.4.计算方式

计算方式是根据ELF32_R_TYPE宏定义得到类型决定的，例如i386架构的计算方式：

被重定位域(relocatable field)是一个 32 位的域，占 4 字节并且地址向4字节对齐，其字节序与所在体系结构下其他双字长数据的字节序相同。重定位项用于描述如何修改如下的指令和数据域:

![img](https://upload-images.jianshu.io/upload_images/23598658-f42d288cfb97ceeb.png?imageMogr2/auto-orient/strip|imageView2/2/w/438/format/webp)

为了下面的描述方便,这里定义以下几种运算符号:

* A 表示用于计算重定位域值的加数。
* B 表示在程序运行期,共享ELF被装入内存时的基地址。一般来说,共享ELF文件在构建时基地址为 0,但在运行时则不是。
* G 表示可重定位项在全局偏移量表中的位置,这里存储了此重定位项在运行期间的地址。更多信息参见下文“全局偏移量表”。
* GOT 表示全局偏移量表的地址。
* L 表示一个符号的函数连接表项的所在之处,可能是节内偏移量,或者是内存地址。函数连接表项把函数调用定位到合适的位置。在构建期间,连接编辑器创建初始的函数连接表;在运行期间,动态连接器会修改表项。更多信息参见“函数连接表”部分。
* P 表示被重定位的存储单元在节内的偏移量或者内存地址,由 r_offset 计算得到。
* S 表示重定位项中某个索引值所代表的符号的值。

重定位类型指定了哪些位需要被修改以及如何算计它们的值，下面使用x86系统处理器的重定位类型的计算方法说明。

| 名字           | 值 | 数据类型 | 计算    |
| -------------- | -- | -------- | ------- |
| R_386_GOT32    | 3  | word32   | G+A     |
| R_386_PLT32    | 4  | word32   | L+A-P   |
| R_386_COPY     | 5  | none     | none    |
| R_386_GLOB_DAT | 6  | word32   | S       |
| R_386_JMP_SLOT | 7  | word32   | S       |
| R_386_RELATIVE | 8  | word32   | B+A     |
| R_386_GOTOFF   | 9  | word32   | S+A-GOT |
| R_386_GOTPC    | 10 | word32   | GOT+A-P |

### 1.3.符号表symtab和动态符号表dynsym

#### 1.3.1.符号介绍

符号表有.symtab和.dynsym这两个节区。动态符号表(.dynsym)用来保存与动态链接相关的导入导出符号，不包括模块内部的符号。而.symtab则保存所有符号，包括.dynsym中的符号。

我们将函数和全局变量（static局部变量也算）统称为符号（Symbol），函数名和变量名就是符号名（Symbol Name），我们可以将符号看做是链接中的粘合剂，整个链接过程正是基于符号才能够正确完成。每个目标文件都会有一个符号表（Symbol Table），即上图的.symtab段，这个表里记录了目标文件所用到的所有符号。每个定义的符号有一个对应的值，叫做符号值（Symbol Value），对于变量和函数来说，符号值就是它们的地址。

在本目标文件中引用的全局符号，却没有定义在本目标文件中，一般叫做外部符号（External Symbol）。**局部变量临时分配在栈中，不会在过程外被引用，所以不会产生符号。**

实际上段名（比如.text），汇编语言里的标签（比如_start、_edata、_end），源文件名也是符号。

#### 1.3.2.数据结构

符号表的本质是一个符号表项数组，每个符号表项都是一个Elf32_Sym结构体，如下所示：

```c
typedef struct
{
  Elf32_Word	st_name;		/* Symbol name (string tbl index) */
  Elf32_Addr	st_value;		/* Symbol value */
  Elf32_Word	st_size;		/* Symbol size */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char	st_other;		/* Symbol visibility */
  Elf32_Section	st_shndx;		/* Section index */
} Elf32_Sym;
```

1. st_name：符号名索引。在读取的时候，要从strtab表中，根据st_name索引，找到对应的符号名。
2. st_value：符号值。这个值与符号相关，可能是一个绝对值，也可能是一个地址等，不同的符号，它所对应的含义不同。
   每一个符号都有一个对应的值，如果这个符号是一个函数或变量的定义，那么符号的值就是这个函数或变量的地址。有如下的规则：

   - 在目标文件中，如果是符号的定义并且符号不是COMMON块类型，那么st_value表示该符号在节区中的偏移。即符号所对应的函数或变量位于st_shndx索引的节区，偏移st_value的位置。
   - 在目标文件中，如果符号是COMMON块类型的，则st_value表示对齐属性。这段话的意思是例如有多个未初始化的全局变量在COMMON块内的话，会选取空间最大的那个弱符号，因此这个st_value表示对齐属性。
   - 在可执行文件中，st_value表示符号的虚拟地址。
3. st_size：符号大小。对于包含数据的符号，这个值是该数据类型的大小。比如一个double 型的符号它占用8个字节。如果该值为0，则表示该符号大小为0或未知。
4. st_info：符号类型和绑定信息。该成员低4位表示符号的类型(SymbolType)，高28 位表示符号绑定信息(SymbolBinding)。
   符号类型：

   | 宏定义名    | 值 | 说明                                                                                                         |
   | ----------- | -- | ------------------------------------------------------------------------------------------------------------ |
   | STT_NOTYPE  | 0  | 未知类型符号                                                                                                 |
   | STT_OBEJECT | 1  | 该符号是个数据对象，例如变量、数组等                                                                         |
   | STT_FUNC    | 2  | 该符号是个函数或者其它可执行代码                                                                             |
   | STT_SECTION | 3  | 该符号表示一个段，这种符号必须是STB_LOCAL的                                                                  |
   | STT_FILE    | 4  | 该符号表示文件名，一般都是该目标文件所对应的源文件名，他一定是STB_LOCAL类型的，并且它的st_shndx一定是SNH_ABS |

   符号绑定信息：

   | 宏定义名   | 值 | 说明                               |
   | ---------- | -- | ---------------------------------- |
   | STB_LOCAL  | 0  | 局部符号，对于目标文件的外部不可见 |
   | STB_GLOBAL | 1  | 全局符号，外部可见                 |
   | STB_WEAK   | 2  | 弱引用                             |
5. sh_other：改成员目前为空，暂时没用。
6. sh_shndx：符号所在的节区索引。如果符号定义在本目标文件中，那么这个成员表示符号所在的节区的下标；但是如果符号不是定义在本目标文件中，或者对于有些特殊的符号，sh_shndx的值有些特殊。如下表符号所在节区特殊常量：

   | 宏定义名   | 值     | 说明                                                                                 |
   | ---------- | ------ | ------------------------------------------------------------------------------------ |
   | SHN_ABS    | 0xfff1 | 表示该符号包含了一个绝对的值。比如表示文件名的符号就是属于这种类型的                 |
   | SHN_COMMON | 0xfff2 | 表示该符号是一个COMMON块类型的符号，一般来说，为初始化的全局变量的定义就是这种类型的 |
   | SHN_UNDEF  | 0      | 表示该符号未定义、这个符号表示该符号在本目标文件被引用到，但是定义在其他目标文件中   |

## 2.选项-s 的作用

```
readelf -s
        --symbols
	--syms
```

显示符号表.sym节区的详细信息，可以直接查看符号表的内容。

我们在这里实现显示输出的是.dynsym动态符号表，为了后面的-d动态链接做准备。

## 3.选项-s 显示的信息解释

```shell
Symbol table '.dynsym' contains 7 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __cxa_finalize
     2: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
     3: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
     4: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     5: 0000114d    20 FUNC    GLOBAL DEFAULT   10 testfun
     6: 00004010     4 OBJECT  GLOBAL DEFAULT   19 libGLobal

Symbol table '.symtab' contains 51 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000174     0 SECTION LOCAL  DEFAULT    1 
     2: 00000198     0 SECTION LOCAL  DEFAULT    2 
     3: 000001b4     0 SECTION LOCAL  DEFAULT    3 
     4: 000001d8     0 SECTION LOCAL  DEFAULT    4 
     5: 00000248     0 SECTION LOCAL  DEFAULT    5 
     6: 000002b0     0 SECTION LOCAL  DEFAULT    6 
     7: 00001000     0 SECTION LOCAL  DEFAULT    7 
     8: 00001030     0 SECTION LOCAL  DEFAULT    8 
     9: 00001040     0 SECTION LOCAL  DEFAULT    9 
    10: 00001050     0 SECTION LOCAL  DEFAULT   10 
    11: 0000117c     0 SECTION LOCAL  DEFAULT   11 
    12: 00002000     0 SECTION LOCAL  DEFAULT   12 
    13: 00002034     0 SECTION LOCAL  DEFAULT   13 
    14: 00003f40     0 SECTION LOCAL  DEFAULT   14 
    15: 00003f44     0 SECTION LOCAL  DEFAULT   15 
    16: 00003f48     0 SECTION LOCAL  DEFAULT   16 
    17: 00003ff0     0 SECTION LOCAL  DEFAULT   17 
    18: 00004000     0 SECTION LOCAL  DEFAULT   18 
    19: 0000400c     0 SECTION LOCAL  DEFAULT   19 
    20: 00004014     0 SECTION LOCAL  DEFAULT   20 
    21: 00000000     0 SECTION LOCAL  DEFAULT   21 
    22: 00000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    23: 00001060     0 FUNC    LOCAL  DEFAULT   10 deregister_tm_clones
    24: 000010a0     0 FUNC    LOCAL  DEFAULT   10 register_tm_clones
    25: 000010f0     0 FUNC    LOCAL  DEFAULT   10 __do_global_dtors_aux
    26: 00004014     1 OBJECT  LOCAL  DEFAULT   20 completed.7623
    27: 00003f44     0 OBJECT  LOCAL  DEFAULT   15 __do_global_dtors_aux_fin
    28: 00001140     0 FUNC    LOCAL  DEFAULT   10 frame_dummy
    29: 00003f40     0 OBJECT  LOCAL  DEFAULT   14 __frame_dummy_init_array_
    30: 00000000     0 FILE    LOCAL  DEFAULT  ABS B.c
    31: 00000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    32: 000020d8     0 OBJECT  LOCAL  DEFAULT   13 __FRAME_END__
    33: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
    34: 00003f48     0 OBJECT  LOCAL  DEFAULT   16 _DYNAMIC
    35: 00004014     0 OBJECT  LOCAL  DEFAULT   19 __TMC_END__
    36: 00001175     0 FUNC    LOCAL  DEFAULT   10 __x86.get_pc_thunk.ax
    37: 0000400c     0 OBJECT  LOCAL  DEFAULT   19 __dso_handle
    38: 00001149     0 FUNC    LOCAL  DEFAULT   10 __x86.get_pc_thunk.dx
    39: 00001000     0 FUNC    LOCAL  DEFAULT    7 _init
    40: 00001050     4 FUNC    LOCAL  DEFAULT   10 __x86.get_pc_thunk.bx
    41: 00002000     0 NOTYPE  LOCAL  DEFAULT   12 __GNU_EH_FRAME_HDR
    42: 0000117c     0 FUNC    LOCAL  DEFAULT   11 _fini
    43: 00004000     0 OBJECT  LOCAL  DEFAULT   18 _GLOBAL_OFFSET_TABLE_
    44: 00001161    20 FUNC    LOCAL  DEFAULT   10 testfun2
    45: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __cxa_finalize
    46: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    47: 00004010     4 OBJECT  GLOBAL DEFAULT   19 libGLobal
    48: 0000114d    20 FUNC    GLOBAL DEFAULT   10 testfun
    49: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
    50: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__

```

列名的含义如下：

| 列名 | 含义                                 | 变量                 | 大小       |
| ---- | ------------------------------------ | -------------------- | ---------- |
| Name | 节区名                               | 特殊，从shstrtab读取 | max20B     |
| Type | 节区类型                             | sh_type              | Elf32_Word |
| Addr | 节区在被执行时的虚拟地址             | sh_addr              | Elf32_Addr |
| Off  | 节区在文件中的偏移地址               | sh_offset            | Elf32_Off  |
| Size | 节区的大小                           | sh_size              | Elf32_Word |
| ES   | (如果节区含有表)节区每一个条目的大小 | sh_entsize           | Elf32_Word |
| Flg  | 节区标志位                           | sh_flags             | Elf32_Word |
| Lk   | 包含的条目的符号表节区的节头索引     | sh_link              | Elf32_Word |
| Inf  | 包含的条目的符号表索引               | sh_info              | Elf32_Word |
| Al   | 节区对齐                             | sh_addralign         | Elf32_Word |

接下来，我们来进行详细分析：

1. 对于开头的一段话 `There are 23 section headers, starting at offset 0x748:`表明了节区头表有23个节区头项，且节区头表起始于0x748位置。使用原生工具readelf -h进行验证，可以得到ELF头数据如下：

   ```shell
   dp@ubuntu:~/Desktop/elf/7.9/testg$ readelf -h ./testg-template.o
   ELF Header:
   Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
   Class:                             ELF32
   Data:                              2's complement, little endian
   Version:                           1 (current)
   OS/ABI:                            UNIX - System V
   ABI Version:                       0
   Type:                              REL (Relocatable file)
   Machine:                           Intel 80386
   Version:                           0x1
   Entry point address:               0x0
   Start of program headers:          0 (bytes into file)
   Start of section headers:          1864 (bytes into file)
   Flags:                             0x0
   Size of this header:               52 (bytes)
   Size of program headers:           0 (bytes)
   Number of program headers:         0
   Size of section headers:           40 (bytes)
   Number of section headers:         23
   Section header string table index: 22
   ```

   验证发现，节区头部表的位置在文件第1864字节处开始，转换为16进制，得到偏移地址为0x748，这与我们的工具是一致的。另外，ELF头表中表明了节区头表共有23项，这也与我们的输出是一致的。
2. Name：表明节区名，例如.text、.group都是节区名。节区名是通过sh_name属性的索引查shstrtab表得到的字符串。通过节区的sh_name属性得到节区名字在shstrtab表中的索引，然后加上shstrtab表的偏移地址，得到最终名字字符串的起始地址，直接读取20字字符，因为里面存的每一个字符串都是以'\\0'结尾的，因此直接读取输出即可。
3. Addr：全0且无意义，因为我们查看的是目标文件，还会对这些地址进行重定位的。
4. Off：.symtab符号表节区在文件中的偏移地址为0x000274，表示从这个位置开始是本节区的数据。
5. Size：.symtab符号表节区的大小为0x000200即512个字节。
6. ES：由于.symtab符号表节区是一个数组，每一项表示一个符号。标识数组中每个条目的大小为0x10字节(16字节)，即该节区含有512B/16B=32个条目。使用原生readelf工具进行验证，如下图所示：

```shell
dp@ubuntu:~/Desktop/elf/7.9/testg$ readelf -s testg-template.o

Symbol table '.symtab' contains 32 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 FILE    LOCAL  DEFAULT  ABS testg-template.cpp
     2: 00000000     0 SECTION LOCAL  DEFAULT    4 
     3: 00000000     0 SECTION LOCAL  DEFAULT    6 
     4: 00000000     0 SECTION LOCAL  DEFAULT    7 
     5: 00000000     0 SECTION LOCAL  DEFAULT    8 
     6: 00000000     1 OBJECT  LOCAL  DEFAULT    8 _ZStL19piecewise_construc
     7: 00000000     1 OBJECT  LOCAL  DEFAULT    7 _ZStL8__ioinit
     8: 00000000     0 SECTION LOCAL  DEFAULT    9 
     9: 00000057    93 FUNC    LOCAL  DEFAULT    4 _Z41__static_initializati
    10: 000000b4    40 FUNC    LOCAL  DEFAULT    4 _GLOBAL__sub_I_main
    11: 00000000     0 SECTION LOCAL  DEFAULT   11 
    12: 00000000     0 SECTION LOCAL  DEFAULT   13 
    13: 00000000     0 SECTION LOCAL  DEFAULT   14 
    14: 00000000     0 SECTION LOCAL  DEFAULT   16 
    15: 00000000     0 SECTION LOCAL  DEFAULT   17 
    16: 00000000     0 SECTION LOCAL  DEFAULT   18 
    17: 00000000     0 SECTION LOCAL  DEFAULT   15 
    18: 00000000     0 SECTION LOCAL  DEFAULT    1 
    19: 00000000     0 SECTION LOCAL  DEFAULT    2 
    20: 00000000     0 SECTION LOCAL  DEFAULT    3 
    21: 00000000    87 FUNC    GLOBAL DEFAULT    4 main
    22: 00000000     0 FUNC    GLOBAL HIDDEN    14 __x86.get_pc_thunk.bx
    23: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _GLOBAL_OFFSET_TABLE_
    24: 00000000    27 FUNC    WEAK   DEFAULT    9 _Z3addIiET_S0_S0_
    25: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZSt4cout
    26: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZNSolsEi
    27: 00000000     0 FUNC    GLOBAL HIDDEN    13 __x86.get_pc_thunk.ax
    28: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZNSt8ios_base4InitC1Ev
    29: 00000000     0 NOTYPE  GLOBAL HIDDEN   UND __dso_handle
    30: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZNSt8ios_base4InitD1Ev
    31: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND __cxa_atexit
```

恰好有32个符号，这与我们的工具是一致的。

* Flg：标志位空。
* Lk：由于我们的节区.symtab的类型为SHT_SYMTAB，所以sh_link表示的是关联字符串表的节区索引。在字符串表中的索引为21。使用原生readelf工具进行验证，如下图节区表所示：

  ```shell
  dp@ubuntu:~/Desktop/elf/7.9/testg$ readelf -S testg-template.o
  There are 23 section headers, starting at offset 0x748:

  Section Headers:
    [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
    [ 0]                   NULL            00000000 000000 000000 00      0   0  0
    [ 1] .group            GROUP           00000000 000034 00000c 04     20  24  4
    [ 2] .group            GROUP           00000000 000040 000008 04     20  27  4
    [ 3] .group            GROUP           00000000 000048 000008 04     20  22  4
    [ 4] .text             PROGBITS        00000000 000050 0000dc 00  AX  0   0  1
    [ 5] .rel.text         REL             00000000 0005a8 000078 08   I 20   4  4
    [ 6] .data             PROGBITS        00000000 00012c 000000 00  WA  0   0  1
    [ 7] .bss              NOBITS          00000000 00012c 000001 00  WA  0   0  1
    [ 8] .rodata           PROGBITS        00000000 00012c 000001 00   A  0   0  1
    [ 9] .text._Z3addIiET_ PROGBITS        00000000 00012d 00001b 00 AXG  0   0  1
    [10] .rel.text._Z3addI REL             00000000 000620 000010 08  IG 20   9  4
    [11] .init_array       INIT_ARRAY      00000000 000148 000004 04  WA  0   0  4
    [12] .rel.init_array   REL             00000000 000630 000008 08   I 20  11  4
    [13] .text.__x86.get_p PROGBITS        00000000 00014c 000004 00 AXG  0   0  1
    [14] .text.__x86.get_p PROGBITS        00000000 000150 000004 00 AXG  0   0  1
    [15] .comment          PROGBITS        00000000 000154 00002c 01  MS  0   0  1
    [16] .note.GNU-stack   PROGBITS        00000000 000180 000000 00      0   0  1
    [17] .note.gnu.propert NOTE            00000000 000180 00001c 00   A  0   0  4
    [18] .eh_frame         PROGBITS        00000000 00019c 0000d8 00   A  0   0  4
    [19] .rel.eh_frame     REL             00000000 000638 000030 08   I 20  18  4
    [20] .symtab           SYMTAB          00000000 000274 000200 10     21  21  4
    [21] .strtab           STRTAB          00000000 000474 000134 00      0   0  1
    [22] .shstrtab         STRTAB          00000000 000668 0000dd 00      0   0  1
  Key to Flags:
    W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
    L (link order), O (extra OS processing required), G (group), T (TLS),
    C (compressed), x (unknown), o (OS specific), E (exclude),
    p (processor specific)

  ```

  索引为21的节区正好是字符串表strtab所在的节区，用于给symtab提供符号名。
* Inf：由于我们的节区.symtab的类型为SHT_SYMTAB，所以sh_info表示的是比最后一个局部符号的索引大1。标识元素的符号表索引为21。使用原生readelf工具打印其符号表，如下图所示：

  ```shell
  dp@ubuntu:~/Desktop/elf/7.9/testg$ readelf -s testg-template.o

  Symbol table '.symtab' contains 32 entries:
     Num:    Value  Size Type    Bind   Vis      Ndx Name
       0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
       1: 00000000     0 FILE    LOCAL  DEFAULT  ABS testg-template.cpp
       2: 00000000     0 SECTION LOCAL  DEFAULT    4 
       3: 00000000     0 SECTION LOCAL  DEFAULT    6 
       4: 00000000     0 SECTION LOCAL  DEFAULT    7 
       5: 00000000     0 SECTION LOCAL  DEFAULT    8 
       6: 00000000     1 OBJECT  LOCAL  DEFAULT    8 _ZStL19piecewise_construc
       7: 00000000     1 OBJECT  LOCAL  DEFAULT    7 _ZStL8__ioinit
       8: 00000000     0 SECTION LOCAL  DEFAULT    9 
       9: 00000057    93 FUNC    LOCAL  DEFAULT    4 _Z41__static_initializati
      10: 000000b4    40 FUNC    LOCAL  DEFAULT    4 _GLOBAL__sub_I_main
      11: 00000000     0 SECTION LOCAL  DEFAULT   11 
      12: 00000000     0 SECTION LOCAL  DEFAULT   13 
      13: 00000000     0 SECTION LOCAL  DEFAULT   14 
      14: 00000000     0 SECTION LOCAL  DEFAULT   16 
      15: 00000000     0 SECTION LOCAL  DEFAULT   17 
      16: 00000000     0 SECTION LOCAL  DEFAULT   18 
      17: 00000000     0 SECTION LOCAL  DEFAULT   15 
      18: 00000000     0 SECTION LOCAL  DEFAULT    1 
      19: 00000000     0 SECTION LOCAL  DEFAULT    2 
      20: 00000000     0 SECTION LOCAL  DEFAULT    3 
      21: 00000000    87 FUNC    GLOBAL DEFAULT    4 main
      22: 00000000     0 FUNC    GLOBAL HIDDEN    14 __x86.get_pc_thunk.bx
      23: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _GLOBAL_OFFSET_TABLE_
      24: 00000000    27 FUNC    WEAK   DEFAULT    9 _Z3addIiET_S0_S0_
      25: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZSt4cout
      26: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZNSolsEi
      27: 00000000     0 FUNC    GLOBAL HIDDEN    13 __x86.get_pc_thunk.ax
      28: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZNSt8ios_base4InitC1Ev
      29: 00000000     0 NOTYPE  GLOBAL HIDDEN   UND __dso_handle
      30: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _ZNSt8ios_base4InitD1Ev
      31: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND __cxa_atexit
  ```

  得到符号表对应的符号为main，正好为最后一个局部符号（LOCAL）加1的符号。
* Al：节区对齐4B，因为本节区是一个表，所以项的大小是固定的，需要进行对齐。

## 4.代码实现

### 4.1.算法思路

以Elf32位为例，我们要读取的是节区头表的每一个项，它的每一项的定义如下：

```shell
typedef struct
{
  Elf32_Word	sh_name;		/* 节名称（字符串表索引） */
  Elf32_Word	sh_type;		/* 节类型 */
  Elf32_Word	sh_flags;		/* 节标志 */
  Elf32_Addr	sh_addr;		/* 执行时的节虚拟地址 */
  Elf32_Off	sh_offset;		/* 节在文件中的偏移量 */
  Elf32_Word	sh_size;		/* 节的字节大小 */
  Elf32_Word	sh_link;		/* 链接到另一个节 */
  Elf32_Word	sh_info;		/* 附加的节信息 */
  Elf32_Word	sh_addralign;		/* 节对齐方式 */
  Elf32_Word	sh_entsize;		/* 如果节保存表格，则是条目的大小 */
} Elf32_Shdr;

```

节区头表是一个数组，数组的类型是Elf32_Shdr，因此直接根据节区头表的开始位置可以直接进行遍历访问。我们遍历每一个节区头表项，并输出打印它的结构体中的信息。特别的是sh_name索引需要通过访问shstrtab来获取真正的字符串名称。其实现步骤如下：

1. 首先检查 ELF 头中的节头表项目数（e_shnum），如果为0，表示异常情况，可能是损坏的 ELF 文件头或者文件中没有节区。进行相应的处理并返回。
2. 如果需要解析内容选项包括-t、-S或-e，则打印节头表的项目数量和偏移位置。
3. 如果是32位的 ELF 文件，读取32位的节区头表。
4. 读取字符串表（shstrtab），以便后续显示节区名称。
5. 如果需要解析内容选项包括-t、-S或-e，则打印节头信息。
6. 遍历节区头表的所有项，对于每个节区，进行如下处理：

   * 计算节区名称在字符串表中的偏移地址。
   * 读取节区名称。
   * 打印节区名称。
   * 打印节区类型。
   * 打印节区的虚拟地址、文件中偏移地址、大小和条目大小。
   * 如果节区有标志位，打印标志位；否则用空格填充。
   * 打印 sh_link 和 sh_info。
   * 如果节区名称是特定的动态链接信息表、动态链接重定位表、动态链接符号表或动态链接字符表，记录相应的偏移地址和大小。
   * 如果需要解析内容选项包括-t，打印标志位。
7. 如果需要解析内容选项包括-x或-u，则遍历节区头表的所有项，记录指定节区的索引。
8. 返回成功。

### 4.2.流程图

按照上述的思想，设计的程序的流程图如下图所示：

![1687487560681](image/t指令的原理与ELF实现/1687487560681.png)

### 4.3.测试

对于测试节区信息，我们编写了简单的程序，用于测试-s指令输出动态符号表。程序需要有两部分组成，因为是动态链接，所以需要编译一个so动态链接库，还有一个主程序。主程序A.c如下代码：

```cpp
/*A.c*/
#include<stdio.h>

static int mystaticVar = 3 ;
int myglobalvar=5;
int myglobalvar2=6;
extern void testfun();
int main(){
        testfun();
        printf("myStaticVar %d\n", mystaticVar);
        return 0;
}
void hell(){
     testfun();
}

```

然后编写动态链接库程序B.c如下代码：

```c
/*B.c*/ 
__attribute__((visibility("default"))) void testfun(){
}
__attribute__((visibility("hidden")))  void testfun2(){
}
int libGLobal=2;
```

接下来进行编译，首先将B.c文件编译为.so动态链接库文件：

```shell
dp@ubuntu:~/Desktop/elf/7.9$ gcc -m32 -fPIC -shared B.c -o B.so
```

然后进行链接，将A.c文件编译输出为A文件，同时与B.so动态链接。

```shell
dp@ubuntu:~/Desktop/elf/7.9$ gcc -m32 A.c -o A B.so
```

### 4.4.代码详细解释

#### 4.4.1.处理打印节区头表

处理打印节区头表在函数中定义为process_section_headers，它的功能是实现遍历节区头表并输出每项的结构体信息，另一方面，负责了依据sh_name索引查shstrtab表的任务和做一些参数如unwind的初始化等，如下面的代码注释：

```c
//解析和处理文件的节头表
int ELF_process::process_section_headers(FILE *file,int option,char *target_section_name)
{
    //file：文件指针
    //option：解析的内容选项
    //target_section_name：目标节区索引，这个只有当-x指令时生效

    Elf32_Shdr * section;
    section = NULL;
    char * string_table;

    unsigned int  flag_shoff;

    //依据ELF头判断节区头表的项目数，如果项目数为0那么存在异常
    if (elf_header.e_shnum == 0)
    {   
        //依据elf头中的节区表头偏移地址是不是0，如果是0，那么就说明不存在节区表头，有异常
        if (elf_header.e_shoff!=0)
            printf("possibly corrupt ELF file header - it has a non-zero section header offset, but no section headers\n");
        //节区表头存在但是没有表项，就说明文件中没有节区
        else
            printf ("\nThere are no sections in this file.\n");
        return 1;
    }

    //如果指令为-t -S -e，那么就先打印出节头表的项目数量和节头表的偏移位置，
    //这些指令分别是：显示节的详细信息、 显示节头信息、显示全部头信息
    if((option & (1<<4))||(option & (1<<2) ) || (option & (1<<6)))
        printf ("  There are %d section headers, starting at offset 0x%lx:\n",elf_header.e_shnum, (unsigned long) elf_header.e_shoff);

    //如果是32位的文件
    if (is_32bit_elf)
    {
        //读取32位的节区头表，依据elf头中的信息
        if (! get_32bit_section_headers (file, elf_header.e_shnum))
            return 0;
    }

    /* Read in the string table, so that we have names to display.  */
    //找到shstrtab的位置
    //先判断shstrtab的节区索引是不是未定义，然后判断是不是小于总节区数
    if (elf_header.e_shstrndx != SHN_UNDEF  && elf_header.e_shstrndx < elf_header.e_shnum)
    {
        //获取shstrtab节区，位置是节区头的偏移地址加上对应的索引，即节区头表中的第e_shstrndx个元素
        section = section_headers + elf_header.e_shstrndx;

        //保存shstrtab的偏移地址
        flag_shoff = section->sh_offset;

    }

    //如果指令为-t -S -e，那么接下来打印节头信息
    //这些指令分别是：显示节的详细信息、 显示节头信息、显示全部头信息
    if((option & (1<<4))||(option & (1<<2) ) || (option & (1<<6)))
    {
        //如果节头表项个数大于1，那么单词用复数
	    if (elf_header.e_shnum > 1)
            printf ("\nSection Headers:\n");
        else
            printf ("\nSection Header:\n");
    }
    section = section_headers;
    unsigned int countC;
    //只做32位文件的分析
    if (is_32bit_elf)
    {
        //如果是-t指令，显示节区的详细信息
        if(option & (1<<4)) //-t
        {
            //打印列名
            printf("  [Nr] Name\n      Type            Addr     Off    Size   ES Flg Lk Inf Al\n      Flags\n");
            //遍历节区头表的所有项，每一个项都对应一个节区
            for (int i = 0; i < elf_header.e_shnum; i++, section++)
            {
                printf ("  [%2u] ", i);

                //计算该节区名在shstrtab中的偏移地址
                countC = flag_shoff + section->sh_name;

                //将文件指针移动到这个地方
                fseek(file,countC,SEEK_SET);
                //名字字符串，长度为20，名字以'\0'结尾，所以多读一些没有关系
                char string_name[20];
                //从文件中读取名字字符串
                fread(string_name,20,1,file);

        //判断节区名是不是"IA_64",记录unwind节区的索引
		if(!strcmp(string_name,"IA_64")) unwind_idx=i;
        //判断节区名和目标的节区名是否一致，相同的话记录目标节区名对应的索引，这个是给-x用的
		if(!strcmp(string_name,target_section_name)) target_section_idx=i;

                //打印输出节区名
                printf("%-16s \n",string_name);

                //打印输出节区类型
                printf("%-16s ",get_section_type_name (section->sh_type));

                //打印输出节区虚拟地址、文件中偏移地址、节区大小、节区条目大小
                printf("%6.8lx",(unsigned long) section->sh_addr);
                printf ( " %6.6lx %6.6lx %2.2lx",
                         (unsigned long) section->sh_offset,
                         (unsigned long) section->sh_size,
                         (unsigned long) section->sh_entsize);

                //如果节区有标志位那么就打印输出标志
                if (section->sh_flags)
                    printf (" %2.2x ", section->sh_flags);
                //否则用空格填充
                else
                    printf("%4c",32);

                //打印输出sh_link和sh_info，这个因不同节区类型而异，最后输出对齐信息
                printf ("%2u ", section->sh_link);
                printf ("%3u %3lu", section->sh_info,
                        (unsigned long) section->sh_addralign);

                //如果节区名是.dynamic动态链接信息表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".dynamic")==0)
                {
                    dynamic_addr   = section->sh_offset;
                    dynamic_size   = section->sh_size;
                }

                //如果节区名是.rel.dyn动态链接重定位表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".rel.dyn")==0)
                {
                    rel_dyn_offset = section->sh_offset;
                    rel_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynsym动态链接符号表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynsym")==0)
                {
                    sym_dyn_offset = section->sh_offset;
                    sym_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynstr动态链接字符表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynstr")==0)
                {
                    str_dyn_offset = section->sh_offset;
                    str_dyn_size   = section->sh_size;
                }

                printf("\n");
                //打印输出标志位
                printf("      [%x]\n",section->sh_flags);
            }
        }
        //如果是-S或-e指令，显示节头信息和全部头信息
        else if((option & (1<<2) ) || (option & (1<<6)))        //-S || -e
        {
  
            //打印列名
            printf("  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n");
            //遍历节区头表的所有项，每一个项都对应一个节区
            for (int i = 0; i < elf_header.e_shnum; i++, section++)
            {
                printf ("  [%2u] ", i);

                //计算该节区名在shstrtab中的偏移地址
                countC = flag_shoff + section->sh_name;

                //将文件指针移动到这个地方
                fseek(file,countC,SEEK_SET);
                //名字字符串，长度为20，名字以'\0'结尾，所以多读一些没有关系
                char string_name[20];
                //从文件中读取名字字符串
                fread(string_name,20,1,file);

        //判断节区名是不是"IA_64",记录unwind节区的索引
		if(!strcmp(string_name,"IA_64")) unwind_idx=i;
        //判断节区名和目标的节区名是否一致，相同的话记录目标节区名对应的索引，这个是给-x用的
		if(!strcmp(string_name,target_section_name)) target_section_idx=i;

                //打印输出节区名
                printf("%-16s ",string_name);


                //打印输出节区类型
                printf ( " %-15.15s ",
                         get_section_type_name (section->sh_type));

                //打印输出节区虚拟地址、文件中偏移地址、节区大小、节区条目大小
                printf("%6.8lx",(unsigned long) section->sh_addr);
                printf ( " %6.6lx %6.6lx %2.2lx",
                         (unsigned long) section->sh_offset,
                         (unsigned long) section->sh_size,
                         (unsigned long) section->sh_entsize);

                //如果节区有标志位那么就打印输出标志
                if (section->sh_flags)
                    printf (" %2.2x ", section->sh_flags);
                //否则用空格填充
                else
                    printf("%4c",32);

                //打印输出sh_link和sh_info，这个因不同节区类型而异，最后输出对齐信息
                printf ("%2u ", section->sh_link);
                printf ("%3u %3lu", section->sh_info,
                        (unsigned long) section->sh_addralign);

                //如果节区名是.dynamic动态链接信息表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".dynamic")==0)
                {
                    dynamic_addr   = section->sh_offset;
                    dynamic_size   = section->sh_size;
                }

                //如果节区名是.rel.dyn动态链接重定位表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".rel.dyn")==0)
                {
                    rel_dyn_offset = section->sh_offset;
                    rel_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynsym动态链接符号表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynsym")==0)
                {
                    sym_dyn_offset = section->sh_offset;
                    sym_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynstr动态链接字符表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynstr")==0)
                {
                    str_dyn_offset = section->sh_offset;
                    str_dyn_size   = section->sh_size;
                }

                printf("\n");

            }
        }
        //如果是指令-x或-u，显示指定节区详细信息或.unwind节区详细信息
        else if(option & (1<<14) || option & (1<<9))
        {
            //遍历节区头表的所有项，每一个项都对应一个节区
            for (int i = 0; i < elf_header.e_shnum; i++, section++)
            {
                //计算该节区名在shstrtab中的偏移地址
                countC = flag_shoff + section->sh_name;
                //将文件指针移动到这个地方
                fseek(file,countC,SEEK_SET);
                //名字字符串，长度为20，名字以'\0'结尾，所以多读一些没有关系
                char string_name[20];
                //从文件中读取名字字符串
                fread(string_name,20,1,file);

                //判断节区名是不是"IA_64",记录unwind节区的索引，这个是给-u用的
                if(!strcmp(string_name,"IA_64")) unwind_idx=i;
                //判断节区名和目标的节区名是否一致，相同的话记录目标节区名对应的索引，这个是给-x用的
                if(!strcmp(string_name,target_section_name)) target_section_idx=i;
            }
        }
    }

    //成功返回
    return 1;
}
```

#### 4.4.2.获取节头表数据

获取节头表数据在函数get_32bit_section_headers中定义，其功能是将节头表数据结构从文件中读取到内存中，并初始化完数据结构，最终返回给处理打印程序。其详细注释代码如下所示：

```c
//读取32位ELF文件的节区头表
int ELF_process::get_32bit_section_headers(FILE *file, unsigned int num)
{
    //file：ELF文件指针
    //num：节区头表的项目数

    Elf32_External_Shdr * shdrs;
    Elf32_Shdr* internal;

    //利用elf头中的关于节头的信息，读取节头的数据，并强制转换为节头指针数据类型
    //这里读取的shdrs是节头表数组的首地址
    shdrs = (Elf32_External_Shdr *) get_data (NULL, file, elf_header.e_shoff,
            elf_header.e_shentsize, num,
            ("section headers"));
    //如果节头数组为空，就表示读取失败了，所以直接返回
    if (!shdrs)
        return 0;

    //申请内存存放节头数组，之前的读取的节头数组是
    section_headers = (Elf32_Shdr *) cmalloc (num,sizeof (Elf32_Shdr));

    //如果申请内存失败，返回内存溢出异常
    if (section_headers == NULL)
    {
        printf("Out of memory\n");
        return 0;
    }

    internal = section_headers;

    //将从文件中读取的小端数据的节区头表数据转换为结构体的数据
    //这里的转换有讲究的，因为我们读取使用的char[]数组读4个字节的，是不会进行转换的，会直接读取原始的小端数据
    //这里需要将小端数据转换一下，变成正常的字数据
    //那为什么不直接用uint_32t?
    //因为保存的ELF文件不一定都是我定义的Elf32Word类型，变个名字就失效了
    for (int i = 0; i < num; i++, internal++)
    {
        internal->sh_name      = BYTE_GET (shdrs[i].sh_name);
        internal->sh_type      = BYTE_GET (shdrs[i].sh_type);
        internal->sh_flags     = BYTE_GET (shdrs[i].sh_flags);
        internal->sh_addr      = BYTE_GET (shdrs[i].sh_addr);
        internal->sh_offset    = BYTE_GET (shdrs[i].sh_offset);
        internal->sh_size      = BYTE_GET (shdrs[i].sh_size);
        internal->sh_link      = BYTE_GET (shdrs[i].sh_link);
        internal->sh_info      = BYTE_GET (shdrs[i].sh_info);
        internal->sh_addralign = BYTE_GET (shdrs[i].sh_addralign);
        internal->sh_entsize   = BYTE_GET (shdrs[i].sh_entsize);
    }

    free (shdrs);

    return 1;
}
```

#### 4.4.3.从文件中读取数据结构

在读取节区头表中有一个函数get_data，负责从文件中指定的偏移地址开始读取，将数据读取到内存中，并按照大小端的格式进行转换，是能够适应结构体的初始化。其代码的初始化如下：

```c
//将硬盘中文件中的数据提取到内存中的变量
void * ELF_process::get_data (void * var, FILE * file, long offset, size_t size, size_t nmemb,const char * reason)
{
    //var：存放数据的变量地址
    //file：读取的文件
    //offset：被读取的数据的偏移地址
    //size：每个元素的大小
    //nmemb：一共有多少个这样的元素
    //reason：读取的原因，便于报错


    //临时变量保存地址
    void * mvar;

    //如果每个元素的大小为0，或者有0个元素，那么读空
    if (size == 0 || nmemb == 0)
        return NULL;

    //将文件的读取指针移动到偏移地址处
    if (fseek (file, offset, SEEK_SET))
    {
        //error (_("Unable to seek to 0x%lx for %s\n"),
        //  (unsigned long) archive_file_offset + offset, reason);
        //若失败则返回读空
        return NULL;
    }

    mvar = var;
    //判断用户的变量是不是为空，如果为空，那么需要申请空间
    if (mvar == NULL)
    {
        /* Check for overflow.  */
        //检查是否溢出，即将要读取的元素个数大于等于了地址空间全部存放这个元素时的最大个数
        if (nmemb < (~(size_t) 0 - 1) / size)
            /* + 1 so that we can '\0' terminate invalid string table sections.  */
            //如果没有溢出，那么就申请 元素大小*元素个数+1 的内存空间
            //+1是为了最后添加'/0'给字符串节区用
            mvar = malloc (size * nmemb + 1);

        //如果此时变量指针还是空的，说明没有成功申请到内存，所以返回读空
        if (mvar == NULL)
        {
            //error (_("Out of memory allocating 0x%lx bytes for %s\n"),
            //(unsigned long)(size * nmemb), reason);
            return NULL;
        }

        //申请成功的话，将最后一位（+1的那个字节）设置为'\0'字符串结束符
        ((char *) mvar)[size * nmemb] = '\0';
    }

    //从文件读取指针处读取nmemb个size大小的数据到mvar所指向的内存空间中
    if (fread (mvar, size, nmemb, file) != nmemb)
    {
        //若实际读取的元素个数和想要读取的元素个数不一致，那么就异常
        //error (_("Unable to read in 0x%lx bytes of %s\n"),
        // (unsigned long)(size * nmemb), reason);
        //如果两个指针所指向的地址不一样，那么就释放内存，避免内存泄漏
        if (mvar != var)
            free (mvar);
        //返回读空
        return NULL;
    }

    //读取完毕，成功返回复制到内存中的数据
    return mvar;
}
```

#### 4.4.4.申请内存

在从文件中读取数据到内存的函数get_data中，引用了一个函数cmalloc，其目的是为了在内存中申请空间，而做的一个溢出判断，避免申请内存时出现内存溢出的情况。其代码的注释如下：

```c
//在内存中申请空间
void *ELF_process::cmalloc (size_t nmemb, size_t size)
{
    /* Check for overflow.  */
    //检查溢出，判断要申请的空间存放的元素个数 是否大于等于 假如全部装这个元素时能存放的最大元素个数
    if (nmemb >= ~(size_t) 0 / size)
        return NULL;
    //如果没有溢出，那么就申请 元素大小*元素个数 的内存空间大小
    else
        return malloc (nmemb * size);
}
```

#### 4.4.5.大小端转换

在get_32bit_section_headers中，有进行数据转换的宏函数BYTE_GET，用于读取到chat数组的原始小端数据进行转换，使得能够与结构体需要的信息进行匹配。

```c
#define BYTE_GET(field)  byte_get_little_endian (field,sizeof(field))
```

其调用的是函数byte_get_little_endian，其注释代码如下：

```c
//按照小端方式对字进行转换读取，转换为大端方式
int byte_get_little_endian (unsigned char *field, int size)
{
    //依据字的长度进行转换
    switch (size)
    {
    case 1:
        return *field;
    case 2:
        return ((unsigned int)(field[0]))
               | (((unsigned int)(field[1])) << 8);
    case 3:
        return  ((unsigned long) (field[0]))
                |    (((unsigned long) (field[1])) << 8)
                |    (((unsigned long) (field[2])) << 16);

    case 4:
        return  ((unsigned long) (field[0]))
                |    (((unsigned long) (field[1])) << 8)
                |    (((unsigned long) (field[2])) << 16)
                |    (((unsigned long) (field[3])) << 24);
    }

}
```
