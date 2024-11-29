#include "userprog/syscall.h"

static void syscall_handler (struct intr_frame *f UNUSED);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  struct thread* cur = thread_current();

  if (!is_user_stack_addr(f->esp))
  {
    sys_exit(cur, -1);
  }

  cur->esp = f->esp;

  switch(*((uint32_t*)f->esp)) //syscall number
  {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      sys_exit(cur, *((uint32_t*)(f->esp + 4)));
      break;

    case SYS_EXEC:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      f->eax = process_execute(*((char**)(f->esp + 4)));
      break;

    case SYS_WAIT:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      int ret = process_wait(*((uint32_t*)(f->esp + 4)));
      f->eax = ret;
      break;

    case SYS_CREATE:
      if (!is_user_stack_addr (f->esp+4) || !is_user_stack_addr (f->esp+8))
        sys_exit(cur, -1);
      if (*((char**)(f->esp + 4)) == NULL)
        sys_exit(cur, -1);
      f->eax = filesys_create(*((char**)(f->esp + 4)), *((uint32_t*)(f->esp + 8)));
      break;

    case SYS_REMOVE:
      if (!is_user_stack_addr(f->esp+4))
        sys_exit(cur, -1);
      f->eax = filesys_remove(*((char**)(f->esp + 4)));
      break;

    case SYS_OPEN:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      
      f->eax = sys_open(cur, *((char**)(f->esp + 4)));
      break;

    case SYS_FILESIZE:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      f->eax = sys_filesize(cur, *((uint32_t*)(f->esp + 4)));
      break;

    case SYS_READ:
      if (!is_user_stack_addr (f->esp+4) || !is_user_stack_addr (f->esp+8) || !is_user_stack_addr (f->esp+12))
        sys_exit(cur, -1);
      f->eax = sys_read(cur, *(uint32_t*)(f->esp + 4), *(void**)(f->esp + 8), *(uint32_t*)(f->esp + 12));
      break;

    case SYS_WRITE:
      if (!is_user_stack_addr(f->esp+4) || !is_user_stack_addr (f->esp+8) || !is_user_stack_addr(f->esp+12))
        sys_exit(cur, -1);
      f->eax = sys_write(cur, *((uint32_t*)(f->esp + 4)), *((void**)(f->esp + 8)), *((uint32_t*)(f->esp + 12)));
      break;

    case SYS_SEEK:
      if (!is_user_stack_addr(f->esp+4) || !is_user_stack_addr(f->esp+8))
        sys_exit(cur, -1);
      sys_seek(cur, *((uint32_t*)(f->esp + 4)), *((uint32_t*)(f->esp + 8)));
      break;

    case SYS_TELL:
      if (!is_user_stack_addr(f->esp+4))
        sys_exit(cur, -1);
      f->eax = sys_tell(cur, *((uint32_t*)(f->esp + 4)));
      break;

    case SYS_CLOSE:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      sys_close(cur, *((uint32_t*)(f->esp + 4)));
      break;
    
    case SYS_MMAP:
      if (!is_user_stack_addr(f->esp+4) || !is_user_stack_addr(f->esp+8))
        sys_exit(cur, -1);
      f->eax = sys_mmap(cur, *((uint32_t*)(f->esp + 4)), *((void**)(f->esp + 8)));
      break;
    
    case SYS_MUNMAP:
      if (!is_user_stack_addr (f->esp+4))
        sys_exit(cur, -1);
      sys_munmap(cur, *((uint32_t*)(f->esp + 4)));
      break;

    default:
      /*초기 syscall_handler, 아직 다루지 않는 project 3~4의 syscall들은 여기로*/
      printf ("undefined system call!\n");
      thread_exit();
      break;
  }
}

void sys_exit(struct thread* cur, int status){
  cur->exit_code = status;
  printf("%s: exit(%d)\n", cur->name, cur->exit_code);
  thread_exit();
}

int sys_open(struct thread* cur, const char* f){
  if (f == NULL)
    sys_exit(cur, -1);

  lock_acquire(&filesys_lock);
  struct file* opened = filesys_open(f);
  lock_release(&filesys_lock);

  if(opened == NULL)
    return -1;

  if(cur->fd_count > 127) 
    sys_exit(cur, -1);

  for (int i = 2; i < 128; i++)
  {
    if (cur->fdt[i] == NULL)
    {
      cur->fdt[i] = opened;
      cur->fd_count++;
      return i;
    }
  }
  sys_exit(cur, -1);
  return -1;
}

int sys_filesize(struct thread* cur, int fd){
  struct file* found = cur->fdt[fd];
      if ( found == NULL)
        sys_exit(cur, -1);
      return file_length(found);
}

