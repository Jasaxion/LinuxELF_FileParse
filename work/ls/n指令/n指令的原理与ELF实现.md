# 选项- n的原理和具体实现



## 1 选项- n 的介绍

> 读取注释与版本信息

类型为PT_NOTE的段往往会包含类型为SHT_NOTE的节，SHT_NOTE节可以为目标文件提供一些特别的信息，用于给其它的程序检查目标文件的一致性和兼容性。这些信息我们称为“注释信息”，这样的节称为“注释节(notesection)”，所在的段即为“注释段(note segment)”。注释信息可以包含任意数量的“注释项”，每一个注释项是一个数组，数组的每一个成员大小为4字节，格式依目标处理器而定。下图解释了注释信息是如何组织的：

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624085310551.png" alt="image-20230624085310551" style="zoom:30%;" />

- namesz和name Namesz 和name成对使用。Namesz是一个4字节整数，而name是一个以’null’结尾的字符串。Namesz是name字符串的长度。字符串name的内容是本项的所有者的名字。没有正式的机制来避免名字冲突，一般按照惯例，系统提供商应把他们自己的名字写进name项里，比如”XYZ Computer Company”。如果没有名字的话，namesz是0。由于数组项的大小是向4字节对齐的，所以如果字符串长度不是整4字节的话，需要填0补位。如果有补位的话，namesz只计字符串长度，不计所补的空位。

- descsz和descDescsz和desc也成对使用，它们的格式与namesz/name完全相同。不过，desc的内容没有任何规定、限制，甚至建议，它包含哪些信息完全是自由的。

- type：这个字段给出描述项(desc)的解释，或者说是描述项的类型。每一个提供商都会定义自己的类型，所以同一类型值对于不同的提供商其解释也是不同的。当一个程序读取注释信息的时候，它必须同时辨认出name和type才能理解desc的内容。

注释段的示例：

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624085433248.png" alt="image-20230624085433248" style="zoom:50%;" />

- 这里解释一下注释节的设计逻辑。注释节中包含三种信息：名字(name/namesz)、类型(type)和描述(desc/descsz)。名字用于区别不同的信息提供者，比如不同的厂商，”ABCComputerCompany”或”XYZComputerCompany”。
- 一个信息提供者可能会提供很多种信息，为了区别，把信息分类，即给每一种信息加一个ID，或者说是类型type。比如”ABCComputerCompany”公司规定，1号分类代表这里的信息是一种花的名字，2号分类代表这里的信息是一种香水的名字，……。
- 有了name和type的约束，desc的内容才有意义，才知道它所描述的是什么。比如，如果没有前面的name和type信息的话，如果只知道desc包含一个字符串”Daisy”，你就不知道这到底是表示雏菊花，还是Daisy香水，或者是一个女孩的名字。而如果你知道它的name和type分别是”ABCComputerCompany”和2的话，显然”Daisy”说的是香水。

## 2 选项- n的作用

1. 遍历ELF文件的节头部，查找并处理注释节的信息。
2. 检查ELF文件的版本号，并根据版本号输出相应的版本信息或提示信息。

## 3 选项- n显示的信息解释





## 4 代码实现

### 4.1算法思路



### 4.2 流程图



### 4.3 代码详细解释

#### 4.3.1 主程序判断如下：

> `process_note(file)`：遍历ELF文件的节头部，查找并处理注释节的信息。
>
> `process_version(file)`：检查ELF文件的版本号，并根据版本号输出相应的版本信息或提示信息。

```c++
if(option & (1<<7))                     //-n
    {
        process_note(file); 
    }

if(option & (1<<11))                     //-n
    {
        process_version(file);  //版本
    }
```

#### 4.3.2 `process_note(file)`

> 1. 遍历节头部，查找注释节。
> 2. 如果找到注释节，则设置标记为true。
> 3. 读取注释头部。
> 4. 计算注释名称的指针。
> 5. 打印找到的注释的偏移量和长度。
> 6. 打印注释的所有者、数据大小和描述信息。
> 7. 根据注释类型进行检查，如果适用，打印附加信息。
>    - 如果注释类型为NT_GNU_BUILD_ID，打印构建ID。
>    - 如果注释类型为ELF_NOTE_PAGESIZE_HINT，打印页面大小。
>    - 如果注释类型为NT_GNU_ABI_TAG，打印操作系统和ABI版本。
>    - 如果注释类型为NT_GNU_HWCAP，打印启用或禁用状态。
>    - 如果注释类型为NT_GNU_GOLD_VERSION，打印版本信息。
>    - 如果注释类型为NT_GNU_PROPERTY_TYPE_0，打印属性和特性信息。
> 8. 返回0。

