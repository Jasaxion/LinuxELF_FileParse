# 选项- S的原理和具体实现

## 一、选项- S 的介绍

段（segment) 和 节（section)是有区别的。 节不是段。 段是程序执行的必要组成部分。 在每个段中会有代码或者数据被划分为不同的节。 而 **节头表** 则是对这些**节的**位置和大小的描述，主要是用于链接和调试。

因为节头表没有对程序的内存布局进行描述，对程序内存布局的描述的是 程序头表的任务。 所以节头表就是对程序头的补充。所以我们将**“节头表”**都填充为无用的值的话程序还是可以正常启动的。 虽然可以修改，但是随着版本的升级，保不准以后会使用。 就跟window平台上的最小PE一样，在xp下可以是100kb以下。win7下则对各个表的限制更严格了。所以需要100kb以上。 win10同理。 所以节头表以后可能会使用。

- 节头和节

​	如果二进制文件中缺少了 **节头** ，并不意味着节是不存在的。只是没有办法通过节头来引用节了，对于调试器或者反编译（IDA)程序来说，参考的信息就变少了。

​	每一个节都保存了某种类型的代码和数据。 数据可以是程序中的全局变量，也可以是连接器所需要的动态链接的信息。 这点跟windows平台的 windowsPE文件格式设计很像。 节头可以被抹除，默认的ELF文件是有节头的。如果节头被抹除，那么 objcopy objdump gdb IDA 等工具就可能无法使用。而节头表中的 .dynsym这样的节（记录了函数名和偏移地址，导入导出等符号）就无法分析。进而就增大了逆向的难度。

## 二、选项- S 的作用

选项-S用于列出段的头信息

## 三、选项- S 显示的信息解释

<img src="./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230623105529458.png" alt="image-20230623105529458" style="zoom:50%;" />

- sh_name：0——无名称
- sh_type：SHT_NULL——非活动
- sh_flags：0——无标志
  - 此字段定义了一个节区中包含的内容是否可以修改，是否可以执行的信息。 如果一个标志位被设置，则该位取值为1. 定义的各位都设置为0
    - SHF_WRITE：0x1
    - SHF_ALLOC：0x2
    - SHF_EXECINSTR：0x4 ——节区包含可执行的机器指令
    - SHF_MASKPROC：0xF0000000
- sh_addr：0——无地址
- sh_offset：0——无文件偏移
- sh_size：0——无尺寸大小
- sh_link：SHN_UNDEF——无链接信息
- sh_inf：0——无辅助信息
- sh_addralign：0——无对齐要求
- sh_entsize：0——无表项

<img src="./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230623110417066.png" alt="image-20230623110417066" style="zoom:50%;" />

- 特殊节区

