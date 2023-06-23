#include <elf.h>
#include <cstring>
#include "ELF_process.h"
#define file_name "/home/hx/cProgram/Process/libhello-jni.so"
//将原始的小端字数据，转换为大端数据，与结构体对齐
#define BYTE_GET(field)  byte_get_little_endian (field,sizeof(field))
static int is_32bit_elf;
Elf32_Ehdr  elf_header;
Elf32_Shdr* section_headers;
Elf32_Phdr* program_headers;
Elf32_Sym*  sym_dyn;

static unsigned int dynamic_addr;
static unsigned int dynamic_offset;
unsigned int dynamic_strings;
unsigned int dynamic_size;
static unsigned int rel_nent;
static unsigned int rel_dyn_offset;
static unsigned int rel_dyn_size;
static unsigned int sym_dyn_offset;
static unsigned int sym_dyn_size;
static unsigned int str_dyn_offset;
static unsigned int str_dyn_size;
unsigned int sym_nent;
Elf32_Dyn* dynamic_section;

int unwind_idx,target_section_idx;

static unsigned int dynamic_nent;

#define NT_GNU_PROPERTY_TYPE_0 5
#define SHT_PARISC_ANNOT    0x70000003
#define SHT_PARISC_SYMEXTN    SHT_LOPROC + 8
#define SHT_PARISC_STUBS      SHT_LOPROC + 9
#define SHT_PARISC_DLKM        0x70000004

#define PT_PARISC_WEAKORDER    0x70000002
#define PT_HP_CORE_UTSNAME    (PT_LOOS + 0x15)

#define SHT_IA_64_PRIORITY_INIT (SHT_LOPROC + 0x9000000)
#define SHT_IA_64_VMS_TRACE             0x60000000
#define SHT_IA_64_VMS_TIE_SIGNATURES    0x60000001
#define SHT_IA_64_VMS_DEBUG             0x60000002
#define SHT_IA_64_VMS_DEBUG_STR         0x60000003
#define SHT_IA_64_VMS_LINKAGES          0x60000004
#define SHT_IA_64_VMS_SYMBOL_VECTOR     0x60000005
#define SHT_IA_64_VMS_FIXUP             0x60000006
#define SHT_IA_64_LOPSREG    (SHT_LOPROC + 0x8000000)



#define EM_L1OM        180    /* Intel L1OM */
#define EM_K1OM        181    /* Intel K1OM */
#define EM_TI_C6000    140    /* Texas Instruments TMS320C6000 DSP family */
#define EM_MSP430    105    /* TI msp430 micro controller */


#define SHT_ARM_DEBUGOVERLAY   0x70000004    /* Section holds overlay debug info.  */
#define SHT_ARM_OVERLAYSECTION 0x70000005    /* Section holds GDB and overlay integration info.  */

#define SHT_X86_64_UNWIND    0x70000001    /* unwind information */


#define SHT_AARCH64_ATTRIBUTES    0x70000003  /* Section holds attributes.  */

#define SHT_C6000_UNWIND    0x70000001
#define SHT_C6000_PREEMPTMAP    0x70000002
#define SHT_C6000_ATTRIBUTES    0x70000003
#define SHT_TI_ICODE        0x7F000000
#define SHT_TI_XREF        0x7F000001
#define SHT_TI_HANDLER        0x7F000002
#define SHT_TI_INITINFO        0x7F000003
#define SHT_TI_PHATTRS        0x7F000004


#define SHT_MSP430_ATTRIBUTES    0x70000003    /* Section holds ABI attributes.  */
#define SHT_MSP430_SEC_FLAGS    0x7f000005    /* Holds TI compiler's section flags.  */
#define SHT_MSP430_SYM_ALIASES    0x7f000006    /* Holds TI compiler's symbol aliases.  */



#define PT_AARCH64_ARCHEXT    (PT_LOPROC + 0)

//按照小端方式对字进行转换读取，转换为大端方式
int byte_get_little_endian (unsigned char *field, int size)
{
    //依据字的长度进行转换
    switch (size)
    {
    case 1:
        return *field;
    case 2:
        return ((unsigned int)(field[0]))
               | (((unsigned int)(field[1])) << 8);
    case 3:
        return  ((unsigned long) (field[0]))
                |    (((unsigned long) (field[1])) << 8)
                |    (((unsigned long) (field[2])) << 16);

    case 4:
        return  ((unsigned long) (field[0]))
                |    (((unsigned long) (field[1])) << 8)
                |    (((unsigned long) (field[2])) << 16)
                |    (((unsigned long) (field[3])) << 24);
    }

}

ELF_process::ELF_process()
{
    printf("NO parameter\n");
}

int ELF_process::Process_object(FILE *file,int option,char * target_section_name)
{


    if (!get_file_header(file))
    {
        printf("get file header Failed");
        return 0;
    }


    /********* start     process ***********/
    if(option & (1<<0) || option & (1<<6) ) // -h -e 
    {    
	    if (!process_file_header())
    	{
            return 0;
        }
    }
    if(!process_section_headers(file,option,target_section_name))
    {
        return 0;
    }
    if(option & (1<<3) )                    //-g
    {
        process_section_group(file);
    }
    if(option & (1<<1) || option & (1<<6))  //-l   -e
    { 
        process_program_headers(file);
    }

    if(option & (1<<7))                     //-n
    {
        process_note(file);
    }

    if(option & (1<<11))                     //-n
    {
        process_version(file);
    }

    if(option & (1<<10))                    //-d
    {
        process_dynamic_section(file);
    }
    if(option & (1<<8))                     //-r
    {
	process_relocs(file);
    }
    if((option & (1<<13))||(option & (1<<2)))  //-S -I
    {
        process_symbol_table(file,option);
    }
    if(option & (1<<9))                    //-u
    {
		print_unwind(file);
    }
    if(option & (1<<14))                    //-x
    {
	if(target_section_name[0]>='0'&&target_section_name[0]<='9') 	target_section_idx=target_section_name[0]-'0';
        print_xout(file, target_section_idx);
    }
}
//获取文件头信息的其他部分
//下面的代码为32位信息显示
//只支持32位的信息显示目前
int ELF_process::get_file_header(FILE *file)
{

    /* 读取ELF信息，前16个字节，只有当读取成功了才往后面走  */
    if (fread (elf_header.e_ident, EI_NIDENT, 1, file) != 1)
        return 0;

    /* 目前只是实现了32位环境下的表示  */
    is_32bit_elf = (elf_header.e_ident[EI_CLASS] != ELFCLASS64);

    /* 读取头部信息的其他部分  */
    if (is_32bit_elf)
    {

        Elf32_External_Ehdr ehdr32; //文件头信息的其他部分
        //读取信息除了前16个字节，从后一个字节开始读取
        if (fread (ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1, file) != 1)
            return 0;
        //读取对应部分的字节信息
        elf_header.e_type      = BYTE_GET (ehdr32.e_type);
        elf_header.e_machine   = BYTE_GET (ehdr32.e_machine);
        elf_header.e_version   = BYTE_GET (ehdr32.e_version);
        elf_header.e_entry     = BYTE_GET (ehdr32.e_entry);
        elf_header.e_phoff     = BYTE_GET (ehdr32.e_phoff);
        elf_header.e_shoff     = BYTE_GET (ehdr32.e_shoff);
        elf_header.e_flags     = BYTE_GET (ehdr32.e_flags);
        elf_header.e_ehsize    = BYTE_GET (ehdr32.e_ehsize);
        elf_header.e_phentsize = BYTE_GET (ehdr32.e_phentsize);
        elf_header.e_phnum     = BYTE_GET (ehdr32.e_phnum);
        elf_header.e_shentsize = BYTE_GET (ehdr32.e_shentsize);
        elf_header.e_shnum     = BYTE_GET (ehdr32.e_shnum);
        elf_header.e_shstrndx  = BYTE_GET (ehdr32.e_shstrndx);


        {
            if (is_32bit_elf)
                get_32bit_section_headers(file,1);
            else
            {
                //64λ ...
            }
        }

    }
    return 1;
}
  
int ELF_process::process_version(FILE *file){
    // EV_NONE 0 非法版本 
    // EV_CURRENT 1 当前版本
    if(elf_header.e_version==1){
        return printf("readELF 0.1.1, build time 2021.9.10\n");
    }
    return printf("No version information found in this file.\n");
}

