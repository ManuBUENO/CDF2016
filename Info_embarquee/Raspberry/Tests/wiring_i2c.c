
/*
 * Tests de la communication entre Arduino et Raspberry avec le protocole I2C: peu concluant
 */

////
// Include files
////
#include <stdio.h>
#include <stdlib.h>
// #include <fcntl.h> 
#include <stdlib.h>
#include <strings.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>

 volatile int Rx1=0;

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
*/

void RX1()
{
	Rx1=1;
	printf("RX\n");
}

int I2Cread(int fd)
{
	while(!Rx1){}
	Rx1=0;
	return wiringPiI2CRead(fd);
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
	int dataIn;

	if (wiringPiSetup() == -1){
    printf("Setup problem\n");exit (1);}

/*
	system("gpio mode 0 out");
	system("gpio mode 1 out");
	system("gpio mode 2 out");
	//pinMode(pin1, "OUTPUT");
	//pinMode(pin2, "OUTPUT");*/

	printf("OK\n");
	fd=wiringPiI2CSetup(0x12);
	if (fd<0)
		{printf("I2C setup problem\n");exit(1);}

	if (wiringPiISR(0, INT_EDGE_RISING, &RX1)<0){printf("Unable to setup interruption\n");exit(1);}
	else{printf("Interrupt setup\n");}

/*
	digitalWrite(pin0,1);
	digitalWrite(pin1,0);
	digitalWrite(pin2,0);*/

	printf("FD0 = %d\n",fd);
	delay(3000);

	wiringPiI2CWrite (fd,'I');
	printf("Sent !\n");
	delay(1000);
	dataIn=I2Cread(fd);
	printf("Receive %d:%d\n",fd,dataIn); 
	
	return 0;
}
