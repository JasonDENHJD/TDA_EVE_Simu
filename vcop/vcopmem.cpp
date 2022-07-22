//
// Created by jason on 2022/7/13.
//
#include "vcopmem.h"

#include <iostream>
#include <string>
vcop_memheap_info_t VCOP_MemHeap_Info;
using namespace std;
string VCOP_MEMNAME[] = {"VCOP_IBUFLA",
                         "VCOP_IBUFHA",
                         "VCOP_IBUFLB",
                         "VCOP_IBUFHB",
                         "VCOP_WMEM"
};
void VCOP_Mem_Init(void)
{
    int i = 0;
    for(i = 0; i < VCOP_WMEM; i++)
    {
        auto *meminfo = (vcop_mem_info_t *)((uint32_t )&VCOP_MemHeap_Info + i*(sizeof(vcop_mem_info_t)));
        meminfo->MEM_UESD = 0;
        meminfo->MEM_MAX = 16 * 1024;
    }
    auto *meminfo = (vcop_mem_info_t *)((uint32_t )&VCOP_MemHeap_Info + i*(sizeof(vcop_mem_info_t)));
    meminfo->MEM_UESD = 0;
    meminfo->MEM_MAX = 32 * 1024;
}

void *VCOP_Mem_Malloc(VCOP_MEMHEAP heap, uint16_t size)
{
    vcop_mem_info_t *meminfo = (vcop_mem_info_t *)((uint32_t )&VCOP_MemHeap_Info + heap*(sizeof(vcop_mem_info_t)));
    uint16_t free_size =  meminfo->MEM_MAX - meminfo->MEM_UESD;
    if(free_size >= size)
    {
        void *addr = malloc(sizeof(vcop_mem_data_info_t) + size);
        auto *mem_data_info = (vcop_mem_data_info_t *)addr;
        mem_data_info -> HEAP = heap;
        mem_data_info -> SIZE = size;
        meminfo->MEM_UESD += mem_data_info -> SIZE;
        return addr + sizeof(vcop_mem_data_info_t);
    }
    else
    {
        cout << "malloc failed" << endl;
        exit(0);
        return NULL;
    }
}

void VCOP_Mem_Print_Usage(void)
{
    for(int i = 0; i < VCOP_MEM_ALL; i++)
    {
        auto *meminfo = (vcop_mem_info_t *)((uint32_t )&VCOP_MemHeap_Info + i*(sizeof(vcop_mem_info_t)));
        cout << VCOP_MEMNAME[i] << " : " << meminfo->MEM_UESD / 1024 << "KB" << " / " << meminfo->MEM_MAX / 1024 << "KB" << endl;

    }
}

void VCOP_Mem_Free(void *addr)
{
    addr = addr - sizeof(vcop_mem_data_info_t);
    auto *mem_data_info = (vcop_mem_data_info_t *)addr;
    auto *meminfo = (vcop_mem_info_t *)((uint32_t )&VCOP_MemHeap_Info + mem_data_info -> HEAP*(sizeof(vcop_mem_info_t)));
    meminfo->MEM_UESD -= mem_data_info -> SIZE;
    free(addr);
}