#include "P2.h"

uint32_t weight( uint32_t x ) {
  x = ( x & 0x55555555 ) + ( ( x >>  1 ) & 0x55555555 );
  x = ( x & 0x33333333 ) + ( ( x >>  2 ) & 0x33333333 );
  x = ( x & 0x0F0F0F0F ) + ( ( x >>  4 ) & 0x0F0F0F0F );
  x = ( x & 0x00FF00FF ) + ( ( x >>  8 ) & 0x00FF00FF );
  x = ( x & 0x0000FFFF ) + ( ( x >> 16 ) & 0x0000FFFF );

  return x;
}

void P2() {
  //int nid = fork();
  //write(0, "hello 2\n", 8);
  int i = 1;
  while( i ) {
    // compute the Hamming weight of each x for 2^8 < x < 2^24
    /*for( uint32_t x = ( 1 << 8 ); x < ( 1 << 9 ); x++ ) {
      uint32_t r = weight( x );
    //  if(nid == 0) write(0, "Child: ", 7);
    //  else write(0, "Parent: ", 8);    
      write(0,"weight(",7);writenum(x);write(0,")=",2);writenum(r);
      write(0,"\n",1);
    }*/
    write(0, "in P2\n", 6);
    i = 0;
  }
}

void (*entry_P2)() = &P2;