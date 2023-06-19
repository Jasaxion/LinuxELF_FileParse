#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <elf.h>

void fileheader(const char *pbuff);

void secheader(const char *pbuff);

void tableheader(const char *pbuff);

void outputsyminfo(const Elf64_Sym *psym, const char *pbuffstr, int ncount);


void proheader(const char *pbuff);

void relocheader(const char *pbuff);

int main(int argc, char * argv[])
{
   if(argc != 3)
   {
        printf("input error!\n");
        return 0;
    }

    //open file
    int fd = open("hello", O_RDONLY, 0);
    if(-1  == fd)
    {
        perror("open file error!\n");
        return 0;
    }
    //calc file size
    long int end = lseek(fd, 0, SEEK_END);
    long int begin = lseek(fd, 0, SEEK_SET);
    //alloc size
    char* pbuff = malloc(end);
    if(!pbuff)
    {
        printf("alloc fail\n");
        return 0;
    }
    memset(pbuff, 0, end);
    //read file
    if(-1 == read(fd, pbuff, end))
    {
        perror("read fail");
        return 0;
    }
    //strcmp command
    if(!strcmp(argv[1], "-h"))
    {
        //read File Header
        fileheader(pbuff);
    }
    else if(!strcmp(argv[1], "-S"))
    {
        //read Section Header
        secheader(pbuff);
    }
    else if(!strcmp(argv[1], "-s"))
    {
        //read symbol table
        tableheader(pbuff);
    }
   else if(!strcmp(argv[1], "-l"))
    {
        //read program header
        proheader(pbuff);
    }
    else if(!strcmp(argv[1], "-r"))
    {
        //read reloc header
        relocheader(pbuff);
    }
    return 0;
}
//read reloc header
void relocheader(const char *pbuff)
{
    //get sectionheader
    Elf64_Ehdr* pfilehead = (Elf64_Ehdr*)pbuff;
    Elf64_Shdr* psecheader = (Elf64_Shdr*)(pbuff + pfilehead->e_shoff);
    Elf64_Shdr* pshstr = (Elf64_Shdr*)(psecheader + pfilehead->e_shstrndx);
    char* pstrbuff = (char*)(pbuff + pshstr->sh_offset);
    for(int i = 0;i<pfilehead->e_shnum;++i)
    {
        if(!strncmp(psecheader[i].sh_name + pstrbuff, ".rel", 4))
        {
            int ncount = psecheader[i].sh_size / psecheader[i].sh_entsize;
            printf("\r\nRelocation section '%s' at offset %0lX contains %d entries:\r\n", psecheader[i].sh_name + pstrbuff, psecheader[i].sh_offset,
                   ncount);
            Elf64_Rela* preltable = (Elf64_Rela*)(pbuff + psecheader[i].sh_offset);

            printf("%-16s  %-16s  %-16s  %-16s  %-16s\r\n", "Offset", "Info", "Type", "Sym.Value", "Sym.Name + Addend");
            int symnum = psecheader[i].sh_link;
            int strnum = psecheader[symnum].sh_link;
            //str addr
            char* prelstrbuf = (char*)(psecheader[strnum].sh_offset + pbuff);
            //symbol
            Elf64_Sym* psym = (Elf64_Sym*)(pbuff + psecheader[symnum].sh_offset);
            for(int n = 0;n<ncount;++n)
            {
                printf("%016lX  %016lX  ", preltable[n].r_offset, preltable[n].r_info);
                switch(ELF64_R_TYPE(preltable[n].r_info))
                {
                    case R_386_NONE:
                        printf("%-16s", "R_386_NONE");break;
                    case R_386_32:
                        printf("%-16s", "R_386_32");break;
                    case R_386_PC32:
                        printf("%-16s", "R_386_PC32");break;
                    case R_386_GOT32:
                        printf("%-16s", "R_386_GOT32");break;
                    case R_386_PLT32:
                        printf("%-16s", "R_386_PLT32");break;
                    case R_386_COPY:
                        printf("%-16s", "R_386_COPY");break;
                    case R_386_GLOB_DAT:
                        printf("%-16s", "R_386_GLOB_DAT");break;
                    case R_386_JMP_SLOT:
                        printf("%-16s", "R_386_JMP_SLOT");break;
                    case R_386_RELATIVE:
                        printf("%-16s", "R_386_RELATIVE");break;
                    case R_386_GOTOFF:
                        printf("%-16s", "R_386_GOTOFF");break;
                    case R_386_GOTPC:
                        printf("%-16s", "R_386_GOTPC");break;
                    default:
                        break;
                }
                printf("  %016lX  ", (psym + ELF64_R_SYM(preltable[n].r_info))->st_value);

                printf("%s + %lu\r\n", (char*)(prelstrbuf + (psym + ELF64_R_SYM(preltable[n].r_info))->st_name), preltable[n].r_addend);
            }

        }
    }
}

