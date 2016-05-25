#ifndef __LIBC_H
#define __LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// These are the user programs I make the child terminal run//

void P0();
void P1();
void P2();
void table();
void createFile();
void readFile();
void deleteFile();
void listFiles();

//***********************************************************//

int write( int fd, void* x, size_t n );
int read(void* command);
int fork();

//int runSame(int id);

void exit();                                  //exit currently running i.e. scheduled process
int exitP(int id);							  //exit a specific process that is running in background, i.e. still exists in pcb

//These send and receives were the ones I first tried IPC with. Anything to do with these two have been commented out because I changed approach//							

int send(char* message, int recipient);
char* receive(int sender);

//********Anything to do with creating and readin files*********//

void writeToDisk(char* input, char* content);
void readFromFile(char* input);
void deleteFromDisk(char* input);
void list();

//**************************************************************//

void stayPut();                              //this is probably cheating, I use it for places that read-in from user and it gives higher priority to
void exec();								 //allow more time and exec just calls scheduler.			

//*setting the channels and philosophersand the kernel functions for them*//

void setPhils(int id);
void setChan(int first, int second);
int getFork(int hand);
int leaveFork(int hand);
int eat();

//*************************************************************//

int noOfProc();							   //counts the number of processes running
void writenum(int x);
void dig2char(int x);
int getlength(int x);
int get_Id();						       //gets id of currently running process

#endif
