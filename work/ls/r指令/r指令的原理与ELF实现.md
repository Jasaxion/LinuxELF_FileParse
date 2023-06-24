# 选项- r的原理和具体实现

## 1 选项- r的介绍

重定位是将符号引用与符号定义进行连接的过程。例如，当程序调用了一个函数时，相关的调用指令必须把控制传输到适当的目标执行地址。

### 1.1 重定位表项

重定位项可具有以下结构。请参见 `elf.h`。

```c++
// 无加号的重定位表项(在类型为SHT_REL的section中)。
typedef struct
{
  Elf32_Addr	r_offset;		/* 地址 */
  Elf32_Word	r_info;			/* 重定位类型和符号索引*/
} Elf32_Rel;

/*Elf64_Rel和Elf64_Rela结构的两种不同的定义，直到Novell(或任何人)得到他们的行动。*/
/*以下至少在Sparc v9, MIPS和Alpha上使用。*/
typedef struct
{
  Elf64_Addr	r_offset;		/* 地址 */
  Elf64_Xword	r_info;			/* 重定位类型和符号索引 */
} Elf64_Rel;


/* 带加数的重定位表项(在类型为SHT_RELA的section中)。 */
typedef struct
{
  Elf32_Addr	r_offset;		/* 地址 */
  Elf32_Word	r_info;			/* 重定位类型和符号索引 */
  Elf32_Sword	r_addend;		/* 加数 */
} Elf32_Rela;

typedef struct
{
  Elf64_Addr	r_offset;		/* 地址 */
  Elf64_Xword	r_info;			/* 重定位类型和符号索引 */
  Elf64_Sxword	r_addend;		/* 加数 */
} Elf64_Rela;
```

其中，各个字段的说明如下表：

| 成员     | 说明                                                         |
| -------- | ------------------------------------------------------------ |
| r_offset | 此成员给出了重定位动作所适用的位置。对于一个可重定位文件而言，<br/>此值是从节区头部开始到将被重定位影响的存储单位之间的字节偏<br/>移。对于可执行文件或者共享目标文件而言，其取值是被重定位影响<br/>到的存储单元的虚拟地址。 |
| r_info   | 此成员给出要进行重定位的符号表索引，以及将实施的重定位类型。<br/>例如一个调用指令的重定位项将包含被调用函数的符号表索引。如果索引是 STN_UNDEF，那么重定位使用 0 作为“符号值”。重定位类型是和处理器相关的。当程序代码引用一个重定位项的重定位类型或者符号表索引，则表示对表项的 r_info 成员应用 ELF32_R_TYPE 或 者 ELF32_R_SYM 的结果。<br/>#define ELF32_R_SYM(i) ((i)>>8) <br/>#define ELF32_R_TYPE(i) ((unsigned char)(i)) <br/>#define ELF32_R_INFO(s, t) (((s)<<8) + (unsigned char)(t)) |
| r_addend | 此成员给出一个常量补齐，用来计算将被填充到可重定位字段的数值。 |

- 只有 Elf32_Rela 项目可以明确包含补齐信息。
- 类型为 Elf32_Rel 的表项在将被修改的位置保存隐式的补齐信息。
- 依赖于处理器体系结构，各种形式都可能存在，甚至是必需的。因此，对特定机器的实现可以仅使用一种形式，也可以根据上下文使用不同的形式。
- 重定位节区会引用两个其它节区：
  - 符号表、
  - 要修改的节区。
- 节区头部的 sh_info 和sh_link 成员给出这些关系。不同目标文件的重定位表项对 r_offset 成员具有略微不同的解释。 
  (1). 在可重定位文件中，r_offset 中包含节区偏移。就是说重定位节区自身描述了如何修改文件中的其他节区；重定位偏移 指定了被修改节区中的一个存储单元。 
  (2). 在可执行文件和共享的目标文件中，r_offset 中包含一个虚拟地址。为了使得这些文件的重定位表项对动态链接器更为有用，节区偏移（针对文件的解释）让位于虚地址（针对内存的解释）。 尽管对 r_offset 的解释会有少许不同，重定位类型的含义始终不变。