//read program header
void proheader(const char *pbuff)
{
    //get program header
    Elf64_Ehdr* pfilehead = (Elf64_Ehdr*)pbuff;
    Elf64_Phdr* pproheader = (Elf64_Phdr*)(pbuff + pfilehead->e_phoff);
    Elf64_Shdr* psecheader = (Elf64_Shdr*)(pbuff + pfilehead->e_shoff);
    Elf64_Shdr* pshstr = (Elf64_Shdr*)(psecheader + pfilehead->e_shstrndx);
    char* pstrbuff = (char*)(pbuff + pshstr->sh_offset);
    printf("Elf file type is");
    switch(pfilehead->e_type)
    {
        case 0:
            printf(" No file type\r\n");
            break;
        case 1:
            printf(" Relocatable file\r\n");
            break;
        case 2:
            printf(" Executable file\r\n");
            break;
        case 3:
            printf(" Shared object file\r\n");
            break;
        case 4:
            printf(" Core file\r\n");
            break;
        default:
            printf(" ERROR\r\n");
            break;
    }
    printf("Entry point 0X%0lX\r\n", pfilehead->e_entry);
    printf("There are %d program headers, starting at offset %lu\r\n\r\n", pfilehead->e_phnum, pfilehead->e_phoff);
    printf("Program Headers:\r\n");
    printf("  %-14s  %-16s  %-16s  %-16s\r\n", "Type", "Offset", "VirtAddr", "PhysAddr");
    printf("  %-14s  %-16s  %-16s  %-6s  %-6s\r\n", "", "FileSiz", "MemSiz", "Flags", "Align");
    //travelist program
    for(int i=0;i<pfilehead->e_phnum;++i)
    {
        //type
        switch(pproheader[i].p_type)
        {
            case PT_NULL:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_LOAD:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "LOAD", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_DYNAMIC:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "DYNAMIC", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_INTERP:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "INTERP", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_NOTE:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "NOTE", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_SHLIB:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "SHLIB", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_PHDR:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "PHDR", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_TLS:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "TLS", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_NUM:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "NUM", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_GNU_EH_FRAME:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "GNU_EH_FRAME", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_GNU_RELRO:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "GNU_RELRO", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            case PT_GNU_STACK:
                printf("  %-14s  %016lX  %016lX  %016lX\r\n  %-14s  %016lX  %016lX  ", "GNU_STACK", pproheader[i].p_offset, pproheader[i].p_vaddr,
                       pproheader[i].p_paddr, "", pproheader[i].p_filesz, pproheader[i].p_memsz);break;
            default:
                break;
        }
        //
        switch(pproheader[i].p_flags)
        {
            case PF_X:
                printf("%-6s  %-lX\r\n", "  E", pproheader[i].p_align);break;
            case PF_W:
                printf("%-6s  %-lX\r\n", " W ", pproheader[i].p_align);break;
            case PF_R:
                printf("%-6s  %-lX\r\n", "R  ", pproheader[i].p_align);break;
            case PF_X|PF_W:
                printf("%-6s  %-lX\r\n", " WE", pproheader[i].p_align);break;
            case PF_X|PF_R:
                printf("%-6s  %-lX\r\n", "R E", pproheader[i].p_align);break;
            case PF_W|PF_R:
                printf("%-6s  %-lX\r\n", "RW ", pproheader[i].p_align);break;
            case PF_X|PF_R|PF_W:
                printf("%-6s  %-lX\r\n", "RWE", pproheader[i].p_align);break;
            default:
                printf("\r\n");
                break;
        }
        if(PT_INTERP == pproheader[i].p_type)
            printf("      [Requesting program interpreter: %s]\r\n", (char*)(pbuff + pproheader[i].p_offset));
    }
    printf("\r\n Section to Segment mapping:\r\n");
    printf("  Segment Sections...\r\n");
    for(int i=0;i<pfilehead->e_phnum;++i)
    {
        printf("   %-7d", i);
        for(int n = 0;n<pfilehead->e_shnum;++n)
        {
            Elf64_Off temp = psecheader[n].sh_addr + psecheader[n].sh_size;
            if((psecheader[n].sh_addr>pproheader[i].p_vaddr && psecheader[n].sh_addr<pproheader[i].p_vaddr + pproheader[i].p_memsz)  ||
                    (temp > pproheader[i].p_vaddr && temp<=pproheader[i].p_vaddr + pproheader[i].p_memsz))
            {
                printf("%s ", (char*)(psecheader[n].sh_name + pstrbuff));
            }
        }
        printf("\r\n");
    }
}

