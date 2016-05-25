#ifndef __KERNEL_H
#define __KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"
#include "disk.h"

#include "interrupt.h"

#include <string.h>

#include "P0.h"
#include "P1.h"
#include "P2.h"
#include "Philosopher.h"
#include "terminal.h"
#include "disco.h"
#include "table.h"

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef int pid_t;

typedef struct {
  pid_t pid;
  int priority;
  ctx_t ctx;
} pcb_t;

typedef struct {
	int on[1];
} channel;

typedef struct {
	char name[16];
	uint32_t location;
	uint32_t finish; // keep track of the size of the 
	int filled;
}lookup;

#endif
