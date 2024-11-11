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
      if ( !is_user_stack_addr (*((char**)(f->esp + 4))) ) //FILE이 valid한가?
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
  if(is_user_stack_addr(found) ||!is_user_stack_addr(buffer) || !is_user_stack_addr(buffer + size))
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
  if(is_user_stack_addr(found) ||!is_user_stack_addr(buffer) || !is_user_stack_addr(buffer + size))
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
  return PHYS_BASE > vaddr &&  (void*)(1 << 27) <= vaddr;
}