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
