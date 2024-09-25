#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

// 定义页表项的大小
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 512

// 定义页表项的结构
typedef struct {
    uint64_t entries[PAGE_TABLE_ENTRIES];
} PageTable;

// 获取物理页的起始地址
void* get_physical_page() {
    return mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

// 创建一个新的页表
PageTable* create_page_table() {
    PageTable* new_table = (PageTable*)get_physical_page();
    if (new_table == MAP_FAILED) {
        perror("Error allocating page table");
        exit(EXIT_FAILURE);
    }
    // 初始化页表项
    for (int i = 0; i < PAGE_TABLE_ENTRIES; ++i) {
        new_table->entries[i] = 0; // 此处简化为全部初始化为0，实际应根据需要设置
    }
    return new_table;
}

// 切换进程的页表
void switch_page_table(PageTable* new_page_table) {
    // 将新的页表加载到页表基址寄存器
    uint64_t cr3_value = (uint64_t)new_page_table;
    asm volatile("mov %0, %%cr3" : : "r"(cr3_value));
}

int main() {
    // 创建一个新的页表
    PageTable* new_page_table = create_page_table();

    // 切换进程的页表
    switch_page_table(new_page_table);

    // 以下为示例代码，假设切换成功，继续进程的其他工作

    printf("Page table switched successfully.\n");

    return 0;
}

