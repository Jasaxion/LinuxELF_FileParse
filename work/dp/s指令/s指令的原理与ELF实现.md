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
dp@ubuntu:~/Desktop/elf/7.9$ readelf -s A

Symbol table '.dynsym' contains 9 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
     2: 00000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.0 (2)
     3: 00000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@GLIBC_2.1.3 (3)
     4: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     5: 00000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.0 (2)
     6: 00000000     0 FUNC    GLOBAL DEFAULT  UND testfun
     7: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
     8: 00002004     4 OBJECT  GLOBAL DEFAULT   18 _IO_stdin_used

Symbol table '.symtab' contains 75 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 000001b4     0 SECTION LOCAL  DEFAULT    1 
     2: 000001c8     0 SECTION LOCAL  DEFAULT    2 
     3: 000001ec     0 SECTION LOCAL  DEFAULT    3 
     4: 00000208     0 SECTION LOCAL  DEFAULT    4 
     5: 00000228     0 SECTION LOCAL  DEFAULT    5 
     6: 00000248     0 SECTION LOCAL  DEFAULT    6 
     7: 000002d8     0 SECTION LOCAL  DEFAULT    7 
     8: 00000382     0 SECTION LOCAL  DEFAULT    8 
     9: 00000394     0 SECTION LOCAL  DEFAULT    9 
    10: 000003c4     0 SECTION LOCAL  DEFAULT   10 
    11: 00000404     0 SECTION LOCAL  DEFAULT   11 
    12: 00001000     0 SECTION LOCAL  DEFAULT   12 
    13: 00001030     0 SECTION LOCAL  DEFAULT   13 
    14: 00001070     0 SECTION LOCAL  DEFAULT   14 
    15: 00001080     0 SECTION LOCAL  DEFAULT   15 
    16: 000010b0     0 SECTION LOCAL  DEFAULT   16 
    17: 000012dc     0 SECTION LOCAL  DEFAULT   17 
    18: 00002000     0 SECTION LOCAL  DEFAULT   18 
    19: 00002018     0 SECTION LOCAL  DEFAULT   19 
    20: 00002074     0 SECTION LOCAL  DEFAULT   20 
    21: 00003ecc     0 SECTION LOCAL  DEFAULT   21 
    22: 00003ed0     0 SECTION LOCAL  DEFAULT   22 
    23: 00003ed4     0 SECTION LOCAL  DEFAULT   23 
    24: 00003fd4     0 SECTION LOCAL  DEFAULT   24 
    25: 00004000     0 SECTION LOCAL  DEFAULT   25 
    26: 00004014     0 SECTION LOCAL  DEFAULT   26 
    27: 00000000     0 SECTION LOCAL  DEFAULT   27 
    28: 00000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    29: 00001100     0 FUNC    LOCAL  DEFAULT   16 deregister_tm_clones
    30: 00001140     0 FUNC    LOCAL  DEFAULT   16 register_tm_clones
    31: 00001190     0 FUNC    LOCAL  DEFAULT   16 __do_global_dtors_aux
    32: 00004014     1 OBJECT  LOCAL  DEFAULT   26 completed.7623
    33: 00003ed0     0 OBJECT  LOCAL  DEFAULT   22 __do_global_dtors_aux_fin
    34: 000011e0     0 FUNC    LOCAL  DEFAULT   16 frame_dummy
    35: 00003ecc     0 OBJECT  LOCAL  DEFAULT   21 __frame_dummy_init_array_
    36: 00000000     0 FILE    LOCAL  DEFAULT  ABS A.c
    37: 00004008     4 OBJECT  LOCAL  DEFAULT   25 mystaticVar
    38: 00000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    39: 000021cc     0 OBJECT  LOCAL  DEFAULT   20 __FRAME_END__
    40: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
    41: 00003ed0     0 NOTYPE  LOCAL  DEFAULT   21 __init_array_end
    42: 00003ed4     0 OBJECT  LOCAL  DEFAULT   23 _DYNAMIC
    43: 00003ecc     0 NOTYPE  LOCAL  DEFAULT   21 __init_array_start
    44: 00002018     0 NOTYPE  LOCAL  DEFAULT   19 __GNU_EH_FRAME_HDR
    45: 00003fd4     0 OBJECT  LOCAL  DEFAULT   24 _GLOBAL_OFFSET_TABLE_
    46: 00001000     0 FUNC    LOCAL  DEFAULT   12 _init
    47: 000012d0     5 FUNC    GLOBAL DEFAULT   16 __libc_csu_fini
    48: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
    49: 000010f0     4 FUNC    GLOBAL HIDDEN    16 __x86.get_pc_thunk.bx
    50: 00004000     0 NOTYPE  WEAK   DEFAULT   25 data_start
    51: 00000000     0 FUNC    GLOBAL DEFAULT  UND printf@@GLIBC_2.0
    52: 000012d5     0 FUNC    GLOBAL HIDDEN    16 __x86.get_pc_thunk.bp
    53: 00004014     0 NOTYPE  GLOBAL DEFAULT   25 _edata
    54: 000012dc     0 FUNC    GLOBAL HIDDEN    17 _fini
    55: 000011e9     0 FUNC    GLOBAL HIDDEN    16 __x86.get_pc_thunk.dx
    56: 00000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@@GLIBC_2.1
    57: 00004000     0 NOTYPE  GLOBAL DEFAULT   25 __data_start
    58: 00004010     4 OBJECT  GLOBAL DEFAULT   25 myglobalvar2
    59: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    60: 00004004     0 OBJECT  GLOBAL HIDDEN    25 __dso_handle
    61: 00002004     4 OBJECT  GLOBAL DEFAULT   18 _IO_stdin_used
    62: 00000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@@GLIBC_
    63: 00001260   101 FUNC    GLOBAL DEFAULT   16 __libc_csu_init
    64: 00004018     0 NOTYPE  GLOBAL DEFAULT   26 _end
    65: 000010b0    58 FUNC    GLOBAL DEFAULT   16 _start
    66: 00002000     4 OBJECT  GLOBAL DEFAULT   18 _fp_hw
    67: 00004014     0 NOTYPE  GLOBAL DEFAULT   26 __bss_start
    68: 000011ed    75 FUNC    GLOBAL DEFAULT   16 main
    69: 0000125b     0 FUNC    GLOBAL HIDDEN    16 __x86.get_pc_thunk.ax
    70: 00001238    35 FUNC    GLOBAL DEFAULT   16 hell
    71: 00004014     0 OBJECT  GLOBAL HIDDEN    25 __TMC_END__
    72: 00000000     0 FUNC    GLOBAL DEFAULT  UND testfun
    73: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    74: 0000400c     4 OBJECT  GLOBAL DEFAULT   25 myglobalvar
