
/*
 * Test de communication série avec un arduino : c'est le bordel mais ça marche
 */

////
// Include files
////
#include <stdio.h>
#include <stdlib.h>
// #include <fcntl.h> 
#include <stdlib.h>
#include <strings.h>
#include <wiringSerial.h>
#include <wiringPi.h>


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
		printf("%c\n",c);
		delay(5);
		i++;
	}

 	float value=atof(buffer);
 	printf("Float: %f\n",value);
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

////
// Main function
////

int main(void)
{
	int pin0 = 0;
	int pin1 = 1;
	int pin2 = 2;

	
	float f= 358.154;
	int i;
	int fd=-1;

	if (wiringPiSetup() == -1){
    printf("Setup problem\n");exit (1);}

	system("gpio mode 0 out");
	system("gpio mode 1 out");
	system("gpio mode 2 out");
	//pinMode(pin1, "OUTPUT");
	//pinMode(pin2, "OUTPUT");

	printf("OK\n");
	fd=serialOpen("/dev/ttyACM0",9600);
	if (fd<0)
		{fd=serialOpen("/dev/ttyACM1",9600);}


	digitalWrite(pin0,1);
	digitalWrite(pin1,0);
	digitalWrite(pin2,0);

	printf("FD0 = %d\n",fd);
	delay(3000);
	serialFlush(fd);
	serialPutchar(fd,'D');
	printf("Sent !\n");

	fd = writefloat(fd,f);
	f=721.15;
	fd = writefloat(fd,f);

	printf("Fd: %d\n",fd);


	//printf("recu1 sur %d: %f\n",fd,f);
	printf("Fd2: %d\n",fd);
	
	f = readfloat(fd);

	printf("Fd3: %d\n",fd);
	
	f = readfloat(fd);

	printf("recu2 sur %d: %f\n",fd,f);



	////////////////////////////////////////////////////////////////////
	/*

	char buffer[4];
	sprintf(buffer, "%f", f);
	for (i=0;i<4;i++)
	{
		serialPutchar(fd,buffer[i]);
	}
	printf("FD_loc = %d\n",fd);


	*////////////////////////////////////////////////////////////////////

	
	//writefloat(fd,3.14);
	

 
	while(1){}

	
	

	readfloat(fd);


	digitalWrite(pin2,1);
	delay(1000);
	digitalWrite(pin0,0);
	digitalWrite(pin1,0);
	digitalWrite(pin2,0);
	serialClose(fd);
	return 0;
}
