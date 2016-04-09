#include "libc.h"

int write( int fd, void* x, size_t n ) {
  int r;
  asm volatile( "mov r0, %1 \n"
                "mov r1, %2 \n"
                "mov r2, %3 \n"
                "svc #1     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );
  return r;
}

int fork() {
  int r;
  asm volatile("svc #2     \n"
               "mov %0, r0 \n" 
             : "=r" (r));
  return r;
}

int exit(){
  int r;
  asm volatile( "svc #3     \n"
                "mov %0, r0 \n"
                : "=r" (r));
  return r;
}

int read(void *command){
  int rd;
  asm volatile( "mov r0, %1 \n"
                "svc #4     \n"
                "mov %0, r0 \n"
                : "=rd" (rd)
                : "rd" (command)
                : "r0");
  return rd;
}

int getlength(int x){
  int i = 10, l = 1, t = 1;
  while(t){ 
    if(x<i){
      t = 0;
      return l;
    } 
    else{
      i = i*10;
      l++;
    }
  }
}

void dig2char(int x){
  switch(x){
    case 1:
      write(0,"1",1);
      break;
    case 2:
      write(0,"2",1);
      break;
    case 3:
      write(0,"3",1);
      break;
    case 4:
      write(0,"4",1);
      break;
    case 5:
      write(0,"5",1);
      break;
    case 6:
      write(0,"6",1);
      break;
    case 7:
      write(0,"7",1);
      break;
    case 8:
      write(0,"8",1);
      break;
    case 9:
      write(0,"9",1);
      break;
    case 0:
      write(0,"0",1);
      break;
    default:
      write(0, "Digit not found ", 16);
      break;
  }
}

void writenum(int x){
  int l = getlength(x), d = 10;
  int dig[l];
  for(int i = 0; i<l;i++){
      dig[i] = x%d;
      x = (x - (x % d))/d;
  }
  for(int i  = l; i>0; i--)dig2char(dig[i-1]);
}