| 名称        | 类型          | 属性                         | 含义                                                         |
| ----------- | ------------- | ---------------------------- | ------------------------------------------------------------ |
| .bss        | SHT NO BITS   | SHF_ALLOC     SHF_WRITE      | 包含将出现在程序的内存映像中的为初始     化数据。根据定义，当程序开始执行，系统     将把这些数据初始化为0。此节区不占用文     件空间。 |
| .comment    | SHT_PROG BITS | (无)                         | 包含版本控制信息。                                           |
| .data       | SHT PROG BITS | SHF_ALLOC     SHF_WRITE      | 这些节区包含初始化了的数据，将出现在程     序的内存映像中。  |
| .data  l    | SHT_PROG BITS | SHF_ALLOC                    |                                                              |
| SHF WRITE   |               |                              |                                                              |
| .debug      | SHT_PROG BITS | (无)                         | 此节区包含用于符号调试的信息。                               |
| .dynamic    | SHT DYNAMIC   |                              | 此节区包含动态链接信息。节区的属性将包     含SHF_ALLOC位。是否SHF_WRITE位     被设置取决于处理器。 |
| .dyn str    | SHT_STRTAB    | SHF_ALLOC                    | 此节区包含用于动态链接的字符串，大多数     情况下这些字符串代表了与符号表项相关     的名称。 |
| .dynsym     | SHT DYNSYM    | SHF ALLOC                    | 此节区包含了动态链接符号表。                                 |
| .fini       | SHT_PROG BITS | SHF_ALLOC     SHF_EXEC INSTR | 此节区包含了可执行的指令，是进程终止代     码的一部分。程序正常退出时，系统将安排     执行这里的代码。 |
| ·got        | SHT_PROG BITS |                              | 此节区包含全局偏移表。                                       |
| .hash       | SHT_HASH      | SHF_ALLOC                    | 此节区包含了一个符号哈希表。                                 |
| .in it      | SHT_PROG BITS | SHF_ALLOC     SHF_EXEC INSTR | 此节区包含了可执行指令，是进程初始化代     码的一部分。当程序开始执行时，系统要在     开始调用主程序入口之前(通常指C语言     的main函数) 执行这些代码。 |
| .interp     | SHT PROG BITS |                              | 此节区包含程序解释器的路径名。如果程序包含一个可加载的段，段中包含此节区，那么节区的属性将包含SHF_ALLOC位， 否则该位为0。 |
| .line       | SHT_PROG BITS | (无)                         | 此节区包含符号调试的行号信息，其中描述     了源程序与机器指令之间的对应关系。其内     容是未定义的。 |
| .note       | SHT_NOTE      | (无)                         | 此节区中包含注释信息，有独立的格式。                         |
| ·plt        | SHT_PROG BITS |                              | 此节区包含过程链接表(procedure linkage                       |
| table) 。   |               |                              |                                                              |
| .relname    | SHT_REL       |                              | 这些节区中包含了重定位信息。如果文件中     包含可加载的段，段中有重定位内容，节区     的属性将包含SHF_ALLOC位， 否则该位     置0。传统上name根据重定位所适用的节     区给定。例如     .text节区的重定位节区名字     将是：.rel.text或者.rela.text。 |
| .rela  name | SHT RELA      |                              |                                                              |
| .rodata     | SHT PROG BITS | SHF ALLOC                    | 这些节区包含只读数据，这些数据通常参与                       |
| .rodata l   | SHT_PROG BITS | SHF_ALLOC                    | 进程映像的不可写段。                                         |
| .shstrtab   | SHT STRTAB    |                              | 此节区包含节区名称。                                         |
| .strtab     | SHT STRTAB    |                              | 此节区包含字符串，通常是代表与符号表项     相关的名称。如果文件拥有一个可加载的     段，段中包含符号串表，节区的属性将包含     SHF ALLOC位， 否则该位为0。 |
| .symtab     | SHT_SYMTAB    |                              | 此节区包含一个符号表。如果文件中包含一     个可加载的段，并且该段中包含符号表，那     么节区的属性中包含SHF_ALLOC位， 否则     该位置为0。 |
| .text       | SHT PROG BITS | SHF_ALLOC     SHF EXEC INSTR | 此节区包含程序的可执行指令                                   |

## 四、代码实现

### 4.1算法思路

下面以32为ELF字长为例进行解析

- 数据结构定义「32位」

```cpp
/* Section header */
/* 节区头表32位 */
typedef struct {
    unsigned char    sh_name[4];        /* 节名，字符串 tbl 中的索引 */
    unsigned char    sh_type[4];        /* 节头表的类型 */
    unsigned char    sh_flags[4];        /* 杂项节头表的属性 */
    unsigned char    sh_addr[4];        /* 执行时的节头虚拟地址*/
    unsigned char    sh_offset[4];        /* 节头文件的偏移 */
    unsigned char    sh_size[4];        /* 节的大小（以字节为单位） */
    unsigned char    sh_link[4];        /* 另一节的索引*/
    unsigned char    sh_info[4];        /* 附加节头信息 */
    unsigned char    sh_addralign[4];    /* 节头对齐 */
    unsigned char    sh_entsize[4];        /* 如果节头包含表的话条目大小 */
} Elf32_External_Shdr;
```

- 数据结构定义「64位」

```cpp
/* 节区头表64位 */
typedef struct {
    unsigned char    sh_name[4];        /* Section name, index in string tbl */
    unsigned char    sh_type[4];        /* Type of section */
    unsigned char    sh_flags[8];        /* Miscellaneous section attributes */
    unsigned char    sh_addr[8];        /* Section virtual addr at execution */
    unsigned char    sh_offset[8];        /* Section file offset */
    unsigned char    sh_size[8];        /* Size of section in bytes */
    unsigned char    sh_link[4];        /* Index of another section */
    unsigned char    sh_info[4];        /* Additional section information */
    unsigned char    sh_addralign[8];    /* Section alignment */
    unsigned char    sh_entsize[8];        /* Entry size if section holds table */
} Elf64_External_Shdr;
```

- 处理-S操作

```cpp
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
```

### 4.2流程图

