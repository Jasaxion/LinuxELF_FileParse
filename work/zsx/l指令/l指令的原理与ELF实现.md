# 选项- l的原理和具体实现

## 选项- l 的介绍

ELF 程序头是对二进制文件中段的描述，是程序装载必须的一部分。

段（segment) 是在内核装载时被解析的。

- 主要作用就是描述磁盘上可执行文件的内存布局以及如何映射到内存中。

- 可以通过引用原始的ELF头中名为： **e_phoff**(程序头表的偏移量)的偏移量来得到程序头表。

基本的几个程序段内容如下：

### PT_LOAD段

p_type描述了段的类型。 一个可执行文件至少要有一个PT_LOAD类型的段。 这类程序头描述的是可装载的段，

也就是说，这种类型的段会被装载或者映射到内存中。

一般来说，一个动态链接的ELF可执行文件通常包含两个可装载的段。 段类型都为PT_LOAD

1. 一个是存放程序代码的text段
2. 另一个是存放全局变量和动态链接信息的data段。

上面两个段则会根据p_align的对齐值在内存中对齐。并且映射到内存中。

 一般来说，TEXT段也称为代码段，权限一般都是可读可执行的。 对应取值就是PF_READ_EXEC

data段一般就是读写权限。 可以修改p_flags来让我们的程序权限增大。

### PT_PHDR段

此段一般位于elf文件的第一个段。PT_PHDR段保存了程序头表本身的位置和大小。 phdr表保存了所有的phdr对文件（以及内存镜像）中段的描述信息。

### PT_INTERP段

PT_INTERP：存放一个以null结尾的字符串位置和大小信息，是对解释器位置的描述；

### PT_DYNAMIC

动态段是动态链接可执行文件所特有的，包含了动态链接器所需要的一些信息，它包括以下内容

- 运行时需要链接的共享库列表
- 全局偏移表(GOT)
- 重定位条目相关信息

### PT_NOTE

可以保存于特定供应商或者系统相关附加信息，这个段在程序运行时是不需要的，因为系统会假设可执行文件是本地的，这个段很容易被感染

## 选项- l 的作用

-l 选项用于显示ELF文件中的程序头表和节区头表信息。

作用:

1. 显示程序头表信息。程序头表描述ELF文件各个段(代码段、数据段等)在文件中的布局信息。

2. 显示节区头表信息。节区头表描述ELF文件各个节区(代码节、数据节等)在文件中的布局信息。
3. 帮助分析ELF文件 segments和sections的组织结构。

## 选项- l显示的信息解释

程序头表显示如下信息：

- p_type：表示程序段的类型信息

  - 标识了段的类型

    主要类型包括PT_LOAD，PT_DYNAMIC，PT_INFERP

    PT_LOAD类型的段会在创建进程时加载到内存中

    PT_INFERP类型的段包含了.interp节，该节提供了加载二进制文件的解释器的名称

    PT_DYNAMIC包含了.dynamic节，该节告诉解释器如何解析二进制文件用于执行

- p_offset：表示分段文件的偏移信息

- p_vaddr：表示段段虚拟地址信息

- p_paddr：表示段物理地址信息

- p_filesz：文件中段的大小

- p_memsz：内存中段段大小

- p_flags：段的标志信息

  - 指示了段在运行时的访问权限，有三种重要的类型：

    PF_X（可执行），PF_W（可写），PF_R（可读）

- p_align：段、文件、内存对齐信息：指定了段所需的内存对齐方式（字节为单位）

提供了二进制文件的段视图，ELF包括零或多个节，实际上就是把这些节捆绑成单个块

**段提供的可执行视图，只有二进制文件会用到**

## 代码实现

### 算法思路

- 数据结构定义「32位」

```cpp
/* Program header */
/* 程序头表 */
typedef struct {
    unsigned char    p_type[4];        /* 标识程序段类型 */
    unsigned char    p_offset[4];        /* 分段文件偏移 */
    unsigned char    p_vaddr[4];        /* 段虚拟地址 */
    unsigned char    p_paddr[4];        /* 段物理地址 */
    unsigned char    p_filesz[4];        /* 文件中的段大小 */
    unsigned char    p_memsz[4];        /* 内存中的段大小 */
    unsigned char    p_flags[4];        /* 段标志 */
    unsigned char    p_align[4];        /* 段对齐、文件和内存 */
} Elf32_External_Phdr;
```

- 数据结构定义「64位」

```cpp
typedef struct {
    unsigned char    p_type[4];        /* 标识程序段类型 */
    unsigned char    p_flags[4];        /* 段标志 */
    unsigned char    p_offset[8];        /* 段文件偏移量 */
    unsigned char    p_vaddr[8];        /* 段虚拟地址 */
    unsigned char    p_paddr[8];        /* 段物理地址 */
    unsigned char    p_filesz[8];        /* 文件中的段大小 */
    unsigned char    p_memsz[8];        /* 内存中的段大小 */
    unsigned char    p_align[8];        /* 段对齐、文件和内存 */
} Elf64_External_Phdr;
```

- 获取程序头表信息