//read symbol table
void tableheader(const char *pbuff)
{
    //get sectionheader
    Elf64_Ehdr* pfilehead = (Elf64_Ehdr*)pbuff;
    Elf64_Half eshstrndx = pfilehead->e_shstrndx;//shstrndex num
    Elf64_Shdr* psecheader = (Elf64_Shdr*)(pbuff + pfilehead->e_shoff);
    Elf64_Shdr* pshstr = (Elf64_Shdr*)(psecheader + eshstrndx);
    char* pshstrbuff = (char *)(pbuff + pshstr->sh_offset);//str addr
    //find .symtab/  .dynsym
    for(int i = 0;i<pfilehead->e_shnum;++i)
    {
        //Is .dynsym
        if(!strcmp(psecheader[i].sh_name + pshstrbuff, ".dynsym") || !strcmp(psecheader[i].sh_name + pshstrbuff, ".symtab"))
        {
            //get address
            Elf64_Sym* psym = (Elf64_Sym*)(pbuff + psecheader[i].sh_offset);
            //get num
            int ncount = psecheader[i].sh_size / psecheader[i].sh_entsize;
            //get str add
            char* pbuffstr = (char*)((psecheader + psecheader[i].sh_link)->sh_offset + pbuff);
            printf("Symbol table '%s' contains %d entries:\r\n", psecheader[i].sh_name + pshstrbuff, ncount);
            outputsyminfo(psym, pbuffstr, ncount);
            continue;
        }
    }
}


//output syminfo
void outputsyminfo(const Elf64_Sym *psym, const char *pbuffstr, int ncount)
{
    printf("%7s  %-8s          %s  %s    %s   %s      %s  %s\r\n",
           "Num:", "Value", "Size", "Type", "Bind", "Vis", "Ndx", "Name");
    for(int i = 0;i<ncount;++i)
    {
        printf("%6d:  %016lx  %-6lu", i, psym[i].st_value, psym[i].st_size);
        char typelow = ELF32_ST_TYPE(psym[i].st_info);
        char bindhig = ELF32_ST_BIND(psym[i].st_info);
        switch(typelow)
        {
            case STT_NOTYPE:
                printf("%-8s", "NOTYPE");break;
            case STT_OBJECT:
                printf("%-8s", "OBJECT");break;
            case STT_FUNC:
                printf("%-8s", "FUNC");break;
            case STT_SECTION:
                printf("%-8s", "SECTION");break;
            case STT_FILE:
                printf("%-8s", "FILE");break;
            default:
                break;
        }
        switch(bindhig)
        {
            case STB_LOCAL:
                printf("%-8s", "LOCAL"); break;
            case STB_GLOBAL:
                printf("%-8s", "GLOBAL"); break;
            case STB_WEAK:
                printf("%-8s", "WEAK"); break;
            default:
                break;
        }
        printf("%-8d", psym[i].st_other);
        switch(psym[i].st_shndx)
        {
            case SHN_UNDEF:
                printf("%s  %s\r\n", "UND", psym[i].st_name + pbuffstr);break;
            case SHN_ABS:
                printf("%s  %s\r\n", "ABS", psym[i].st_name + pbuffstr);break;
            case SHN_COMMON:
                printf("%s  %s\r\n", "COM", psym[i].st_name + pbuffstr);break;
            default:
                printf("%3d  %s\r\n", psym[i].st_shndx, psym[i].st_name + pbuffstr);break;
        }
    }
}

