#include <iostream>
#include <cstring>
#include "elf.h"
#include "stdio.h"
#include "ELF_process.h"
#include <stdio.h>

#define SARMAG 8

int main(int argc, char **argv)
{
    char op[255];
    op['h']=0;
    op['l']=1;
    op['S']=2;
    op['g']=3;
    op['t']=4;
    op['s']=5;
    op['e']=6;
    op['n']=7;
    op['r']=8;
    op['u']=9;
    op['d']=10;
    op['V']=11;
    op['A']=12;
    op['I']=13;
    op['x']=14;

    int option=0;
    char *file_name=argv[1];
    char *target_section_name;
    for(int i=2;i<argc;i++)
    {
        char * tp=argv[i];
	if(!strcmp(tp,"-x"))
	{
	    target_section_name=argv[++i];
	}
        option |= (1<<op[tp[1]]) ;
    }

    if(!option) option=0x3FFF;
    FILE *file = fopen(file_name,"rb");
    char armag[SARMAG];


    if (file == NULL)
    {
        printf("Input file '%s' is not readable.\n", file_name);
        return 0;
    }

    if (fread (armag, SARMAG, 1, file) != 1)
    {
        printf("%s: Failed to read file's magic number\n", file_name);
        fclose (file);
        return 0;
    }

    rewind(file);
    ELF_process *pro;
    pro->Process_object(file,option,target_section_name);
    fclose (file);
    //free(pro);

    return 0;
}
