
/*
 * Test des interruptions sur les pins d'entrée de la raspberry
 */
 
/*
Petit rappel: Une interruption est caractérisée par un front montant 0 -> 5V ou encore un front descendant sur un pin d'entrée d'un processeur.
En paramétrant la raspberry, il est possible d'associer cette interruption avec une fonction.
En gros: Ou qu'en soit le déroulement du programme, il va s'arreter et appeler la fonction associé lorsqu'il y a une interruption. Une fois cette fonction terminée, il reprend la ou il en était.
Exemple: 

On a connecté un bouton poussoir à une entrée de la Raspberry, et on a parametré une interruption sur cette entrée pour appeler la fonction interrupt() lors d'un front montant.

Au lancement du programme, il fait les configurations et se coince dans une boucle infinie. Lors de l'appui du bouton, la fonction "Interrupt()" est appelée même si le programme est dans la boucle.

C'est chouette
*/


////
// Include files
////

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>

 //volatile var:
 //volatile int fd;
 volatile bool event = false;

/*
 float readfloat(int fd)
{
	int i = 0;
	char c = 'NULL',
	buffer[8]={'0'};
	while(serialDataAvail(fd)<1){}
	while(c!='\n')
	{
		c=serialGetchar(fd);
		if (c!='\n')
			buffer[i]=c;
		//printf("%c\n",c);
		delay(5);
		i++;
	}

 	float value=atof(buffer);
 	//printf("Float: %f\n",value);
 	return value;
}


int writefloat(int fd, float x)
{
	int i;
	char buffer[8]={'0'};
	sprintf(buffer, "%f", x);
	for (i=0;i<8;i++)
	{
		serialPutchar(fd,buffer[i]);
		delay(2);
	}
	//printf("FD_loc = %d\n",fd);
	return fd;
}

*/


void interrupt()
{
	//char c=serialGetchar(fd);
	//printf("Char:%c",c);
	printf("Event\n");
	event=true;
}

////
// Main function
////

int main(void)
{
	int pin1 = 1;
	int pin2 = 2;
	//char buffer[4];
	//char c;
	int i;
	int fd;

	if (wiringPiSetup() == -1){
    printf("Setup problem\n");exit (1);}

    if (wiringPiISR(0, INT_EDGE_RISING, &interrupt)<0){printf("Unable to setup interruption\n");exit(1);}
	else{printf("Interrupt setup\n");}
    
    
    while(1){}
    
    
/*
	fd=serialOpen("/dev/ttyACM0",9600);
	if (fd<0)
		{fd=serialOpen("/dev/ttyACM1",9600);}

	printf("FD = %d\n",fd);
	delay(3000);
	serialFlush(fd);
*/
	
//	system("gpio mode 1 out");
//	system("gpio mode 2 out");
	//pinMode(pin1, "OUTPUT");
	//pinMode(pin2, "OUTPUT");
	//printf("OK\n");

/*
digitalWrite(pin1,1);
	digitalWrite(pin2,0);

	serialPutchar(fd,'I');

	while(!event){}
	event=false;
	printf("ID: %c\n",serialGetchar(fd));

	serialPutchar(fd,'D');
	fd = writefloat(fd,1.2);
	fd = writefloat(fd,1.5);

	while(!event){}
	event=false;
	printf("cmd1: %f\n",readfloat(fd));
	printf("cmd2: %f\n",readfloat(fd));

	while(!event){}
	event=false;
	printf("state: %c\n",serialGetchar(fd));


	for (i=0;i<4;i++)
	{
		buffer[i] = serialGetchar(fd);
		delay(2);
	}

	printf("char : %f\n",atof(buffer));
	
	digitalWrite(pin1,0);
	digitalWrite(pin2,0);
*/
}
