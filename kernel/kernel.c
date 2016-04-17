#include "kernel.h"

pcb_t pcb[ 7 ], *current = NULL;
int ids[7];
int processes = 1;
char* oneToTwo[7];
int buffer = 0;

void scheduler( ctx_t* ctx ) {
  pid_t id = current->pid;
  pid_t nid = 0;
  for(int i = 1; i < 7; i++){
    if(pcb[i].priority > pcb[nid].priority) nid = i;
  }
  //write(0, "nid is: ", 8); writenum(nid); write(0, "\n", 1);
  memcpy( &pcb[ id ].ctx, ctx, sizeof( ctx_t ) );
  memcpy( ctx, &pcb[ nid].ctx, sizeof( ctx_t ) );
  current = &pcb[ nid ];
}

int getid(){
  //int b = 1;
  //int i = 1;
  if(processes > 7){
    processes = 7;
    return -1;
  }
  else{
    for(int i = 0; i < 7; i++){
      if( ids[i] == 0){ 
        ids[i] = 1; return i;
      }
    }
    return -1;
    /*while(b){
      if(ids[i] == 1){
        if(i < 7) i++;
        else i = 1;
      } 
      else{
        b = 0;
        ids[i] = 1;
        return i;
      }
    }*/
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


}

void kernel_handler_rst(ctx_t* ctx) {
  setTimer();
  irq_enable();
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each 
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 0;
  pcb[ 0 ].priority = 2;
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
  pid_t running = current -> pid;
  if( id == GIC_SOURCE_TIMER0 ) { // you needed to separate the 2 if statements and put the scheduler inside the if statement => 
    if (running != 0 ) {          // this is because there are other irqs than the timer0 => crashed and was stuck because maybe lot of irq ? 
      pcb[ running ].priority -= 2;
    }
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
      processes += 1;
      pid_t ppid = current->pid;

      pid_t cpid = getid();
      if(cpid != -1){ 
        newPcb(ppid, cpid);
        memcpy( &pcb[ cpid ].ctx, ctx, sizeof( ctx_t ) );
        ctx->gpr[0] = cpid;
        pcb[cpid].ctx.gpr[0] = 0;
        pcb[cpid].priority = 3;
        scheduler(ctx);
      }else write(0, "ERROR: Can't create new process, quit a process to make room\n", 61);
        //current = &pcb[ cpid ];
      break;
    }
    case 0x03 : { //exit()
      //write(0, "IN\n", 3);
      pid_t toexit = current->pid;
      ids[toexit] = 0;
      processes--;
      ctx->gpr[0] = -1;
      memset(&pcb[toexit], 0, sizeof(pcb_t));
      pcb[toexit].priority = -1;
      if(toexit == 0){
        //write(0, "terminal exit succesful", 23);
        pcb[toexit].priority = 0;
        memset(&pcb[1], 0, sizeof(pcb_t));
        memset(&pcb[2], 0, sizeof(pcb_t));
        memset(&pcb[3], 0, sizeof(pcb_t));
        memset(&pcb[4], 0, sizeof(pcb_t));
        memset(&pcb[5], 0, sizeof(pcb_t));
        memset(&pcb[6], 0, sizeof(pcb_t));
        break;
      } 

      scheduler(ctx);
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
    /*case 0x05 : {// runSame(id)
      int id = ctx -> gpr[0];
      pcb[id].priority = 3;
      scheduler(ctx);

    }*/
    case 0x06 : { //No of Proceses
      int total = 0;
      for(int i = 0; i<7; i++) total += ids[i];
      write(0, "There are ", 10); writenum(total); write(0, " processes running.\n", 20);
      break;
    }
    case 0x07 : { //exit specific process exitP()
      pid_t toexit = ctx -> gpr[0];
      ids[toexit] = 0;
      ctx->gpr[0] = -1;
      memset(&pcb[toexit], 0, sizeof(pcb_t));
      pcb[toexit].priority = 0;
      processes--;
      scheduler(ctx);
      break;
    }
    case 0x08 : { //send(message, recipient)
      char* message   = (char*)(ctx -> gpr[0]);
      int recipient = ctx -> gpr[1];
      //write(0, "in send\n", 8);
      if(recipient == 2){
        if(buffer < 7){
          oneToTwo[buffer] = (char*)message;
          for( int i = 0; i < 10; i++ ) {
            //PL011_putc( UART0, *message++ );
          }
          buffer++;
        }
        else write(0, "Error: Philosopher 2 has not been receiving, channel is in deadlock\n", 68);
       
      }
      break;
    }
    case 0x09 : {//receive(sender) message from sender
      int sender = ctx -> gpr[0];
      if(sender == 1){
        if(buffer > 0){
          ctx->gpr[0] = (char*)oneToTwo[buffer-1];
         // write(0, "HERE\n",5);
          /*for( int i = 0; i < 3; i++ ) {
            PL011_putc( UART0, *oneToTwo[buffer-1]++ );
          }*/
          buffer--;
        }else write(0, "Philosopher 1 has not said anything new\n", 40);
      } 
      break;
    }
    case 10 : {
      pcb[current->pid].priority = 5;
      //irq_unable();
      break;
    }
    default   : { // unknown
      break;
    }
  }
}
  //Old scheduler
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
  //My first scheduler
  /*int b = 1;
  int i = id+1;
  while(b){
    if(ids[i] == 1 && i < 9){
      b = 0;
      nid = i;
    } else if (i == 8) i = 0;
    else i++;
  }
  if(id+1 <= ids) nid = id+1;
  else nid = 0;*/

  //old handler_rst:
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