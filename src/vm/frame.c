#include "vm/frame.h"

static struct list frame_table; //file table


void init_frame_table()
{
    list_init(&frame_table);
}

//palloc_get_page(PAL_USER)을 대체할 함수. flag 없으면 0 넣음, 무조건 PAL_USER 조건에서 실행
void* alloc_get_frame(void* vaddr, enum palloc_flags flags)
{
  void * kaddr = palloc_get_page(PAL_USER | flags);
  
  if(kaddr != NULL)
  {
    struct fte *frame_entry = malloc(sizeof(struct fte)); 
    frame_entry->kaddr = kaddr; 
    frame_entry->uaddr = vaddr;
    frame_entry->t = thread_current();
    list_push_back(&frame_table,&frame_entry->ft_elem);
  }
  else
  {
    return NULL;
  }
  return kaddr;
}

void remove_relevant_frame_entries(struct thread* t)
{
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
}