//read Section Header
void secheader(const char *pbuff)
{
    //get number Section
    int nNumSec = *(Elf64_Half*)(pbuff + 60);
    //get shstrndex
    Elf64_Ehdr* pfilehead = (Elf64_Ehdr*)pbuff;
    Elf64_Half eshstrndx = pfilehead->e_shstrndx;
    //get section offset
    Elf64_Shdr* psecheader = (Elf64_Shdr*)(pbuff + pfilehead->e_shoff);
    Elf64_Shdr* pshstr = (Elf64_Shdr*)(psecheader + eshstrndx);
    char* pshstrbuff = (char *)(pbuff + pshstr->sh_offset);
    //output info
    printf("There are %d section headers, starting at offset 0x%lx:\r\n\r\n",
           nNumSec, *(Elf64_Off*)(pbuff + 40));
    printf("Section Headers:\r\n");
    printf("  [Nr] %-16s  %-16s  %-16s  %-16s\r\n", "Name", "Type", "Address", "Offset");
    printf("       %-16s  %-16s  %-5s  %-5s  %-5s  %-5s\r\n", "Size", "EntSize", "Flags", "Link", "Info", "Align");
    //travlest
    for(int i = 0;i<nNumSec;++i)
    {
        printf("  [%2d] %-16s  ", i, (char *)(psecheader[i].sh_name + pshstrbuff));
        //Type
        switch(psecheader[i].sh_type)
        {
            case SHT_NULL:
                printf("%-16s  ", "NULL");break;
            case SHT_PROGBITS:
                printf("%-16s  ", "PROGBITS");break;
            case SHT_SYMTAB:
                printf("%-16s  ", "SYMTAB");break;
            case SHT_STRTAB:
                printf("%-16s  ", "STRTAB");break;
            case SHT_RELA:
                printf("%-16s  ", "RELA");break;
            case SHT_HASH:
                printf("%-16s  ", "GNU_HASH");break;
            case SHT_DYNAMIC:
                printf("%-16s  ", "DYNAMIC");break;
            case SHT_NOTE:
                printf("%-16s  ", "NOTE");break;
            case SHT_NOBITS:
                printf("%-16s  ", "NOBITS");break;
            case SHT_REL:
                printf("%-16s  ", "REL");break;
            case SHT_SHLIB:
                printf("%-16s  ", "SHLIB");break;
            case SHT_DYNSYM:
                printf("%-16s  ", "DYNSYM");break;
            case SHT_INIT_ARRAY:
                printf("%-16s  ", "INIT_ARRY");break;
            case SHT_FINI_ARRAY:
                printf("%-16s  ", "FINI_ARRY");break;
            case SHT_PREINIT_ARRAY:
                printf("%-16s  ", "PREINIT_ARRAY");break;
            case SHT_GNU_HASH:
                printf("%-16s  ", "GNU_HASH");break;
            case SHT_GNU_ATTRIBUTES:
                printf("%-16s  ", "GNU_ATTRIBUTES");break;
            case SHT_GNU_LIBLIST:
                printf("%-16s  ", "GNU_LIBLIST");break;
            case SHT_GNU_verdef:
                printf("%-16s  ", "GNU_verdef");break;
            case SHT_GNU_verneed:
                printf("%-16s  ", "GNU_verneed");break;
            case SHT_GNU_versym:
                printf("%-16s  ", "GNU_versym");break;
            default:
                printf("%-16s  ", "NONE");break;
        }
        printf("%016lX  %08lX\r\n", psecheader[i].sh_addr, psecheader[i].sh_offset);
        printf("       %016lX  %016lx  ", psecheader[i].sh_size, psecheader[i].sh_entsize);
            switch (psecheader[i].sh_flags) {
                case 0:
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 1:
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "W", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 2:
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "A", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 4:
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "X", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 3:
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "WA", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 5://WX
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "WX", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 6://AX
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "AX", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case 7://WAX
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "WAX", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                case SHF_MASKPROC://MS
                    printf("%3s    %4u  %4u  %4lu\r\n",
                           "MS", psecheader[i].sh_link, psecheader[i].sh_info, psecheader[i].sh_addralign);
                    break;
                default:
                    printf("NONE\r\n");
                    break;
            }

    }
    printf("Key to Flags:\r\n");
    printf("  W (write), A (alloc), X (execute), M (merge), S (strings), l (large)\r\n");
    printf("  I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)\r\n");
    printf("  O (extra OS processing required) o (OS specific), p (processor specific)\r\n");
}

