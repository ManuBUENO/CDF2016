/*
Commandes Rasp -> Ardu1 (asservissement)
- "I\n": demande l'id de l'ardu
- "angle\nAV": demande un deplacement angulaire avec A:angle (float,deg) et V:vitesse angulaire (float,deg/s)  
- "distance\nDV": demande un deplacement rectiligne avec D:distance (float,cm) et V:vitesse (float,cm/s)  
- "stop\n": demande au robot de s'immobiliser

Commandes Rasp -> Ardu2 (capteurs, servos)
- "I\n": demande l'id de l'ardu
- "capteurs\nC": set les capteurs avec C: byte (ou char) avec les 4 bits de poids faible: états des capteurs (0000xxxx)
- "servos\nAAAAAAAA": bouge la pince avec A:angle d'un servo (float,deg)

Commandes Ardu1 -> Rasp
-"Ix": donne l'ID demandée x
-"success\n": le déplacement est un succès
-"bloque\n": le robot est bloqué 
-"timeout\n": le robot a mis trop de temps pour faire son déplacement, abandon

Commandes Ardu2 -> Rasp
-"Ix": donne l'ID demandée x
-"obstacle": Obstacle rencontré
-"libre": La voie est libre




*/

////
// Volatile vars
////

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

//GPIO
#include <wiringPi.h>

//Serial
#include <wiringSerial.h>

//Liste_chaine
#include "listechaine.h"

#define			PINIT1					27
#define			PINIT2					28

#define			SUMBLOQUE				648
#define			SUMSUCCESS				761
#define			SUMOBSTACLE				1


// Identificateurs série
volatile int fd_A1;
volatile int fd_A2;

//Variables de réception série

typedef struct sbuffer
{
    int available;      // Nombre de commandes en stock
	Liste_chainee Lmsgs; //faire une liste chainee plutot
} Sbuffer;

Sbuffer Serial_buffer;

////
// Interruption série: récupération du caractère
////

void RX1()
{
	printf("RX1 IT\n");
    int cmd=0;
    char c=0;
    int i=0;
    while(serialDataAvail(fd_A1)<1){}
    while(c!='\n')
    {
        c=serialGetchar(fd_A1);
        //printf("fd: %c\n",c);
        if (c!='\n')
            cmd+=c;
    }
    printf("sum success: %d\n",cmd);
    ajout_tete(&(Serial_buffer.Lmsgs),cmd);
    //strcpy(Serial_buffer.Lmsgs->msg,received);
    Serial_buffer.available+=1;
}

void RX2()
{
	printf("RX2 IT\n");
    int cmd=0;
    char c=0;
    int i=0;
    while(serialDataAvail(fd_A2)<1){}
    while(c!='\n')
    {
        c=serialGetchar(fd_A2);
        if (c!='\n')
            cmd+=c;
    }
    
    ajout_tete(&(Serial_buffer.Lmsgs),cmd);
    //strcpy(Serial_buffer.Lmsgs->msg,received);
    Serial_buffer.available+=1;
}

// Fonction d'envoi d'une flottante sur le port série spécifié
void serialPutfloat(int fd, float f)
{
	int i;
	char buffer[8];
	sprintf(buffer, "%f", f);
	
	for (i=0;i<8;i++)
	{
		serialPutchar(fd,buffer[i]);
		delay(1);
	}
}



// Fonction d'envoi d'une string terminée par \n sur le port spécifié
void serialPutstringln(int fd,char* msg,int size)
{
	
    int i;
    char c;
	for (i=0;i<size;i++)
	{
		c=msg[i];
		//printf("char: %c\n",c);
		serialPutchar(fd,c);
	}
    serialPutchar(fd,'0');
}



