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

#### 3.1 Magic魔数

每种可执行文件的格式的开头几个字节都是很特殊的，特别是开头4个字节，通常被称为**魔数（Magic Number）**。通过对魔数的判断可以确定文件的格式和类型。如：ELF的可执行文件格式的头4个字节为`0x7F`、`e`、`l`、`f`；Java的可执行文件格式的头4个字节为`c`、`a`、`f`、`e`；如果被执行的是Shell脚本或perl、python等解释型语言的脚本，那么它的第一行往往是`#!/bin/sh`或`#!/usr/bin/perl`或`#!/usr/bin/python`，此时前两个字节`#`和`!`就构成了魔数，系统一旦判断到这两个字节，就对后面的字符串进行解析，以确定具体的解释程序路径。

#### 3.2 ELF文件类型



## 四、代码实现

### 4.1算法思路



### 4.2流程图



### 4.3代码详细解释



## 五、测试用例