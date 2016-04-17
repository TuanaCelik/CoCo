#ifndef __Philosopher_H
#define __Philosopher_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

typedef struct{
	int id;
	char thought[100];
	char lesson[100];
}philosopher;

#endif