### 1.2 重定位类型

重定位表项描述如何修改后面的指令和数据字段。一般，共享目标文件在创建时，其基本虚拟地址是 0，不过执行地址将随着动态加载而发生变化。

重定位的过程，按照如下标记：

#### 1.2.1 重定位计算

| 表示 | 计算                                                         |
| ---- | ------------------------------------------------------------ |
| A    | 用来计算可重定位字段的取值的补齐。                           |
| B    | 共享目标在执行过程中被加载到内存中的位置（基地址）。         |
| G    | 在执行过程中，重定位项的符号的地址所处的位置 —— 全局偏移表的索引。 |
| GOT  | 全局偏移表（GOT）的地址。                                    |
| L    | 某个符号的过程链接表项的位置（节区偏移/地址）。过程链接表项<br/>把函数调用重定位到正确的目标位置。链接编辑器构造初始的过程链<br/>接表，动态链接器在执行过程中修改这些项目。 |
| P    | 存储单位被重定位（用 r_offset 计算）到的位置（节区偏移或者地<br/>址）。 |
| S    | 其索引位于重定位项中的符号的取值。                           |

重定位项的 r_offset 取值给定受影响的存储单位的第一个字节的偏移或者虚拟地址。重定位类型给出那些位需要修改以及如何计算它们的取值。

SYSTEM V 仅使用 Elf32_Rel 重定位表项，在被重定位的字段中包含补齐量。补齐量和计算结果始终采用相同的字节顺序。

#### 1.2.2 SPARC的重定位

在 SPARC 平台上，重定位项适用于字节 (`byte8`)、半字 (`half16`)、字 (`word32`) 和扩展字 (`xword64`)。

![image:SPARC 基本重定位项。](r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/RelocSparcBasic-20230624094619736.png)

重定位字段 (`disp19`, `disp22`, `disp30`) 的 `dispn` 系列都是字对齐、带符号扩展的 PC 相对位移。全部将值编码为其最低有效位都位于字的位置 0，仅在分配给值的位数方面有所不同。

![image:SPARC disp 重定位项。](r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/RelocSparcDisp-20230624094621349.png)

`d2/disp8` 和 `d2/disp14` 变体使用两个非连续位字段 `d2` 和 `disp n` 对 16 位和 10 位位移值进行编码。

![image:SPARC d2/disp 重定位项。](r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/RelocSparcD2Disp-20230624094619753.png)

重定位字段的 `immn` 系列（`imm5`、`imm6`、`imm7`、`imm10`、`imm13` 和 `imm22`）表示无符号整型常数。全部将值编码为其最低有效位都位于字的位置 0，仅在分配给值的位数方面有所不同。

![image:SPARC imm 重定位项。](r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/RelocSparcImm-20230624094619736.png)

重定位字段的 `simmn` 系列（`simm10`、`simm11`、`simm13` 和 `simm22`）表示带符号的整型常数。全部将值编码为其最低有效位都位于字的位置 0，仅在分配给值的位数方面有所不同。

![image:SPARC simm 重定位项。](r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/RelocSparcSimm-20230624094619708.png)



#### 1.2.3 X86 的重定位类型：

<img src="r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624094225800.png" alt="image-20230624094225800" style="zoom:67%;" />

名称值字段计算:

| 名称             | 值   | 字段     | 计算                   |
| :--------------- | :--- | :------- | :--------------------- |
| `R_386_NONE`     | `0`  | 无       | 无                     |
| `R_386_32`       | `1`  | `word32` | `S + A`                |
| `R_386_PC32`     | `2`  | `word32` | `S + A - P`            |
| `R_386_GOT32`    | `3`  | `word32` | `G + A`                |
| `R_386_PLT32`    | `4`  | `word32` | `L + A - P`            |
| `R_386_COPY`     | `5`  | 无       | 请参阅此表后面的说明。 |
| `R_386_GLOB_DAT` | `6`  | `word32` | `S`                    |
| `R_386_JMP_SLOT` | `7`  | `word32` | `S`                    |
| `R_386_RELATIVE` | `8`  | `word32` | `B + A`                |
| `R_386_GOTOFF`   | `9`  | `word32` | `S + A - GOT`          |
| `R_386_GOTPC`    | `10` | `word32` | `GOT + A - P`          |
| `R_386_32PLT`    | `11` | `word32` | `L + A`                |
| `R_386_16`       | `20` | `word16` | `S + A`                |
| `R_386_PC16`     | `21` | `word16` | `S + A - P`            |
| `R_386_8`        | `22` | `word8`  | `S + A`                |
| `R_386_PC8`      | `23` | `word8`  | `S + A - P`            |
| `R_386_SIZE32`   | `38` | `word32` | `Z + A`                |

一些重定位类型的语义不只是简单的计算：

- `R_386_GOT32`

  计算 `GOT` 的基本地址与符号的 `GOT` 项之间的距离。此重定位还指示链接编辑器创建全局偏移表。

- `R_386_PLT32`

  计算符号的过程链接表项的地址，并指示链接编辑器创建一个过程链接表。

- `R_386_COPY`

  由链接编辑器为动态可执行文件创建，用于保留只读文本段。此重定位偏移成员指向可写段中的位置。符号表索引指定应在当前目标文件和共享目标文件中同时存在的符号。执行过程中，运行时链接程序将与共享目标文件的符号关联的数据复制到偏移所指定的位置。

- `R_386_GLOB_DAT`

  用于将 `GOT` 项设置为所指定符号的地址。使用特殊重定位类型，可以确定符号和 `GOT` 项之间的对应关系。

- `R_386_JMP_SLOT`

  由链接编辑器为动态目标文件创建，用于提供延迟绑定。此重定位偏移成员可指定过程链接表项的位置。运行时链接程序会修改过程链接表项，以将控制权转移到指定的符号地址。

- `R_386_RELATIVE`

  由链接编辑器为动态目标文件创建。此重定位偏移成员可指定共享目标文件中包含表示相对地址的值的位置。运行时链接程序通过将装入共享目标文件的虚拟地址与相对地址相加，计算对应的虚拟地址。此类型的重定位项必须为符号表索引指定值零。

- `R_386_GOTOFF`

  计算符号的值与 `GOT` 的地址之间的差值。此重定位还指示链接编辑器创建全局偏移表。

- `R_386_GOTPC`

  与 `R_386_PC32` 类似，不同的是它在其计算中会使用 `GOT` 的地址。此重定位中引用的符号通常是 `_GLOBAL_OFFSET_TABLE_`，该符号还指示链接编辑器创建全局偏移表。

## 2 选项- r的作用

> 该代码的主要功能是处理重定位表（relocation table）的信息。
>
> 它根据ELF文件的位数（32位或64位），获取重定位表的相关信息，并打印出重定位表的偏移量、条目数量以及每个条目的具体信息。

## 3 选项- r显示的信息解释

### 3.1 重定位节名称、偏移量、条目数

```c++
重定位节 '.rel.dyn' at offset 0x6b8 contains 4 entries:
重定位节 '.rel.plt' at offset 0x6d8 contains 15 entries:
```

- 重定位节的名称（Relocation Section）包含了程序中需要进行地址重定位的位置和相关信息。例如测试中有两个重定位节被列出：`.rel.dyn`和`.rel.plt`。

- 重定位节的偏移量
  - 重定位节 `.rel.dyn`的偏移量为0x6b8
  - 重定位节`.rel.plt`的偏移量为0x6d8 
- 重定位节的条目数
  - 重定位节`.rel.dyn`有4条
  - 重定位节`.rel.plt`有15条 

### 3.2 每个数据的条目的详细信息

每个条目的格式：偏移量、信息、类型、符号值、符号名称