readelf -S 选项用于显示ELF文件中的节区信息。其实现步骤如下:1. 解析ELF头部,获取节区头表的位置和大小。ELF头部中包含节区头表的起始地址和大小。2. 读取节区头表。节区头表由多个Elf32_Shdr或Elf64_Shdr结构组成,每个结构描述一个节区的信息。3. 解析每个节区头,获取节区名、类型、地址、偏移、大小等信息。4. 打印每个节区的信息。按照节区类型和地址顺序打印。对于每个节区,打印其节区名、类型、地址、偏移、大小等信息。5. 打印节区内容的摘要信息。对于典型的节区类型,如.text、.data、.bss等,打印其中的符号数量和大小信息。6. 打印编译器版本信息。如果文件包含.comment节区,从中解析出GCC版本信息并打印。7. 打印动态链接信息。如果文件是动态链接库,从.dynamic节区解析动态链接信息并打印。以上就是readelf -S 选项实现的主要步骤。它通过解析ELF文件中的各种节区,获取并打印节区信息,为分析和调试ELF文件提供帮助。总的来说,该选项的实现过程就是:

1. 解析ELF头和节区头表获取节区信息

2) 解析每个节区头获取详细节区信息
3) 打印获取到的节区信息
4) 解析和打印额外的信息,如编译器版本、动态链接信息等。

![image-20230624102020731](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624102020731.png)

## 五、测试用例

下面我们尝试编写代码并进行编译：

```
#include <iostream>
using namespace std;
int main(){
    int a = 1;
    cout << "Hello Linux!!HZAU\n";
    return 0;
}
```

编译产生.so文件

```
g++ -o test-h.so test-h.cpp
```

首先我们先查看一下文件头信息

```
ELF Header:
  Magic:     7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX System V ABI
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Intel 80386
  Version:                           0x1
  Entry point address:               0x8048500
  Start of program headers:          52 (bytes into file)
  Start of section headers:          15612 (bytes into file)
  Flags:                             0x0
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         9
  Size of section headers:           40 (bytes)
  Number of section headers:         30
  Section header string table index: 29
```

> 从中可以发现节头表的开始偏移位置位15612字节，每个节头表的大小为40字节，一共有30个节点头表
>
> 补充：
>
> - hexdump命令
>
> ```
> hexdump主要用来查看“二进制”文件的十六进制编码。*注意：它能够查看任何文件，不限于与二进制文件。
> 
> -n length：格式化输出文件的前length个字节
> -C：输出规范的十六进制和ASCII码
> -s：从偏移量开始输出
> -e 指定格式字符串，格式字符串由单引号包含，格式字符串形如：’a/b “format1” “format2”。每个格式字符串由三部分组成，每个由空格分割，如a/b表示，b表示对每b个输入字节应用format1格式，a表示对每个a输入字节应用format2，一般a>b，且b只能为1,2,4，另外a可以省略，省略a=1。format1和format2中可以使用类似printf的格斯字符串。 
> %02d：两位十进制
> %03x：三位十六进制
> %02o：两位八进制
> %c：单个字符等
> %_ad：标记下一个输出字节的序号，用十进制表示
> %_ax：标记下一个输出字节的序号，用十六进制表示
> %_ao：标记下一个输出字节的序号，用八进制表示
> %_p：对不能以常规字符显示的用.代替
> 同一行显示多个格式字符串，可以跟多个-e选项
> hexdump test
> 
> 
> 格式化输出文件的前10个字节
> hexdump -n 10 test
> 
> 格式化输出文件的前10个字节，并以16进制显示
> hexdump -n 10 -C test
> 
> 格式化输出从10开始的10个字节，并以16进制显示
> hexdump -n 10 -C -s 20
> 
> 格式化输出文件字符
> hexdump -e ‘16/1 “%02X ” ” | “’ -e ‘16/1 “%_p” “\n”’ test
> hexdump -e ‘1/1 “0x%08_ax “’ -e ‘8/1 “%02X ” ” * “’ -e ‘8/1 “%_p” “\n”’ 
> test
> hexdump -e ‘1/1 “%02_ad# “’ -e ‘/1 “hex = %02X * “’ -e ‘/1 “dec = %03d | “’ -e ‘/1 “oct = %03o”’ -e ‘/1 ” _\n”’ -n 20 test
> ```
>
> <img src="./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624100901728.png" alt="image-20230624100901728" style="zoom:50%;" />

使用`./main test-h.so -S` 查看节信息

