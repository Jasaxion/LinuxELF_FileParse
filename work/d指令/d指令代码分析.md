

## 4 代码实现

### 4.1算法思路

1. 定义一个名为`ELF_process`的类的成员函数`process_dynamic_section`，参数为一个文件指针`file`。
2. 声明一个指向`Elf32_Dyn`结构的指针`entry`。
3. 如果`dynamic_addr`非空（即存在动态节）：
    - 打印动态节的偏移地址和条目数。
    - 打印动态节的表头。
4. 否则，说明文件中没有动态节，打印相应的提示信息并返回0。
5. 如果是32位ELF文件：
    - 调用`get_32bit_dynamic_section`函数从文件中获取32位动态节。
    - 如果获取失败，返回0。
6. 如果是64位ELF文件：
    - 调用`get_64bit_dynamic_section`函数从文件中获取64位动态节。
    - 如果获取失败，返回0。
7. 进入循环，遍历动态节的每个条目，指针`entry`从`dynamic_section`开始，直到`entry`指向的条目超过动态节的最后一个条目。
    - 声明一个指向字符常量的指针`dtype`、输出一个空格。
    - 打印当前条目的标签。
    - 通过调用`get_dynamic_type`函数获取条目的动态类型，并将结果赋给`dtype`。
    - 打印动态类型，并根据类型的长度进行对齐。
    - 根据当前条目的标签进行判断：
        - 如果标签是`DT_FLAGS`，调用`print_dynamic_flags`函数打印动态标志。
        - 如果标签是`DT_AUXILIARY`、`DT_FILTER`、`DT_CONFIG`、`DT_DEPAUDIT`或`DT_AUDIT`，根据不同的标签打印对应的信息。
        - 否则，打印当前条目的值。
    - 换行。
9. 函数执行完毕。

### 4.2 流程图

![image-20230624210323744](d%E6%8C%87%E4%BB%A4%E4%BB%A3%E7%A0%81%E5%88%86%E6%9E%90.assets/image-20230624210323744.png)

### 4.3 代码详细解释

#### 4.3.1 主程序判断如下：

```c++
if(option & (1<<10))                    //-d
    {
        process_dynamic_section(file);
    }
```

#### 4.3.2 `process_dynamic_section(file)`

> 函数功能如下：
>
> 1. 定义一个名为`ELF_process`的类的成员函数`process_dynamic_section`，参数为一个文件指针`file`。
> 2. 声明一个指向`Elf32_Dyn`结构的指针`entry`。
> 3. 如果`dynamic_addr`非空（即存在动态节）：
>     - 打印动态节的偏移地址和条目数。
>     - 打印动态节的表头。
> 4. 否则，说明文件中没有动态节，打印相应的提示信息并返回0。
> 5. 如果是32位ELF文件：
>     - 调用`get_32bit_dynamic_section`函数从文件中获取32位动态节。
>     - 如果获取失败，返回0。
> 6. 如果是64位ELF文件：
>     - 调用`get_64bit_dynamic_section`函数从文件中获取64位动态节。
>     - 如果获取失败，返回0。
> 7. 进入循环，遍历动态节的每个条目，指针`entry`从`dynamic_section`开始，直到`entry`指向的条目超过动态节的最后一个条目。
> 8. 在循环内部：
>     - 声明一个指向字符常量的指针`dtype`。
>     - 输出一个空格。
>     - 打印当前条目的标签。
>     - 通过调用`get_dynamic_type`函数获取条目的动态类型，并将结果赋给`dtype`。
>     - 打印动态类型，并根据类型的长度进行对齐。
>     - 根据当前条目的标签进行判断：
>         - 如果标签是`DT_FLAGS`，调用`print_dynamic_flags`函数打印动态标志。
>         - 如果标签是`DT_AUXILIARY`、`DT_FILTER`、`DT_CONFIG`、`DT_DEPAUDIT`或`DT_AUDIT`，根据不同的标签打印对应的信息。
>         - 否则，打印当前条目的值。
>     - 换行。

