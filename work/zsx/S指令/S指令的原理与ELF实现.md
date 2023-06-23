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









### 4.2流程图



### 4.3代码详细解释





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

使用`./main test-h.so -S` 查看头文件信息

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









