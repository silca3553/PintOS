#include "vm/swap.h"

struct block* swap_slot;
struct bitmap* swap_table;
size_t swap_size;

void swap_init()
{
    swap_slot = block_get_role(BLOCK_SWAP);
    swap_size = block_size(swap_slot) * BLOCK_SECTOR_SIZE / PGSIZE;
    //swap slot의 sector 개수 * block 별 sector bytes / PGSIZE = swap slot page 단위 개수
    swap_table = bitmap_create(swap_size);
}

uint32_t swap_out(void *kaddr)
{
    uint32_t idx = bitmap_scan_and_flip(swap_table, 0, 1, false);
    for(size_t i=0; i < PGSIZE / BLOCK_SECTOR_SIZE; i++)
        block_write(swap_slot, idx*(PGSIZE / BLOCK_SECTOR_SIZE) + i, kaddr + BLOCK_SECTOR_SIZE * i);
    return idx;
}

void swap_in(uint32_t swap_idx, void* kaddr)
{
    for(size_t i=0; i < PGSIZE / BLOCK_SECTOR_SIZE; i++)
        block_read(swap_slot, swap_idx*(PGSIZE / BLOCK_SECTOR_SIZE) + i, kaddr + BLOCK_SECTOR_SIZE * i);
    bitmap_set(swap_table, swap_idx, false);
}

void swap_free(uint32_t swap_idx)
{
    bitmap_set(swap_table, swap_idx, false);
}