```cpp
int ELF_process::process_program_headers(FILE *file)
{

    Elf32_Phdr* segment;
    unsigned long dynamic_addr;
    if(elf_header.e_phnum == 0)
    {

        if(elf_header.e_phoff!=0)
        {
            printf ("possibly corrupt ELF header - it has a non-zero program"
                    " header offset, but no program headers");
        }
        else
        {
            printf ("\nThere are no program headers in this file.\n");
            return 0;
        }

    }
    else
    {
        /*打印相关的信息*/
        if(elf_header.e_phnum>1)
            printf ("\nProgram Headers:\n");
        else
            printf ("\nProgram Header:\n");

        if(is_32bit_elf)
            printf("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n");
        else
            printf("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n");

    }

    /*获取程序段头表信息*/
    if (! get_program_headers (file))
        return 0;


    unsigned int i;
    for (i = 0, segment = program_headers;
            i < elf_header.e_phnum;
            i++, segment++)
    {
        printf ("  %-14.14s ", get_segment_type (segment->p_type));

        if(is_32bit_elf) /*如果是32位的话，那么输出程序段的相关信息*/
        {
            printf ("0x%6.6x ", (unsigned int) segment->p_offset);
            printf ("0x%8.8x ", (unsigned int) segment->p_vaddr);
            printf ("0x%8.8x ", (unsigned int) segment->p_paddr);
            printf ("0x%5.5x ", (unsigned int) segment->p_filesz);
            printf ("0x%5.5x ", (unsigned int) segment->p_memsz);
            printf ("%c%c%c ",
                    (segment->p_flags & PF_R ? 'R' : ' '),
                    (segment->p_flags & PF_W ? 'W' : ' '),
                    (segment->p_flags & PF_X ? 'E' : ' '));
            printf ("%#x", (unsigned int) segment->p_align);
        }

        printf("\n");

    }

    return 0;
}
```

### 流程图

实现步骤简述：

1. 解析ELF头部,获取程序头表和节区头表的位置和大小。ELF头部包含两个表的起始地址和大小。
2. 读取程序头表,解析每个程序头,打印程序头信息。信息包括类型、偏移、虚拟地址、物理地址、大小等。
3. 读取节区头表,解析每个节区头,打印节区头信息。信息包括名称、类型、地址、偏移、大小等。
4. 根据需要打印额外信息,如节区内容摘要信息等。
5. 如果文件是动态库,还需要解析和打印.dynamic节区中的动态链接信息。
6. 打印编译器版本信息(如果有.comment节区)。

实现的程序流程图如下：

![image-20230624162900808](./l%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624162900808.png)

## 样例分析

针对下面的代码：

<img src="./l%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624112023695.png" alt="image-20230624112023695" style="zoom:50%;" />

```
[zhansx@iZwz9ag8659h7gzr9glt3wZ Mytest]$ ./main test-h -h
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

-l 得到的程序段信息如下：

```
[zhansx@iZwz9ag8659h7gzr9glt3wZ Mytest]$ ./main test-h -l

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  PHDR           0x000034 0x08048034 0x08048034 0x00120 0x00120 R   0x4
  INTERP         0x000154 0x08048154 0x08048154 0x00013 0x00013 R   0x1
  LOAD           0x000000 0x08048000 0x08048000 0x00900 0x00900 R E 0x1000
  LOAD           0x000ee8 0x08049ee8 0x08049ee8 0x0013c 0x001e8 RW  0x1000
  DYNAMIC        0x000ef4 0x08049ef4 0x08049ef4 0x00100 0x00100 RW  0x4
  NOTE           0x000168 0x08048168 0x08048168 0x00044 0x00044 R   0x4
  GNU_EH_FRAME   0x000780 0x08048780 0x08048780 0x0004c 0x0004c R   0x4
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10
  GNU_RELRO      0x000ee8 0x08049ee8 0x08049ee8 0x00118 0x00118 R   0x1
```

Flg标志的含义：

1. R:可读段
2. W:可写段
3. X:可执行段

Align对齐方式的含义：

段对齐,表示段在文件和内存中的对齐方式。常见的对齐方式有:

1. 0x1:1字节对齐

2. 0x2:2字节对齐

3. 0x4:4字节对齐

4. 0x8:8字节对齐

5. 0x10:16字节对齐

   更大的对齐数表示更严格的对齐方式。

   对齐方式会影响段在文件和内存中的起始地址,起始地址会是对齐数的整数倍。

**如上可知，程序段的名字的内容如下：**

- PHDR:程序头表段，描述ELF文件中的程序头表段。
- INTERP:程序解释器名,描述动态链接器的路径名。
- LOAD:可加载段,描述可映射到内存中的段信息。
  - 这里有两个LOAD段,第一个是只读段,第二个是读写段。
- DYNAMIC:动态链接信息,描述动态链接信息。
- NOTE:注释段,包含一些注释信息。
- GNU_EH_FRAME:异常帧信息,包含堆栈中的异常处理信息。
- GNU_STACK:堆栈段,标记堆栈的可执行性。
- GNU_RELRO:只读数据段,标记的数据段中的只读部分。