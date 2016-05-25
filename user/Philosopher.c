#include "Philosopher.h"

//philosopher phils[2];
int p = 2; 
char toSend[70];

void Philosopher1(){
	int i = 1;
	while(i){
			char message[70];
			write(0, "Tell philosopher 2 something:\n", 30);
			if(i == 0)stayPut();
			read(message);
			for(int i = 0; i < 70; i++){
		        toSend[i] = message[i];
		    }
      		write(0, "\n", 1);
			send(toSend, 2);	
			i = 0;
	}
}

void Philosopher2(){
	int i = 1;
	while(i){
		//if(i < 10){
			write(0, "Philosopher 1 said:\n", 20);
			write(0, (char*)receive(1), 70);
			write(0, "\n", 1);
			i = 0;

		//}
	}
}