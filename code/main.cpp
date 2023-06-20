#include <iostream>
#include <cstring>
#include "elf.h"
#include "stdio.h"
#include "ELF_process.h"
#include <stdio.h>

#define SARMAG 8

int main(int argc, char **argv)
{
    char op[255];   // 用于存储选项映射的字符数组
    op['h']=0;      // 为每个选项字符分配对应的数值
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

    int option=0;   // 用于存储合并后的选项
    char *file_name=argv[1];    // ELF 文件的名称
    char *target_section_name;  // 目标段的名称
    
    for(int i=2;i<argc;i++) // 从索引 2 开始遍历命令行参数
    {
        char * tp=argv[i];  // 当前参数
	if(!strcmp(tp,"-x"))    // 检查参数是否为 '-x'
	{
	    target_section_name=argv[++i];  // 将下一个参数作为目标段的名称
	}
        option |= (1<<op[tp[1]]) ;      // 使用按位或运算符计算合并后的选项
    }

    if(!option) option=0x3FFF;   // 如果未指定选项，则设置所有选项
    FILE *file = fopen(file_name,"rb"); // 以二进制模式打开 ELF 文件
    char armag[SARMAG]; // 用于存储文件的魔数


    if (file == NULL)
    {
        printf("Input file '%s' is not readable.\n", file_name);    //无法读取输入文件
        return 0;
    }

    if (fread (armag, SARMAG, 1, file) != 1)
    {
        printf("%s: Failed to read file's magic number\n", file_name);  //无法读取文件的魔数
        fclose (file);
        return 0;
    }

    rewind(file);       // 将文件指针重置到开头
    ELF_process *pro;   // 创建 ELF_process 类的实例
    pro->Process_object(file,option,target_section_name);   // 使用指定的选项处理 ELF 文件
    fclose (file);  // 关闭文件
    //free(pro);    // 释放为 ELF_process 对象分配的内存（不必要，因为它没有动态分配）

    return 0;
}
