#include "terminal.h"

int running[3];
int np0s[6];
int np1s[6];
int np2s[6];
int allIds[7];
int ipc[10];
int this;
int check = 0;
int ok = 0;
int currentId;
int rstPhils = 0;
int id1, id2, id3, id4, id5;

void update(int op, int input[6], int id){
	if(op == 1){
		for(int i = 0; i < 6; i++){
			if( input[i] == 0 ){
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
	if(*input == '\r') return 0;
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
			else{
				exec();
				update(1, np0s, i);
			} 
	}

	else if(getCommand(input, "P1\r")){
			int i = fork();
			if(i == 0){
				P1();
				//exit();
			}
			else{
				exec();
				update(1, np1s, i);
			} 		
	}

	else if(getCommand(input, "P2\r")){
			int i = fork();
			if(i == 0){
				P2();
				//exit();
			}
			else{
				exec();
			 	update(1, np2s, i);
			}
	}
	else if(getCommand(input, "/cf\r")){
		int i = fork();
		if(i == 0){
			createFile();
			exit();
		}else if(i > 0) exec();
	}
	else if(getCommand(input, "/rf\r")){
		int i = fork();
		if(i == 0){
			readFile();
			exit();
		}else if (i > 0) exec();
	}

	else if(getCommand(input, "/ls\r")){
		int i = fork();
		if(i == 0){
			listFiles();
			exit();
		}else if(i > 0) exec();
	}

	else if(getCommand(input, "/df\r")){
		int i = fork();
		if(i == 0){
			deleteFile();
			exit();
		}else if(i > 0) exec();
	}

	else if(getCommand(input, "dine\r")){

		check = 0;
		int p1, p2, p3, p4, p5;
		if(check == 0)p1 = fork();
		if(check == 0)p2 = fork();
		if(check == 0)p3 = fork();
	 	if(check == 0)p4 = fork();
	    if(check == 0)p5 = fork();
	    if(check == 0){
			id1 = p1; setPhils(id1);//exec();
		}
	    if( check == 0){
			id2 = p2; setPhils(id2);//exec();
		}
		if(check == 0){
			id3 = p3; setPhils(id3);//exec();	
		}
		if(check == 0){
		  	id4 = p4; setPhils(id4);//exec();
		}
		if(check == 0){
			id5 = p5; setPhils(id5);//exec();	
		}
		if(check == 0)setChan(id1, id2);
		if(check == 0)setChan(id2, id3);
		if(check == 0)setChan(id3, id4);
		if(check == 0)setChan(id4, id5);
		if(check == 0)setChan(id1, id5);
		check = 1;
		currentId = get_Id();
		if(currentId == id5) rstPhils = 5;
		
		if ( ok == 1 ){
			table(currentId);
		}
		if(ok == 0){
			ok = 1; 
			exec();
			//exit();
		}
		ok = 0;
		check = 0;
	}

	else if(getCommand(input, "quit P0\r")){
		char input[3];
		if(noOfP(np0s) > 1){
			write(0, "There are multiplie processes of P0 running with ids:\n", 54);
			for(int i = 0; i<6; i++) {
				if(np0s[i] != 0){
					writenum(np0s[i]); write(0, " ", 1);
				}
			}
			write(0, "\n", 1);write(0, "Enter id you want to quit: ", 27); read(input); 
			exitP(char2int(input)); update(0, np0s, char2int(input)); //running[0] = findId(np0s);
		}else if(noOfP(np0s) == 1){
			exitP(findId(np0s));
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
			update(0, np2s, findId(np0s));
		}else if(noOfP(np2s) == 0) write(0, "There are no P2s running currently\n", 35);
	}

	else if(getCommand(input, "quit dine\r")){
		if(id1 != 0 && id2 != 0 && id3 != 0 && id4 != 0 && id5 != 0){
			exitP(id1); exitP(id2); exitP(id3); exitP(id4); exitP(id5);
			setChan(id1, id2);
			setChan(id2, id3);
			setChan(id3, id4);
			setChan(id4, id5);
			setChan(id1, id5);
			id1 = 0; id2 = 0; id3 = 0; id4 = 0; id5 = 0;
			rstPhils = 0;
			setPhils(0); setPhils(0); setPhils(0); setPhils(0); setPhils(0);
		}else write(0, "The philosophers are not currently at the table\n", 48);
	}
	else if(getCommand(input, "/ps\r")) noOfProc();
	else if(getCommand(input, "exit\r")){
		exit();
	}
	else write(0, "Error: Command not found.\n", 26);
	
}

void terminal() {
  char input[101]; 
  int i = 0;
  int b = 1;
  while (b){
    
    if(i == 0) write( 0, "termite$ ", 9);
    else write(0, "\ntermite$ ", 11);
    read( input );
    write (0, "\n", 1);
    run(input);
    /*if(getCommand(input, "dine\r") && rstPhils == 5){
    	setChan(id1, id2);
		setChan(id2, id3);
		setChan(id3, id4);
		setChan(id4, id5);
		setChan(id1, id5);
		id1 = 0; id2 = 0; id3 = 0; id4 = 0; id5 = 0;
		rstPhils = 0;
		setPhils(0); setPhils(0); setPhils(0); setPhils(0); setPhils(0);
    }*/
    if(getCommand(input, "exit\r")) b = 0;
    i++;
  }
}


void (*entry_terminal)() = &terminal;

