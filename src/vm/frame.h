#ifndef FRAME_H
#define FRAME_H

#include "lib/kernel/list.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

struct fte
{
    void* kaddr; //kerenl virtual address (frame address + PHYS_BASE)
    void* uaddr; //user virtual address (page address)
    struct thread* t; //process pointer
    struct list_elem ft_elem; //frame table elem
    bool is_pinned; //if true, can not be evicted. (read/write)
};

void init_frame_table(void); //initialize at init.c
void* alloc_get_frame(void* vaddr, enum palloc_flags flags);
void remove_relevant_frame_entries(struct thread* t);
struct fte* select_evict_fte(void);

void frame_pin(void* vaddr, bool pin);

#endif