#ifndef __LIBC_H
#define __LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int write( int fd, void* x, size_t n );
int read();
int fork();
int exit();
void writenum(int x);
void dig2char(int x);
int getlength(int x);
#endif
