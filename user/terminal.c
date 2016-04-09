#include "terminal.h"

int getCommand(char* input, char* command){
	while(*input != '\r'){
		if(*input != *command) return 0;
		input++;
		command++;
	}
	return 1;
}
void run(char* input){
	if(getCommand(input, "P0\r")){
		int i = fork();
		if(i == 0){
			//write(0, "in\n", 3);
			P0();
			exit();
		}
		else write(0,"P0\n", 3);
	}
	else if(getCommand(input, "P1\r")){
		int i = fork();
		if(i == 0){
			//write(0, "in\n", 3);
			P1();
			exit();
		}
		else write(0,"P1\n", 3);
	}
	else if(getCommand(input, "P2\r")){
		int i = fork();
		if(i == 0){
			//write(0, "in\n", 3);
			P2();
			exit();
		}
		else write(0,"P2\n", 3);
	}
	else if(getCommand(input, "exit\r")){
		int i = exit();
		if(i == -1) write(0, "exit successful\n", 16);
		else write(0, "ERROR\n", 6);
	}
	else write(0, "Error: Command not found. Program names have to be exact.\n", 58);
}

void terminal() {
  char input[101]; 
  
  while (1){
    //writeNumb(i);
    write( 0, "termite$ ", 9);
    read( input );
    write (0, "\n", 1);
    run(input);
  }
}


void (*entry_terminal)() = &terminal;