//读取32位ELF文件的节区头表
int ELF_process::get_32bit_section_headers(FILE *file, unsigned int num)
{
    //file：ELF文件指针
    //num：节区头表的项目数

    Elf32_External_Shdr * shdrs;
    Elf32_Shdr* internal;

    //利用elf头中的关于节头的信息，读取节头的数据，并强制转换为节头指针数据类型
    //这里读取的shdrs是节头表数组的首地址
    shdrs = (Elf32_External_Shdr *) get_data (NULL, file, elf_header.e_shoff,
            elf_header.e_shentsize, num,
            ("section headers"));
    //如果节头数组为空，就表示读取失败了，所以直接返回
    if (!shdrs)
        return 0;

    //申请内存存放节头数组，之前的读取的节头数组是
    section_headers = (Elf32_Shdr *) cmalloc (num,sizeof (Elf32_Shdr));

    //如果申请内存失败，返回内存溢出异常
    if (section_headers == NULL)
    {
        printf("Out of memory\n");
        return 0;
    }

    internal = section_headers;

    //将从文件中读取的小端数据的节区头表数据转换为结构体的数据
    //这里的转换有讲究的，因为我们读取使用的char[]数组读4个字节的，是不会进行转换的，会直接读取原始的小端数据
    //这里需要将小端数据转换一下，变成正常的字数据
    //那为什么不直接用uint_32t?
    //因为保存的ELF文件不一定都是我定义的Elf32Word类型，变个名字就失效了
    for (int i = 0; i < num; i++, internal++)
    {
        internal->sh_name      = BYTE_GET (shdrs[i].sh_name);
        internal->sh_type      = BYTE_GET (shdrs[i].sh_type);
        internal->sh_flags     = BYTE_GET (shdrs[i].sh_flags);
        internal->sh_addr      = BYTE_GET (shdrs[i].sh_addr);
        internal->sh_offset    = BYTE_GET (shdrs[i].sh_offset);
        internal->sh_size      = BYTE_GET (shdrs[i].sh_size);
        internal->sh_link      = BYTE_GET (shdrs[i].sh_link);
        internal->sh_info      = BYTE_GET (shdrs[i].sh_info);
        internal->sh_addralign = BYTE_GET (shdrs[i].sh_addralign);
        internal->sh_entsize   = BYTE_GET (shdrs[i].sh_entsize);
    }

    free (shdrs);

    return 1;
}

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

//在内存中申请空间
void *ELF_process::cmalloc (size_t nmemb, size_t size)
{
    /* Check for overflow.  */
    //检查溢出，判断要申请的空间存放的元素个数 是否大于等于 假如全部装这个元素时能存放的最大元素个数
    if (nmemb >= ~(size_t) 0 / size)
        return NULL;
    //如果没有溢出，那么就申请 元素大小*元素个数 的内存空间大小
    else
        return malloc (nmemb * size);
}
/*读取文件头*/
/*头文件信息在Magic 数中*/
int ELF_process::process_file_header(void)
{
    /*判断是否Magic是否存在问题*/
    if (   elf_header.e_ident[EI_MAG0] != ELFMAG0
            || elf_header.e_ident[EI_MAG1] != ELFMAG1
            || elf_header.e_ident[EI_MAG2] != ELFMAG2
            || elf_header.e_ident[EI_MAG3] != ELFMAG3)
    {
        printf("Not an ELF file - it has the wrong magic bytes at the start\n");
        return 0;
    }
    /*输出M相关参数*/
    printf("ELF Header:\n");
    printf("  Magic:     ");
    for (int i = 0; i <EI_NIDENT ; ++i)
        printf ("%2.2x ", elf_header.e_ident[i]); /*读取Magic 数*/
    printf("\n");
    printf("  Class:                             %s\n",
           get_elf_class(elf_header.e_ident[EI_CLASS]));

    printf ("  Data:                              %s\n",
            get_data_encoding (elf_header.e_ident[EI_DATA]));
    const char *str=elf_header.e_ident[EI_VERSION] == EV_CURRENT ? 
	"(current)" : (elf_header.e_ident[EI_VERSION] != EV_NONE ? "<unknown>" : "");
    printf ("  Version:                           %d %s\n",
            elf_header.e_ident[EI_VERSION],str);
    printf ("  OS/ABI:                            %s\n",
            get_osabi_name (elf_header.e_ident[EI_OSABI]));

    printf ("  ABI Version:                       %d\n",
            elf_header.e_ident[EI_ABIVERSION]);

    printf ("  Type:                              %s\n",
            get_file_type (elf_header.e_type));

    printf ("  Machine:                           %s\n",
            get_machine_name (elf_header.e_machine));

    printf ("  Version:                           0x%lx\n",
            (unsigned long) elf_header.e_version);

    printf ("  Entry point address:               0x%x",elf_header.e_entry);

    printf ("\n  Start of program headers:          %d",elf_header.e_phoff);

    printf (" (bytes into file)\n  Start of section headers:          %d",elf_header.e_shoff);
    printf (" (bytes into file)\n");

    printf ("  Flags:                             0x%lx\n",(unsigned  long)elf_header.e_flags);

    printf ("  Size of this header:               %ld (bytes)\n",(long)elf_header.e_ehsize);

    printf ("  Size of program headers:           %ld (bytes)\n",(long)elf_header.e_phentsize);

    printf ("  Number of program headers:         %ld\n",(long)elf_header.e_phnum);

    printf ("  Size of section headers:           %ld (bytes)\n",
            (long) elf_header.e_shentsize);

    printf ("  Number of section headers:         %ld\n",
            (long) elf_header.e_shnum);

    if (section_headers != NULL && elf_header.e_shnum == SHN_UNDEF)
        printf (" (%ld)", (long) section_headers[0].sh_size);

    printf ("  Section header string table index: %ld\n",
            (long) elf_header.e_shstrndx);

    return 1;

}

/*从magic数中，提取elf_class信息*/
const char *ELF_process::get_elf_class (unsigned int elf_class)
{
    static char buff[32];
    /*判断elf_class处的字节信息*/
    switch (elf_class)
    {
    case ELFCLASSNONE:
        return ("none");
    case ELFCLASS32:
        return "ELF32";
    case ELFCLASS64:
        return "ELF64";
    default:
        snprintf (buff, sizeof (buff), ("<unknown: %x>"), elf_class);
        return buff;
    }
}
/*获取data的编码信息*/
const char *ELF_process::get_data_encoding (unsigned int encoding)
{
    static char buff[32];
    /*表示数据存储的方式，大端、小端*/
    switch (encoding)
    {
    case ELFDATANONE:
        return ("none");
    case ELFDATA2LSB:
        return ("2's complement, little endian");
    case ELFDATA2MSB:
        return ("2's complement, big endian");
    default:
        snprintf (buff, sizeof (buff), ("<unknown: %x>"), encoding);
        return buff;
    }
}
const char *ELF_process::get_osabi_name (unsigned int osabi)
{

    static    char buff[32];
    switch (osabi)
    {
    case  ELFOSABI_NONE:
        return "UNIX System V ABI";
    case  ELFOSABI_HPUX:
        return "HP-UX";
    case  ELFOSABI_NETBSD:
        return "NetBSD";
    case  ELFOSABI_GNU:
        return "Object uses GNU ELF extensions";
    case  ELFOSABI_SOLARIS:
        return "Sun Solaris";
    case  ELFOSABI_AIX:
        return "IBM AIX";
    case  ELFOSABI_IRIX:
        return "SGI Irix";
    case  ELFOSABI_FREEBSD:
        return "FreeBSD";
    case  ELFOSABI_TRU64:
        return "Compaq TRU64 UNIX";
    case  ELFOSABI_MODESTO:
        return "Novell Modesto";
    case  ELFOSABI_OPENBSD:
        return "OpenBSD";
    case  ELFOSABI_ARM_AEABI:
        return "ARM EABI";
    case  ELFOSABI_ARM:
        return "ARM";
    case  ELFOSABI_STANDALONE:
        return "Standalone (embedded) application";
    default:
        break;
    }

    snprintf (buff, sizeof (buff), ("<unknown: %x>"), osabi);
    return buff;
}
/*返回文件类型*/