对于每个条目：
- 偏移量：指定了需要进行重定位的位置的地址偏移量。
- 信息：提供了关于重定位的一些附加信息。
- 类型：指定了重定位的类型，这里的类型为 `R_386_GLOB_DAT` 和 `R_386_COPY`。
- 符号值：在重定位过程中需要用到的符号的值。
- 符号名称：与符号值相关联的符号的名称。

在这个例子中，`.rel.dyn`重定位节包含了4个条目，每个条目对应一个需要进行重定位的位置。其中：
- 第一个条目表示需要对 `_ITM_deregisterTMClone` 进行重定位，类型为 `R_386_GLOB_DAT`，符号值为 0x00000000。
- 第二个条目表示需要对 `__gmon_start__` 进行重定位，类型为 `R_386_GLOB_DAT`，符号值为 0x00000000。
- 第三个条目表示需要对 `_ITM_registerTMCloneTa` 进行重定位，类型为 `R_386_GLOB_DAT`，符号值为 0x00000000。
- 第四个条目表示需要对 `_ZSt4cout@GLIBCXX_3.4` 进行重定位，类型为 `R_386_COPY`，符号值为 0x0804a060。

在这个例子中，`.rel.plt`重定位节包含了15个条目，每个条目对应一个需要进行重定位的位置。其中：

   - 第一个条目表示需要对 `_ZSt4endlIcSt11char_tr@GLIBCXX_3.4` 进行重定位，类型为 `R_386_JUMP_SLOT`，符号值为 0x08048790。
   - 第二个条目表示需要对 `_ZNSt7__cxx1112basic_s@GLIBCXX_3.4.21` 进行重定位，类型为 `R_386_JUMP_SLOT`，符号值为 0x00000000。
   - 第三个条目表示需要对 `__cxa_atexit@GLIBC_2.1.3` 进行重定位，类型为 `R_386_JUMP_SLOT`，符号值为 0x00000000。
   - ...（依此类推）

这些重定位节中的条目指示了需要在程序加载和执行时进行地址重定位的位置和相关符号的信息。

重定位的过程将在程序加载时由链接器完成，以确保程序中引用的符号能够正确地解析和链接到实际的内存地址上。这对于程序的正确执行至关重要。

## 4 代码实现

### 4.1 算法思路

该代码的主要功能是处理重定位表（relocation table）的信息。

它根据ELF文件的位数（32位或64位），获取重定位表的相关信息，并打印出重定位表的偏移量、条目数量以及每个条目的具体信息，包括

- 偏移量、
- 信息（info）
- 类型（type）
- 符号值（symbol value）和
- 符号名称（symbol name）。

这有助于对ELF文件中的重定位信息进行分析和理解。

1. 打印"relocs:"：输出字符串"relocs:"。
2. 定义指向`Elf32_Rel`结构的指针`entry_rel`。
3. 判断是否为32位的ELF文件：
   - 如果是32位的ELF文件，调用`get_32bit_rel`函数，并传入`file`和`rel_dyn_offset`参数。
   - 如果不是32位的ELF文件，执行64位的处理（此处代码未给出具体实现）。
4. 打印"\nat offset 0x%2.2x contains %u entries:\n"，其中`%2.2x`和`%u`是占位符，分别用`rel_dyn_offset`和`rel_nent`填充。
5. 打印"  Offset          Info           Type           Sym. Value    Sym. Name"：输出表头信息。
6. 遍历`elf32_rel`指针从开始位置到`elf32_rel + rel_nent`的位置：
   - 打印"%10.8x "，其中`%10.8x`是占位符，用`entry_rel->r_offset`填充，打印`r_offset`字段的值。
   - 打印"%15.8x\n"，其中`%15.8x`是占位符，用`entry_rel->r_info`填充，打印`r_info`字段的值。

### 4.2 流程图

<img src="r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624105710901.png" alt="image-20230624105710901" style="zoom:50%;" />

### 4.3 代码详细解释

#### 4.3.1 主程序判断如下：

若为r，则执行`process_relocs(file)`

```c++
if(option & (1<<8))                     //-r
    {
	    process_relocs(file);
    }
```

#### 4.3.2 `process_relocs(FILE *file)`

