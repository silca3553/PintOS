#ifndef SPAGE_H
#define SPAGE_H

#include "lib/kernel/hash.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "vm/frame.h"
#include "vm/swap.h"


enum page_type
{
    PAGE_FILE,
    PAGE_SWAP,
    PAGE_ZERO
};

struct spte
{
    struct hash_elem spt_elem;
    enum page_type type;
    void *uaddr; //이 entry가 의미하는 page address
    struct file* file;
    off_t file_offset;
    bool writable;
    uint32_t swap_idx;
    uint32_t read_bytes;
    uint32_t zero_bytes;
};

unsigned spt_hash_func (const struct hash_elem *e, void *aux UNUSED);
bool spt_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED);
void spte_destructor(struct hash_elem *elem, void *aux UNUSED);

struct hash* spt_create(void);
struct spte* spt_find(struct hash *spt, void *uaddr);

bool spt_load(struct spte *e);
void* spt_load_file(struct spte* e);
void* spt_load_zero(struct spte* e);
void* spt_load_swap(struct spte* e);

//mmap, lazy loading 시 page들을 entry로 만들어 spt에 넣기 함수
bool spt_insert_file(struct hash* spt, void* uaddr, struct file *file, off_t offset, size_t read_bytes, size_t zero_bytes, bool writable);
bool spt_insert_swap(struct hash* spt, void* uaddr, uint32_t swap_index);
bool spt_insert_zero(struct hash* spt, void* uaddr);
//+swap 시 entry로 만들어 spt에 넣기 함수
//address 넣으면 그에 해당하는 spte 찾아서 리턴하는 함수

bool spt_munmap(struct hash* spt, void* uaddr);

#endif