```c++
int ELF_process::process_dynamic_section(FILE *file)
{
    // 定义指向Elf32_Dyn结构的指针entry
    Elf32_Dyn * entry;

	if(dynamic_addr) // 如果dynamic_addr非空
    {
        // 打印动态节的偏移地址和条目数
        printf ("\nDynamic section at offset 0x%x contains %u entries:\n",
                dynamic_addr, dynamic_nent);
        // 打印表头
        printf ("  Tag        Type                         Name/Value\n");
    }else{
        // 文件中没有动态节
    	printf ("\nThere are no dynamic section in this file.\n");
    	return 0;
    }
    // 如果是32位ELF文件
    if (is_32bit_elf)
    {
        // 调用get_32bit_dynamic_section函数获取32位动态节
        if (! get_32bit_dynamic_section (file))
            return 0;
    }
        // 如果是64位ELF文件，调用get_64bit_dynamic_section函数获取64位动态节
    else if (! get_64bit_dynamic_section (file))
        return 0;

    // 遍历动态节的每一条目
    for (entry = dynamic_section;
            entry < dynamic_section + dynamic_nent;
            entry++)
    {
        const char * dtype; // 定义记录条目的动态类型的指针
        // 输出空格
        putchar (' ');
        printf("0x%2.8x ",entry->d_tag);    // 打印条目的标签
        dtype = get_dynamic_type(entry->d_tag); // 获取条目的动态类型
        // 打印动态类型
        printf("(%s)%*s",dtype,(int)(27-strlen(dtype))," ");

        // 根据条目的标签进行判断
        switch (entry->d_tag)
        {
        case DT_FLAGS:  // 如果标签是DT_FLAGS
            // 调用print_dynamic_flags函数打印动态标志
            print_dynamic_flags (entry->d_un.d_val);
            break;

        case DT_AUXILIARY:
        case DT_FILTER:
        case DT_CONFIG:
        case DT_DEPAUDIT:
        case DT_AUDIT:  // 如果标签是DT_AUXILIARY、DT_FILTER、DT_CONFIG、DT_DEPAUDIT或DT_AUDIT
            switch (entry->d_tag)   // 根据标签进行判断
            {
            case DT_AUXILIARY:
                printf ("Auxiliary library");    // 辅助库
                break;

            case DT_FILTER:
                printf ("Filter library");  // 过滤器库
                break;

            case DT_CONFIG:
                printf ("Configuration file");  // 配置文件
                break;

            case DT_DEPAUDIT:
                printf ("Dependency audit library");    // 依赖审核库
                break;

            case DT_AUDIT:
                printf ("Audit library");   // 审计库
                break;
            }
            break;

        default:
            printf("0x%x",entry->d_un.d_val); // 默认情况下打印条目的值
        }

        printf("\n");
    }
}

```

##### 4.3.2.1 `get_32bit_dynamic_section`

> 函数功能如下下：
>
> 1. 定义一个名为`ELF_process`的类的成员函数`get_32bit_dynamic_section`，参数为一个文件指针`file`。
> 2. 分配内存，用于存储32位动态节的外部表示，将其转换为`Elf32_External_Dyn`类型的指针`edyn`。
> 3. 定义一个指向32位动态节外部表示的指针`ext`、一个指向32位动态节的指针`entry`。
> 5. 将文件指针定位到动态节的偏移地址处。
> 6. 从文件中读取32位动态节数据到`edyn`内存块中。
> 7. 如果`edyn`为空，说明内存分配失败，return 0。
> 8. 遍历32位动态节的每个外部表示：
>     - 动态节条目数加1。
>     - 如果外部表示的标签为`DT_NULL`，表示动态节结束，跳出循环。
> 9. 分配内存，用于存储32位动态节的内部表示，大小为动态节条目数乘以`Elf32_Dyn`结构的大小。
> 10. 如果动态节内部表示的内存分配失败：
>     - 打印内存不足的提示信息。
>     - 释放`edyn`的内存。
>     - 返回0。
> 11. 遍历动态节的每个条目，进行内外部表示的转换：
>     - 将外部表示的标签转换为内部表示。
>     - 将外部表示的值转换为内部表示。
> 12. 释放`edyn`的内存。
> 13. 返回1，表示成功获取32位动态节。
>