```c++
int ELF_process::process_note(FILE *file) {
    int tag=false;  // 标记是否找到注释
    Elf32_Shdr * shdr;  // 指向节头部的指针
    Elf32_Nhdr * nhdr;  // 指向注释头部的指针
    int i;

    // 遍历节头部
	for(i=0,shdr=section_headers;i < elf_header.e_shnum;i++,shdr++){
        if(shdr->sh_type==SHT_NOTE) // 判断节类型是否为注释节
        {
        	tag=true;   //赋值为true

             // 读取注释头部
            get_32bit_nhdr(file,shdr->sh_offset,shdr->sh_size,nhdr);
        	char * n_name=(char*)nhdr+sizeof(Elf32_Nhdr);   // 计算名称的指针
        	printf("\nDisplaying notes found at file offset 0x%8x with the length 0x%8x\n",
            shdr->sh_offset,shdr->sh_size);
            
            // 打印注释的信息
        	printf("  Owner            Data size   Description\n");
            //        拥有者            数据  大小    描述
        	// printf("  %-18s0x%-10x%s\n",n_name,nhdr->n_descsz,note_type_map(nhdr->n_type,n_name));

            // 打印注释的信息
        	printf("  GNU              0x%-10x%s\n",nhdr->n_descsz,note_type_map(nhdr->n_type,n_name));
            
            // 检查注释类型，如果适用，打印附加信息
            //构建ID
            if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_BUILD_ID)
            {
            	char * n_desc=(char*)(n_name+nhdr->n_namesz);
            	printf("    Build ID: ");
            	for(int j=0;j<nhdr->n_descsz/sizeof(char);j++) printf("%x",(unsigned)(unsigned char)n_desc[j]);
            	printf("\n");
            }
            // 页面大小
            else if(!strcmp(ELF_NOTE_SOLARIS,n_name)&&nhdr->n_type==ELF_NOTE_PAGESIZE_HINT)
            {
            	int * n_desc=(int*)(n_name+nhdr->n_namesz);
            	printf("    Pagesize: 0x%x\n",*n_desc);
            }
            //操作系统 ｜ ABI版本
            else if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_ABI_TAG)
            {
            	int * n_desc=(int*)(n_name+nhdr->n_namesz);
            	printf("    OS: %s, ABI: %d.%d.%d\n",
            			get_ABI_tag(*n_desc)+12,
            			*(n_desc+1),
            			*(n_desc+2),
            			*(n_desc+3)
            		  );
            }
            //启用 ｜ 禁用
            else if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_HWCAP)
            {
            	int * n_desc=(int*)(n_name+nhdr->n_namesz);
            	int num=*n_desc;
            	int mask=*(n_desc+1);
            	n_desc+=2;
            	for(int j=0;j<num;j++)
            	{
            		char * tmp=(char*)n_desc;
            		printf("    %-30s%s\n",
            				tmp+1,
            				(1U << (*tmp)) & mask?"enabled":"disabled"
            		  	  );
            		n_desc+=strlen(tmp);
            	}
            }
            //版本
            else if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_GOLD_VERSION)
            {
            	char * n_desc=(char*)(n_name+nhdr->n_namesz);
            	printf("    Version: %s\n",n_desc);
            }
            //属性：x86 特性：IBT, SHSTK
            else if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_PROPERTY_TYPE_0)
            {
            	char * n_desc=(char*)(n_name+nhdr->n_namesz);
            	printf("    Properties: x86 feature: IBT, SHSTK\n");
            }
        }
    }

    return 0;
}
```



##### `get_32bit_nhdr`

> 主要功能是从ELF文件中获取32位的ELF笔记头，并将获取到的字段值转换为合适的字节顺序。它帮助解析ELF文件中的笔记头信息，并进行必要的转换，以便后续的处理和分析。
>
> 1. 获取32位的ELF笔记头（note header）：函数用于从文件中获取32位的ELF笔记头。
>    - 参数`file`是一个指向文件的指针。
>    - 参数`offset`表示笔记头在文件中的偏移量。
>    - 参数`size`表示笔记头的大小。
>    - 参数`note`是一个指向`Elf32_Nhdr`结构体的指针，用于存储获取到的笔记头。
>
> 2. 获取数据并赋值：通过调用`get_data`函数获取指定偏移量处的数据，数据类型为`Elf32_External_Nhdr`，将获取到的数据赋值给`nhdr`指针。
>    - 如果获取数据失败，返回0。
>
> 3. 转换字节顺序：将`nhdr`中的字段值按照字节顺序进行转换，并赋值给`note`中相应的字段。
>    - 将`external`中的字节顺序转换后的字段值赋值给`internal`的相应字段。
>    - 字段包括`n_namesz`（名称大小）、`n_descsz`（描述大小）和`n_type`（类型）。
>
> 4. 释放内存空间：释放`nhdr`指针指向的内存空间。
>
> 5. 返回结果：返回1表示成功执行。
>

