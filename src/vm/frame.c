#include "vm/frame.h"
#include "vm/swap.h"
static struct list frame_table; //file table
static struct lock ft_lock;

void init_frame_table()
{
    list_init(&frame_table);
    lock_init(&ft_lock);
}

//palloc_get_page(PAL_USER)을 대체할 함수. flag 없으면 0 넣음, 무조건 PAL_USER 조건에서 실행
void* alloc_get_frame(void* vaddr, enum palloc_flags flags)
{
  lock_acquire(&ft_lock);
  void * kaddr = palloc_get_page(PAL_USER | flags);

  if(kaddr == NULL) //SWAP OUT
  {
    struct fte* entry = select_evict_fte();
    
    uint32_t swap_index = swap_out(entry->kaddr);
    spt_insert_swap(entry->t->spt, entry->uaddr, swap_index);
    list_remove(&entry->ft_elem);
    palloc_free_page(entry->kaddr);
    kaddr = palloc_get_page(PAL_USER | flags);
    pagedir_clear_page(entry->t->pagedir, entry->uaddr);
    free(entry);
  }

  struct fte *frame_entry = malloc(sizeof(struct fte)); 
  frame_entry->kaddr = kaddr; 
  frame_entry->uaddr = vaddr;
  frame_entry->t = thread_current();
  frame_entry->is_pinned = true;
  list_push_back(&frame_table,&frame_entry->ft_elem);
  lock_release(&ft_lock);
  return kaddr;
}

void remove_relevant_frame_entries(struct thread* t)
{
   lock_acquire(&ft_lock);
    struct list_elem* e = list_begin (&frame_table);

    while(e != list_end (&frame_table))
    {
      struct fte* frame_entry = list_entry (e, struct fte, ft_elem);
      e = list_next (e);
      if(frame_entry->t == t)
      {
        list_remove(&frame_entry->ft_elem);
        //palloc_free_page(frame_entry->kaddr); //어차피 process_exit에서 page_destory 함
        free(frame_entry);
      }
    }
    lock_release(&ft_lock);
}

struct fte* select_evict_fte()
{
  struct list_elem* elem = list_begin (&frame_table);
  struct fte* entry = NULL;

  while(true)
  { 
    entry = list_entry(elem, struct fte, ft_elem);
    if(entry->is_pinned) continue;
    
    if(pagedir_is_accessed(entry->t->pagedir,entry->uaddr))
    {
      pagedir_set_accessed(entry->t->pagedir,entry->uaddr,false);
      elem = list_next(elem);
      if(elem == list_end(&frame_table))
        elem = list_begin (&frame_table);
    }
    else
      break;
  }
  return entry;
}

void frame_pin(void* uaddr, bool pin)
{
  struct list_elem* e = list_begin (&frame_table);

  while(true)
  {
    struct fte* frame_entry = list_entry (e, struct fte, ft_elem);
    e = list_next (e);
    if(frame_entry -> uaddr == uaddr)
    {
      frame_entry->is_pinned = pin;
      break;
    }
  }
}