> 该代码的主要功能是处理重定位表（relocation table）的信息。
>
> 它根据ELF文件的位数（32位或64位），获取重定位表的相关信息，并打印出重定位表的偏移量、条目数量以及每个条目的具体信息，包括
>
> - 偏移量、
> - 信息（info）
> - 类型（type）
> - 符号值（symbol value）和
> - 符号名称（symbol name）。
>
> 这有助于对ELF文件中的重定位信息进行分析和理解。
>
> 1. 打印"relocs:"：输出字符串"relocs:"。
>
> 2. 定义指向`Elf32_Rel`结构的指针`entry_rel`。
>
> 3. 判断是否为32位的ELF文件：
>    - 如果是32位的ELF文件，调用`get_32bit_rel`函数，并传入`file`和`rel_dyn_offset`参数。
>    - 如果不是32位的ELF文件，执行64位的处理（此处代码未给出具体实现）。
>
> 4. 打印"\nat offset 0x%2.2x contains %u entries:\n"，其中`%2.2x`和`%u`是占位符，分别用`rel_dyn_offset`和`rel_nent`填充。
>
> 5. 打印"  Offset          Info           Type           Sym. Value    Sym. Name"：输出表头信息。
>
> 6. 遍历`elf32_rel`指针从开始位置到`elf32_rel + rel_nent`的位置：
>    - 打印"%10.8x "，其中`%10.8x`是占位符，用`entry_rel->r_offset`填充，打印`r_offset`字段的值。
>    - 打印"%15.8x\n"，其中`%15.8x`是占位符，用`entry_rel->r_info`填充，打印`r_info`字段的值。
>
> 7. 返回0。
>

```c++
int ELF_process::process_relocs(FILE *file)
{
		// 打印"relocs:"
    printf("\nrelocs:");
  	// 定义指向Elf32_Rel结构的指针entry_rel
    Elf32_Rel* entry_rel;

    if(is_32bit_elf)
    {
      	 // 如果是32位的ELF文件，则调用get_32bit_rel函数并传入file和rel_dyn_offset参数
        get_32bit_rel(file,rel_dyn_offset);
    }
    else
    {
        // 如果不是32位的ELF文件，则执行64位的处理...
    }
  
  	// 打印"\nat offset 0x%2.2x contains %u entries:\n"，其中%2.2x和%u是占位符，分别用rel_dyn_offset和rel_nent填充
    printf (" \nat offset 0x%2.2x contains %u entries:\n",
            rel_dyn_offset, rel_nent);
  
  	 // 打印"  Offset          Info           Type           Sym. Value    Sym. Name"
    printf ("  Offset          Info           Type           Sym. Value    Sym. Name\n");
  
 		// 遍历elf32_rel指针从开始到elf32_rel + rel_nent的位置
    for (entry_rel = elf32_rel;
            entry_rel < elf32_rel + rel_nent;
            entry_rel++)
    {
      	// 打印"%10.8x "，其中%10.8x是占位符，用entry_rel->r_offset填充
        printf("%10.8x ",entry_rel->r_offset);
				// 打印"%15.8x\n"，其中%15.8x是占位符，用entry_rel->r_info填充
        printf("%15.8x\n",entry_rel->r_info);
    }


    return 0;
}
```

##### `is_32bit_elf`

```c++
 is_32bit_elf = (elf_header.e_ident[EI_CLASS] != ELFCLASS64);
```

用于确定一个 ELF 文件是否为 32 位格式的 ELF 文件。

每个部分的含义：
- `elf_header` 是一个结构体或对象，代表 ELF 文件的头部信息。
- `elf_header.e_ident` 是 ELF 文件头部的标识字段，包含了一系列的标识符。
- `EI_CLASS` 是 ELF 文件头部标识字段中用于指示 ELF 类型的索引常量。
- `ELFCLASS64` 是一个常量，表示 64 位 ELF 类型。
- `is_32bit_elf` 是一个布尔变量，用于存储判断结果，初始值为 `true`（32 位 ELF）。

