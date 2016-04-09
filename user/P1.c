#include "P1.h"

uint32_t gcd( uint32_t x, uint32_t y ) {
  if     ( x == y ) {
    return x;
  }
  else if( x >  y ) {
    return gcd( x - y, y );
  }
  else if( x <  y ) {
    return gcd( x, y - x );
  }
}

void P1() {
  //int nid = fork();
  //write(0, "hello 1\n", 8);
  int i = 1;
  while( i ) {
    // compute the gcd between pairs of x and y for 2^8 < x, y < 2^24
    /*for( uint32_t x = ( 1 << 4 ); x < ( 1 << 9 ); x++ ) {
       for( uint32_t y = ( 1 << 4 ); y < ( 1 << 8 ); y++ ) {
         uint32_t r = gcd( x, y ); 
        // if(nid == 0) write(0, "Child: ", 7);
        // else write(0, "Parent: ", 8); 
         write(0, "gcd(", 4);writenum(x); write(0, ",", 1);writenum(y);write(0, ")=",2); writenum(r);
         write(0,"\n",1);
       }
     }*/
     write(0, "in P1\n", 6);
     i = 0;
    }
}

void (*entry_P1)() = &P1;