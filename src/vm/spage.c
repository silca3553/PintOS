#include "vm/spage.h"
#include "userprog/process.h"
#include <string.h>

//hash function: 일단 제일 위에있는거 써봄.
unsigned spt_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
    struct spte *entry = hash_entry(e, struct spte, spt_elem);
    return hash_int( (int)entry->uaddr);
}

//hash 내 두 element 비교 function.
bool spt_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
    return hash_entry(a, struct spte, spt_elem)->uaddr < hash_entry(b, struct spte, spt_elem)->uaddr ;
}

//hash destroy에서 사용하는 destructor
void spte_destructor(struct hash_elem *elem, void *aux UNUSED)
{
    return;
}

struct hash* spt_create()
{
    struct hash* h = (struct hash*) malloc(sizeof(struct hash));
    hash_init(h, spt_hash_func, spt_less_func, NULL);
    return h;
}

struct spte* spt_find(struct hash *spt, void *uaddr)
{
    struct spte temp;
    temp.uaddr = uaddr;
    struct hash_elem *elem = hash_find(spt, &temp.spt_elem);
    return hash_entry(elem, struct spte, spt_elem);
}

bool spt_load(struct spte *e)
{
    switch(e->type)
    {
        case PAGE_FILE:
            if(!spt_load_file(e))
                return false;
            break;
        case PAGE_ZERO:
            if(!spt_load_zero(e))
                return false;
            break;
        case PAGE_SWAP:
            //SWAP 관련 작업 필요
            break;
        default:
            PANIC("NO");
    }
}

bool spt_load_file(struct spte* e)
{
    void* upage = e->uaddr;
    struct file* file = e->file;
    file_seek(file,e->file_offset);
    uint8_t *kpage = alloc_get_frame(upage,0);
    if (kpage == NULL)
      return false;

    if (file_read (file, kpage, e->read_bytes) != (int) e->read_bytes)
    {
        palloc_free_page (kpage);
        return false; 
    }
    
    memset (kpage + e->read_bytes, 0, e->zero_bytes);

    if (!install_page (upage, kpage, e->writable)) 
    {
        palloc_free_page (kpage);
        return false; 
    }
    //spte를 지워야하나..?
    return true;
}

bool spt_load_zero(struct spte* e)
{
    void* kaddr = alloc_get_frame(e->uaddr, PAL_ZERO);
    if (!install_page (e->uaddr, kaddr, e->writable)) 
    {
        palloc_free_page (kaddr);
        return false; 
    }
    return true;
}

bool spt_insert_file(struct hash* spt, void* uaddr, struct file *file, off_t offset, size_t read_bytes, size_t zero_bytes, bool writable)
{
    struct spte* spte = (struct spte *)malloc(sizeof(struct spte));
    spte->type = PAGE_FILE;
    spte->file = file;
    spte->uaddr = uaddr;
    spte->file_offset = offset;
    spte->writable = writable;
    spte->read_bytes = read_bytes;
    spte->zero_bytes = zero_bytes;
    if(hash_insert(spt, &spte->spt_elem)==NULL)
        return true;
    return false;
}