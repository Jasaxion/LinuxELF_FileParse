#include <iostream>
#include <stdlib.h>
#include <stdio.h>




/* ELF Header (32-bit implementations) */



typedef struct {
    unsigned char    e_ident[16];        /* ELF "magic number" */
    unsigned char    e_type[2];        /* Identifies object file type */
    unsigned char    e_machine[2];        /* Specifies required architecture */
    unsigned char    e_version[4];        /* Identifies object file version */
    unsigned char    e_entry[4];        /* Entry point virtual address */
    unsigned char    e_phoff[4];        /* Program header table file offset */
    unsigned char    e_shoff[4];        /* Section header table file offset */
    unsigned char    e_flags[4];        /* Processor-specific flags */
    unsigned char    e_ehsize[2];        /* ELF header size in bytes */
    unsigned char    e_phentsize[2];        /* Program header table entry size */
    unsigned char    e_phnum[2];        /* Program header table entry count */
    unsigned char    e_shentsize[2];        /* Section header table entry size */
    unsigned char    e_shnum[2];        /* Section header table entry count */
    unsigned char    e_shstrndx[2];        /* Section header string table index */
} Elf32_External_Ehdr;

typedef struct {
    unsigned char    e_ident[16];        /* ELF "magic number" */
    unsigned char    e_type[2];        /* Identifies object file type */
    unsigned char    e_machine[2];        /* Specifies required architecture */
    unsigned char    e_version[4];        /* Identifies object file version */
    unsigned char    e_entry[8];        /* Entry point virtual address */
    unsigned char    e_phoff[8];        /* Program header table file offset */
    unsigned char    e_shoff[8];        /* Section header table file offset */
    unsigned char    e_flags[4];        /* Processor-specific flags */
    unsigned char    e_ehsize[2];        /* ELF header size in bytes */
    unsigned char    e_phentsize[2];        /* Program header table entry size */
    unsigned char    e_phnum[2];        /* Program header table entry count */
    unsigned char    e_shentsize[2];        /* Section header table entry size */
    unsigned char    e_shnum[2];        /* Section header table entry count */
    unsigned char    e_shstrndx[2];        /* Section header string table index */
} Elf64_External_Ehdr;

/* Section header */

typedef struct {
    unsigned char    sh_name[4];        /* Section name, index in string tbl */
    unsigned char    sh_type[4];        /* Type of section */
    unsigned char    sh_flags[4];        /* Miscellaneous section attributes */
    unsigned char    sh_addr[4];        /* Section virtual addr at execution */
    unsigned char    sh_offset[4];        /* Section file offset */
    unsigned char    sh_size[4];        /* Size of section in bytes */
    unsigned char    sh_link[4];        /* Index of another section */
    unsigned char    sh_info[4];        /* Additional section information */
    unsigned char    sh_addralign[4];    /* Section alignment */
    unsigned char    sh_entsize[4];        /* Entry size if section holds table */
} Elf32_External_Shdr;

typedef struct {
    unsigned char    sh_name[4];        /* Section name, index in string tbl */
    unsigned char    sh_type[4];        /* Type of section */
    unsigned char    sh_flags[8];        /* Miscellaneous section attributes */
    unsigned char    sh_addr[8];        /* Section virtual addr at execution */
    unsigned char    sh_offset[8];        /* Section file offset */
    unsigned char    sh_size[8];        /* Size of section in bytes */
    unsigned char    sh_link[4];        /* Index of another section */
    unsigned char    sh_info[4];        /* Additional section information */
    unsigned char    sh_addralign[8];    /* Section alignment */
    unsigned char    sh_entsize[8];        /* Entry size if section holds table */
} Elf64_External_Shdr;

/* Program header */

typedef struct {
    unsigned char    p_type[4];        /* Identifies program segment type */
    unsigned char    p_offset[4];        /* Segment file offset */
    unsigned char    p_vaddr[4];        /* Segment virtual address */
    unsigned char    p_paddr[4];        /* Segment physical address */
    unsigned char    p_filesz[4];        /* Segment size in file */
    unsigned char    p_memsz[4];        /* Segment size in memory */
    unsigned char    p_flags[4];        /* Segment flags */
    unsigned char    p_align[4];        /* Segment alignment, file & memory */
} Elf32_External_Phdr;

typedef struct {
    unsigned char    p_type[4];        /* Identifies program segment type */
    unsigned char    p_flags[4];        /* Segment flags */
    unsigned char    p_offset[8];        /* Segment file offset */
    unsigned char    p_vaddr[8];        /* Segment virtual address */
    unsigned char    p_paddr[8];        /* Segment physical address */
    unsigned char    p_filesz[8];        /* Segment size in file */
    unsigned char    p_memsz[8];        /* Segment size in memory */
    unsigned char    p_align[8];        /* Segment alignment, file & memory */
} Elf64_External_Phdr;


// 注释note header
typedef struct
{
  unsigned char n_namesz[4];			/* Length of the note's name.  */
  unsigned char n_descsz[4];			/* Length of the note's descriptor.  */
  unsigned char n_type[4];			/* Type of the note.  */
} Elf32_External_Nhdr;

typedef struct
{
  unsigned char n_namesz[4];			/* Length of the note's name.  */
  unsigned char n_descsz[4];			/* Length of the note's descriptor.  */
  unsigned char n_type[4];			/* Type of the note.  */
} Elf64_External_Nhdr;





