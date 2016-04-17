#include "Philosopher.h"

philosopher phils[2];
int p = 2; 

void Philosopher1(int ipc[6]){
	int i = 0;
	stayPut();
	while(1){
		if(i == 0){
			char message[70];
			write(0, "Tell philosopher 2 something:\n", 30);
			//writenum(9);
			read(message);
			//while(1) write(0, "t", 1);
			send(message, 2);	
			i++;
		}
	}
}

void Philosopher2(){
	int i = 0;
	while(1){
		if(i==0){
			write(0, "Philosopher 1 said:\n", 20);
			//receive(1);
			write(0, (void*)(receive(1)), 70);
			//writenum((int)receive(1));
			i++;
		}
	}
}
/*void told(char lesson[100]){

}

void think(char thought[100]){


}

void talk(char thought[100]){
	
}*/