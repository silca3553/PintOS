#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "process.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "filesys/file.h"

void syscall_init (void);

void sys_exit(struct thread* cur, int status);
int sys_open(struct thread* cur, const char* f);
int sys_filesize(struct thread* cur, int fd);
int sys_read(struct thread* cur, int fd, void *buffer, int size);
int sys_write(struct thread* cur, int fd, const void *buffer, unsigned size);
void sys_seek(struct thread* cur, int fd, unsigned position);
unsigned sys_tell(struct thread* cur, int fd);
void sys_close(struct thread* cur, int fd);
bool is_user_stack_addr(const void* vaddr);

struct lock filesys_lock;

#endif /* userprog/syscall.h */
