#include "kernel.h"

pcb_t pcb[ 7 ], *current  = NULL;
int ids[7];                       //keeps trsck of currently occupied id numbers -> id[0] = 1 -> there is a process with pid 0
int processes             = 1;    //keeps track of the number of processes currently running
char* oneToTwo[1];                //channel for the chatting philosophers
channel chans[5];                 //channel struct for the dining philosophers
int philosopher[5];               //keeps track of the philosopher ids for the dining philosophers              
int ps                    = 0;    //start index for above                             
uint32_t locContent       = 100;  //file contents dtart at location 100 on disk
uint32_t locFileTable     = 0;    //fileTable is an array of a struct called "lookup" that contains the start and finish points
lookup fileTable[10];             //of a file content, as well as its name.. For now fileTable has a capacity of 10 and starts at 0 on disk        
char file[16];                    //used for file name read-ins
int fileNo                = 0 ;   //keeps track of the number of files on disk.
uint32_t stack            = ( uint32_t )(  &tos_terminal )/* + 0x00001000*/; 

void scheduler( ctx_t* ctx , int exiting) {
  pid_t id = current->pid;
  pid_t nid = 0;
  for(int i = 1; i < 7; i++){
    if(pcb[i].priority > pcb[nid].priority) nid = i;
  }
  if ( exiting != 1 ) memcpy( &pcb[ current -> pid ].ctx, ctx, sizeof( ctx_t ) );
  memcpy( ctx, &pcb[ nid].ctx, sizeof( ctx_t ) );
  current = &pcb[ nid ];
}

//***gives available id to child process and puts relevant ids spot to 1***//
int getid(){
  
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
  }
}

//**Calculates the length of input**//
int size(char* input){
  int i = 0;
  while ( *input != '\r'){
    i ++;
    input++;
  }
  return i;
}

//**This is used to read the desired file on the disk by finding the name**//
//**it is also used to delete the right file and check that there are no **//
//**other files with the same name when you want to create a new one*******//
int getFileName(char* input, char* command){
  while(*input != '\r' && *input != '\000'){
    if(*input != *command) return 0;
    input++;
    command++;
  }
  return 1;
}

//**This is used to give write out the correct length when you want to /ls**// 
int nameSize(char* input){
  int i = 0;
  while ( *input != '\r' && *input != '\000'){
    i ++;
    input++;
  }
  return i;
}

//**This is used to calculate the right 'block' size for the file content **//
//**creating a new one******************************************************//
int space( int size ){
  int i = 0;
  int broken = 0;
  
  while ( !broken ){
    i ++;
    if ( size <= 16*i) broken = 1;
  }
  return i;
}


