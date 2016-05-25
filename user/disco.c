#include "disco.h"


void createFile(){
	write(0, "Filename: ", 10);
	char fileName[16];
	stayPut();
	read(fileName);
	write(0, "\n", 1);

	write(0, "Content: ", 9);
	char readContent[50];
	stayPut();
	read(readContent);
	stayPut();

	writeToDisk((char*)fileName, (char*)readContent);
}

void readFile(){
	write(0, "Filename: ", 10);
	char fileName2[16];
	stayPut();
	read(fileName2);
	write(0, "\n", 1);
	readFromFile(fileName2);
}

void listFiles(){
	list();
}

void deleteFile(){
	write(0, "Filename: ", 10);
	char fileName3[16];
	stayPut();
	read(fileName3);
	write(0, "\n", 1);
	deleteFromDisk(fileName3);
	write(0, "File has been deleted\n", 23);
}

