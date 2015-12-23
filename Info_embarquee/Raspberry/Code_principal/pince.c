
/*-----------------ROBOTECH LILLE-------------------
Programme de controle du robot Coupe de France 2016

add -lwiringPi when compiling

Library WiringPi:
Must be installed on Raspberry Pi, see:

https://projects.drogon.net/raspberry-pi/wiringpi/

Usefull for GPIOs, Interrupt, Serial, Timing

Complation avec le makefile (commande make)
*/


////
// Include files
////
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <strings.h>

// WiringPi
#include <wiringSerial.h>
#include <wiringPi.h>


#include "arduinoserial.h"


////
// Constants
////

#define 		BAUDRATE				9600


////
// Volatile vars
////





////
// Fonctions de lecture des fichiers étape
////


void lecture_pince(int se)
{
	char path[20];
	printf("Entrez le nom du fichier: ");
	scanf("%s",path);
	FILE* fd = fopen(path,"w");
	if (fd<0){printf("unable to open file\n");exit(-1);}

	serialPutchar(se,'1');
	if (serialGetchar(se)<0)
	{
		printf("No response from arduino..\nMake sure it has the appropriate software and is connected has /dev/ttyACM0\n");
	}

	delay(5);
	int ID, position,i;

	for(i=0;i<8;i++)
	{
		ID = serialGetchar(se)+48;
		position=serialGetfloat(se);
		fprintf(fd,"%c %d\n",ID,position);
	}
	//printf("N_steps: %d\n");
}







////
// Main function
////

int main(void)
{
	//LEDs: Init, Ready, Running

	//---------------- Configuration WiringPi

	if (wiringPiSetup()<0){printf("Unable to setup wiringPi\n");exit(1);}
	else{printf("Wiring setup\n");}


	//---------------- Configuration série
    //Connexion des arudinos

    int fd = serialOpen("/dev/ttyACM0",BAUDRATE);

	if (fd < 0)
	{printf("Unable to connect to Arduino\n");exit(-1);}
	delay(3000);
	

	lecture_pince(fd);
	
	serialClose(fd);
	return(0);
}