void newPcb(pid_t ppid, pid_t cpid, ctx_t* ctx){
  memcpy((uint32_t*)stack + (cpid-1)*0x00001000, (uint32_t*)stack + (ppid-1)*0x00001000, 0x00001000);
  pcb[cpid].priority = 3;
  pcb[ cpid ].pid      = cpid;
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
  pcb[ 0 ].ctx.sp   = ( uint32_t ) stack;

  current = &pcb[ 0 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

  char name[16];
  char* locat;
  char* fini;
  char* fil;
  int x = 0;
  for(int i = 0; i < 10; i++){
    disk_rd(x, (char*)name, 16);
    x++;
    disk_rd(x, (uint8_t*)&fileTable[i].location, 16);
    x++;
    disk_rd(x, (uint8_t*)&fileTable[i].finish, 16);
    x++;
    disk_rd(x, (uint8_t*)&fileTable[i].filled, 16);
    x++;
    for(int j = 0; j < 16; j++){
      fileTable[i].name[j] = name[j];
      if(name[j] == '\r') j = 16;
    }
    fileNo += fileTable[i].filled;
  }

  return;
}

void kernel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.
  // This part reduces the priority of the currently running process as time goes by
  // this allows my scheduler to work in a mixed round robin and priority based fashion
  pid_t running = current -> pid;
  if( id == GIC_SOURCE_TIMER0 ) { 
    if (running != 0 ) {          
      pcb[ running ].priority -= 2;
    }
    TIMER0->Timer1IntClr = 0x01;
    scheduler(ctx, 0); 
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
      ctx->gpr[ 0 ] = n;
      break;
    }

    case 0x02 : { // fork()
      processes += 1;
      pid_t ppid = current->pid;

      pid_t cpid = getid();
      if(cpid != -1){ 
        newPcb(ppid, cpid, ctx);
        memcpy( &pcb[ cpid ].ctx, ctx, sizeof( ctx_t ) );
        memcpy( &pcb[ ppid ].ctx, ctx, sizeof( ctx_t ) );
        pcb[ cpid ].ctx.sp   = pcb[ ppid ].ctx.sp + (cpid-ppid)*(0x00001000);
        ctx->gpr[0] = cpid;
        pcb[cpid].ctx.gpr[0] = 0;
      }else write(0, "ERROR: Can't create new process, quit a process to make room\n", 61);
      break;
    }

    case 0x03 : { //exit() --> This used to return int for me, now it's void again WATCH OUT
      pid_t toexit = current->pid;
      ids[toexit] = 0;
      processes--;
      memset(&pcb[toexit], 0, sizeof(pcb_t));
      pcb[toexit].priority = -1;
      if(toexit == 0){
        write(0, "Terminal exit succesful", 23);
        pcb[toexit].priority = 0;
        memset(&pcb[1], 0, sizeof(pcb_t));
        memset(&pcb[2], 0, sizeof(pcb_t));
        memset(&pcb[3], 0, sizeof(pcb_t));
        memset(&pcb[4], 0, sizeof(pcb_t));
        memset(&pcb[5], 0, sizeof(pcb_t));
        memset(&pcb[6], 0, sizeof(pcb_t));
        break;
      } 

      scheduler(ctx, 1);
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
    /*case 0x05 : {// runSame(id)  --> This part was for run same functionality I wanted to add for processes that are running in the back
      int id = ctx -> gpr[0];             grounf.. e.g. P0 -> There are 2 P0s running, run P0 1 or 2?
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
      scheduler(ctx, 0);
      break;
    }

    /*case 0x08 : { //send(message, recipient)
      char* message   = (char*)(ctx -> gpr[0]);
      int recipient = ctx -> gpr[1];
      if(recipient == 2 && current -> pid == 1){
          if(oneToTwo[0] == 0 )oneToTwo[0] = (char*)message; 
          else write(0, "Cannot send message until previous one is received\n", 51);      
      }
      ctx->gpr[0] = 1;
      break;
    }

    case 0x09 : {//receive(sender) message from sender
      int sender = ctx -> gpr[0];
      if(sender == 1 && current -> pid == 2){
          if(oneToTwo[0] != 0)pcb[current->pid].ctx.gpr[0] = (char*)oneToTwo[0];
          else{ 
            //write(0, "Waiting for new message\n", 24);
            pcb[current->pid].ctx.gpr[0] = (char*)("Waiting for new message\n");
          }
          oneToTwo[0] = 0;
      } 
      //ctx->gpr[0] = 1;
      break;
    }*/

    case 10 : {
      pcb[current->pid].priority = 5;
      break;
    }
    
    case 11 : { //write to disk
      char* input  = (char*)ctx->gpr[0];
      char* inside = (char*)ctx->gpr[1]; 
      int length1 = size(input);
      int length2 = size(inside);
      
      int block   = space(length2);                          //see what this does at line 79  
      if(fileNo > 0) locContent = fileTable[fileNo-1].finish;//not very efficient, but if there are alerady existent files, 
                                                             //it will start the new content where the last one finishes
      for(int i = 0; i < 16; i++){
        file[i] = input[i];
      }
      //**********check if file name already exists***********//
      int check = 0;
      for(int i = 0; i < fileNo; i++){
        if(getFileName(fileTable[i].name, file)) check = 1;
      }
      //***Create the new file if it does not already exist**//
      if(check == 0){
        disk_wr(locContent, inside, length2);
        fileTable[fileNo].finish = locContent + block;
        for(int i = 0;i <16; i++){
          fileTable[fileNo].name[i] = file[i];
        }

        fileTable[fileNo].location = locContent;
        locContent = fileTable[fileNo].finish;

        fileTable[fileNo].filled = 1;

        fileNo++;
        int x = 0;
        for(int i = 0; i < 10; i++){
          x += fileTable[i].filled;
        }

        //stayPut();
        //*****Update the file table on the disk********//
        for(int i = 0; i < x; i++){
          uint8_t* fLoc = (uint8_t*)fileTable[i].location;
          uint8_t* fFin = (uint8_t*)fileTable[i].finish;
          uint8_t* fFill= (uint8_t*)fileTable[i].filled;
          disk_wr(locFileTable, fileTable[i].name, length1);
          locFileTable++;
          disk_wr(locFileTable, (uint8_t*)&fLoc, sizeof(uint32_t));
          locFileTable++;
          disk_wr(locFileTable, (uint8_t*)&fFin, sizeof(uint32_t));
          locFileTable++;
          disk_wr(locFileTable, (uint8_t*)&fFill, sizeof(uint32_t));
          locFileTable++;
        }
        locFileTable = 0;
      }else if(check == 1) write(0, "\nCould not create file, other file with same name\n", 49);
      
      break;
    }
    
    case 12 : {
      scheduler(ctx, 0);
      break;
    }
    
    case 13 : { //read from disk
      char* name = (char*)ctx->gpr[0];
      char input2[50];
      int a = 0;
      for(int i = 0; i < 10; i++){
        if(getFileName(name, fileTable[i].name)){ 
            int size = fileTable[i].finish - fileTable[i].location;
            int at = 0;
            for(int j = 0; j < size; j++){
              at = fileTable[i].location+j;
              disk_rd(at, input2, 16);
              write(0, input2, 16);
            }
            a = 1;
          }
      }if(a == 0) write(0, "No such file found\n", 19);
      break;
    }
    
    case 14 : {//list file names
      int in;
      for(int i = 0; i < fileNo; i++){
        for(int x = i; x <10; x++){
          i = x;
          if(fileTable[x].filled){
             in = x;
             write(0, (char*)fileTable[in].name, nameSize((char*)fileTable[in].name));
             if(i != fileNo-1)write(0, "\n", 1);
          }
        }
      }
      break;
    }
    
    case 15 : { //setting the channelsws for the dining philosophers to 0 -> nothing is being sent mode
      int first  = ctx -> gpr[0];
      int second = ctx -> gpr[1];
      if(first == 1 && second == 2) chans[0].on[0] = 0;
      else if(first == 2 && second == 3) chans[1].on[0] = 0;
      else if(first == 3 && second == 4) chans[2].on[0] = 0;
      else if(first == 4 && second == 5) chans[3].on[0] = 0;
      else if(first == 1 && second == 5) chans[4].on[0] = 0;
      break;
    }

    case 16 : { //getFork(0 or 1) 0 -> left 1 -> right
      int hand = ctx -> gpr[0];
      int wait = 1;
      if(current -> pid == philosopher[0]){
        if(hand == 0){
          if(chans[4].on[0] != -1){
              chans[4].on[0] = -1;
              write(0, "Philosopher 1 has left fork.\n", 29);
              wait = 0; 
          }
        }
        else{
          if(chans[0].on[0] != -1){
            chans[0].on[0] = -1;
            write(0, "Philosopher 1 has right fork.\n", 30);
            wait = 0;
          }
        } 
      }else if(current -> pid == philosopher[1]){
        if(hand == 0){
          if(chans[0].on[0] != -1){
              chans[0].on[0] = -1;
              write(0, "Philosopher 2 has left fork.\n", 29);
              wait = 0; 
          }
        }
        else{
          if(chans[1].on[0] != -1){
            chans[1].on[0] = -1;
            write(0, "Philosopher 2 has right fork.\n", 30);
            wait = 0;
          }
        } 
      }else if(current -> pid == philosopher[2]){
        if(hand == 0){
          if(chans[1].on[0] != -1){
              chans[1].on[0] = -1;
              write(0, "Philosopher 3 has left fork.\n", 29);
              wait = 0; 
          }
        }
        else{
          if(chans[2].on[0] != -1){
            chans[2].on[0] = -1;
            write(0, "Philosopher 3 has right fork.\n", 30);
            wait = 0;
          }
        } 
      }else if(current -> pid == philosopher[3]){
        if(hand == 0){
          if(chans[2].on[0] != -1){
              chans[2].on[0] = -1;
              write(0, "Philosopher 4 has left fork.\n", 29);
              wait = 0; 
          }
        }
        else{
          if(chans[3].on[0] != -1){
            chans[3].on[0] = -1;
            write(0, "Philosopher 4 has right fork.\n", 30);
            wait = 0;
          }
        } 
      }else if(current -> pid == philosopher[4]){
        if(hand == 0){
          if(chans[3].on[0] != -1){
              chans[3].on[0] = -1;
              write(0, "Philosopher 5 has left fork.\n", 29);
              wait = 0; 
          }
        }
        else{
          if(chans[4].on[0] != -1){
            chans[4].on[0] = -1;
            write(0, "Philosopher 5 has right fork.\n", 30);
            wait = 0;
          }
        } 
      }
      ctx -> gpr[0] = wait;
      break; 
    }

    case 17 : { //leaveFork(0 or 1) 0 -> left 1 -> right
      int hand = ctx -> gpr[0];
      if(current -> pid == philosopher[0]){
        if(hand == 0){
          chans[4].on[0] = 1;
          write(0, "Philosopher 1 has put left fork down\n", 37);
        }
        else{
          chans[0].on[0] = 1;
          write(0, "Philosopher 1 has put right fork down\n", 38);
        }
      }else if(current -> pid == philosopher[1]){
        if(hand == 0){
          chans[0].on[0] = 1;
          write(0, "Philosopher 2 has put left fork down\n", 37);
        }
        else{
          chans[1].on[0] = 1;
          write(0, "Philosopher 2 has put right fork down\n", 38);
        }
      }else if(current -> pid == philosopher[2]){
        if(hand == 0) {
          chans[1].on[0] = 1;
          write(0, "Philosopher 3 has put left fork down\n", 37);
        }
        else{ 
          chans[2].on[0] = 1;
          write(0, "Philosopher 3 has put right fork down\n", 38);
        }
      }else if(current -> pid == philosopher[3]){
        if(hand == 0){
          chans[2].on[0] = 1;
          write(0, "Philosopher 4 has put left fork down\n", 37);
        } 
        else{
          chans[3].on[0] = 1;
          write(0, "Philosopher 4 has put right fork down\n", 38);
        }
      }else if(current -> pid == philosopher[4]){
        if(hand == 0){
          chans[3].on[0] = 1;
          write(0, "Philosopher 5 has put left fork down\n", 37);
        }
        else{
          chans[4].on[0] = 1;
          write(0, "Philosopher 5 has put right fork down\n", 38);
        }
      }
      ctx -> gpr[0] = 1;
      break; 
    }

    case 18 : { //eat()
      int i = 0;
      int c = current -> pid;
      if(c == philosopher[0]) i = 1;
      else if(c == philosopher[1]) i = 2;
      else if(c == philosopher[2]) i = 3;
      else if(c == philosopher[3]) i = 4;
      else if(c == philosopher[4]) i = 5;
      write(0, "Philosopher ", 12); writenum(i); write(0, " is eating...\n", 15);
      break; 
    }

    case 19 : { //setPhils() -> sets the philosopher array to the ids of philiosophers
      int id = ctx -> gpr[0];
      philosopher[ps] = id;
      ps++;
      if(ps == 5) ps = 0;
      break; 
    }

    case 20: {
      ctx -> gpr[0] = current -> pid;
      break;
    }

    case 21 : { //delete file from disk
      char* name = (char*)ctx->gpr[0];
      for(int i = 0; i < 10; i++){
        if(getFileName(name, fileTable[i].name)){ 
            int contentLoc  = fileTable[i].location; 
            int contentSize = fileTable[i].finish - fileTable[i].location;
            disk_wr(i*4, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16);
            disk_wr((i*4)+1, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16);
            disk_wr((i*4)+2, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16);
            disk_wr((i*4)+3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16);
            disk_wr(contentLoc, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16*contentSize);
            for(int x = 0;x <16; x++){
              fileTable[i].name[x] = '\0';
            }
            fileTable[i].location = 0;
            fileTable[i].finish   = 0;
            fileTable[i].filled   = 0;
            fileNo--;

          }
      }
     
      break;
    }
    default   : { // unknown
      break;
    }
  }
}
  