// Fonction de reception d'une flottante sur le port série spécifié
float serialGetfloat(int fd)
{
	int i = 0;
	char c = 0,
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


//Demande de l'ID de l'Arduino
int Ardu_ID(int fd)
{
    serialPutstringln(fd,"I",1);
	delay(1);
	return serialGetchar(fd);
}


//Connection Série avec les Arduinos
int Connect_Ardus(int baudrate)
{
	printf("Connection aux Arduinos...\n");
	char ID;
	char path[20];
	int fd_temp;
	int N_ardus = 0;
	int i = 0;
    //Tant que les deux arduinos ne sont pas connectées  
    //On teste différents ports Série
	while(N_ardus<2 && i<5)
	{
        //Chemin du port Série i
		sprintf(path,"/dev/ttyACM%d",i);
		//printf("Test Path: %s\n",path);
        //Test d'ouverture du port série
		fd_temp = serialOpen(path,baudrate);
        //Si ce Port est connecté
		if (fd_temp>0)
		{
            //On attend que l'Arduino s'initialise
			delay(3000);
			//printf("fd_temp: %d\n",fd_temp);
            // On demance l'ID de l'arduino connectée
			ID = Ardu_ID(fd_temp);
			if (ID == '1')
			{
				fd_A1 = fd_temp;
				N_ardus++;
				printf("	Arduino 1 connected !\tfd: %d\n",fd_A1);
			}
			else if (ID == '2')
			{
				fd_A2 = fd_temp;
				N_ardus++;
				printf("	Arduino 2 connected !\tfd: %d\n",fd_A2);
			}
		}
		i++;
	}
    
    //Configuration du buffer série maison
    Serial_buffer.available=0;
    Serial_buffer.Lmsgs=NULL;
    
    //configuration des interruptions série (voir si on fait pas de la scrutation plutot)
	int IT_set=2;
	if (wiringPiISR(PINIT1, INT_EDGE_RISING, &RX1)<0){printf("Unable to setup interruption\n");IT_set-=1;}
	else{printf("Interrupt setup RX1\n");}

	if (wiringPiISR(PINIT2, INT_EDGE_RISING, &RX2)<0){printf("Unable to setup interruption\n");IT_set-=1;}
	else{printf("Interrupt setup RX2\n");}

    pullUpDnControl (PINIT1, PUD_UP);
    pullUpDnControl (PINIT2, PUD_UP);

    
    //Si moins de 2 ardus sont connectées ou que moins de 2 IT ont étées confiugurés, on renvoie -1 pour avertir d'une erreur
	if (N_ardus < 2 || IT_set < 2)
		return(-1);
    //Sinon On renvoie 2 pour signifier que les 2 ardus sont connectées
	else return(2);
}



//  prévoir un timeout !!! 
//  retourne 2 si le déplacement est terminé
//  retourne 1 si le robot est bloqué
//  retourne 0 si le robot a rencontré un obstacle
int dep_distance(float cmd, float speed, int fd)
{
	serialPutstringln(fd,"distance",8);
	serialPutfloat(fd,cmd);
	serialPutfloat(fd,speed);
    //RX1(); //debug
    int result=-1,i;
	while(result<0)
    {
        Liste_chainee tmp = Serial_buffer.Lmsgs;
        for(i=0;i<Serial_buffer.available;i++)
        {
            if(tmp->msg==SUMSUCCESS)
            {
                sup_tete(&tmp);Serial_buffer.available--; result = 2;
            }
            else if(tmp->msg==SUMBLOQUE)
            {
                sup_tete(&tmp);Serial_buffer.available--; result = 1;
            }
            else if(tmp->msg==SUMOBSTACLE)
            {
                sup_tete(&tmp);Serial_buffer.available--; result = 0;
            }
            else
            {
                tmp=tmp->suivant;
                result=-1;
            }
        }
    }
	return result;
}


int dep_angle(float cmd, float speed, int fd)
{
    serialPutstringln(fd,"angle",5);
	serialPutfloat(fd,cmd);
	serialPutfloat(fd,speed);
	//RX1();//debug
    int result=-1,i;
    while(result<0)
    {
        Liste_chainee tmp = Serial_buffer.Lmsgs;
        for(i=0;i<Serial_buffer.available;i++)
        {
            if(tmp->msg==SUMSUCCESS)
            {
                sup_tete(&tmp);Serial_buffer.available--; result = 2;
            }
            else if(tmp->msg==SUMBLOQUE)
            {
                sup_tete(&tmp);Serial_buffer.available--; result = 1;
            }
            else if(tmp->msg==SUMOBSTACLE)
            {
                sup_tete(&tmp);Serial_buffer.available--; result = 0;
            }
            else
            {
            	result=-1;
                tmp=tmp->suivant;

            }
        }
    }
	return result;
}

int dep_stop(int fd)
{
    serialPutstringln(fd,"stop",4);
}

void set_capteurs(unsigned char capt, int fd)
{
	serialPutstringln(fd,"capteurs",8);
	serialPutchar(fd,capt);
}

void move_pince(int pos[8], int fd)
{
	int i;
    serialPutstringln(fd,"servos",5);
	for (i=0;i<8;i++)
	{
		serialPutfloat(fd,pos[i]);
	}
}