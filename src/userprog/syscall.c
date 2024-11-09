#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // switch(*((uint32_t*)f->esp))
  // {
  //   case SYS_HALT:
  //     shutdown_power_off();
  //     break;
  //   case SYS_EXIT:
  //     // struct thread* cur = thread_current();
  //     // cur->exit_code = *(f->esp + 8);
  //     // printf("%s: exit(%d)\n", cur->name, cur->exit_code);
  //     // thread_exit();
  //     break;
  //   case SYS_EXEC:
  //     // return process_execute(*(f->esp + 8));
  //     break;
  //   case SYS_WAIT:
  //     // return process_wait(*(f->esp + 8));
  //     break;
  //   case SYS_CREATE:
  //     // return filesys_create(*(f->esp + 8), *(f->esp + 12));
  //     break;
  //   case SYS_REMOVE:
  //     // return filesys_remove(*(f->esp + 8));
  //     break;
  //   case SYS_OPEN:
  //     // return filesys_open(*(f->esp+8));
  //     break;
  //   case SYS_FILESIZE:
  //     break;
  //   case SYS_READ:
  //     break;
  //   case SYS_WRITE:
  //     break;
  //   case SYS_SEEK:
  //     break;
  //   case SYS_TELL:
  //     break;
  //   case SYS_CLOSE:
  //     break;
  //   default:
  //     /*초기 syscall_handler, 아직 다루지 않는 project 3~4의 syscall들은 여기로*/
      
  //     break;
  // }
  printf ("system call!\n");
  thread_exit ();
}