通过比较 ELF 文件头部标识字段中的 ELF 类型值和常量 `ELFCLASS64`，判断 ELF 文件的类型是 32 位还是 64 位。如果两者不相等，即 `elf_header.e_ident[EI_CLASS]` 不等于 `ELFCLASS64`，则说明 ELF 文件是 32 位格式的，因此将 `is_32bit_elf` 的值设置为 `true`。如果相等，则说明 ELF 文件是 64 位格式的，将 `is_32bit_elf` 的值保持为初始值 `false`。

通过这个代码片段，可以在后续的程序中使用 `is_32bit_elf` 变量来判断 ELF 文件的位数类型，以便进行不同的处理或逻辑分支。

##### `get_32bit_rel(FILE *pFILE, unsigned int offset)`

> 该代码的主要功能是根据32位的ELF文件中的重定位表信息，将相关数据转换为适当的数据结构，并存储在`elf32_rel`中。它使用字节顺序转换将`rel`中的字段值赋值给`relt`，以便后续对重定位表的处理和分析。
>
> 1. 分配大小为`rel_dyn_size`的内存空间，并将其转换为`Elf32_External_Rel`指针类型，赋值给`rel`。
>
> 2. 定义指向`Elf32_External_Rel`结构的指针`ext`。
>
> 3. 定义指向`Elf32_Rel`结构的指针`relt`。
>
> 4. 将文件指针`pFILE`定位到`offset`处。
>
> 5. 从`pFILE`中读取大小为`rel_dyn_size`的数据，存储到`rel`中。
>
> 6. 如果`rel`为空指针，返回。
>
> 7. 使用循环遍历`rel`，计算`rel_nent`的值，即重定位表的条目数量。
>
> 8. 使用`cmalloc`函数分配大小为`dynamic_nent * sizeof(*relt)`的内存空间，将返回的指针赋值给`elf32_rel`，其中`dynamic_nent`表示动态节（dynamic section）的条目数量。
>
> 9. 使用循环遍历`rel`和`elf32_rel`，将`ext`中的字段值通过字节顺序转换后赋值给`relt`相应的字段。
>
> 10. 释放`rel`指针指向的内存空间。
>
> 11. 返回。
>

```c++
void ELF_process::get_32bit_rel(FILE *pFILE, unsigned int offset)
{
		// 分配rel_dyn_size大小的内存，并将其转换为Elf32_External_Rel指针类型，赋值给rel
    Elf32_External_Rel* rel= (Elf32_External_Rel *) malloc(rel_dyn_size);
	  // 定义指向Elf32_External_Rel结构的指针ext
    Elf32_External_Rel* ext;
  	// 定义指向Elf32_Rel结构的指针relt
    Elf32_Rel* relt;

  	// 将文件指针pFILE定位到offset处
    fseek(pFILE,offset,SEEK_SET);
	  // 从pFILE中读取rel_dyn_size大小的数据，存储到rel中
    fread(rel,rel_dyn_size,1,pFILE);

  	// 如果rel为空指针，则返回
    if(rel==NULL)
        return;
		
  	// 使用循环遍历rel，计算rel_nent的值，即重定位表的条目数量
    for (ext = rel, rel_nent = 0;
            (char *) ext < (char *) rel + rel_dyn_size;
            ext++)
    {
        rel_nent++;
      
      	// 如果rel的r_offset字段值等于DT_NULL，则跳出循环
        if (BYTE_GET (rel->r_offset) == DT_NULL)
            break;
    }
		
  // 使用cmalloc函数分配大小为dynamic_nent * sizeof(*relt)的内存空间，将返回的指针赋值给elf32_rel
    elf32_rel = (Elf32_Rel *) cmalloc (dynamic_nent,
                                       sizeof (* relt));

		// 使用循环遍历rel和elf32_rel，将ext中的字段值通过字节顺序转换后赋值给relt相应的字段
    for (ext = rel, relt = elf32_rel;
            relt < elf32_rel + rel_nent;
            ext++, relt++)
    {
        relt->r_offset     = BYTE_GET (ext->r_offset);
        relt->r_info       = BYTE_GET (ext->r_info);
    }
		
  	// 释放rel指针指向的内存空间
    free(rel);

    return;
}
```