//read File Header
void fileheader(const char *pbuff)
{
    printf("ELF Header:\r\n");
    //Magic
    printf("  Magic:   ");
    for(int i = 0;i<EI_NIDENT;++i)
    {
        printf("%02X", pbuff[i]);
        putchar(' ');
    }
    printf("\r\n");
    //Class
    printf("  %-33s:", "Class");
    switch(pbuff[4])
    {
        case 0:
            printf(" Invalid class\r\n");
            break;
        case 1:
            printf(" ELF32\r\n");
            break;
        case 2:
            printf(" ELF64\r\n");
            break;
        default:
            printf(" ERROR\r\n");
            break;
    }
    //Data
    printf("  %-33s:", "Data");
    switch(pbuff[5])
    {
        case 0:
            printf(" Invalid data encoding\r\n");
            break;
        case 1:
            printf(" 2's complement, little endian\r\n");
            break;
        case 2:
            printf(" 2's complement, big endian\r\n");
            break;
        default:
            printf(" ERROR\r\n");
            break;
    }
    //Version
    printf("  %-33s: %s\r\n", "Version", "1(current)");
    //OS/ABI
    printf("  %-33s: %s\r\n", "OS/ABI", "UNIX - System V");
    //ABI Version
    printf("  %-33s: %s\r\n", "ABI Version", "0");
    pbuff += EI_NIDENT;
    //Type
    printf("  %-33s:", "Type");
    switch(*(uint16_t*)pbuff)
    {
        case 0:
            printf(" No file type\r\n");
            break;
        case 1:
            printf(" Relocatable file\r\n");
            break;
        case 2:
            printf(" Executable file\r\n");
            break;
        case 3:
            printf(" Shared object file\r\n");
            break;
        case 4:
            printf(" Core file\r\n");
            break;
        default:
            printf(" ERROR\r\n");
            break;
    }
    pbuff += sizeof(uint16_t);
    //Machine
    printf("  %-33s:", "Machine");
    switch(*(uint16_t*)pbuff)
    {
        case EM_386:
            printf(" Intel 80386\r\n");
            break;
        case EM_ARM:
            printf(" ARM\r\n");
            break;
        case EM_X86_64:
            printf(" AMD X86-64 arrchitecture\r\n");
            break;
        default:
            printf(" ERROR\r\n");
            break;
    }
    pbuff += sizeof(uint16_t);
    //Version
    printf("  %-33s: %s\r\n", "version", "0X1");
    pbuff += sizeof(uint32_t);
    //Entry point address
    printf("  %-33s: 0X%lx\r\n", "Entry point address", *(uint64_t*)pbuff);
    pbuff += sizeof(uint64_t);
    //Start of program headers
    printf("  %-33s: %lu (bytes into file)\r\n", "Start of program headers", *(uint64_t*)pbuff);
    pbuff += sizeof(uint64_t);
    //Start of section headers
    printf("  %-33s: %lu (bytes into file)\r\n", "Start of section headers", *(uint64_t*)pbuff);
    pbuff += sizeof(uint64_t);
    //Flags
    printf("  %-33s: 0X0\r\n", "Flags");
    pbuff += sizeof(Elf32_Word);
    //Size of this header
    printf("  %-33s: %d (bytes)\r\n", "Size of this header", *(Elf32_Half*)pbuff);
    pbuff += sizeof(Elf32_Half);
    //Size of program headers
    printf("  %-33s: %d (bytes)\r\n", "Size of program headers", *(Elf32_Half*)pbuff);
    pbuff += sizeof(Elf32_Half);
    //Number of program headers
    printf("  %-33s: %d\r\n", "Number of program headers", *(Elf32_Half*)pbuff);
    pbuff += sizeof(Elf32_Half);
    //Size of section headers
    printf("  %-33s: %d (bytes)\r\n", "Size of section headers", *(Elf32_Half*)pbuff);
    pbuff += sizeof(Elf32_Half);
    //Number of section headers
    printf("  %-33s: %d\r\n", "Number of section headers", *(Elf32_Half*)pbuff);
    pbuff += sizeof(Elf32_Half);
    //Section header string table index
    printf("  %-33s: %d\r\n", "Section header string table index", *(Elf32_Half*)pbuff);
}
