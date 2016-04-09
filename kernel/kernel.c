#include "kernel.h"

pcb_t pcb[ 7 ], *current = NULL;
int ids[7];

void scheduler( ctx_t* ctx ) {
  pid_t id = current->pid;
  pid_t nid;
  int b = 1;
  int i = id+1;
  while(b){
    if(ids[i] == 1 && i < sizeof(ids)){
      b = 0;
      nid = i;
    } else if (i == sizeof(ids)) i = 0;
    else i++;
  }
  if(id+1 <= ids) nid = id+1;
  else nid = 0;
  memcpy( &pcb[ id ].ctx, ctx, sizeof( ctx_t ) );
  memcpy( ctx, &pcb[ nid].ctx, sizeof( ctx_t ) );
  current = &pcb[ nid ];
  /*if      ( current == &pcb[ 0 ] ) {
      memcpy( &pcb[ 0 ].ctx, ctx, sizeof( ctx_t ) );
      memcpy( ctx, &pcb[ 1 ].ctx, sizeof( ctx_t ) );
      current = &pcb[ 1 ];
    }
    else if ( current == &pcb[ 1 ] ) {
      memcpy( &pcb[ 1 ].ctx, ctx, sizeof( ctx_t ) );
      memcpy( ctx, &pcb[ 2 ].ctx, sizeof( ctx_t ) );
      current = &pcb[ 2 ];
    }
    else if ( current == &pcb[ 2 ] ) {
      memcpy( &pcb[ 2 ].ctx, ctx, sizeof( ctx_t ) );
      memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
      current = &pcb[ 0 ];
    }*/
}

int getid(){
  int b = 1;
  int i = 0;
  while(b){
    if(ids[i] == 1) i++;
    else{
      b = 0;
      ids[i] = 1;
      return i;
    }
  }
}

void newPcb(pid_t ppid, pid_t cpid){
  memset( &pcb[ cpid ], 0, sizeof( pcb_t ) );
  pcb[ cpid ].pid      = cpid;
  pcb[ cpid ].ctx.cpsr = pcb[ ppid ].ctx.cpsr; 
  pcb[ cpid ].ctx.pc   = pcb[ ppid ].ctx.pc;
  pcb[ cpid ].ctx.sp   = pcb[ ppid ].ctx.sp + ((cpid-ppid)*(0x00001000));
}

void setTimer(){
  TIMER0->Timer1Load     = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl     = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl    |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl    |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl    |= 0x00000080; // enable          timer

  GICC0->PMR             = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER[ 1 ] |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR            = 0x00000001; // enable GIC interface
  GICD0->CTLR            = 0x00000001; // enable GIC distributor

  irq_enable();
}

void kernel_handler_rst(ctx_t* ctx) {
  //setTimer();
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each 
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */
  /*memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 0;
  ids[ 0 ]          = 1;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( entry_P0 );
  pcb[ 0 ].ctx.sp   = ( uint32_t )(  &tos_P0 );

  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 1 ].pid      = 1;
  ids[ 1 ]          = 1;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( entry_P1 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )(  &tos_P1 );

  memset( &pcb[ 2 ], 0, sizeof( pcb_t ) );
  pcb[ 2 ].pid      = 2;
  ids[ 2 ]          = 1;
  pcb[ 2 ].ctx.cpsr = 0x50;
  pcb[ 2 ].ctx.pc   = ( uint32_t )( entry_P2 );
  pcb[ 2 ].ctx.sp   = ( uint32_t )(  &tos_P2 );*/

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 0;
  ids[ 0 ]          = 1;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( entry_terminal );
  pcb[ 0 ].ctx.sp   = ( uint32_t )(  &tos_terminal );

  current = &pcb[ 0 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );


  return;
}

void kernel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    TIMER0->Timer1IntClr = 0x01;
    scheduler(ctx); 
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;
}

void kernel_handler_svc( ctx_t* ctx, uint32_t id ) { 
  /* Based on the identified encoded as an immediate operand in the
   * instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x01 : { // write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++ );
      }
      //scheduler(ctx);
      ctx->gpr[ 0 ] = n;
      break;
    }
    case 0x02 : { // fork()
      pid_t ppid = current->pid;

      pid_t cpid = getid(); 
      newPcb(ppid, cpid);
      memcpy( &pcb[ cpid ].ctx, ctx, sizeof( ctx_t ) );
      ctx->gpr[0] = cpid;
      pcb[cpid].ctx.gpr[0] = 0;
      //current = &pcb[ cpid ];
      scheduler(ctx);
      break;
    }
    case 0x03 : { //exit()
      //write(0, "IN\n", 3);
      pid_t toexit = current->pid;
      ids[toexit] = 0;
      memset(&pcb[toexit], 0, sizeof(pcb_t));
      

      scheduler(ctx);
      ctx->gpr[0] = -1;
      break;
    }  
    case 0x04 : { // read( input ) 
      char*  buffer = ( char* )( ctx->gpr[ 0 ] );
      int index     = 0;
      int broken    = 1;

      while(broken){
        buffer[ index ] = PL011_getc( UART0 );
        if (buffer[ index ] == '\r')   broken = 0;
        PL011_putc( UART0, buffer[ index ] );
        index++;
      }

      ctx -> gpr[ 0 ] = index - 1; 
      break;
    }
    default   : { // unknown
      break;
    }
  }
}

