#include "table.h"


void table(int p){
	while(1){                                           //comment while loop out and uncommentt exit for a quick display, also enable philo
		int i = 0;									    //reset in terminal if you do so.
		while(getFork(0)){
			if(i == 0){
				write(0, "Waiting for left fork\n", 22);
				i++;
			}
		}i = 0;
		while(getFork(1)){
			if(i == 0){
				write(0, "Waiting for right fork\n", 23);
				i++;
			}
		}i = 0;
		eat();
		leaveFork(0);
		leaveFork(1);
		//exit();
	}	
	
}
void (*entry_table)() = &table;

