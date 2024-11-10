#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "process.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  struct thread* cur = thread_current();
  //print("syscall number:%d\\n",*((uint32_t*)f->esp));
  switch(*((uint32_t*)f->esp)) //syscall number
  {
    case SYS_HALT:
      shutdown_power_off();

    case SYS_EXIT:
      cur->exit_code = *((uint32_t*)(f->esp + 4));
      printf("%s: exit(%d)\n", cur->name, cur->exit_code);
      sema_up(&cur->sema_wait);
      sema_down(&cur->sema_exit);
      thread_exit();

    case SYS_EXEC:
      process_execute(*((char**)(f->esp + 4)));
      return;
    case SYS_WAIT:
      process_wait(*((uint32_t*)(f->esp + 4)));
      return;
    case SYS_CREATE:
      // return filesys_create(*((char**)(f->esp + 4)), *(f->esp + 8));
      break;
    case SYS_REMOVE:
      // return filesys_remove(*((char**)(f->esp + 4)));
      break;
    case SYS_OPEN:
      // struct file* opened = filesys_open(*((char**)(f->esp + 4)));
      // cur->fdt[fd_count++] = opened;
      // return fd_count - 1;
      break;
    case SYS_FILESIZE:
      // struct file *file = cur->fdt[*((uint32_t*)(f->esp + 4))];
      // return file_length(file);
      break;
    case SYS_READ:
      // struct file *file = cur->fdt[*((uint32_t*)(f->esp + 4))];
      // return file_read(file, *((uint32_t*)(f->esp + 8)), *((uint32_t*)(f->esp + 12)))
      break;
    case SYS_WRITE:
      // struct file *file = cur->fdt[*((uint32_t*)(f->esp + 4))];
      // return file_write(file, *((uint32_t*)(f->esp + 8)), *((uint32_t*)(f->esp + 12)))
      break;
    case SYS_SEEK:
      // struct file *file = cur->fdt[*((uint32_t*)(f->esp + 4))];
      // return file_seek (file, *((uint32_t*)(f->esp + 8)))
      break;
    case SYS_TELL:
      // struct file *file = cur->fdt[*((uint32_t*)(f->esp + 4))];
      // return file_tell(file)
      break;
    case SYS_CLOSE:
      // struct file *file = cur->fdt[*((uint32_t*)(f->esp + 4))];
      // cur->fdt[*((uint32_t*)(f->esp + 4))] = NULL;
      // fd_count--;
      // return file_close(file)
      break;
    default:
      /*초기 syscall_handler, 아직 다루지 않는 project 3~4의 syscall들은 여기로*/
      printf ("system call!\n");
      thread_exit();
      break;
  }
}