```cpp
  There are 30 section headers, starting at offset 0x3cfc:

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00     0   0   0
  [ 1] .interp           PROGBITS        08048154 000154 000013 00 02  0   0   1
  [ 2] .note.ABI-tag     NOTE            08048168 000168 000020 00 02  0   0   4
  [ 3] .note.gnu.build-id  NOTE            08048188 000188 000024 00 02  0   0   4
  [ 4] .gnu.hash         GNU_HASH        080481ac 0001ac 00002c 04 02  5   0   4
  [ 5] .dynsym           DYNSYM          080481d8 0001d8 0000b0 10 02  6   1   4
  [ 6] .dynstr           STRTAB          08048288 000288 000139 00 02  0   0   1
  [ 7] .gnu.version      VERSYM          080483c2 0003c2 000016 02 02  5   0   2
  [ 8] .gnu.version_r    VERNEED         080483d8 0003d8 000050 00 02  6   2   4
  [ 9] .rel.dyn          REL             08048428 000428 000020 08 02  5   0   4
  [10] .rel.plt          REL             08048448 000448 000028 08 42  5  22   4
  [11] .init             PROGBITS        08048470 000470 000024 00 06  0   0   4
  [12] .plt              PROGBITS        080484a0 0004a0 000060 04 06  0   0  16
  [13] .text             PROGBITS        08048500 000500 000245 00 06  0   0  16
  [14] .fini             PROGBITS        08048748 000748 000018 00 06  0   0   4
  [15] .rodata           PROGBITS        08048760 000760 000020 00 02  0   0   4
  [16] .eh_frame_hdr     PROGBITS        08048780 000780 00004c 00 02  0   0   4
  [17] .eh_frame         PROGBITS        080487cc 0007cc 000134 00 02  0   0   4
  [18] .init_array       INIT_ARRAY      08049ee8 000ee8 000008 04 03  0   0   4
  [19] .fini_array       FINI_ARRAY      08049ef0 000ef0 000004 04 03  0   0   4
  [20] .dynamic          DYNAMIC         08049ef4 000ef4 000100 08 03  6   0   4
  [21] .got              PROGBITS        08049ff4 000ff4 00000c 04 03  0   0   4
  [22] .got.plt          PROGBITS        0804a000 001000 000020 04 03  0   0   4
  [23] .data             PROGBITS        0804a020 001020 000004 00 03  0   0   1
  [24] .bss              NOBITS          0804a040 001024 000090 00 03  0   0  32
  [25] .comment          PROGBITS        00000000 001024 00002d 01 30  0   0   1
  [26] .gnu.build.attribute  NOTE            0804a0d0 001054 001cfc 00     0   0   4
  [27] .symtab           SYMTAB          00000000 002d50 000710 10    28  86   4
  [28] .strtab           STRTAB          00000000 003460 000784 00     0   0   1
  [29] .shstrtab         STRTAB          00000000 003be4 000117 00     0   0   1
```

idex为0的位置：

```cpp
[Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
[ 0]                   NULL            00000000 000000 000000 00     0   0   0
```

可以看到 index=0 的 Section Header，内容全部为0。（在某些情况下，它的某部分字段不为0，这些特殊情况当前先跳过）
这是一个非常特别的 Section Header，它的作用是表示 undefined 。它的 index（=0） 也有一个特别的名字，叫 `SHN_UNDEF`。后面会讲到它是如何发挥作用。

![image-20230624084346925](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624084346925.png)

idex为1的位置：offset = 15612 + 40

```
[Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al 
[ 1] .interp           PROGBITS        08048154 000154 000013 00 02  0   0   1
```

![image-20230624084429642](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624084429642.png)

对于这一段，此节区包含程序解释器的路径名。如果程序包含一个可加载的段，段中包含此节区，那么节区的属性将包含SHF_ALLOC位， 否则该位为0。

那么程序的可执行段在哪个位置呢？在`.text`节区

```
[13] .text             PROGBITS        08048500 000500 000245 00 06  0   0  16
```

其偏移地址为16132，长度为40

![image-20230624085153190](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624085153190.png)

首先前4个字节表示的是`sh_name`：

```
92 00 00 00
```

值为 90 00 00 00 + little endian = 0x92 = 146

接下来的4个字节表示`sh_type`

```
01 00 00 00
```

值为：“01 00 00 00” + little endian = 0x1
对照表，1对应的类型是 SHT_PROGBITS，表示这部分的信息，格式和意义完全由程序来定义。内容可以是已初始化的数据，未初始化的数据，comment 或者是程序代码等。

接下来的4个字节表示`sh_flag`

```
06 00 00 00 
```