int sys_read(struct thread* cur, int fd, void *buffer, int size){
  struct file* found = cur->fdt[fd];
  if(!is_user_stack_addr(buffer) || !is_user_stack_addr(buffer + size))
    sys_exit(cur, -1);
  if(fd == 0)
  {
    int count;
    for(count = 0; count <size; count++)
    {
      char c = input_getc();
      *(char*)(buffer + count) = c;
      if(c == '\0')
        break;
    }
    return count;
  }
  else if (fd == 1)
    sys_exit(cur, -1);
  else
  {
    if (found == NULL)
      sys_exit(cur, -1);

    off_t result =  file_read(cur->fdt[fd], buffer, size);
    return result;
  }
  return 0;
}

int sys_write(struct thread* cur, int fd, const void *buffer, unsigned size){
  
  struct file* found = cur->fdt[fd];
  if(!is_user_stack_addr(buffer) || !is_user_stack_addr(buffer + size))
    sys_exit(cur, -1);

  if (fd == 1) //fd == 1: 콘솔에 직접 write
  {
    putbuf (buffer, size);
    return size;
  }
  else if (fd == 0)
  {
    sys_exit(cur, -1);
  }
  else
  {
    if (found == NULL)
    {
      sys_exit(cur, -1);
    }
    else
    {
      off_t result = file_write(found, buffer, size); //if deny_file_write, return 0
      return result;
    }
  }
  return 0;
}

void sys_seek(struct thread* cur, int fd, unsigned position){
    struct file* found = cur->fdt[fd];
      if (found == NULL)
        sys_exit(cur, -1);
    file_seek (found, position);
}

unsigned sys_tell(struct thread* cur, int fd){
  struct file* found = cur->fdt[fd];
  if (found == NULL)
    sys_exit(cur, -1);
  return file_tell(found);
}

void sys_close(struct thread* cur, int fd){
  struct file *close = cur->fdt[fd];
  if (close == NULL)
    sys_exit(cur, -1);
  cur->fdt[fd] = NULL;
  cur->fd_count--;
  file_close(close);    
}

bool is_user_stack_addr(const void* vaddr)
{
  return PHYS_BASE > vaddr && vaddr > 0;
}

int sys_mmap(struct thread* cur, int fd, void* uaddr)
{
  if(fd == 0 || fd == 1 || uaddr == NULL || cur->fdt[fd] == NULL 
  || pg_ofs(uaddr) != 0 || uaddr > PHYS_BASE - 0x800000)
    return -1;

  lock_acquire(&filesys_lock);
  struct file* f = file_reopen(cur->fdt[fd]);
  off_t file_size = file_length(f);
  if(file_size == 0)
  {
    file_close(f);
    lock_release(&filesys_lock);
    return -1;
  }
  off_t offset = 0;
  while(offset < file_size)
  {
    if (spt_find(cur->spt, uaddr + offset) != NULL)
    {
      file_close(f);
      lock_release(&filesys_lock);
      return -1;
    }
    size_t read_bytes = (offset + PGSIZE < file_size ? PGSIZE : file_size - offset);
    size_t zero_bytes = PGSIZE - read_bytes;
    spt_insert_file(cur->spt, uaddr+offset, f, offset, read_bytes, zero_bytes, true);
    offset = offset + PGSIZE;
  }
  struct mmap_entry* e = (struct mmap_entry *)malloc(sizeof(struct mmap_entry));
  e->f = f;
  e->uaddr = uaddr;
  e->mapid = list_size(&cur->mmap_table);
  list_push_back(&cur->mmap_table, &e->mmap_elem);
  lock_release(&filesys_lock);
  return e->mapid;
}

void sys_munmap(struct thread* cur, int mapid)
{
  lock_acquire(&filesys_lock);
  struct list_elem *e = list_begin(&cur->mmap_table);
  while(e != list_end(&cur->mmap_table))
  {
    struct mmap_entry* entry = list_entry(e, struct mmap_entry, mmap_elem);
    e = list_next(e);
    if(entry->mapid == mapid)
    {
      off_t offset = 0;
      off_t file_size = file_length(entry->f);
      void* uaddr = entry->uaddr;
      while(offset < file_size)
      {
        size_t write_bytes = (offset + PGSIZE < file_size ? PGSIZE : file_size - offset);
        spt_munmap(cur->pagedir, cur->spt, uaddr+offset, write_bytes, offset);
        offset = offset + PGSIZE;
      }
      list_remove(&entry->mmap_elem);
      file_close(entry->f);
      free(entry);
      lock_release(&filesys_lock);
      break;
    } 
  }
}

void mummap_all(struct thread* cur)
{
  int size = list_size(&cur->mmap_table);
  for(int mapid=0; mapid<size; mapid++)
     sys_munmap(cur,mapid);
}