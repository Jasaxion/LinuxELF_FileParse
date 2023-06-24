# 选项- n的原理和具体实现

## 1 选项- n 的介绍

> 读取注释与版本信息。

类型为PT_NOTE的段往往会包含类型为SHT_NOTE的节，SHT_NOTE节可以为目标文件提供一些特别的信息，用于给其它的程序检查目标文件的一致性和兼容性。这些信息我们称为“注释信息”，这样的节称为“注释节(notesection)”，所在的段即为“注释段(note segment)”。注释信息可以包含任意数量的“注释项”，每一个注释项是一个数组，数组的每一个成员大小为4字节，格式依目标处理器而定。下图解释了注释信息是如何组织的：

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624085310551.png" alt="image-20230624085310551" style="zoom:30%;" />

- namesz和name Namesz 和name成对使用。Namesz是一个4字节整数，而name是一个以’null’结尾的字符串。Namesz是name字符串的长度。字符串name的内容是本项的所有者的名字。没有正式的机制来避免名字冲突，一般按照惯例，系统提供商应把他们自己的名字写进name项里，比如”XYZ Computer Company”。如果没有名字的话，namesz是0。由于数组项的大小是向4字节对齐的，所以如果字符串长度不是整4字节的话，需要填0补位。如果有补位的话，namesz只计字符串长度，不计所补的空位。

- descsz和descDescsz和desc也成对使用，它们的格式与namesz/name完全相同。不过，desc的内容没有任何规定、限制，甚至建议，它包含哪些信息完全是自由的。

- type：这个字段给出描述项(desc)的解释，或者说是描述项的类型。每一个提供商都会定义自己的类型，所以同一类型值对于不同的提供商其解释也是不同的。当一个程序读取注释信息的时候，它必须同时辨认出name和type才能理解desc的内容。

注释段的示例：

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624085433248.png" alt="image-20230624085433248" style="zoom: 33%;" />

- 注释节的设计逻辑，注释节中包含三种信息，名字(name/namesz)、类型(type)和描述(desc/descsz)。
  - 名字用于区别不同的信息提供者，比如不同的厂商。
  - 一个信息提供者可能会提供很多种信息，为了区别，把信息分类，即给每一种信息加一个ID，或者说是类型type。
  - 有了name和type的约束，desc的内容才有意义，才知道它所描述的是什么。


## 2 选项- n的作用

1. 遍历ELF文件的节头部，查找并处理注释节的信息。
2. 检查ELF文件的版本号，并根据版本号输出相应的版本信息或提示信息。

## 3 选项- n显示的信息解释

1. 打印注释的来源。
2. 打印注释的所有者、数据大小和描述信息。
3. 根据注释类型进行检查，如果适用，打印附加信息。
   - 如果注释类型为NT_GNU_BUILD_ID，打印构建ID。
   - 如果注释类型为ELF_NOTE_PAGESIZE_HINT，打印页面大小。
   - 如果注释类型为NT_GNU_ABI_TAG，打印操作系统OS和ABI版本。
   - 如果注释类型为NT_GNU_HWCAP，打印启用或禁用状态。
   - 如果注释类型为NT_GNU_GOLD_VERSION，打印版本信息。
   - 如果注释类型为NT_GNU_PROPERTY_TYPE_0，打印属性和特性信息。

如测试用例显示的信息：

```
Displaying notes found in: .note.ABI-tag
  所有者             Data size	Description
  GNU                  0x00000010	NT_GNU_ABI_TAG (ABI version tag)
    OS: Linux, ABI: 3.2.0

Displaying notes found in: .note.gnu.build-id
  所有者             Data size	Description
  GNU                  0x00000014	NT_GNU_BUILD_ID (unique build ID bitstring)
    Build ID: 175854961c4e4fed11f3854059093c8666cc0e00

Displaying notes found in: .gnu.build.attributes
  所有者             Data size	Description
  GA$<version>3p1113   0x00000010	OPEN
    Applies to region from 0x40074f to 0x40074f (.annobin_init.c)
  GA$<tool>running gcc 0x00000000	OPEN
    Applies to region from 0x40074f
```

在节 ".note.ABI-tag" 中找到的注释：

- 所有者（Owner）：GNU
- 数据大小（Data size）：0x00000010
- 描述（Description）：NT_GNU_ABI_TAG（ABI版本标签）
- 操作系统（OS）：Linux，ABI版本：3.2.0

在节 ".note.gnu.build-id" 中找到的注释：

- 所有者（Owner）：GNU
- 数据大小（Data size）：0x00000014
- 描述（Description）：NT_GNU_BUILD_ID（唯一的构建ID位串）
- 构建ID（Build ID）：175854961c4e4fed11f3854059093c8666cc0e00

在节 ".gnu.build.attributes" 中找到的注释：

- 所有者（Owner）：GA$<version>3p1113
- 数据大小（Data size）：0x00000010
- 描述（Description）：OPEN
- 适用范围：从0x40074f到0x40074f的区域（.annobin_init.c）

## 4 代码实现

### 4.1算法思路

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
> 8. 返回。

### 4.2 流程图

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624151357162.png" alt="image-20230624151357162" style="zoom:67%;" />

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



##### 4.3.2.1 `get_32bit_nhdr`

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

##### 4.3.2.2 `get_data`