值为：“06 00 00 00” + little endian = 0x6 = SHF_ALLOC | SHF_EXECINSTR => 这部分是可执行代码，进程运行时需要放置在内存中。
再对照回 Section Header 列表中 “.text” 那一行

```
  [13] .text             PROGBITS        08048500 000500 000245 00  AX  0   0 16
```

所以可以发现 “.text” 对应的就是 A (alloc), X (execute)。

接下来的4个字节表示`sh_addr`

```
00 85 04 08
```

Section 存放于进程内存映像中的虚存地址，如果 Section 不需要出现在内存中，则为0。Relocatable file 的虚存地址都为0。Executable file 和 Shared object file 才会为有需要的 Section 计算虚存地址。
地址计算为 00 85 04 08 + little endian = 08 04 85 00

接下来的4个字节为section在文件中的字节偏移量

```
00 05 00 00
```

00 00 05 00 + little endian = 0x500 = 5 * 16^3 = 20480

接下来4个字节表示`sh_size`

表示 Section 在文件中占据的字节数。
类型为 NOBITS 的 Section，即使这个值不为0，它在文件中也不占据空间。".bss" Section 就是这个类型。bss 全称为 “block started by symbol”（更简单的记法是：Better Save Space），存放的是未初始化的数据，因为数据无初始值，所以只需要记录它在内存中占据的空间即可，在文件中不需要额外的存储。相反，".data" Section 存储的则是已经初始化的数据，所以需要在文件中记录下初始值，才能在进程内存映像中把这些值带进去。

```
45 02 00 00
```

45 02 00 00 + little endian = 00 00 02 45 = 2 * 16^2 + 4 * 15 + 5 = 577

紧接着的4个字节表示`sh_link`

包含另外一个 Section Header 的 index，具体的含义取决于 Section 的类型

```
00 00 00 00
```

后面4个字节就表示`sh_info`

包含了额外的信息，具体的含义取决于 Section 的类型。
需要把 sh_type，sh_link 和 sh_info 联合在一起进行解析。
另外，当 sh_flags=SHF_INFO_LINK 时，sh_info 则表示另外一个 Section Header 的 index。

```
00 00 00 00
```

最后8个字节，分别表示`sh_addralign`和`sh_entsize`

前4个字节表示`sh_addralign` 表示对齐约束。0或1表示无约束。sh_addr % sh_addralign 必须等于0

```
10 00 00 00
```

后4个字节表示`sh_entsize`

有些 Section 会包含一组大小固定的记录，这时 sh_entsize 就表示记录的大小，通过 sh_size / sh_entsize 就能得到记录的个数。如果 Section 没有包含这种类型的记录，则值为0。比如后面会看到的 “.shstrtab” Section，包含的是一组 string，但 string 的长度不一样，于是 sh_entsize 的值就为0。

```
00 00 00 00
```

我们再来反复看一下`.text`节

```
  [13] .text             PROGBITS        08048500 000500 000245 00  AX  0   0 16
```

由此可以得出

1. offset = 0x500 = 20480
2. size = 0x0245 = 577
3. flag = AX => 进程运行期间占据内存 + 这部分是可执行的指令 => 可执行

![image-20230624092740552](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624092740552.png)

可以使用objdump查看其中的内容，红框部分便是mian函数反编译后的结果

![image-20230624094139709](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624094139709.png)

- Section .rodata：这些节区包含只读数据，这些数据通常参与

```
[15] .rodata           PROGBITS        08048760 000760 000020 00   A  0   0  4
```

由此可以得出

1. offset = 0x760
2. size = 0x20 = 32
3. flag = A => 进程运行期间占据内存 => 只读

结合程序源代码

<img src="./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624095455793.png" alt="image-20230624095455793" style="zoom:50%;" />

这里的只读数据为输出内容

![image-20230624095711768](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624095711768.png)

- Section .data：这个 Section 存放的是已经初始化了的数据。

```
[23] .data             PROGBITS        0804a020 001020 000004 00  WA  0   0  1
```

由此可以得出

1. offset = 0x1020
2. size = 0x4 = 4
3. flag = WA => 进程运行期间可写 + 进程运行期间占据内存 => 可读写

![image-20230624100218563](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624100218563.png)

- Section .comment：这个 Section 包含了版本控制信息。

```
 [25] .comment          PROGBITS        00000000 001024 00002d 01  MS  0   0  1
```

1. offset = 0x1024
2. size = 0x2d = 2*16 + 14 = 46
3. flag = WA => 进程运行期间可写 + 进程运行期间占据内存 => 可读写

![image-20230624100410881](./S%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624100410881.png)