#include "P0.h"

int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}

void P0() {
 // pid_t nid = fork(); 
  int x = 0;
  int i = 1;
  while( i ) {
    // test whether each x for 2^8 < x < 2^24 is prime or not
    for( uint32_t x = ( 1 << 8 ); x < ( 1 << 24 ); x++ ) {
      int r = is_prime( x );
      //if(nid == 0) write(0, "Child: ", 7);
      //else write(0, "Parent: ", 8); 
     // if(nid == 0) exit();
      write(0, "is_prime(",9); writenum(x); write(0, ")=",2); writenum(r);
      write(0, "\n", 1);
    }
    //write(0, "in P0\n", 6);
    i = 0;
  }
  write(0, "This process has terminated\n", 28);
}

void (*entry_P0)() = &P0;
