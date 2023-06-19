#ifndef __MEMORY_H__
#define __MEMORY_H__

// 对内存区封装
typedef struct _memory{
    // 起始地址
    char * ptr;
    // 内存大小
    int len;
} mem_t;

#endif
