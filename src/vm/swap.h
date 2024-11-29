#include "lib/kernel/hash.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "vm/frame.h"
#include "vm/spage.h"
#include "lib/kernel/bitmap.h"
#include "devices/block.h"

void swap_init(void);
uint32_t swap_out(void *uaddr);
void swap_in(uint32_t swap_idx, void* uaddr);
void swap_free(uint32_t swap_idx);