```

列名的含义如下：

| 列名  | 含义                                 | 变量     | 大小          |
| ----- | ------------------------------------ | -------- | ------------- |
| Value | 符号值，具体与符号类型相关           | st_value | Elf32_Addr    |
| Size  | 符号大小                             | st_size  | Elf32_Word    |
| Type  | 符号类型，st_info低四位              | st_info  | unsigned char |
| Bind  | 符号绑定类型                         | st_info  | unsigned char |
| Vis   | 可见性，由DEFAULT和HIDDEN            |          |               |
| Ndx   | 符号所在节区索引                     | st_shndx | Elf32_Section |
| Name  | 符号名索引，即符号在符号名表中的索引 | st_name  | Elf32_Word    |

接下来，我们来进行详细分析：

1. 这里有两个表输出了，一个是符号表，一个是动态符号表，查看他们的数据结构，这两个表的定义是一样的。

   对于第一个动态符号表有一行输出：`Symbol table '.dynsym' contains 9 entries:`，这表明了动态符号表的有9个项目。

   对于第二个符号表有一行输出：`Symbol table '.symtab' contains 75 entries:`，表明了符号表有75个项目。
2. 我们以 `37: 00004008     4 OBJECT  LOCAL  DEFAULT   25 mystaticVar`这一行作为具体的例子进行分析，在我们的代码中，变量mystaticVar的定义为：`static int mystaticVar = 3 ;`

   首先是他的Value为0x00004008，由于这是可执行文件，所以，这表示了mystaticVar所在的虚拟地址。我们查看这一区域的内容：

   ```shell
   dp@ubuntu:~/Desktop/elf/7.9$ readelf -x 25 A

   Hex dump of section '.data':
     0x00004000 00000000 04400000 03000000 05000000 .....@..........
     0x00004010 06000000                            ....


   ```

   根据上吧可以发现，在地址0x00004008地址处的值为0x0300，转换为大端格式为0x00003，恰好为我们初始化的值3.

   其次我们定义的变量类型为int类型的，int的大小为4字节，在符号表中，也标识了符号大小为4字节。

   符号mystaticVar并没有共享给其他的文件，只在当前文件使用，所以为LOCAL类型。相反的，存在一个函数testfun，如下：

   ```shell
   6: 00000000     0 FUNC    GLOBAL DEFAULT  UND testfun
   ```

   该符号指向了一个函数testfun，这个函数是位于B.so中的，在A可执行程序中进行了引用，所以这个符号是GLOBAL类型的。

   接下来是这个符号所在的节区索引，对于mystaticVar符号而言，其符号索引为25，我们打印查看节区头表。

   ```shell
   dp@ubuntu:~/Desktop/elf/7.9$ readelf -S A
   There are 31 section headers, starting at offset 0x3898:

   Section Headers:
     [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
     [ 0]                   NULL            00000000 000000 000000 00      0   0  0
     [ 1] .interp           PROGBITS        000001b4 0001b4 000013 00   A  0   0  1
     [ 2] .note.gnu.build-i NOTE            000001c8 0001c8 000024 00   A  0   0  4
     [ 3] .note.gnu.propert NOTE            000001ec 0001ec 00001c 00   A  0   0  4
     [ 4] .note.ABI-tag     NOTE            00000208 000208 000020 00   A  0   0  4
     [ 5] .gnu.hash         GNU_HASH        00000228 000228 000020 04   A  6   0  4
     [ 6] .dynsym           DYNSYM          00000248 000248 000090 10   A  7   1  4
     [ 7] .dynstr           STRTAB          000002d8 0002d8 0000aa 00   A  0   0  1
     [ 8] .gnu.version      VERSYM          00000382 000382 000012 02   A  6   0  2
     [ 9] .gnu.version_r    VERNEED         00000394 000394 000030 00   A  7   1  4
     [10] .rel.dyn          REL             000003c4 0003c4 000040 08   A  6   0  4
     [11] .rel.plt          REL             00000404 000404 000018 08  AI  6  24  4
     [12] .init             PROGBITS        00001000 001000 000024 00  AX  0   0  4
     [13] .plt              PROGBITS        00001030 001030 000040 04  AX  0   0 16
     [14] .plt.got          PROGBITS        00001070 001070 000010 10  AX  0   0 16
     [15] .plt.sec          PROGBITS        00001080 001080 000030 10  AX  0   0 16
     [16] .text             PROGBITS        000010b0 0010b0 000229 00  AX  0   0 16
     [17] .fini             PROGBITS        000012dc 0012dc 000018 00  AX  0   0  4
     [18] .rodata           PROGBITS        00002000 002000 000018 00   A  0   0  4
     [19] .eh_frame_hdr     PROGBITS        00002018 002018 00005c 00   A  0   0  4
     [20] .eh_frame         PROGBITS        00002074 002074 00015c 00   A  0   0  4
     [21] .init_array       INIT_ARRAY      00003ecc 002ecc 000004 04  WA  0   0  4
     [22] .fini_array       FINI_ARRAY      00003ed0 002ed0 000004 04  WA  0   0  4
     [23] .dynamic          DYNAMIC         00003ed4 002ed4 000100 08  WA  7   0  4
     [24] .got              PROGBITS        00003fd4 002fd4 00002c 04  WA  0   0  4
     [25] .data             PROGBITS        00004000 003000 000014 00  WA  0   0  4
     [26] .bss              NOBITS          00004014 003014 000004 00  WA  0   0  1
     [27] .comment          PROGBITS        00000000 003014 00002b 01  MS  0   0  1
     [28] .symtab           SYMTAB          00000000 003040 0004b0 10     29  47  4
     [29] .strtab           STRTAB          00000000 0034f0 00028e 00      0   0  1
     [30] .shstrtab         STRTAB          00000000 00377e 000118 00      0   0  1
   Key to Flags:
     W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
     L (link order), O (extra OS processing required), G (group), T (TLS),
     C (compressed), x (unknown), o (OS specific), E (exclude),
     p (processor specific)
   ```

   可以发现，节区头表中，第25号节区是.data段，说明了变量符号mystaticVar所在的节区为.data节区。当我们输出.data节区的内容时：

   ```shell
   dp@ubuntu:~/Desktop/elf/7.9$ readelf -x 25 A

   Hex dump of section '.data':
    0x00004000 00000000 04400000 03000000 05000000 .....@..........
    0x00004010 06000000                            ....


   ```

   其中0x000040008正好的符号所在的位置，值也正确。

   以后是符号名，我们首先查看符号表的十六进制数据：

   ```shell
   dp@ubuntu:~/Desktop/elf/7.9$ readelf -x 28 A

   Hex dump of section '.symtab':
     0x00000000 00000000 00000000 00000000 00000000 ................
     0x00000010 00000000 b4010000 00000000 03000100 ................
     0x00000020 00000000 c8010000 00000000 03000200 ................
     0x00000030 00000000 ec010000 00000000 03000300 ................
     0x00000040 00000000 08020000 00000000 03000400 ................
     0x00000050 00000000 28020000 00000000 03000500 ....(...........
     0x00000060 00000000 48020000 00000000 03000600 ....H...........
     0x00000070 00000000 d8020000 00000000 03000700 ................
     0x00000080 00000000 82030000 00000000 03000800 ................
     0x00000090 00000000 94030000 00000000 03000900 ................
     0x000000a0 00000000 c4030000 00000000 03000a00 ................
     0x000000b0 00000000 04040000 00000000 03000b00 ................
     0x000000c0 00000000 00100000 00000000 03000c00 ................
     0x000000d0 00000000 30100000 00000000 03000d00 ....0...........
     0x000000e0 00000000 70100000 00000000 03000e00 ....p...........
     0x000000f0 00000000 80100000 00000000 03000f00 ................
     0x00000100 00000000 b0100000 00000000 03001000 ................
     0x00000110 00000000 dc120000 00000000 03001100 ................
     0x00000120 00000000 00200000 00000000 03001200 ..... ..........
     0x00000130 00000000 18200000 00000000 03001300 ..... ..........
     0x00000140 00000000 74200000 00000000 03001400 ....t ..........
     0x00000150 00000000 cc3e0000 00000000 03001500 .....>..........
     0x00000160 00000000 d03e0000 00000000 03001600 .....>..........
     0x00000170 00000000 d43e0000 00000000 03001700 .....>..........
     0x00000180 00000000 d43f0000 00000000 03001800 .....?..........
     0x00000190 00000000 00400000 00000000 03001900 .....@..........
     0x000001a0 00000000 14400000 00000000 03001a00 .....@..........
     0x000001b0 00000000 00000000 00000000 03001b00 ................
     0x000001c0 01000000 00000000 00000000 0400f1ff ................
     0x000001d0 0c000000 00110000 00000000 02001000 ................
     0x000001e0 0e000000 40110000 00000000 02001000 ....@...........
     0x000001f0 21000000 90110000 00000000 02001000 !...............
     0x00000200 37000000 14400000 01000000 01001a00 7....@..........
     0x00000210 46000000 d03e0000 00000000 01001600 F....>..........
     0x00000220 6d000000 e0110000 00000000 02001000 m...............
     0x00000230 79000000 cc3e0000 00000000 01001500 y....>..........
     0x00000240 98000000 00000000 00000000 0400f1ff ................
     0x00000250 9c000000 08400000 04000000 01001900 .....@..........
     0x00000260 01000000 00000000 00000000 0400f1ff ................
     0x00000270 a8000000 cc210000 00000000 01001400 .....!..........
     0x00000280 00000000 00000000 00000000 0400f1ff ................
     0x00000290 b6000000 d03e0000 00000000 00001500 .....>..........
     0x000002a0 c7000000 d43e0000 00000000 01001700 .....>..........
     0x000002b0 d0000000 cc3e0000 00000000 00001500 .....>..........
     0x000002c0 e3000000 18200000 00000000 00001300 ..... ..........
     0x000002d0 f6000000 d43f0000 00000000 01001800 .....?..........
     0x000002e0 1b020000 00100000 00000000 02000c00 ................
     0x000002f0 0c010000 d0120000 05000000 12001000 ................
     0x00000300 1c010000 00000000 00000000 20000000 ............ ...
     0x00000310 38010000 f0100000 04000000 12021000 8...............
     0x00000320 b1010000 00400000 00000000 20001900 .....@...... ...
     0x00000330 4e010000 00000000 00000000 12000000 N...............
     0x00000340 60010000 d5120000 00000000 12021000 `...............
     0x00000350 76010000 14400000 00000000 10001900 v....@..........
     0x00000360 16010000 dc120000 00000000 12021100 ................
     0x00000370 7d010000 e9110000 00000000 12021000 }...............
     0x00000380 93010000 00000000 00000000 22000000 ............"...
     0x00000390 af010000 00400000 00000000 10001900 .....@..........
     0x000003a0 bc010000 10400000 04000000 11001900 .....@..........
     0x000003b0 c9010000 00000000 00000000 20000000 ............ ...
     0x000003c0 d8010000 04400000 00000000 11021900 .....@..........
     0x000003d0 e5010000 04200000 04000000 11001200 ..... ..........
     0x000003e0 f4010000 00000000 00000000 12000000 ................
     0x000003f0 11020000 60120000 65000000 12001000 ....`...e.......
     0x00000400 c2000000 18400000 00000000 10001a00 .....@..........
     0x00000410 b5010000 b0100000 3a000000 12001000 ........:.......
     0x00000420 21020000 00200000 04000000 11001200 !.... ..........
     0x00000430 28020000 14400000 00000000 10001a00 (....@..........
     0x00000440 34020000 ed110000 4b000000 12001000 4.......K.......
     0x00000450 39020000 5b120000 00000000 12021000 9...[...........
     0x00000460 4f020000 38120000 23000000 12001000 O...8...#.......
     0x00000470 54020000 14400000 00000000 11021900 T....@..........
     0x00000480 60020000 00000000 00000000 12000000 `...............
     0x00000490 68020000 00000000 00000000 20000000 h........... ...
     0x000004a0 82020000 0c400000 04000000 11001900 .....@..........


   ```

   我们分析的符号mystaticVar是第37个，即0x25，由于符号表项是16字节一个，所以我们找到位置0x250，即 `0x00000250 9c000000 08400000 04000000 01001900 .....@..........`，在这里面，根据结构体的定义，第一个Elf32_Word字为符号索引，其值为9c000000，转换为大端格式为0x0000009c。接下来我们再去查.strtab表，这里面记录了符号表中符号名的字符串。打印输出符号名表如下：

   ```shell
   dp@ubuntu:~/Desktop/elf/7.9$ readelf -x .strtab A

   Hex dump of section '.strtab':
     0x00000000 00637274 73747566 662e6300 64657265 .crtstuff.c.dere
     0x00000010 67697374 65725f74 6d5f636c 6f6e6573 gister_tm_clones
     0x00000020 005f5f64 6f5f676c 6f62616c 5f64746f .__do_global_dto
     0x00000030 72735f61 75780063 6f6d706c 65746564 rs_aux.completed
     0x00000040 2e373632 33005f5f 646f5f67 6c6f6261 .7623.__do_globa
     0x00000050 6c5f6474 6f72735f 6175785f 66696e69 l_dtors_aux_fini
     0x00000060 5f617272 61795f65 6e747279 00667261 _array_entry.fra
     0x00000070 6d655f64 756d6d79 005f5f66 72616d65 me_dummy.__frame
     0x00000080 5f64756d 6d795f69 6e69745f 61727261 _dummy_init_arra
     0x00000090 795f656e 74727900 412e6300 6d797374 y_entry.A.c.myst
     0x000000a0 61746963 56617200 5f5f4652 414d455f aticVar.__FRAME_
     0x000000b0 454e445f 5f005f5f 696e6974 5f617272 END__.__init_arr
     0x000000c0 61795f65 6e64005f 44594e41 4d494300 ay_end._DYNAMIC.
     0x000000d0 5f5f696e 69745f61 72726179 5f737461 __init_array_sta
     0x000000e0 7274005f 5f474e55 5f45485f 4652414d rt.__GNU_EH_FRAM
     0x000000f0 455f4844 52005f47 4c4f4241 4c5f4f46 E_HDR._GLOBAL_OF
     0x00000100 46534554 5f544142 4c455f00 5f5f6c69 FSET_TABLE_.__li
     0x00000110 62635f63 73755f66 696e6900 5f49544d bc_csu_fini._ITM
     0x00000120 5f646572 65676973 74657254 4d436c6f _deregisterTMClo
     0x00000130 6e655461 626c6500 5f5f7838 362e6765 neTable.__x86.ge
     0x00000140 745f7063 5f746875 6e6b2e62 78007072 t_pc_thunk.bx.pr
     0x00000150 696e7466 4040474c 4942435f 322e3000 intf@@GLIBC_2.0.
     0x00000160 5f5f7838 362e6765 745f7063 5f746875 __x86.get_pc_thu
     0x00000170 6e6b2e62 70005f65 64617461 005f5f78 nk.bp._edata.__x
     0x00000180 38362e67 65745f70 635f7468 756e6b2e 86.get_pc_thunk.
     0x00000190 6478005f 5f637861 5f66696e 616c697a dx.__cxa_finaliz
     0x000001a0 65404047 4c494243 5f322e31 2e33005f e@@GLIBC_2.1.3._
     0x000001b0 5f646174 615f7374 61727400 6d79676c _data_start.mygl
     0x000001c0 6f62616c 76617232 005f5f67 6d6f6e5f obalvar2.__gmon_
     0x000001d0 73746172 745f5f00 5f5f6473 6f5f6861 start__.__dso_ha
     0x000001e0 6e646c65 005f494f 5f737464 696e5f75 ndle._IO_stdin_u
     0x000001f0 73656400 5f5f6c69 62635f73 74617274 sed.__libc_start
     0x00000200 5f6d6169 6e404047 4c494243 5f322e30 _main@@GLIBC_2.0
     0x00000210 005f5f6c 6962635f 6373755f 696e6974 .__libc_csu_init
     0x00000220 005f6670 5f687700 5f5f6273 735f7374 ._fp_hw.__bss_st
     0x00000230 61727400 6d61696e 005f5f78 38362e67 art.main.__x86.g
     0x00000240 65745f70 635f7468 756e6b2e 61780068 et_pc_thunk.ax.h
     0x00000250 656c6c00 5f5f544d 435f454e 445f5f00 ell.__TMC_END__.
     0x00000260 74657374 66756e00 5f49544d 5f726567 testfun._ITM_reg
     0x00000270 69737465 72544d43 6c6f6e65 5461626c isterTMCloneTabl
     0x00000280 65006d79 676c6f62 616c7661 7200     e.myglobalvar.

   ```

   查看位置0x0000009c的符号名表，可以发现，字符串正好是mystaticVar，最后是一个'\0'结束符。

## 4.代码实现

### 4.1.算法思路

以Elf32位为例，我们要读取的是符号表或动态符号表的每一个项，它的每一项的定义如下：

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

符号表表是一个数组，数组的类型是Elf32_Sym，因此直接根据符号表的开始位置可以直接进行遍历访问。我们遍历每一个符号表项，并输出打印它的结构体中的信息。特别的是st_name索引需要通过访问strtab（对于.dynsym来说符号名表是.dynstr）来获取真正的字符串名称。其实现步骤如下：

### 4.2.流程图

按照上述的思想，设计的程序的流程图如下图所示：

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

#### 4.4.1.处理打印符号表

处理打印符号表表在函数中定义为process_symbol_table，它的功能是

```c
//处理打印符号表
void ELF_process::process_symbol_table(FILE *pFILE,int option)
{
    Elf32_Sym* sym; 
    /*定义32位符号表*/
    get_32bit_symbol(pFILE,option);
    unsigned int i;
    if(option & (1<<5))  //-s
    {
        //打印列名
	    printf("Num  Value   NdX  other    info         Size          Name\n");
        //遍历符号表中的每一个符号项
        for (i=0, sym = sym_dyn; sym<sym_dyn+sym_nent; sym++,i++)
        {
            //打印符号在符号表中的索引
            printf("%2d: ",i);
            //打印符号的值，这个值因不同的符号类型而异
            printf("%2.8x ",sym->st_value);
            //打印符号所在的节区的索引
            printf("%-2.2x",sym->st_shndx);
            //打印符号的其他信息
            printf("%-12.2x ",sym->st_other);
            //打印符号的类型和绑定方式，低4为为类型，高28位为绑定方式
            printf("%-12.2x ",sym->st_info);
            //打印符号的大小
            printf("%-12.2x ",sym->st_size);
            //调用函数使用st_name符号名索引在strtab表中找到对应的符号名
            get_32bit_strdyn(pFILE,sym->st_name);
        }
    }
    //这个是打印统计信息的
    if(option & (1<<13)) //-I
    {
        //输出列名和提示
        printf("\nbucket list:\n");
        printf("Num     Size          of total \n");
        int sum=0;
        //遍历符号表中的每一个符号
        for (i=0, sym = sym_dyn; sym<sym_dyn+sym_nent; sym++,i++)
        {
            //统计全部符号占用的大小
            sum+=sym->st_size;
        }
        //再次遍历符号表中的每一个符号
        for (i=0, sym = sym_dyn; sym<sym_dyn+sym_nent; sym++,i++)
        {
            //输出每个符号占用的空间的比例
            printf("%3d:    ",i);
            printf("%-12.2x ",sym->st_size);
            printf(" %f  \n",1.0*(sym->st_size)/sum);
        }
    }
}
```

#### 4.4.2.查询符号名

查询符号名在函数get_32bit_strdyn中定义，其功能是通过符号名索引，在符号名表strtab中查找对应的字符串，最终将这个字符串进行输出。其注释代码如下：

```c
//查询符号名
void ELF_process::get_32bit_strdyn(FILE *pFILE, Elf32_Word name)
{
    //pFILE：文件
    //name：符号名在符号名表中的索引

    //定义临时变量存放符号名
    unsigned char sym_name[1024];
    //将文件的读取指针移动到符号名表的偏移地址处
    fseek(pFILE,(str_dyn_offset+name),SEEK_SET);
    //将数据读入到临时变量中，
    //读取长度为1024是任意的，因此每个符号后面都跟着一个'/0'结束符
    fread(sym_name,1024,1,pFILE);
    //输出符号名
    printf("%s\n",sym_name);
}
```