const char *ELF_process::get_file_type(unsigned e_type)
{

    static char buff[32];

    switch (e_type)
    {
    case ET_NONE:
        return "NONE (None)";
    case ET_REL:
        return "REL (Relocatable file)"; //重定向文件
    case ET_EXEC:
        return "EXEC (Executable file)"; //执行文件
    case ET_DYN:
        return "DYN (Shared object file)"; //共享文件
    case ET_CORE:
        return "CORE (Core file)"; //核心文件

    default: //如果不是如上三种文件，那么返回具体的处理器信息或操作系统信息
        if ((e_type >= ET_LOPROC) && (e_type <= ET_HIPROC))
            snprintf(buff, sizeof(buff), ("Processor Specific: (%x)"), e_type);
        else if ((e_type >= ET_LOOS) && (e_type <= ET_HIOS))
            snprintf(buff, sizeof(buff), ("OS Specific: (%x)"), e_type);
        else
            snprintf(buff, sizeof(buff), ("<unknown>: %x"), e_type);
        return buff;

    }
}
/*获取机器类型*/
const char *ELF_process::get_machine_name(unsigned e_machine)
{

    static    char buff[64];

    switch (e_machine)
    {
    case EM_NONE:
        return ("None");
    case EM_AARCH64:
        return "AArch64";
    case EM_M32:
        return "WE32100";
    case EM_SPARC:
        return "Sparc";
    case EM_386:
        return "Intel 80386";
    case EM_68K:
        return "MC68000";
    case EM_88K:
        return "MC88000";
    case EM_860:
        return "Intel 80860";
    case EM_MIPS:
        return "MIPS R3000";
    case EM_S370:
        return "IBM System/370";
    case EM_MIPS_RS3_LE:
        return "MIPS R4000 big-endian";
    case EM_PARISC:
        return "HPPA";
    case EM_SPARC32PLUS:
        return "Sparc v8+" ;
    case EM_960:
        return "Intel 90860";
    case EM_PPC:
        return "PowerPC";
    case EM_PPC64:
        return "PowerPC64";
    case EM_FR20:
        return "Fujitsu FR20";
    case EM_RH32:
        return "TRW RH32";
    case EM_ARM:
        return "ARM";
    case EM_SH:
        return "Renesas / SuperH SH";
    case EM_SPARCV9:
        return "Sparc v9";
    case EM_TRICORE:
        return "Siemens Tricore";
    case EM_ARC:
        return "ARC";
    case EM_H8_300:
        return "Renesas H8/300";
    case EM_H8_300H:
        return "Renesas H8/300H";
    case EM_H8S:
        return "Renesas H8S";
    case EM_H8_500:
        return "Renesas H8/500";
    case EM_IA_64:
        return "Intel IA-64";
    case EM_MIPS_X:
        return "Stanford MIPS-X";
    case EM_COLDFIRE:
        return "Motorola Coldfire";
    case EM_ALPHA:
        return "Alpha";
    case EM_D10V:
        return "d10v";
    case EM_D30V:
        return "d30v";
    case EM_M32R:
        return "Renesas M32R (formerly Mitsubishi M32r)";
    case EM_V800:
        return "Renesas V850 (using RH850 ABI)";
    case EM_V850:
        return "Renesas V850";
    case EM_MN10300:
        return "mn10300";
    case EM_MN10200:
        return "mn10200";
    case EM_FR30:
        return "Fujitsu FR30";
    case EM_PJ:
        return "picoJava";
    case EM_MMA:
        return "Fujitsu Multimedia Accelerator";
    case EM_PCP:
        return "Siemens PCP";
    case EM_NCPU:
        return "Sony nCPU embedded RISC processor";
    case EM_NDR1:
        return "Denso NDR1 microprocesspr";
    case EM_STARCORE:
        return "Motorola Star*Core processor";
    case EM_ME16:
        return "Toyota ME16 processor";
    case EM_ST100:
        return "STMicroelectronics ST100 processor";
    case EM_TINYJ:
        return "Advanced Logic Corp. TinyJ embedded processor";
    case EM_PDSP:
        return "Sony DSP processor";
    case EM_FX66:
        return "Siemens FX66 microcontroller";
    case EM_ST9PLUS:
        return "STMicroelectronics ST9+ 8/16 bit microcontroller";
    case EM_ST7:
        return "STMicroelectronics ST7 8-bit microcontroller";
    case EM_68HC16:
        return "Motorola MC68HC16 Microcontroller";
    case EM_68HC12:
        return "Motorola MC68HC12 Microcontroller";
    case EM_68HC11:
        return "Motorola MC68HC11 Microcontroller";
    case EM_68HC08:
        return "Motorola MC68HC08 Microcontroller";
    case EM_68HC05:
        return "Motorola MC68HC05 Microcontroller";
    case EM_SVX:
        return "Silicon Graphics SVx";
    case EM_ST19:
        return "STMicroelectronics ST19 8-bit microcontroller";
    case EM_VAX:
        return "Digital VAX";
    case EM_AVR:
        return "Atmel AVR 8-bit microcontroller";
    case EM_CRIS:
        return "Axis Communications 32-bit embedded processor";
    case EM_JAVELIN:
        return "Infineon Technologies 32-bit embedded cpu";
    case EM_FIREPATH:
        return "Element 14 64-bit DSP processor";
    case EM_ZSP:
        return "LSI Logic's 16-bit DSP processor";
    case EM_MMIX:
        return "Donald Knuth's educational 64-bit processor";
    case EM_HUANY:
        return "Harvard Universitys's machine-independent object format";
    case EM_PRISM:
        return "Vitesse Prism";
    case EM_X86_64:
        return "Advanced Micro Devices X86-64";
    case EM_S390:
        return "IBM S/390";
    case EM_OPENRISC:
    case EM_ARC_A5:
        return "ARC International ARCompact processor";
    case EM_XTENSA:
        return "Tensilica Xtensa Processor";
    case EM_MICROBLAZE:
    case EM_TILEPRO:
        return "Tilera TILEPro multicore architecture family";
    case EM_TILEGX:
        return "Tilera TILE-Gx multicore architecture family";
    default:
        snprintf (buff, sizeof (buff), ("<unknown>: 0x%x"), e_machine);
    }

    return buff;

}