###### `BYTE_GET(field)`：小端转大端

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
>

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



## 5 测试样例

`r-test-1.cpp:`

```c++
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

<img src="r%E6%8C%87%E4%BB%A4%E7%9A%84%E5%8E%9F%E7%90%86%E4%B8%8EELF%E5%AE%9E%E7%8E%B0.assets/image-20230624101420972.png" alt="image-20230624101420972" style="zoom:50%;" />

我们使用了`-r`选项，该选项显示了重定位节的信息。

重定位节（Relocation Section）包含了程序中需要进行地址重定位的位置和相关信息。在ELF文件中，有两个重定位节被列出：`.rel.dyn`和`.rel.plt`。

1. 重定位节 `.rel.dyn`:
   - 偏移量（Offset）：0x6b8
   - 包含的条目数：4
   - 每个条目的格式：偏移量、信息、类型、符号值、符号名称

     - 偏移量：指定了需要进行重定位的位置的地址偏移量。
   
     - 信息：提供了关于重定位的一些附加信息。
   
     - 类型：指定了重定位的类型，这里的类型为 `R_386_GLOB_DAT` 和 `R_386_COPY`。
   
     - 符号值：在重定位过程中需要用到的符号的值。
   
     - 符号名称：与符号值相关联的符号的名称。
   
   
   在这个例子中，`.rel.dyn`重定位节包含了4个条目，每个条目对应一个需要进行重定位的位置。其中：
   - 第一个条目表示需要对 `_ITM_deregisterTMClone` 进行重定位，类型为 `R_386_GLOB_DAT`，符号值为 0x00000000。
   - 第二个条目表示需要对 `__gmon_start__` 进行重定位，类型为 `R_386_GLOB_DAT`，符号值为 0x00000000。
   - 第三个条目表示需要对 `_ITM_registerTMCloneTa` 进行重定位，类型为 `R_386_GLOB_DAT`，符号值为 0x00000000。
   - 第四个条目表示需要对 `_ZSt4cout@GLIBCXX_3.4` 进行重定位，类型为 `R_386_COPY`，符号值为 0x0804a060。
   
2. 重定位节 `.rel.plt`:
   - 偏移量（Offset）：0x6d8
   - 包含的条目数：15
   - 每个条目的格式：偏移量、信息、类型、符号值、符号名称

     - 偏移量：指定了需要进行重定位的位置的地址偏移量。
   
     - 信息：提供了关于重定位的一些附加信息。
   
     - 类型：指定了重定位的类型，这里的类型为 `R_386_JUMP_SLOT`。
   
     - 符号值：在重定位过程中需要用到的符号的值。
   
     - 符号名称：与符号值相关联的符号的名称。
   
   
   在这个例子中，`.rel.plt`重定位节包含了15个条目，每个条目对应一个需要进行重定位的位置。其中：
   
      - 第一个条目表示需要对 `_ZSt4endlIcSt11char_tr@GLIBCXX_3.4` 进行重定位，类型为 `R_386_JUMP_SLOT`，符号值为 0x08048790。
   
      - 第二个条目表示需要对 `_ZNSt7__cxx1112basic_s@GLIBCXX_3.4.21` 进行重定位，类型为 `R_386_JUMP_SLOT`，符号值为 0x00000000。
   
      - 第三个条目表示需要对 `__cxa_atexit@GLIBC_2.1.3` 进行重定位，类型为 `R_386_JUMP_SLOT`，符号值为 0x00000000。
   
      - ...（依此类推）
   

这些重定位节中的条目指示了需要在程序加载和执行时进行地址重定位的位置和相关符号的信息。

重定位的过程将在程序加载时由链接器完成，以确保程序中引用的符号能够正确地解析和链接到实际的内存地址上。这对于程序的正确执行至关重要。
