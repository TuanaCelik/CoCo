#include "terminal.h"

int running[3];
int np0s[6];
int np1s[6];
int np2s[6];
int allIds[7];
int ipc[10];

void update(int op, int input[6], int id){
	if(op == 1){
		for(int i = 0; i < 6; i++){
			if( input[i] == 0 ){
				//running[0] = 1;
				if (allIds[id] == 0){
				 	allIds[id] = 1;
				 	input[i] = id;
				}
				i = 7;
			}
		}
	}else if(op == 0){
		for(int i = 0; i < 6; i++){
			if(input[i] == id){
				if (allIds[id] == 1){
					allIds[id] = 0;
					input[i] = 0; 
				}
				i = 7;
			} 
		}
	}
	
}

int noOfP(int input[6]){
	int no = 0;
	for(int i = 0; i <6; i++){
		if(input[i] !=0) no++;
	}
	return no;
}

int findId(int input[6]){
	for(int i = 0; i < 6; i++) if(input[i] != 0) return input[i];
}

int getCommand(char* input, char* command){
	while(*input != '\r'){
		if(*input != *command) return 0;
		input++;
		command++;
	}
	return 1;
}

int char2int(char* input){
	if(getCommand(input, "1\r")) return 1;
	else if(getCommand(input, "2\r")) return 2;
	else if(getCommand(input, "3\r")) return 3;
	else if(getCommand(input, "4\r")) return 4;
	else if(getCommand(input, "5\r")) return 5;
	else if(getCommand(input, "6\r")) return 6;
	//else if(getCommand(input, "all\r")) return 100;
	
}

void run(char* input){
	if(getCommand(input, "P0\r")){
		    int i = fork();
			if(i == 0){
				P0();
				//exit();
			}
			else update(1, np0s, i);
			//return ;
	}

	else if(getCommand(input, "P1\r")){
			int i = fork();
			if(i == 0){
				P1();
				//exit();
			}
			else update(1, np1s, i);		
	}

	else if(getCommand(input, "P2\r")){
			int i = fork();
			if(i == 0){
				P2();
				//exit();
			}
			else update(1, np2s, i);
	}

	else if(getCommand(input, "Philosopher1\r")){
		int i = fork();
		if(i == 0){
			//stayPut();
			Philosopher1();
			exit();
		}
		else update(1, ipc, i);
	}

	else if(getCommand(input, "Philosopher2\r")){
		int i = fork();
		if(i == 0){
			//stayPut();
			Philosopher2();
			exit();
		}
		else update(1, ipc, i);
	}

	else if(getCommand(input, "quit P0\r")){
		char input[3];
		if(noOfP(np0s) > 1){
			write(0, "There are multiplie processes of P0 running with ids:\n", 54);
			for(int i = 0; i<6; i++) {
				if(np0s[i] != 0){
					writenum(np0s[i]); write(0, " ", 1);
				}//testIPC(ipc);
			}
			write(0, "\n", 1);write(0, "Enter id you want to quit: ", 27); read(input); 
			exitP(char2int(input)); update(0, np0s, char2int(input)); //running[0] = findId(np0s);
		}else if(noOfP(np0s) == 1){
			exitP(findId(np0s));
			//running[0] = 0;
			update(0, np0s, findId(np0s));
		}else if(noOfP(np0s) == 0) write(0, "There are no P0s running currently\n", 35);
	}

	else if(getCommand(input, "quit P1\r")){
		char input[3];
		if(noOfP(np1s) > 1){
			write(0, "There are multiple processes of P1 running with ids:\n", 54);
			for(int i = 0; i<6; i++) {
				if(np1s[i] != 0){
					writenum(np1s[i]); write(0, " ", 1);
				}
			}
			write(0, "\n", 1);write(0, "Enter id you want to quit: ", 27); read(input); 
			exitP(char2int(input)); update(0, np1s, char2int(input));//running[1] = findId(np1s);
		}else if(noOfP(np1s) == 1){
			exitP(findId(np1s));
			//running[1] = 0;
			update(0, np1s, findId(np1s));
		}else if(noOfP(np1s) == 0) write(0, "There are no P1s running currently\n", 35);
	}

	else if(getCommand(input, "quit P2\r")){
		char input[3];
		if(noOfP(np2s) > 1){
			write(0, "There are multiple processes of P2 running with ids:\n", 54);
			for(int i = 0; i<6; i++) {
				if(np2s[i] != 0){
					writenum(np2s[i]); write(0, " ", 1);
				}
			}
			write(0, "\n", 1);write(0, "Enter id you want to quit: ", 27); read(input); 
			exitP(char2int(input)); update(0, np2s, char2int(input)); //running[2] = findId(np2s);
		}else if(noOfP(np2s) == 1){
			exitP(findId(np2s));
			//running[2] = 0;
			update(0, np2s, findId(np0s));
		}else if(noOfP(np2s) == 0) write(0, "There are no P2s running currently\n", 35);
	}

	else if(getCommand(input, "/ps\r")) noOfProc();
	else if(getCommand(input, "exit\r")){
		int i = exit();
		if(i == -1) write(0, "Terminal exit successful\n", 25);
		else write(0, "ERROR\n", 6);
	}
	else write(0, "Error: Command not found.\n", 26);
}

void terminal() {
  char input[101]; 
  int i = 0;
  int b = 1;
  while (b){
    //writeNumb(i);
    if(i == 0) write( 0, "termite$ ", 9);
    else write(0, "\ntermite$ ", 11);
    read( input );
    write (0, "\n", 1);
    run(input);
    if(getCommand(input, "exit\r")) b = 0;
    i++;
  }
}


void (*entry_terminal)() = &terminal;

//This is the thing I used for run old option
/*if(getCommand(input, "P0\r")){
		if(running[0] != 0){
			char command[101];
			write(0, "Type 'new' to create new P0 and 'old' to see previously created one\n", 68);
			read(command);
			if(getCommand(command, "new\r")){
				int i = fork();
				if(i == 0){
					P0();
					exit();
				}else{
					running[0] = i; update(1, np0s, i);
				} 
			}
			else runSame(running[0]);
		}
		else{
			int i = fork();
			if(i == 0){
				P0();
				exit();
			}
			else{
				running[0] = i; update(1, np0s, i);
			} 
		}
	}*/