```c++
int ELF_process::get_32bit_dynamic_section(FILE *file)
{
    // 分配内存，用于存储32位动态节的外部表示
    Elf32_External_Dyn * edyn = (Elf32_External_Dyn *) malloc(dynamic_size);
    // 定义指向32位动态节外部表示的指针
    Elf32_External_Dyn * ext;
    // 定义指向32位动态节的指针
    Elf32_Dyn * entry;

    // 将文件指针定位到动态节的偏移地址处
    fseek(file,dynamic_addr,SEEK_SET);
    // 从文件中读取32位动态节数据到edyn内存块中
    fread(edyn,dynamic_size,1,file);

    //如果edyn为空，说明分配内存失败
    if(edyn==NULL)
        return 0;

    // 遍历32位动态节的每个外部表示
    for (ext = edyn, dynamic_nent = 0;
            (char *) ext < (char *) edyn + dynamic_size;
            ext++)
    {
        // 动态节条目数加1
        dynamic_nent++;
        // 如果外部表示的标签为DT_NULL，表示结束
        if (BYTE_GET (ext->d_tag) == DT_NULL)
            break;
    }
    // 分配内存，用于存储32位动态节的内部表示
    dynamic_section = (Elf32_Dyn *) cmalloc (dynamic_nent,
                      sizeof (* entry));

    // 如果动态节内部表示的内存分配失败
    if (dynamic_section == NULL)
    {
        printf("Out of memory\n");  // 打印内存不足
        free (edyn);    // 释放edyn的内存
        return 0;
    }

    // 遍历动态节的每个条目，进行内外部表示的转换
    for (ext = edyn, entry = dynamic_section;
            entry < dynamic_section + dynamic_nent;
            ext++, entry++)
    {
        entry->d_tag      = BYTE_GET (ext->d_tag);  // 将外部表示的标签转换为内部表示
        entry->d_un.d_val = BYTE_GET (ext->d_un.d_val); // 将外部表示的值转换为内部表示
    }

    free(edyn); // 释放edyn的内存

    return 1;
}
```

##### 4.3.2.2 `BYTE_GET(field)`：小端转大端

即`byte_get_little_endian (unsigned char *field, int size)`

> 小端转大端：该函数根据输入的字长度，对字段中的字节进行转换，将其从小端方式转换为大端方式，并返回转换后的值。
>
> 1. 定义函数 `byte_get_little_endian`，该函数用于按照小端方式对字进行转换读取，并将其转换为大端方式。
> 2. 接收参数 `field`，这是一个指向无符号字符的指针，表示要进行转换的字字段。
> 3. 接收参数 `size`，表示要转换的字的长度。
> 4. 使用 `switch` 语句根据字的长度执行相应的转换操作。
> 5. 如果字的长度为 1，直接返回字段的值。
> 6. 如果字的长度为 2，将字段中的两个字节进行转换为大端方式，并返回转换后的值。
> 7. 如果字的长度为 3，将字段中的三个字节进行转换为大端方式，并返回转换后的值。
> 8. 如果字的长度为 4，将字段中的四个字节进行转换为大端方式，并返回转换后的值。

```c++
//按照小端方式对字进行转换读取，转换为大端方式
int byte_get_little_endian (unsigned char *field, int size)
{
    //依据字的长度进行转换
    switch (size)
    {
    case 1:
        // 如果字长度为1，则直接返回字段的值
        return *field;
    case 2: 
        // 如果字长度为2，则将字段中的两个字节进行转换为大端方式，并返回转换后的值
        return ((unsigned int)(field[0]))
               | (((unsigned int)(field[1])) << 8);
    case 3:
        // 如果字长度为3，则将字段中的三个字节进行转换为大端方式，并返回转换后的值
        return  ((unsigned long) (field[0]))
                |    (((unsigned long) (field[1])) << 8)
                |    (((unsigned long) (field[2])) << 16);

    case 4:
        // 如果字长度为4，则将字段中的四个字节进行转换为大端方式，并返回转换后的值
        return  ((unsigned long) (field[0]))
                |    (((unsigned long) (field[1])) << 8)
                |    (((unsigned long) (field[2])) << 16)
                |    (((unsigned long) (field[3])) << 24);
    }

}
```

##### 4.3.2.3 `get_dynamic_type(unsigned long type)`