> 从硬盘中的文件中提取数据并存储到内存中的变量中。
>
> 1. 函数`ELF_process::get_data`用于将硬盘中文件的数据提取到内存中的变量。
> 2. 函数参数说明：
>    - `var`：存放数据的变量地址。
>    - `file`：要读取的文件。
>    - `offset`：被读取数据的偏移地址。
>    - `size`：每个元素的大小。
>    - `nmemb`：一共有多少个这样的元素。
>    - `reason`：读取的原因，便于报错。
> 3. `mvar`是一个临时变量，用于保存地址。
> 4. 如果每个元素的大小为0或者元素个数为0，则返回`NULL`表示读取空。
> 5. 将文件的读取指针移动到偏移地址处。如果移动失败，则返回`NULL`表示读取空。
> 6. 将`mvar`设置为`var`，即将存放数据的变量地址赋给`mvar`。
> 7. 如果用户的变量为空（`var`为`NULL`），则需要申请内存空间。
>    - 检查是否会溢出，即将要读取的元素个数乘以大小是否大于地址空间可以存放的最大元素个数。
>    - 如果不会溢出，分配`size * nmemb + 1`个字节的内存空间。
>    - 如果分配失败，返回`NULL`表示读取空。
>    - 如果分配成功，将最后一个字节设置为`'\0'`，作为字符串结束符。
> 8. 从文件的读取指针处读取`nmemb`个大小为`size`的数据到`mvar`指向的内存空间中。
>    - 如果实际读取的元素个数与期望读取的元素个数不一致，表示读取异常。
>    - 如果`mvar`和`var`指向的地址不一样，释放`mvar`指向的内存空间，避免内存泄漏。
>    - 返回`NULL`表示读取空。
> 9. 读取完成后，成功返回复制到内存中的数据，即`mvar`的地址。

```c++
//将硬盘中文件中的数据提取到内存中的变量
void * ELF_process::get_data (void * var, FILE * file, long offset, size_t size, size_t nmemb,const char * reason)
{
    //var：存放数据的变量地址
    //file：读取的文件
    //offset：被读取的数据的偏移地址
    //size：每个元素的大小
    //nmemb：一共有多少个这样的元素
    //reason：读取的原因，便于报错


    //临时变量保存地址
    void * mvar;

    //如果每个元素的大小为0，或者有0个元素，那么读空
    if (size == 0 || nmemb == 0)
        return NULL;

    //将文件的读取指针移动到偏移地址处
    if (fseek (file, offset, SEEK_SET))
    {
        //error (_("Unable to seek to 0x%lx for %s\n"),
        //  (unsigned long) archive_file_offset + offset, reason);
        //若失败则返回读空
        return NULL;
    }

    mvar = var;
    //判断用户的变量是不是为空，如果为空，那么需要申请空间
    if (mvar == NULL)
    {
        /* Check for overflow.  */
        //检查是否溢出，即将要读取的元素个数大于等于了地址空间全部存放这个元素时的最大个数
        if (nmemb < (~(size_t) 0 - 1) / size)
            /* + 1 so that we can '\0' terminate invalid string table sections.  */
            //如果没有溢出，那么就申请 元素大小*元素个数+1 的内存空间
            //+1是为了最后添加'/0'给字符串节区用
            mvar = malloc (size * nmemb + 1);

        //如果此时变量指针还是空的，说明没有成功申请到内存，所以返回读空
        if (mvar == NULL)
        {
            //error (_("Out of memory allocating 0x%lx bytes for %s\n"),
            //(unsigned long)(size * nmemb), reason);
            return NULL;
        }

        //申请成功的话，将最后一位（+1的那个字节）设置为'\0'字符串结束符
        ((char *) mvar)[size * nmemb] = '\0';
    }

    //从文件读取指针处读取nmemb个size大小的数据到mvar所指向的内存空间中
    if (fread (mvar, size, nmemb, file) != nmemb)
    {
        //若实际读取的元素个数和想要读取的元素个数不一致，那么就异常
        //error (_("Unable to read in 0x%lx bytes of %s\n"),
        // (unsigned long)(size * nmemb), reason);
        //如果两个指针所指向的地址不一样，那么就释放内存，避免内存泄漏
        if (mvar != var)
            free (mvar);
        //返回读空
        return NULL;
    }

    //读取完毕，成功返回复制到内存中的数据
    return mvar;
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

```c++
#include <iostream>

using namespace std;

template<typename T>
T add(T a,T b){
  return a+b;
}

int main(){
  int t=add(1,2);
  cout<<t<<endl;
  return 0;
}
```

### 5.2 结果分析：

<img src="n%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624161154185.png" alt="image-20230624161154185" style="zoom: 50%;" />

在节 ".note.ABI-tag" 中找到的注释：

- 所有者（Owner）：GNU
- 数据大小（Data size）：0x00000010
- 描述（Description）：NT_GNU_ABI_TAG（ABI版本标签）
- 操作系统（OS）：Linux，ABI版本：3.2.0

在节 ".note.gnu.build-id" 中找到的注释：

- 所有者（Owner）：GNU
- 数据大小（Data size）：0x00000014
- 描述（Description）：NT_GNU_BUILD_ID（唯一的构建ID位串）
- 构建ID（Build ID）：175854961c4e4fed11f3854059093c8666cc0e00

在节 ".gnu.build.attributes" 中找到的注释：

- 所有者（Owner）：GA$<version>3p1113
- 数据大小（Data size）：0x00000010
- 描述（Description）：OPEN
- 适用范围：从0x40074f到0x40074f的区域（.annobin_init.c）
