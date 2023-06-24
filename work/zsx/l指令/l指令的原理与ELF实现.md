# 选项- l的原理和具体实现

## 选项- l 的介绍

ELF 程序头是对二进制文件中段的描述，是程序装载必须的一部分。

段（segment) 是在内核装载时被解析的。

- 主要作用就是描述磁盘上可执行文件的内存布局以及如何映射到内存中。

- 可以通过引用原始的ELF头中名为： **e_phoff**(程序头表的偏移量)的偏移量来得到程序头表。

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

### PT_INTERP段和LINK段



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



## 样例分析