> 该函数根据给定的动态类型值（`type`），返回相应的类型字符串。每个`case`语句对应不同的动态类型，当输入的`type`与某个动态类型匹配时，返回对应的字符串表示。如果输入的`type`没有匹配的情况，最后返回`NULL`。注释中的字符串表示对应的动态类型名称。
>
> | 1. 基本类型（Base Types）：  | - `DT_NULL`   - `DT_NEEDED`<br/>   - `DT_PLTRELSZ`   - `DT_PLTGOT`<br/>   - `DT_HASH`   - `DT_STRTAB`<br/>   - `DT_SYMTAB`   - `DT_RELA`<br/>   - `DT_RELASZ`   - `DT_RELAENT`<br/>   - `DT_STRSZ`   - `DT_SYMENT`<br/>   - `DT_INIT`   - `DT_FINI`<br/>   - `DT_SONAME`   - `DT_RPATH`<br/>   - `DT_SYMBOLIC`   - `DT_REL`<br/>   - `DT_RELSZ`   - `DT_RELENT`<br/>   - `DT_PLTREL`   - `DT_DEBUG`<br/>   - `DT_TEXTREL`   - `DT_JMPREL`<br/>   - `DT_BIND_NOW`   - `DT_INIT_ARRAY`<br/>   - `DT_FINI_ARRAY`   - `DT_INIT_ARRAYSZ`<br/>   - `DT_FINI_ARRAYSZ`   - `DT_RUNPATH`<br/>   - `DT_FLAGS` |
> | ---------------------------- | ------------------------------------------------------------ |
> | 扩展类型（Extended Types）： | - `DT_PREINIT_ARRAY` - `DT_PREINIT_ARRAYSZ`<br/>   - `DT_CHECKSUM`   - `DT_PLTPADSZ`<br/>   - `DT_MOVEENT`   - `DT_MOVESZ`<br/>   - `DT_FEATURE`   - `DT_POSFLAG_1`<br/>   - `DT_SYMINSZ`   - `DT_SYMINENT`<br/>   - `DT_ADDRRNGLO`   - `DT_CONFIG`<br/>   - `DT_DEPAUDIT`   - `DT_AUDIT`<br/>   - `DT_PLTPAD`   - `DT_MOVETAB`<br/>   - `DT_SYMINFO`   - `DT_VERSYM`<br/>   - `DT_TLSDESC_GOT`   - `DT_TLSDESC_PLT`<br/>   - `DT_RELACOUNT`   - `DT_RELCOUNT`<br/>   - `DT_FLAGS_1`   - `DT_VERDEF`<br/>   - `DT_VERDEFNUM`   - `DT_VERNEED`<br/>   - `DT_VERNEEDNUM`   - `DT_AUXILIARY`<br/>   - `DT_USED`   - `DT_FILTER`<br/>   - `DT_GNU_PRELINKED`   - `DT_GNU_CONFLICT`<br/>   - `DT_GNU_CONFLICTSZ`   - `DT_GNU_LIBLIST`<br/>   - `DT_GNU_LIBLISTSZ`   - `DT_GNU_HASH`<br/> |

