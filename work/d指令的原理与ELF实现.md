# 选项-d 的原理和具体实现

## 1.选项-d 的介绍

### 1.1.动态节区的数据结构

动态节区表是由Elf32_Dyn表项组成的数组。

```c
typedef struct
{
  Elf32_Sword	d_tag;			/* Dynamic entry type */
  union
    {
      Elf32_Word d_val;			/* Integer value */
      Elf32_Addr d_ptr;			/* Address value */
    } d_un;
} Elf32_Dyn;
```

动态节区表保存了动态链接器所需要的基本信息，比如依赖于哪些共享对象、动态链接符号表的位置、动态链接重定位表的位置、共享对象初始化代码的地址等.

1. d_tag：表明了动态表项的类型。以下是一些常见的类型:
   - DT_SYMTAB：表示动态链接符号表的地址，d_ptr为.dynsym的地址
   - DT_STRTAB：表示动态链接字符串表地址，d_ptr为.dynstr的地址
   - DT_STRSZ：动态链接字符串表大小，d_val为大小
   - DT_NEED：依赖的共享对象文件，d_ptr表示依赖的共享对象文件名
   - DT_REL：动态链接重定位表位置
2. d_val or d_ptr：是一个联合体，要么表示一个值，要么表示一个地址，具体的情况因动态表的类型不同而不同

### 1.2.静态链接

### 1.3.动态链接

## 2.选项-d 的作用

```shell
readelf -d
        --dynamic
```

附加参数-d用于显示动态节区.dynamic的信息。

## 3.选项-d 显示的信息解释

我们根据写的测试样例，使用指令 `./readelf main -d`得到了如下的结果：

```shell
dp@ubuntu:~/Desktop/elf/share$ readelf -d main

Dynamic section at offset 0x2ed8 contains 28 entries:
  Tag        Type                         Name/Value
 0x00000001 (NEEDED)                     Shared library: [lib.so]
 0x00000001 (NEEDED)                     Shared library: [libc.so.6]
 0x0000000c (INIT)                       0x1000
 0x0000000d (FINI)                       0x127c
 0x00000019 (INIT_ARRAY)                 0x3ed0
 0x0000001b (INIT_ARRAYSZ)               4 (bytes)
 0x0000001a (FINI_ARRAY)                 0x3ed4
 0x0000001c (FINI_ARRAYSZ)               4 (bytes)
 0x6ffffef5 (GNU_HASH)                   0x228
 0x00000005 (STRTAB)                     0x2c8
 0x00000006 (SYMTAB)                     0x248
 0x0000000a (STRSZ)                      162 (bytes)
 0x0000000b (SYMENT)                     16 (bytes)
 0x00000015 (DEBUG)                      0x0
 0x00000003 (PLTGOT)                     0x3fd8
 0x00000002 (PLTRELSZ)                   16 (bytes)
 0x00000014 (PLTREL)                     REL
 0x00000017 (JMPREL)                     0x3ec
 0x00000011 (REL)                        0x3ac
 0x00000012 (RELSZ)                      64 (bytes)
 0x00000013 (RELENT)                     8 (bytes)
 0x0000001e (FLAGS)                      BIND_NOW
 0x6ffffffb (FLAGS_1)                    Flags: NOW PIE
 0x6ffffffe (VERNEED)                    0x37c
 0x6fffffff (VERNEEDNUM)                 1
 0x6ffffff0 (VERSYM)                     0x36a
 0x6ffffffa (RELCOUNT)                   4
 0x00000000 (NULL)                       0x0
```