//解析和处理文件的节头表
int ELF_process::process_section_headers(FILE *file,int option,char *target_section_name)
{
    //file：文件指针
    //option：解析的内容选项
    //target_section_name：目标节区索引，这个只有当-x指令时生效

    Elf32_Shdr * section;
    section = NULL;
    char * string_table;

    unsigned int  flag_shoff;

    //依据ELF头判断节区头表的项目数，如果项目数为0那么存在异常
    if (elf_header.e_shnum == 0)
    {   
        //依据elf头中的节区表头偏移地址是不是0，如果是0，那么就说明不存在节区表头，有异常
        if (elf_header.e_shoff!=0)
            printf("possibly corrupt ELF file header - it has a non-zero section header offset, but no section headers\n");
        //节区表头存在但是没有表项，就说明文件中没有节区
        else
            printf ("\nThere are no sections in this file.\n");
        return 1;
    }

    //如果指令为-t -S -e，那么就先打印出节头表的项目数量和节头表的偏移位置，
    //这些指令分别是：显示节的详细信息、 显示节头信息、显示全部头信息
    if((option & (1<<4))||(option & (1<<2) ) || (option & (1<<6)))
        printf ("  There are %d section headers, starting at offset 0x%lx:\n",elf_header.e_shnum, (unsigned long) elf_header.e_shoff);

    //如果是32位的文件
    if (is_32bit_elf)
    {
        //读取32位的节区头表，依据elf头中的信息
        if (! get_32bit_section_headers (file, elf_header.e_shnum))
            return 0;
    }

    /* Read in the string table, so that we have names to display.  */
    //找到shstrtab的位置
    //先判断shstrtab的节区索引是不是未定义，然后判断是不是小于总节区数
    if (elf_header.e_shstrndx != SHN_UNDEF  && elf_header.e_shstrndx < elf_header.e_shnum)
    {
        //获取shstrtab节区，位置是节区头的偏移地址加上对应的索引，即节区头表中的第e_shstrndx个元素
        section = section_headers + elf_header.e_shstrndx;

        //保存shstrtab的偏移地址
        flag_shoff = section->sh_offset;

    }

    //如果指令为-t -S -e，那么接下来打印节头信息
    //这些指令分别是：显示节的详细信息、 显示节头信息、显示全部头信息
    if((option & (1<<4))||(option & (1<<2) ) || (option & (1<<6)))
    {
        //如果节头表项个数大于1，那么单词用复数
	    if (elf_header.e_shnum > 1)
            printf ("\nSection Headers:\n");
        else
            printf ("\nSection Header:\n");
    }
    section = section_headers;
    unsigned int countC;
    //只做32位文件的分析
    if (is_32bit_elf)
    {
        //如果是-t指令，显示节区的详细信息
        if(option & (1<<4)) //-t
        {
            //打印列名
            printf("  [Nr] Name\n      Type            Addr     Off    Size   ES Flg Lk Inf Al\n      Flags\n");
            //遍历节区头表的所有项，每一个项都对应一个节区
            for (int i = 0; i < elf_header.e_shnum; i++, section++)
            {
                printf ("  [%2u] ", i);

                //计算该节区名在shstrtab中的偏移地址
                countC = flag_shoff + section->sh_name;

                //将文件指针移动到这个地方
                fseek(file,countC,SEEK_SET);
                //名字字符串，长度为20，名字以'\0'结尾，所以多读一些没有关系
                char string_name[20];
                //从文件中读取名字字符串
                fread(string_name,20,1,file);

        //判断节区名是不是"IA_64",记录unwind节区的索引
		if(!strcmp(string_name,"IA_64")) unwind_idx=i;
        //判断节区名和目标的节区名是否一致，相同的话记录目标节区名对应的索引，这个是给-x用的
		if(!strcmp(string_name,target_section_name)) target_section_idx=i;

                //打印输出节区名
                printf("%-16s \n",string_name);

                //打印输出节区类型
                printf("%-16s ",get_section_type_name (section->sh_type));

                //打印输出节区虚拟地址、文件中偏移地址、节区大小、节区条目大小
                printf("%6.8lx",(unsigned long) section->sh_addr);
                printf ( " %6.6lx %6.6lx %2.2lx",
                         (unsigned long) section->sh_offset,
                         (unsigned long) section->sh_size,
                         (unsigned long) section->sh_entsize);

                //如果节区有标志位那么就打印输出标志
                if (section->sh_flags)
                    printf (" %2.2x ", section->sh_flags);
                //否则用空格填充
                else
                    printf("%4c",32);

                //打印输出sh_link和sh_info，这个因不同节区类型而异，最后输出对齐信息
                printf ("%2u ", section->sh_link);
                printf ("%3u %3lu", section->sh_info,
                        (unsigned long) section->sh_addralign);

                //如果节区名是.dynamic动态链接信息表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".dynamic")==0)
                {
                    dynamic_addr   = section->sh_offset;
                    dynamic_size   = section->sh_size;
                }

                //如果节区名是.rel.dyn动态链接重定位表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".rel.dyn")==0)
                {
                    rel_dyn_offset = section->sh_offset;
                    rel_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynsym动态链接符号表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynsym")==0)
                {
                    sym_dyn_offset = section->sh_offset;
                    sym_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynstr动态链接字符表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynstr")==0)
                {
                    str_dyn_offset = section->sh_offset;
                    str_dyn_size   = section->sh_size;
                }

                printf("\n");
                //打印输出标志位
                printf("      [%x]\n",section->sh_flags);
            }
        }
        //如果是-S或-e指令，显示节头信息和全部头信息
        else if((option & (1<<2) ) || (option & (1<<6)))        //-S || -e
        {
            
            //打印列名
            printf("  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n");
            //遍历节区头表的所有项，每一个项都对应一个节区
            for (int i = 0; i < elf_header.e_shnum; i++, section++)
            {
                printf ("  [%2u] ", i);

                //计算该节区名在shstrtab中的偏移地址
                countC = flag_shoff + section->sh_name;

                //将文件指针移动到这个地方
                fseek(file,countC,SEEK_SET);
                //名字字符串，长度为20，名字以'\0'结尾，所以多读一些没有关系
                char string_name[20];
                //从文件中读取名字字符串
                fread(string_name,20,1,file);

        //判断节区名是不是"IA_64",记录unwind节区的索引
		if(!strcmp(string_name,"IA_64")) unwind_idx=i;
        //判断节区名和目标的节区名是否一致，相同的话记录目标节区名对应的索引，这个是给-x用的
		if(!strcmp(string_name,target_section_name)) target_section_idx=i;

                //打印输出节区名
                printf("%-16s ",string_name);


                //打印输出节区类型
                printf ( " %-15.15s ",
                         get_section_type_name (section->sh_type));

                //打印输出节区虚拟地址、文件中偏移地址、节区大小、节区条目大小
                printf("%6.8lx",(unsigned long) section->sh_addr);
                printf ( " %6.6lx %6.6lx %2.2lx",
                         (unsigned long) section->sh_offset,
                         (unsigned long) section->sh_size,
                         (unsigned long) section->sh_entsize);

                //如果节区有标志位那么就打印输出标志
                if (section->sh_flags)
                    printf (" %2.2x ", section->sh_flags);
                //否则用空格填充
                else
                    printf("%4c",32);

                //打印输出sh_link和sh_info，这个因不同节区类型而异，最后输出对齐信息
                printf ("%2u ", section->sh_link);
                printf ("%3u %3lu", section->sh_info,
                        (unsigned long) section->sh_addralign);

                //如果节区名是.dynamic动态链接信息表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".dynamic")==0)
                {
                    dynamic_addr   = section->sh_offset;
                    dynamic_size   = section->sh_size;
                }

                //如果节区名是.rel.dyn动态链接重定位表，那么需要记录节区的偏移地址和大小
                if (strcmp(string_name,".rel.dyn")==0)
                {
                    rel_dyn_offset = section->sh_offset;
                    rel_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynsym动态链接符号表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynsym")==0)
                {
                    sym_dyn_offset = section->sh_offset;
                    sym_dyn_size   = section->sh_size;
                }

                //如果节区名是.dynstr动态链接字符表，那么需要记录节区的偏移地址和大小
                if(strcmp(string_name,".dynstr")==0)
                {
                    str_dyn_offset = section->sh_offset;
                    str_dyn_size   = section->sh_size;
                }

                printf("\n");

            }
        }
        //如果是指令-x或-u，显示指定节区详细信息或.unwind节区详细信息
        else if(option & (1<<14) || option & (1<<9))
        {
            //遍历节区头表的所有项，每一个项都对应一个节区
            for (int i = 0; i < elf_header.e_shnum; i++, section++)
            {
                //计算该节区名在shstrtab中的偏移地址
                countC = flag_shoff + section->sh_name;
                //将文件指针移动到这个地方
                fseek(file,countC,SEEK_SET);
                //名字字符串，长度为20，名字以'\0'结尾，所以多读一些没有关系
                char string_name[20];
                //从文件中读取名字字符串
                fread(string_name,20,1,file);

                //判断节区名是不是"IA_64",记录unwind节区的索引，这个是给-u用的
                if(!strcmp(string_name,"IA_64")) unwind_idx=i;
                //判断节区名和目标的节区名是否一致，相同的话记录目标节区名对应的索引，这个是给-x用的
                if(!strcmp(string_name,target_section_name)) target_section_idx=i;
            }
        }
    }

    //成功返回
    return 1;
}

const char *ELF_process::get_section_type_name(unsigned int sh_type)
{

    static char buff[32];
    switch (sh_type)
    {
    case SHT_NULL:
        return "NULL";
    case SHT_PROGBITS:
        return "PROGBITS";
    case SHT_SYMTAB:
        return "SYMTAB";
    case SHT_STRTAB:
        return "STRTAB";
    case SHT_RELA:
        return "RELA";
    case SHT_HASH:
        return "HASH";
    case SHT_DYNAMIC:
        return "DYNAMIC";
    case SHT_NOTE:
        return "NOTE";
    case SHT_NOBITS:
        return "NOBITS";
    case SHT_REL:
        return "REL";
    case SHT_SHLIB:
        return "SHLIB";
    case SHT_DYNSYM:
        return "DYNSYM";
    case SHT_INIT_ARRAY:
        return "INIT_ARRAY";
    case SHT_FINI_ARRAY:
        return "FINI_ARRAY";
    case SHT_PREINIT_ARRAY:
        return "PREINIT_ARRAY";
    case SHT_GNU_HASH:
        return "GNU_HASH";
    case SHT_GROUP:
        return "GROUP";
    case SHT_SYMTAB_SHNDX:
        return "SYMTAB SECTION INDICIES";
    case SHT_GNU_verdef:
        return "VERDEF";
    case SHT_GNU_verneed:
        return "VERNEED";
    case SHT_GNU_versym:
        return "VERSYM";
    case 0x6ffffff0:
        return "VERSYM";
    case 0x6ffffffc:
        return "VERDEF";
    case 0x7ffffffd:
        return "AUXILIARY";
    case 0x7fffffff:
        return "FILTER";
    case SHT_GNU_LIBLIST:
        return "GNU_LIBLIST";

    default:
        if ((sh_type >= SHT_LOPROC) && (sh_type <= SHT_HIPROC))
        {


            const char * result;

            switch (elf_header.e_machine)
            {

            case EM_MIPS:
            case EM_MIPS_RS3_LE:
                result = get_mips_section_type_name (sh_type);
                break;
            case EM_PARISC:
                result = get_parisc_section_type_name (sh_type);
                break;
            case EM_IA_64:
                result = get_ia64_section_type_name (sh_type);
                break;
            case EM_X86_64:
            case EM_L1OM:
            case EM_K1OM:
                result = get_x86_64_section_type_name (sh_type);
                break;
            case EM_AARCH64:
                result = get_aarch64_section_type_name (sh_type);
                break;
            case EM_ARM:
                result = get_arm_section_type_name (sh_type);
                break;
            case EM_TI_C6000:
                result = get_tic6x_section_type_name (sh_type);
                break;
            case EM_MSP430:
                result = get_msp430x_section_type_name (sh_type);
                break;
            default:
                result = NULL;
                break;

            }
            if (result != NULL)
                return result;
            sprintf (buff, "LOPROC+%x", sh_type - SHT_LOPROC);
        }
        else if ((sh_type >= SHT_LOOS) && (sh_type <= SHT_HIOS))
        {
            const char * result;

            switch (elf_header.e_machine)
            {
            case EM_IA_64:
                result = get_ia64_section_type_name (sh_type);
                break;
            default:
                result = NULL;
                break;
            }

            if (result != NULL)
                return result;

            sprintf (buff, "LOOS+%x", sh_type - SHT_LOOS);
        }
        else if ((sh_type >= SHT_LOUSER) && (sh_type <= SHT_HIUSER))
            sprintf (buff, "LOUSER+%x", sh_type - SHT_LOUSER);
        else
            /* This message is probably going to be displayed in a 15
               character wide field, so put the hex value first.  */
            snprintf (buff, sizeof (buff), ("%08x: <unknown>"), sh_type);

        return buff;
    }
}

const char *ELF_process::get_mips_section_type_name(unsigned int sh_type)
{

    switch (sh_type)
    {
    case SHT_MIPS_LIBLIST:
        return "MIPS_LIBLIST";
    case SHT_MIPS_MSYM:
        return "MIPS_MSYM";
    case SHT_MIPS_CONFLICT:
        return "MIPS_CONFLICT";
    case SHT_MIPS_GPTAB:
        return "MIPS_GPTAB";
    case SHT_MIPS_UCODE:
        return "MIPS_UCODE";
    case SHT_MIPS_DEBUG:
        return "MIPS_DEBUG";
    case SHT_MIPS_REGINFO:
        return "MIPS_REGINFO";
    case SHT_MIPS_PACKAGE:
        return "MIPS_PACKAGE";
    case SHT_MIPS_PACKSYM:
        return "MIPS_PACKSYM";
    case SHT_MIPS_RELD:
        return "MIPS_RELD";
    case SHT_MIPS_IFACE:
        return "MIPS_IFACE";
    case SHT_MIPS_CONTENT:
        return "MIPS_CONTENT";
    case SHT_MIPS_OPTIONS:
        return "MIPS_OPTIONS";
    case SHT_MIPS_SHDR:
        return "MIPS_SHDR";
    case SHT_MIPS_FDESC:
        return "MIPS_FDESC";
    case SHT_MIPS_EXTSYM:
        return "MIPS_EXTSYM";
    case SHT_MIPS_DENSE:
        return "MIPS_DENSE";
    case SHT_MIPS_PDESC:
        return "MIPS_PDESC";
    case SHT_MIPS_LOCSYM:
        return "MIPS_LOCSYM";
    case SHT_MIPS_AUXSYM:
        return "MIPS_AUXSYM";
    case SHT_MIPS_OPTSYM:
        return "MIPS_OPTSYM";
    case SHT_MIPS_LOCSTR:
        return "MIPS_LOCSTR";
    case SHT_MIPS_LINE:
        return "MIPS_LINE";
    case SHT_MIPS_RFDESC:
        return "MIPS_RFDESC";
    case SHT_MIPS_DELTASYM:
        return "MIPS_DELTASYM";
    case SHT_MIPS_DELTAINST:
        return "MIPS_DELTAINST";
    case SHT_MIPS_DELTACLASS:
        return "MIPS_DELTACLASS";
    case SHT_MIPS_DWARF:
        return "MIPS_DWARF";
    case SHT_MIPS_DELTADECL:
        return "MIPS_DELTADECL";
    case SHT_MIPS_SYMBOL_LIB:
        return "MIPS_SYMBOL_LIB";
    case SHT_MIPS_EVENTS:
        return "MIPS_EVENTS";
    case SHT_MIPS_TRANSLATE:
        return "MIPS_TRANSLATE";
    case SHT_MIPS_PIXIE:
        return "MIPS_PIXIE";
    case SHT_MIPS_XLATE:
        return "MIPS_XLATE";
    case SHT_MIPS_XLATE_DEBUG:
        return "MIPS_XLATE_DEBUG";
    case SHT_MIPS_WHIRL:
        return "MIPS_WHIRL";
    case SHT_MIPS_EH_REGION:
        return "MIPS_EH_REGION";
    case SHT_MIPS_XLATE_OLD:
        return "MIPS_XLATE_OLD";
    case SHT_MIPS_PDR_EXCEPTION:
        return "MIPS_PDR_EXCEPTION";
    default:
        break;
    }
    return NULL;
}

const char *ELF_process::get_parisc_section_type_name(unsigned int sh_type)
{


    switch (sh_type)
    {
    case SHT_PARISC_EXT:
        return "PARISC_EXT";
    case SHT_PARISC_UNWIND:
        return "PARISC_UNWIND";
    case SHT_PARISC_DOC:
        return "PARISC_DOC";
    case SHT_PARISC_ANNOT:
        return "PARISC_ANNOT";
    case SHT_PARISC_SYMEXTN:
        return "PARISC_SYMEXTN";
    case SHT_PARISC_STUBS:
        return "PARISC_STUBS";
    case SHT_PARISC_DLKM:
        return "PARISC_DLKM";
    default:
        break;
    }
    return NULL;
}

const char *ELF_process::get_ia64_section_type_name(unsigned int sh_type)
{


    /* If the top 8 bits are 0x78 the next 8 are the os/abi ID.  */
    if ((sh_type & 0xFF000000) == SHT_IA_64_LOPSREG)
        return get_osabi_name ((sh_type & 0x00FF0000) >> 16);

    switch (sh_type)
    {
    case SHT_IA_64_EXT:
        return "IA_64_EXT";
    case SHT_IA_64_UNWIND:
        return "IA_64_UNWIND";
    case SHT_IA_64_PRIORITY_INIT:
        return "IA_64_PRIORITY_INIT";
    case SHT_IA_64_VMS_TRACE:
        return "VMS_TRACE";
    case SHT_IA_64_VMS_TIE_SIGNATURES:
        return "VMS_TIE_SIGNATURES";
    case SHT_IA_64_VMS_DEBUG:
        return "VMS_DEBUG";
    case SHT_IA_64_VMS_DEBUG_STR:
        return "VMS_DEBUG_STR";
    case SHT_IA_64_VMS_LINKAGES:
        return "VMS_LINKAGES";
    case SHT_IA_64_VMS_SYMBOL_VECTOR:
        return "VMS_SYMBOL_VECTOR";
    case SHT_IA_64_VMS_FIXUP:
        return "VMS_FIXUP";
    default:
        break;
    }
    return NULL;


}

const char *ELF_process::get_x86_64_section_type_name(unsigned int sh_type)
{


    switch (sh_type)
    {
    case SHT_X86_64_UNWIND:
        return "X86_64_UNWIND";
    default:
        break;
    }
    return NULL;

}

const char *ELF_process::get_aarch64_section_type_name(unsigned int sh_type)
{


    switch (sh_type)
    {
    case SHT_AARCH64_ATTRIBUTES:
        return "AARCH64_ATTRIBUTES";
    default:
        break;
    }
    return NULL;



}

const char *ELF_process::get_arm_section_type_name(unsigned int sh_type)
{

    switch (sh_type)
    {
    case SHT_ARM_EXIDX:
        return "ARM_EXIDX";
    case SHT_ARM_PREEMPTMAP:
        return "ARM_PREEMPTMAP";
    case SHT_ARM_ATTRIBUTES:
        return "ARM_ATTRIBUTES";
    case SHT_ARM_DEBUGOVERLAY:
        return "ARM_DEBUGOVERLAY";
    case SHT_ARM_OVERLAYSECTION:
        return "ARM_OVERLAYSECTION";
    default:
        break;
    }
    return NULL;
}

const char *ELF_process::get_tic6x_section_type_name(unsigned int sh_type)
{

    switch (sh_type)
    {
    case SHT_C6000_UNWIND:
        return "C6000_UNWIND";
    case SHT_C6000_PREEMPTMAP:
        return "C6000_PREEMPTMAP";
    case SHT_C6000_ATTRIBUTES:
        return "C6000_ATTRIBUTES";
    case SHT_TI_ICODE:
        return "TI_ICODE";
    case SHT_TI_XREF:
        return "TI_XREF";
    case SHT_TI_HANDLER:
        return "TI_HANDLER";
    case SHT_TI_INITINFO:
        return "TI_INITINFO";
    case SHT_TI_PHATTRS:
        return "TI_PHATTRS";
    default:
        break;
    }
    return NULL;


}

const char *ELF_process::get_msp430x_section_type_name(unsigned int sh_type)
{

    switch (sh_type)
    {
    case SHT_MSP430_SEC_FLAGS:
        return "MSP430_SEC_FLAGS";
    case SHT_MSP430_SYM_ALIASES:
        return "MSP430_SYM_ALIASES";
    case SHT_MSP430_ATTRIBUTES:
        return "MSP430_ATTRIBUTES";
    default:
        return NULL;
    }
}





const char *ELF_process::get_segment_type(unsigned int p_type)
{

    static char buff[32];

    switch (p_type)
    {
    case PT_NULL:
        return "NULL";
    case PT_LOAD:
        return "LOAD";
    case PT_DYNAMIC:
        return "DYNAMIC";
    case PT_INTERP:
        return "INTERP";
    case PT_NOTE:
        return "NOTE";
    case PT_SHLIB:
        return "SHLIB";
    case PT_PHDR:
        return "PHDR";
    case PT_TLS:
        return "TLS";

    case PT_GNU_EH_FRAME:
        return "GNU_EH_FRAME";
    case PT_GNU_STACK:
        return "GNU_STACK";
    case PT_GNU_RELRO:
        return "GNU_RELRO";

    default:
        if ((p_type >= PT_LOPROC) && (p_type <= PT_HIPROC))
        {
            const char * result;

            switch (elf_header.e_machine)
            {
            case EM_AARCH64:
                result = get_aarch64_segment_type (p_type);
                break;
            case EM_ARM:
                result = get_arm_segment_type (p_type);
                break;
            case EM_MIPS:
            case EM_MIPS_RS3_LE:
                result = get_mips_segment_type (p_type);
                break;
            case EM_PARISC:
                result = get_parisc_segment_type (p_type);
                break;
            case EM_IA_64:
                result = get_ia64_segment_type (p_type);
                break;
            case EM_TI_C6000:
                result = get_tic6x_segment_type (p_type);
                break;
            default:
                result = NULL;
                break;
            }

            if (result != NULL)
                return result;

            sprintf (buff, "LOPROC+%x", p_type - PT_LOPROC);
        }
        else if ((p_type >= PT_LOOS) && (p_type <= PT_HIOS))
        {
            const char * result;

            switch (elf_header.e_machine)
            {
            case EM_PARISC:
                result = get_parisc_segment_type (p_type);
                break;
            case EM_IA_64:
                result = get_ia64_segment_type (p_type);
                break;
            default:
                result = NULL;
                break;
            }

            if (result != NULL)
                return result;

            sprintf (buff, "LOOS+%x", p_type - PT_LOOS);
        }
        else
            snprintf (buff, sizeof (buff), ("<unknown>: %x"), p_type);

        return buff;
    }

}

int ELF_process::process_section_group(FILE *file) {
    unsigned int flag=0;
    Elf32_Shdr * section;
    section = NULL;
    char * string_table;
    unsigned int  flag_shoff;
    /* Read in the string table, so that we have names to display.  */
    if (elf_header.e_shstrndx != SHN_UNDEF
        && elf_header.e_shstrndx < elf_header.e_shnum)
    {
        section = section_headers + elf_header.e_shstrndx;

        flag_shoff = section->sh_offset;

    }
    section = section_headers;
    unsigned int countC;
    if (is_32bit_elf){
        
        for (int i = 0;
             i < elf_header.e_shnum;
             i++, section++)
        {
            if(section->sh_type==SHT_GROUP){
                if(!flag){
                    printf("  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n");
                    flag=1;
                }
                printf ("  [%2u] ", i);

                countC = flag_shoff + section->sh_name;

                fseek(file,countC,SEEK_SET);
                char string_name[20];
                fread(string_name,20,1,file);

                printf("%-16s ",string_name);

                printf ( " %-15.15s ",
                        get_section_type_name (section->sh_type));

                printf("%6.8lx",(unsigned long) section->sh_addr);
                printf ( " %6.6lx %6.6lx %2.2lx",
                            (unsigned long) section->sh_offset,
                            (unsigned long) section->sh_size,
                            (unsigned long) section->sh_entsize);

                if (section->sh_flags)
                    printf (" %2.2x ", section->sh_flags);
                else
                    printf("%4c",32);

                printf ("%2u ", section->sh_link);
                printf ("%3u %3lu", section->sh_info,
                        (unsigned long) section->sh_addralign);
                printf("\n");
            }
        }
        if(!flag){
            printf("There are no section group in this file.\n");
            return 0;
        }
    }

    return 1;
}


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
        if(elf_header.e_phnum>1)
            printf ("\nProgram Headers:\n");
        else
            printf ("\nProgram Header:\n");

        if(is_32bit_elf)
            printf("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n");
        else
            printf("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n");

    }


    if (! get_program_headers (file))
        return 0;


    unsigned int i;
    for (i = 0, segment = program_headers;
            i < elf_header.e_phnum;
            i++, segment++)
    {
        printf ("  %-14.14s ", get_segment_type (segment->p_type));

        if(is_32bit_elf)
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


const char *ELF_process::get_aarch64_segment_type(unsigned long type)
{

    switch (type)
    {
    case PT_AARCH64_ARCHEXT:
        return "AARCH64_ARCHEXT";
    default:
        break;
    }

    return NULL;

}

const char *ELF_process::get_arm_segment_type(unsigned long type)
{

    switch (type)
    {
    case PT_ARM_EXIDX:
        return "EXIDX";
    default:
        break;
    }

    return NULL;

}

const char *ELF_process::get_mips_segment_type(unsigned long type)
{

    switch (type)
    {
    case PT_MIPS_REGINFO:
        return "REGINFO";
    case PT_MIPS_RTPROC:
        return "RTPROC";
    case PT_MIPS_OPTIONS:
        return "OPTIONS";
    default:
        break;
    }

    return NULL;
}

const char *ELF_process::get_parisc_segment_type(unsigned long type)
{
    switch (type)
    {
    case PT_HP_TLS:
        return "HP_TLS";
    case PT_HP_CORE_NONE:
        return "HP_CORE_NONE";
    case PT_HP_CORE_VERSION:
        return "HP_CORE_VERSION";
    case PT_HP_CORE_KERNEL:
        return "HP_CORE_KERNEL";
    case PT_HP_CORE_COMM:
        return "HP_CORE_COMM";
    case PT_HP_CORE_PROC:
        return "HP_CORE_PROC";
    case PT_HP_CORE_LOADABLE:
        return "HP_CORE_LOADABLE";
    case PT_HP_CORE_STACK:
        return "HP_CORE_STACK";
    case PT_HP_CORE_SHM:
        return "HP_CORE_SHM";
    case PT_HP_CORE_MMF:
        return "HP_CORE_MMF";
    case PT_HP_PARALLEL:
        return "HP_PARALLEL";
    case PT_HP_FASTBIND:
        return "HP_FASTBIND";
    case PT_HP_OPT_ANNOT:
        return "HP_OPT_ANNOT";
    case PT_HP_HSL_ANNOT:
        return "HP_HSL_ANNOT";
    case PT_HP_STACK:
        return "HP_STACK";
    case PT_HP_CORE_UTSNAME:
        return "HP_CORE_UTSNAME";
    case PT_PARISC_ARCHEXT:
        return "PARISC_ARCHEXT";
    case PT_PARISC_UNWIND:
        return "PARISC_UNWIND";
    case PT_PARISC_WEAKORDER:
        return "PARISC_WEAKORDER";
    default:
        break;
    }

    return NULL;


}

const char *ELF_process::get_ia64_segment_type(unsigned long type)
{

    switch (type)
    {
    case PT_IA_64_ARCHEXT:
        return "IA_64_ARCHEXT";
    case PT_IA_64_UNWIND:
        return "IA_64_UNWIND";
    case PT_HP_TLS:
        return "HP_TLS";
    case PT_IA_64_HP_OPT_ANOT:
        return "HP_OPT_ANNOT";
    case PT_IA_64_HP_HSL_ANOT:
        return "HP_HSL_ANNOT";
    case PT_IA_64_HP_STACK:
        return "HP_STACK";
    default:
        break;
    }

    return NULL;
}
#define PT_C6000_PHATTR        0x70000000

const char *ELF_process::get_tic6x_segment_type(unsigned long type)
{
    switch (type)
    {
    case PT_C6000_PHATTR:
        return "C6000_PHATTR";
    default:
        break;
    }

    return NULL;


}

int ELF_process::get_program_headers(FILE *file)
{

    Elf32_Phdr* phdrs;
    Elf64_Phdr* phdrs64;

    /* Check cache of prior read.  */
    if (program_headers != NULL)
        return 1;

    phdrs = (Elf32_Phdr *) cmalloc (elf_header.e_phnum,
                                    sizeof (Elf32_Phdr));
    phdrs64 = (Elf64_Phdr *) cmalloc (elf_header.e_phnum,
                                    sizeof (Elf64_Phdr));
    if (phdrs == NULL)
    {
        printf("Out of memory\n");
        return 0;
    }

    if (is_32bit_elf
            ? get_32bit_program_headers (file, phdrs)
            : get_64bit_program_headers (file, phdrs64))
    {
        program_headers = phdrs;
        return 1;
    }

    free (phdrs);
    free (phdrs64);
    return 0;



}

int ELF_process::get_32bit_program_headers(FILE *file, Elf32_Phdr *pheaders)
{

    Elf32_External_Phdr* phdrs;
    Elf32_External_Phdr* external;
    Elf32_Phdr* internal;

    unsigned int i;

    phdrs = (Elf32_External_Phdr *) get_data (NULL, file, elf_header.e_phoff,
            elf_header.e_phentsize,
            elf_header.e_phnum,
            ("program headers"));

    if (!phdrs)
        return 0;

    for (i = 0, internal = pheaders, external = phdrs;
            i < elf_header.e_phnum;
            i++, internal++, external++)
    {

        internal->p_type   = BYTE_GET (external->p_type);
        internal->p_offset = BYTE_GET (external->p_offset);
        internal->p_vaddr  = BYTE_GET (external->p_vaddr);
        internal->p_paddr  = BYTE_GET (external->p_paddr);
        internal->p_filesz = BYTE_GET (external->p_filesz);
        internal->p_memsz  = BYTE_GET (external->p_memsz);
        internal->p_flags  = BYTE_GET (external->p_flags);
        internal->p_align  = BYTE_GET (external->p_align);
    }
    free (phdrs);

    return 1;
}

int ELF_process::get_64bit_program_headers(FILE *file, Elf64_Phdr *pheaders)
{


    return 0;
}


int ELF_process::process_dynamic_section(FILE *file)
{

    Elf32_Dyn * entry;

	if(dynamic_addr)
    {
        printf ("\nDynamic section at offset 0x%x contains %u entries:\n",
                dynamic_addr, dynamic_nent);
        printf ("  Tag        Type                         Name/Value\n");
    }else{
    	printf ("\nThere are no dynamic section in this file.\n");
    	return 0;
    }
    if (is_32bit_elf)
    {
        if (! get_32bit_dynamic_section (file))
            return 0;
    }
    else if (! get_64bit_dynamic_section (file))
        return 0;

    for (entry = dynamic_section;
            entry < dynamic_section + dynamic_nent;
            entry++)
    {

        const char * dtype;
        putchar (' ');
        printf("0x%2.8x ",entry->d_tag);
        dtype = get_dynamic_type(entry->d_tag);
        printf("(%s)%*s",dtype,(int)(27-strlen(dtype))," ");


        switch (entry->d_tag)
        {
        case DT_FLAGS:
            print_dynamic_flags (entry->d_un.d_val);
            break;

        case DT_AUXILIARY:
        case DT_FILTER:
        case DT_CONFIG:
        case DT_DEPAUDIT:
        case DT_AUDIT:
            switch (entry->d_tag)
            {
            case DT_AUXILIARY:
                printf ("Auxiliary library");
                break;

            case DT_FILTER:
                printf ("Filter library");
                break;

            case DT_CONFIG:
                printf ("Configuration file");
                break;

            case DT_DEPAUDIT:
                printf ("Dependency audit library");
                break;

            case DT_AUDIT:
                printf ("Audit library");
                break;
            }
            break;

        default:
            printf("0x%x",entry->d_un.d_val);
        }

        printf("\n");
    }


}



int ELF_process::get_32bit_dynamic_section(FILE *file)
{

    Elf32_External_Dyn * edyn = (Elf32_External_Dyn *) malloc(dynamic_size);
    Elf32_External_Dyn * ext;
    Elf32_Dyn * entry;

    fseek(file,dynamic_addr,SEEK_SET);
    fread(edyn,dynamic_size,1,file);


    if(edyn==NULL)
        return 0;

    for (ext = edyn, dynamic_nent = 0;
            (char *) ext < (char *) edyn + dynamic_size;
            ext++)
    {
        dynamic_nent++;
        if (BYTE_GET (ext->d_tag) == DT_NULL)
            break;
    }

    dynamic_section = (Elf32_Dyn *) cmalloc (dynamic_nent,
                      sizeof (* entry));

    if (dynamic_section == NULL)
    {
        printf("Out of memory\n");
        free (edyn);
        return 0;
    }


    for (ext = edyn, entry = dynamic_section;
            entry < dynamic_section + dynamic_nent;
            ext++, entry++)
    {
        entry->d_tag      = BYTE_GET (ext->d_tag);
        entry->d_un.d_val = BYTE_GET (ext->d_un.d_val);
    }

    free(edyn);

    return 1;


}

int ELF_process::get_64bit_dynamic_section(FILE *file)
{

    return 0;
}

void ELF_process::print_dynamic_flags(Elf32_Word flags)
{

    int first = 1;

    while (flags)
    {
        Elf32_Word flag;

        flag = flags & - flags;
        flags &= ~ flag;

        if (first)
            first = 0;
        else
            putc (' ', stdout);

        switch (flag)
        {
        case DF_ORIGIN:
            fputs ("ORIGIN", stdout);
            break;
        case DF_SYMBOLIC:
            fputs ("SYMBOLIC", stdout);
            break;
        case DF_TEXTREL:
            fputs ("TEXTREL", stdout);
            break;
        case DF_BIND_NOW:
            fputs ("BIND_NOW", stdout);
            break;
        case DF_STATIC_TLS:
            fputs ("STATIC_TLS", stdout);
            break;
        default:
            fputs (("unknown"), stdout);
            break;
        }
    }

}
#define DT_FEATURE    0x6ffffdfc
#define DT_USED        0x7ffffffe
const char *ELF_process::get_dynamic_type(unsigned long type)
{

    static char buff[64];

    switch (type)
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

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define FALSE 0
#define TRUE 1
#define UNKNOWN -1
static struct
{
    const char * name;
    int reloc;
    int size;
    int rela;
} dynamic_relocations [] =
{
    { "REL", DT_REL, DT_RELSZ, FALSE },
    { "RELA", DT_RELA, DT_RELASZ, TRUE },
    { "PLT", DT_JMPREL, DT_PLTRELSZ, UNKNOWN }
};
Elf32_Rel* elf32_rel;
int ELF_process::process_relocs(FILE *file)
{

    printf("\nrelocs:");
    Elf32_Rel* entry_rel;

    if(is_32bit_elf)
    {
        get_32bit_rel(file,rel_dyn_offset);
    }
    else
    {
        //64λ...
    }
    printf (" \nat offset 0x%2.2x contains %u entries:\n",
            rel_dyn_offset, rel_nent);
    printf ("  Offset          Info           Type           Sym. Value    Sym. Name\n");
    for (entry_rel = elf32_rel;
            entry_rel < elf32_rel + rel_nent;
            entry_rel++)
    {
        printf("%10.8x ",entry_rel->r_offset);
        printf("%15.8x\n",entry_rel->r_info);
    }


    return 0;
}



void ELF_process::get_32bit_rel(FILE *pFILE, unsigned int offset)
{

    Elf32_External_Rel* rel= (Elf32_External_Rel *) malloc(rel_dyn_size);
    Elf32_External_Rel* ext;
    Elf32_Rel* relt;

    fseek(pFILE,offset,SEEK_SET);
    fread(rel,rel_dyn_size,1,pFILE);

    if(rel==NULL)
        return;

    for (ext = rel, rel_nent = 0;
            (char *) ext < (char *) rel + rel_dyn_size;
            ext++)
    {
        rel_nent++;
        if (BYTE_GET (rel->r_offset) == DT_NULL)
            break;
    }

    elf32_rel = (Elf32_Rel *) cmalloc (dynamic_nent,
                                       sizeof (* relt));


    for (ext = rel, relt = elf32_rel;
            relt < elf32_rel + rel_nent;
            ext++, relt++)
    {
        relt->r_offset     = BYTE_GET (ext->r_offset);
        relt->r_info       = BYTE_GET (ext->r_info);
    }

    free(rel);

    return;
}

void ELF_process::process_symbol_table(FILE *pFILE,int option)
{
    Elf32_Sym* sym; 
    /*定义32位符号表*/
    get_32bit_symbol(pFILE,option);
    unsigned int i;
    if(option & (1<<5))  //-s
    {
	printf("Num  Value   NdX  other    info         Size          Name\n");
        for (i=0, sym = sym_dyn; sym<sym_dyn+sym_nent; sym++,i++)
        {
            printf("%2d: ",i);
            printf("%2.8x ",sym->st_value);
            printf("%-2.2x",sym->st_shndx);
            printf("%-12.2x ",sym->st_other);
            printf("%-12.2x ",sym->st_info);
            printf("%-12.2x ",sym->st_size);
            get_32bit_strdyn(pFILE,sym->st_name);
        }
    }
    if(option & (1<<13)) //-I
    {
	printf("\nbucket list:\n");
	printf("Num     Size          of total \n");
        int sum=0;
        for (i=0, sym = sym_dyn; sym<sym_dyn+sym_nent; sym++,i++)
        {
            sum+=sym->st_size;
        }
        for (i=0, sym = sym_dyn; sym<sym_dyn+sym_nent; sym++,i++)
        {
            printf("%3d:    ",i);
            printf("%-12.2x ",sym->st_size);
            printf(" %f  \n",1.0*(sym->st_size)/sum);
        }
    }
}
/*获取32位的符号表信息*/
void ELF_process::get_32bit_symbol(FILE *pFILE,int option)
{

    Elf32_External_Sym* exty = (Elf32_External_Sym *) malloc(sym_dyn_size);
    Elf32_External_Sym* ext;
    Elf32_Sym* symbool;
    /*获取动态段的偏移和大小*/
    fseek(pFILE,sym_dyn_offset,SEEK_SET);
    fread(exty,sym_dyn_size,1,pFILE);

    if (!exty)
        return;
    for (ext = exty, sym_nent = 0;(char *) ext < (char *) exty + sym_dyn_size;ext++)
    {
        sym_nent++;
    }
    if(option &(1<<5))
    printf ("\nSymbol tabel '.dynsym' contains %d entries\n",sym_nent);

    sym_dyn = (Elf32_Sym *) cmalloc (sym_nent,sizeof (* exty));

    for (ext = exty, symbool = sym_dyn ;symbool < sym_dyn + sym_nent;ext++, symbool++)
    {

        symbool->st_name       = BYTE_GET(ext->st_name);
        symbool->st_info       = BYTE_GET(ext->st_info);
        symbool->st_other      = BYTE_GET(ext->st_other);
        symbool->st_shndx      = BYTE_GET(ext->st_shndx);
        symbool->st_size       = BYTE_GET(ext->st_size);
        symbool->st_value      = BYTE_GET(ext->st_value);

        //printf("%2.2x ",sym_dyn->st_name);
    }

    free(exty);

    return;

}

void ELF_process::get_32bit_strdyn(FILE *pFILE, Elf32_Word name)
{

    unsigned char sym_name[1024];
    fseek(pFILE,(str_dyn_offset+name),SEEK_SET);
    fread(sym_name,1024,1,pFILE);
    printf("%s\n",sym_name);
}

void ELF_process::print_unwind(FILE *file)
{

    if(!strcmp(get_machine_name (elf_header.e_machine),"IA_64"))
    {
	printf("\nThe decoding of unwind sections for machine type IA_64 is waiting for perfection.\n");
	print_xout(file, unwind_idx);
    }
    else
        printf("\nThe decoding of unwind sections for machine type %s is not currently supported.\n",get_machine_name (elf_header.e_machine));
}

int ELF_process::print_xout(FILE *pFILE, unsigned int offset)
{
        if(offset>=elf_header.e_shnum){
	printf("no.[%d] section is not exist!!!\n",offset);
        return 0;
    }
    Elf32_Shdr *shdr=section_headers+offset;
    //Elf32_External_Shdr *ext;
    Elf32_Shdr * section;
    unsigned int  flag_shoff;
    section= section_headers + elf_header.e_shstrndx;
    flag_shoff = section->sh_offset;
    if(!shdr){
    	return 0;
    }
    unsigned int state=0;
    unsigned int addr=shdr->sh_offset;
    unsigned int countC = flag_shoff + shdr->sh_name;
    char str[16]="";
    char data[1000];
    fseek(pFILE,countC,SEEK_SET);
    char string_name[20];
    fread(string_name,20,1,pFILE);
    countC=shdr->sh_offset;
    fseek(pFILE,countC,SEEK_SET);
    fread(data,shdr->sh_size,1,pFILE);
    printf("\nHex dump of section '%s':\n",string_name);   
    for(int i=0;i<shdr->sh_size;i+=4)
	{
		if(state==0) 
		{
			printf("  0x%08x",addr);
			addr+=16;
			i-=4;
		}
		else{ 
			for(int j=0;j<4;j++) 
			{
				printf("%02x",data[i+j]);
				str[(state-1)*4+j]=data[i+j];
			}
		}
		if(state==4) 
		{
			printf(" %d: %s\n",i+4,str);
			state=0;
		}
		else 
		{
			state++;
			printf(" ");
		}
	}
	for(int i=0;i<(state==0?0:5-state);i++) printf("         ");
	if(state) printf("%s\n\n",str);
    return 1;
}




int ELF_process::process_note(FILE *file) {
    int tag=false;
    Elf32_Shdr * shdr;
    Elf32_Nhdr * nhdr;
    int i;
	for(i=0,shdr=section_headers;i < elf_header.e_shnum;i++,shdr++){
        if(shdr->sh_type==SHT_NOTE)
        {
        	tag=true;
            get_32bit_nhdr(file,shdr->sh_offset,shdr->sh_size,nhdr);
        	char * n_name=(char*)nhdr+sizeof(Elf32_Nhdr);
        	printf("\nDisplaying notes found at file offset 0x%8x with the length 0x%8x\n",
            shdr->sh_offset,shdr->sh_size);
        	printf("  Owner            Data size   Description\n");
        	// printf("  %-18s0x%-10x%s\n",n_name,nhdr->n_descsz,note_type_map(nhdr->n_type,n_name));
        	printf("  GNU              0x%-10x%s\n",nhdr->n_descsz,note_type_map(nhdr->n_type,n_name));
            if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_BUILD_ID)
            {
            	char * n_desc=(char*)(n_name+nhdr->n_namesz);
            	printf("    Build ID: ");
            	for(int j=0;j<nhdr->n_descsz/sizeof(char);j++) printf("%x",(unsigned)(unsigned char)n_desc[j]);
            	printf("\n");
            }
            else if(!strcmp(ELF_NOTE_SOLARIS,n_name)&&nhdr->n_type==ELF_NOTE_PAGESIZE_HINT)
            {
            	int * n_desc=(int*)(n_name+nhdr->n_namesz);
            	printf("    Pagesize: 0x%x\n",*n_desc);
            }
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
            else if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_GOLD_VERSION)
            {
            	char * n_desc=(char*)(n_name+nhdr->n_namesz);
            	printf("    Version: %s\n",n_desc);
            }
            else if(!strcmp(ELF_NOTE_GNU,n_name)&&nhdr->n_type==NT_GNU_PROPERTY_TYPE_0)
            {
            	char * n_desc=(char*)(n_name+nhdr->n_namesz);
            	printf("    Properties: x86 feature: IBT, SHSTK\n");
            }
        }
    }

    return 0;
}

const char * ELF_process::note_type_map(unsigned int type, char *name){
    static char buff[32];
    if(strcmp(name,ELF_NOTE_GNU)){
        if(type==ELF_NOTE_PAGESIZE_HINT)
            return "ELF_NOTE_PAGESIZE_HINT";
    }
    switch (type)
    {
        case NT_GNU_ABI_TAG:            return "NT_GNU_ABI_TAG (ABI version tag)";
		case NT_GNU_HWCAP:              return "NT_GNU_HWCAP";
		case NT_GNU_BUILD_ID:           return "NT_GNU_BUILD_ID (unique build ID bitstring)";
		case NT_GNU_GOLD_VERSION:       return "NT_GNU_GOLD_VERSION";
		case NT_GNU_PROPERTY_TYPE_0:    return "NT_GNU_PROPERTY_TYPE_0";
        
    default:
        snprintf (buff, sizeof (buff), ("<unknown: %x>"), type);
        return buff;
    }
}



const char * ELF_process::get_ABI_tag(unsigned int n){
    switch(n)
	{
		case ELF_NOTE_OS_LINUX:return "ELF_NOTE_OS_LINUX";
		case ELF_NOTE_OS_GNU:return "ELF_NOTE_OS_GNU";
		case ELF_NOTE_OS_SOLARIS2:return "ELF_NOTE_OS_SOLARIS2";
		case ELF_NOTE_OS_FREEBSD:return "ELF_NOTE_OS_FREEBSD";
		default:return NULL;
	}
}
int ELF_process::get_32bit_nhdr(FILE *file,unsigned int offset,unsigned int size,Elf32_Nhdr *note){
    Elf32_External_Nhdr* nhdr;
    Elf32_External_Nhdr* external;
    Elf32_Nhdr* internal;

    nhdr = (Elf32_External_Nhdr*) get_data(NULL,file,offset,size,1,("note"));

    if(!nhdr)
        return 0;
    
    internal = note;
    external = nhdr;
    internal->n_namesz  = BYTE_GET(external->n_namesz);
    internal->n_descsz  = BYTE_GET(external->n_descsz);
    internal->n_type    = BYTE_GET(external->n_type);
    free(nhdr);
    return 1;
}