```c++
const char *ELF_process::get_dynamic_type(unsigned long type)
{

    static char buff[64];// 静态字符数组，用于存储返回的类型字符串

    switch (type)	//判断传入的标志type并输出
    {

    case DT_NULL:
        return "NULL";
    case DT_NEEDED:
        return "NEEDED";
    case DT_PLTRELSZ:
        return "PLTRELSZ";
    case DT_PLTGOT:
        return "PLTGOT";
    case DT_HASH:
        return "HASH";
    case DT_STRTAB:
        return "STRTAB";
    case DT_SYMTAB:
        return "SYMTAB";
    case DT_RELA:
        return "RELA";
    case DT_RELASZ:
        return "RELASZ";
    case DT_RELAENT:
        return "RELAENT";
    case DT_STRSZ:
        return "STRSZ";
    case DT_SYMENT:
        return "SYMENT";
    case DT_INIT:
        return "INIT";
    case DT_FINI:
        return "FINI";
    case DT_SONAME:
        return "SONAME";
    case DT_RPATH:
        return "RPATH";
    case DT_SYMBOLIC:
        return "SYMBOLIC";
    case DT_REL:
        return "REL";
    case DT_RELSZ:
        return "RELSZ";
    case DT_RELENT:
        return "RELENT";
    case DT_PLTREL:
        return "PLTREL";
    case DT_DEBUG:
        return "DEBUG";
    case DT_TEXTREL:
        return "TEXTREL";
    case DT_JMPREL:
        return "JMPREL";
    case DT_BIND_NOW:
        return "BIND_NOW";
    case DT_INIT_ARRAY:
        return "INIT_ARRAY";
    case DT_FINI_ARRAY:
        return "FINI_ARRAY";
    case DT_INIT_ARRAYSZ:
        return "INIT_ARRAYSZ";
    case DT_FINI_ARRAYSZ:
        return "FINI_ARRAYSZ";
    case DT_RUNPATH:
        return "RUNPATH";
    case DT_FLAGS:
        return "FLAGS";

    case DT_PREINIT_ARRAY:
        return "PREINIT_ARRAY";
    case DT_PREINIT_ARRAYSZ:
        return "PREINIT_ARRAYSZ";

    case DT_CHECKSUM:
        return "CHECKSUM";
    case DT_PLTPADSZ:
        return "PLTPADSZ";
    case DT_MOVEENT:
        return "MOVEENT";
    case DT_MOVESZ:
        return "MOVESZ";
    case DT_FEATURE:
        return "FEATURE";
    case DT_POSFLAG_1:
        return "POSFLAG_1";
    case DT_SYMINSZ:
        return "SYMINSZ";
    case DT_SYMINENT:
        return "SYMINENT"; /* aka VALRNGHI */

    case DT_ADDRRNGLO:
        return "ADDRRNGLO";
    case DT_CONFIG:
        return "CONFIG";
    case DT_DEPAUDIT:
        return "DEPAUDIT";
    case DT_AUDIT:
        return "AUDIT";
    case DT_PLTPAD:
        return "PLTPAD";
    case DT_MOVETAB:
        return "MOVETAB";
    case DT_SYMINFO:
        return "SYMINFO"; /* aka ADDRRNGHI */

    case DT_VERSYM:
        return "VERSYM";

    case DT_TLSDESC_GOT:
        return "TLSDESC_GOT";
    case DT_TLSDESC_PLT:
        return "TLSDESC_PLT";
    case DT_RELACOUNT:
        return "RELACOUNT";
    case DT_RELCOUNT:
        return "RELCOUNT";
    case DT_FLAGS_1:
        return "FLAGS_1";
    case DT_VERDEF:
        return "VERDEF";
    case DT_VERDEFNUM:
        return "VERDEFNUM";
    case DT_VERNEED:
        return "VERNEED";
    case DT_VERNEEDNUM:
        return "VERNEEDNUM";

    case DT_AUXILIARY:
        return "AUXILIARY";
    case DT_USED:
        return "USED";
    case DT_FILTER:
        return "FILTER";

    case DT_GNU_PRELINKED:
        return "GNU_PRELINKED";
    case DT_GNU_CONFLICT:
        return "GNU_CONFLICT";
    case DT_GNU_CONFLICTSZ:
        return "GNU_CONFLICTSZ";
    case DT_GNU_LIBLIST:
        return "GNU_LIBLIST";
    case DT_GNU_LIBLISTSZ:
        return "GNU_LIBLISTSZ";
    case DT_GNU_HASH:
        return "GNU_HASH";


    }

    return NULL;
}
```

##### 4.3.2.4 `print_dynamic_flags(Elf32_Word flags)`

> 该函数用于打印 ELF 文件中的动态标志（dynamic flags），用于提供关于 ELF 文件的特性和属性的信息。它接收一个 `Elf32_Word` 类型的标志参数 `flags`。
>
> 函数使用一个 `while` 循环来处理标志，直到所有的标志都被处理完。在每次循环迭代中，它从 `flags` 中提取出最低位的标志，然后通过按位与操作清除该标志位。
>
> 在处理每个标志时，函数根据标志的值进行 `switch` 判断，输出相应的字符串表示该标志的含义。其中包括了一些已知的标志，如 `ORIGIN`、`SYMBOLIC`、`TEXTREL`、`BIND_NOW`、`STATIC_TLS`，对应的字符串会输出到标准输出中。
>
> 如果遇到未知的标志，将输出字符串"unknown"表示未知标志。

```c++
void ELF_process::print_dynamic_flags(Elf32_Word flags)
{

    int first = 1;

    while (flags)
    {
        Elf32_Word flag;    // 当前处理的标志

        flag = flags & - flags; // 提取最低位的标志
        flags &= ~ flag;    // 清除已处理的标志位

        if (first)
            first = 0;
        else
            putc (' ', stdout);     // 在标志之间输出空格

        switch (flag)
        {
        case DF_ORIGIN:
            fputs ("ORIGIN", stdout);   // 输出"ORIGIN"
            break;
        case DF_SYMBOLIC:
            fputs ("SYMBOLIC", stdout);   // 输出"SYMBOLIC"
            break;
        case DF_TEXTREL:
            fputs ("TEXTREL", stdout);   // 输出"TEXTREL"
            break;
        case DF_BIND_NOW:
            fputs ("BIND_NOW", stdout);    // 输出"BIND_NOW"
            break;
        case DF_STATIC_TLS:
            fputs ("STATIC_TLS", stdout);   // 输出"STATIC_TLS"
            break;
        default:
            fputs (("unknown"), stdout);    // 输出"unknown"表示未知标志
            break;
        }
    }

}
```