/* dynamic section structure */

typedef struct {
    unsigned char    d_tag[4];        /* entry tag value */
    union {
        unsigned char    d_val[4];
        unsigned char    d_ptr[4];
    } d_un;
} Elf32_External_Dyn;

typedef struct {
    unsigned char    d_tag[8];        /* entry tag value */
    union {
        unsigned char    d_val[8];
        unsigned char    d_ptr[8];
    } d_un;
} Elf64_External_Dyn;



/* Relocation Entries */
typedef struct {
    unsigned char r_offset[4];    /* Location at which to apply the action */
    unsigned char    r_info[4];    /* index and type of relocation */
} Elf32_External_Rel;

typedef struct {
    unsigned char r_offset[4];    /* Location at which to apply the action */
    unsigned char    r_info[4];    /* index and type of relocation */
    unsigned char    r_addend[4];    /* Constant addend used to compute value */
} Elf32_External_Rela;

typedef struct {
    unsigned char r_offset[8];    /* Location at which to apply the action */
    unsigned char    r_info[8];    /* index and type of relocation */
} Elf64_External_Rel;

typedef struct {
    unsigned char r_offset[8];    /* Location at which to apply the action */
    unsigned char    r_info[8];    /* index and type of relocation */
    unsigned char    r_addend[8];    /* Constant addend used to compute value */
} Elf64_External_Rela;





/* Symbol table entry */

typedef struct {
    unsigned char    st_name[4];        /* Symbol name, index in string tbl */
    unsigned char    st_value[4];        /* Value of the symbol */
    unsigned char    st_size[4];        /* Associated symbol size */
    unsigned char    st_info[1];        /* Type and binding attributes */
    unsigned char    st_other[1];        /* No defined meaning, 0 */
    unsigned char    st_shndx[2];        /* Associated section index */
} Elf32_External_Sym;

typedef struct {
    unsigned char    st_name[4];        /* Symbol name, index in string tbl */
    unsigned char    st_info[1];        /* Type and binding attributes */
    unsigned char    st_other[1];        /* No defined meaning, 0 */
    unsigned char    st_shndx[2];        /* Associated section index */
    unsigned char    st_value[8];        /* Value of the symbol */
    unsigned char    st_size[8];        /* Associated symbol size */
} Elf64_External_Sym;




class ELF_process {

    ELF_process();
    void* get_data(void * var, FILE * file, long offset, size_t size, size_t nmemb,
                   const char * reason);
    void *cmalloc (size_t nmemb, size_t size);
    int  get_32bit_section_headers (FILE * file, unsigned int num);
    int  get_file_header(FILE *file);

    int  process_file_header();
    const char*  get_elf_class (unsigned int elf_class);
    const char * get_data_encoding (unsigned int encoding);
    const char * get_osabi_name (unsigned int osabi);
    const char *get_file_type (unsigned e_type);
    const char *get_machine_name (unsigned e_machine);

    int   process_section_headers (FILE * file,int option,char *target_section_name);
    const char *get_section_type_name (unsigned int sh_type);
    const char *get_mips_section_type_name (unsigned int sh_type);
    const char *get_parisc_section_type_name (unsigned int sh_type);
    const char *get_ia64_section_type_name (unsigned int sh_type);
    const char *get_x86_64_section_type_name (unsigned int sh_type);
    const char *get_aarch64_section_type_name (unsigned int sh_type);
    const char *get_arm_section_type_name (unsigned int sh_type);
    const char *get_tic6x_section_type_name (unsigned int sh_type);
    const char *get_msp430x_section_type_name (unsigned int sh_type);



    int  process_program_headers (FILE * file);
    const char *get_segment_type (unsigned int p_type);
    const char *get_aarch64_segment_type (unsigned long type);
    const char *get_arm_segment_type (unsigned long type);
    const char *get_mips_segment_type (unsigned long type);
    const char *get_parisc_segment_type (unsigned long type);
    const char *get_ia64_segment_type (unsigned long type);
    const char *get_tic6x_segment_type (unsigned long type);
    int get_program_headers (FILE * file);
    int get_32bit_program_headers (FILE * file, Elf32_Phdr * pheaders);
    int get_64bit_program_headers (FILE * file, Elf64_Phdr * pheaders);



    int process_dynamic_section (FILE * file);
    int get_32bit_dynamic_section (FILE * file);
    int get_64bit_dynamic_section(FILE * file);


    void print_dynamic_flags (Elf32_Word flags);
    const char *get_dynamic_type (unsigned long type);

    int process_relocs (FILE * file);

    void get_32bit_rel(FILE *pFILE, unsigned int offset);
public:
    int  Process_object(FILE *file,int option,char *target_section_name);

    int  process_section_group(FILE *file);
    int  process_note(FILE *file);
    const char * note_type_map(unsigned int type, char *name);
    const char * get_ABI_tag(unsigned int n);
    int get_32bit_nhdr(FILE *file,unsigned int offset,unsigned int size,Elf32_Nhdr *note);
    
    int process_version(FILE *file);

    void process_symbol_table(FILE *pFILE,int option);

    void get_32bit_symbol(FILE *pFILE,int option);


    void get_32bit_strdyn(FILE *pFILE, Elf32_Word name);

    void print_unwind(FILE *file);

    int print_xout(FILE *pFILE, unsigned int offset);
};