```c++
int ELF_process::get_32bit_nhdr(FILE *file,unsigned int offset,unsigned int size,Elf32_Nhdr *note){
    // 函数用于获取 32 位的 ELF 笔记头（note header）
    // 参数 file 是一个指向文件的指针
    // 参数 offset 表示笔记头在文件中的偏移量
    // 参数 size 表示笔记头的大小
    // 参数 note 是一个指向 Elf32_Nhdr 结构体的指针，用于存储获取到的笔记头
    Elf32_External_Nhdr* nhdr;
    Elf32_External_Nhdr* external;
    Elf32_Nhdr* internal;

    // 通过调用 get_data 函数获取指定偏移量处的数据，数据类型为 Elf32_External_Nhdr
    // 将获取到的数据赋值给 nhdr 指针
    nhdr = (Elf32_External_Nhdr*) get_data(NULL,file,offset,size,1,("note"));

    // 如果获取数据失败，返回 0
    if(!nhdr)
        return 0;
    
    // 将 note 指针赋值给 internal 和 nhdr 指针赋值给 external
    internal = note;
    external = nhdr;

    // 将 external 中的字节顺序转换后的字段值赋值给 internal 相应的字段
    internal->n_namesz  = BYTE_GET(external->n_namesz);
    internal->n_descsz  = BYTE_GET(external->n_descsz);
    internal->n_type    = BYTE_GET(external->n_type);

    // 释放 nhdr 指针指向的内存空间
    free(nhdr);
    return 1;
}
```



#### 4.3.2  ```process_version()```

> 主要功能是检查ELF文件的版本号，并根据版本号输出相应的版本信息或提示信息。
>
> - 帮助开发人员确认文件的版本，并在必要时进行相应的处理。
>
> 1. 检查ELF文件头的版本号：检查`elf_header`中的版本号(`e_version`)。
>    - 如果版本号为1，表示当前版本(EV_CURRENT)。
>    - 如果版本号为0，表示非法版本(EV_NONE)。
>
> 2. 处理当前版本号：如果ELF文件头的版本号为1，表示当前版本。
>    - 打印版本信息：输出字符串"readELF 0.1.1, build time 2021.9.10"。
>    - 返回打印的字符数。
>
> 3. 处理非当前版本号：如果ELF文件头的版本号不为1，表示没有找到版本信息。
>    - 打印提示信息：输出字符串"No version information found in this file."。
>    - 返回打印的字符数。
>

```c++
int ELF_process::process_version(FILE *file){
    // 函数用于处理 ELF 文件的版本信息
    // 参数 file 是一个指向文件的指针
    
    // 检查 ELF 文件头的版本号
    // EV_NONE 表示非法版本，值为 0
    // EV_CURRENT 表示当前版本，值为 1
    if(elf_header.e_version==1){
        // 如果 ELF 文件头的版本号为 1，表示当前版本
        // 打印版本信息并返回输出的字符数
        return printf("readELF 0.1.1, build time 2021.9.10\n");
    }
    
    // 如果 ELF 文件头的版本号不为 1，表示没有找到版本信息
    // 打印相应的提示信息并返回输出的字符数
    return printf("No version information found in this file.\n");
}
```



## 5 测试样例



### 5.1 编写`n-test-1.cpp:`

```
#include <iostream>

using namespace std;

int main(){
  string out="这是关于‘-n’指令的调试";
  for(int num=1; num<=3;num++){
        cout<<out<<"-"<<num<<endl;
  }
  return 0;
}
```

#### 5.1.1 编译产生.so：

```
g++ -o n-test-1.so n-test-1.cpp
```

用`./main n-test-1.so -n`查看、版本信息

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624010600185.png" alt="image-20230624010600185" style="zoom:50%;" />

显示：<font color='red'>There are no sections in this file.</font>。

#### 5.1.2 `32`位编译产生.so：

```
g++ -m32 -o n-test-1.so n-test-1.cpp
```

用`./main n-test-1.so -n`查看、版本信息

显示：<font color='red'>段错误（核心已转储）</font>。

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624010651508.png" alt="image-20230624010651508" style="zoom:50%;" />

### 5.1缺一个含“注释节”信息+版本号信息的测试用例
