# 选项-d 的原理和具体实现

## 1.选项-d 的介绍

## 2.选项-d 的作用

## 3.选项-d 显示的信息解释

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
