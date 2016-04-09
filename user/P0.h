#ifndef __P0_H
#define __P0_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

extern void (*entry_P0)(); 
extern uint32_t tos_P0;
extern void (*entry_C0)(); 
extern uint32_t tos_C0;

#endif