1. Type：表项的类型。
   我们以第一个为例 ` 0x00000001 (NEEDED) Shared library: [lib.so]`，可以得到，这一项表明了对共享库lib.so的依赖，这一点，我们可以通过ldd指令来查看一个可执行程序的共享库依赖，如下：

   ```shell
   dp@ubuntu:~/Desktop/elf/share$ ldd main
   linux-gate.so.1 (0xf7f18000)
   lib.so (0xf7f08000)
   libc.so.6 => /lib32/libc.so.6 (0xf7d01000)
   /lib/ld-linux.so.2 (0xf7f1a000)
   ```

   可以发现，这个可执行程序main依赖于我们自定义的lib.so，lib.so又依赖于stdio(libc.so)。这是由于我们的可执行程序引用了lib.c中的函数func，而函数func又引用了stdio中的printf函数。

   我们再以 `0x00000006 (SYMTAB)  0x248`为例，当我们使用指令 `readelf -S main`输出节区信息，结果如下：

   ```shell
   dp@ubuntu:~/Desktop/elf/share$ readelf -S  main
   There are 31 section headers, starting at offset 0x3800:

   Section Headers:
     [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
     [ 0]                   NULL            00000000 000000 000000 00      0   0  0
     [ 1] .interp           PROGBITS        000001b4 0001b4 000013 00   A  0   0  1
     [ 2] .note.gnu.build-i NOTE            000001c8 0001c8 000024 00   A  0   0  4
     [ 3] .note.gnu.propert NOTE            000001ec 0001ec 00001c 00   A  0   0  4
     [ 4] .note.ABI-tag     NOTE            00000208 000208 000020 00   A  0   0  4
     [ 5] .gnu.hash         GNU_HASH        00000228 000228 000020 04   A  6   0  4
     [ 6] .dynsym           DYNSYM          00000248 000248 000080 10   A  7   1  4
     [ 7] .dynstr           STRTAB          000002c8 0002c8 0000a2 00   A  0   0  1
     [ 8] .gnu.version      VERSYM          0000036a 00036a 000010 02   A  6   0  2
     [ 9] .gnu.version_r    VERNEED         0000037c 00037c 000030 00   A  7   1  4
     [10] .rel.dyn          REL             000003ac 0003ac 000040 08   A  6   0  4
     [11] .rel.plt          REL             000003ec 0003ec 000010 08  AI  6  24  4
     [12] .init             PROGBITS        00001000 001000 000024 00  AX  0   0  4
     [13] .plt              PROGBITS        00001030 001030 000030 04  AX  0   0 16
     [14] .plt.got          PROGBITS        00001060 001060 000010 10  AX  0   0 16
     [15] .plt.sec          PROGBITS        00001070 001070 000020 10  AX  0   0 16
     [16] .text             PROGBITS        00001090 001090 0001e9 00  AX  0   0 16
     [17] .fini             PROGBITS        0000127c 00127c 000018 00  AX  0   0  4
     [18] .rodata           PROGBITS        00002000 002000 000008 00   A  0   0  4
     [19] .eh_frame_hdr     PROGBITS        00002008 002008 000054 00   A  0   0  4
     [20] .eh_frame         PROGBITS        0000205c 00205c 000128 00   A  0   0  4
     [21] .init_array       INIT_ARRAY      00003ed0 002ed0 000004 04  WA  0   0  4
     [22] .fini_array       FINI_ARRAY      00003ed4 002ed4 000004 04  WA  0   0  4
     [23] .dynamic          DYNAMIC         00003ed8 002ed8 000100 08  WA  7   0  4
     [24] .got              PROGBITS        00003fd8 002fd8 000028 04  WA  0   0  4
     [25] .data             PROGBITS        00004000 003000 000008 00  WA  0   0  4
     [26] .bss              NOBITS          00004008 003008 000004 00  WA  0   0  1
     [27] .comment          PROGBITS        00000000 003008 00002b 01  MS  0   0  1
     [28] .symtab           SYMTAB          00000000 003034 000460 10     29  46  4
     [29] .strtab           STRTAB          00000000 003494 000252 00      0   0  1
     [30] .shstrtab         STRTAB          00000000 0036e6 000118 00      0   0  1
   Key to Flags:
     W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
     L (link order), O (extra OS processing required), G (group), T (TLS),
     C (compressed), x (unknown), o (OS specific), E (exclude),
     p (processor specific)

   ```

   可以发现，节区.dynsym的地址0x248与节区头表中的信息一致，进一步验证了正确性。
2. Name/Value：名称或值。与具体的类型相关，当类型为NEEDED时，他的名称就是依赖的动态链接库，当类型的对应的符号表时，值就是动态链接符号表的地址。

## 4.代码实现

### 4.1.算法思路

### 4.2.流程图

### 4.3.测试

我们先编写测试代码。要实现动态链接的方式，我们需要定义库函数，实现打印输出的功能，同时，这个库也使用外部库stdio的printf符号。lib.c代码如下：

```c
#include<stdio.h>
int cnt = 0;
int func(){
        cnt++;
        printf("%d\n",cnt);
}
```

接下来对这段代码进行编译，使用-fPIC指令让其编译为位置无关的代码，作为动态链接库lib.so使用。

```shell
dp@ubuntu:~/Desktop/elf/share$ gcc -shared -fpic -o lib.so lib.c
```

然后我们编写主测试程序，这个程序引用这个动态链接库的cnt变量和func函数。main.c代码如下：

```c
extern void func();
extern int cnt;

int main(){
        func();
}
```

然后进行编译为可执行文件，下面编译中使用库文件，不是为了将库中代码段和数据段合并入可执行文件，而仅仅只是传递符号表和重定位信息告诉可执行文件这里可以进行动态重定位。

```shell
dp@ubuntu:~/Desktop/elf/share$ gcc -m32 -o main main.c lib.so
```

然后执行，执行的时候需要修改环境变量，将当前目录下的动态链接文件添加到环境变量中去。

```shell
dp@ubuntu:~/Desktop/elf/share$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./ 
dp@ubuntu:~/Desktop/elf/share$ ./main
1
```

自此完成了测试程序的编写，上面的解释将基于这个测试程序。

### 4.4.代码详细解释
