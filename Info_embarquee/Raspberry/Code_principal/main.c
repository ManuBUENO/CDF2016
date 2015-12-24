
/*-----------------ROBOTECH LILLE-------------------
Programme de controle du robot Coupe de France 2016

-----------------FONCTIONNEMENT----------------
- Lancement automatique du programme au démarrge de la Raspberry
- Initialisation 
    - Entrées/Sorties de la Raspberry
    - Communications série avec arduinos
    - Interruptions
    - Lecture du fichier du parcours (Créé par l'application Qt)
    - Timer pour fin au bout de 90s
- Démarrage de la boucle principale au signal de départ
- Calcul des déplacements
- Envoi données aux Arduinos (Déplacements, capteurs IR actifs, Controle servomoteurs)
- Gestion des obstacles
- ....


-----------------AVANCEMENT--------------------

A faire:
- Définir une structure de données pour chaque étape du robot (position à atteindre, pince, etc..)				OK
- Faire des librairies (notamment pour les listes_chainées)														OK
- Faire un MakeFile pour compliler proprement																	OK
- Lecture des fichiers contenant les étapes du ROBOT    !!!!!!! Temriner parcours par 'f' !!!!!!!!!        		En cours													
- Selection de la bonne disposition de la piste																	En cours
    (un bouton par dispo? Un bouton qui switche les dispos, avec un afficheur? D'autres idées ?)				
- Boucle principale																								En cours
- Timing 90 sec 																								A faire
- Initialiser le zéro de l'Odométrie 																			A faire
    - On place le robot dos à une paroi
    - On init X=0 et angle=0 (fct "reset_X_angle();")
    - On place le robot dos à une autre paroi
    - On init Y=0 (fct "reset_Y();")
- Lancement du programme au démarrage de la raspberry 															A faire
- Gérer les modulos d'angles																					A faire
- Définir des leds d'état (Programme lancé, Initialisations faites, Parcours en cours, Parcours terminé, ...)	A faire
- Développer une interface utilisateur (écran tactile ?, carte elec ? ...)                                      A faire
- Améliorer la gestion série                                                                                    En cours



//--------------Connexions:

Codeuse gauche:
    Signal A: GPIO 0
    Signal B: GPIO 1
Codeuse droite:
    Signal A: GPIO 2
    Signal B: GPIO 3

LED Init: GPIO 4
LED Ready: GPIO 5
LED Running: GPIO 6



		OO +5V
	SDA OO
	SCL OO GND
	    OO
	GND OO
	  0 OO 1
	  2 OO
	  3 OO 4
	    OO 5
		OO
	    OO 6
	 	OO
	 	OO
	 	OO
	 21	OO
	 22	OO 26
	 23	OO
	 24	OO 27
	 25	OO 28
	 	OO 29
	 


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

// odometrie
#include "libodo.h"
//serial
#include "arduinoserial.h"
//Listes chainees
#include "listechaine.h"

////
// Constants
////

#define 		PI 						3.1416		

//Pins des codeuses pour odométrie
#define 		PINGA					0
#define 		PINGB					1
#define 		PINDA					2
#define 		PINDB					3

#define 		PINLEDA					21
#define			PINLEDB 				22
#define			PINLEDC					23
#define			PINITSTART				29
#define         PINSELECT               24
#define         PINVALID                25

#define 		BAUDRATE				9600


////
// Volatile vars
////


volatile int activate;
volatile int N_steps;



////
// Decalration structure etape
////

// Cette structure contient tous les paramètres d'une étape
//(position à atteindre, Etat capteurs, Commandes pince, delays, etc..)
typedef struct etape
{
	//Mode 	1: X,Y,Va,Vd, capteurs 
	//		2: angle, Va, capteurs
	//		3: bras, capteurs
	//		4: wait,capteur 
	// espaces entre param, \n fin etape
	
	char mode;
	//Position à atteindre
	int X;					//(mm)
	int Y;					//(mm)
	float angle;				//(radian)
	float Va;					//(rad/s)
	int Vd;					//(mm/s)
	unsigned char capteurs;
	int servos[8];				//(deg)
	int wait;					//(ms)
} Etape;

Etape step[100];


////
// Fonctions de lecture des fichiers étape
////


void lecture_parcours()
{
    int parcours = 0;
    /*
    int bouton_select=0,bouton_valid=1;
    while(bouton_valid==0)
    {
        //lire bouton_select
        if(digitalread(PINSELECT)==1)
        {
            parcours++;
            while(digitalread(PINSELECT)==1){}
        }
        if (parcours==10)
            parcours=0;
        //Afficher valeur(7segments?)
        bouton_valid=digitalread(PINVALID);
    }
	*/
	char path[20];
	//sprintf(path,"parcours%d.txt",parcours);
	
	FILE* fd = fopen("parcours0.txt","r");
	if (fd<0){printf("unable to open file\n");exit(-1);}
	int i=0,j;
	char c,a,b,d; //
	fscanf(fd,"%c ",&c);	
	while (c!=255)
	{
		//printf("\nboucle %d\n",i);
		step[i].mode = c;
		switch(step[i].mode)
		{
			case '0':
			fscanf(fd,"%d %d %f %d %c %c %c %c\n", &(step[i].X),&(step[i].Y),&(step[i].Va),&(step[i].Vd), &(step[i].capteurs),&a,&b,&d);
			break;
			case '1':
			fscanf(fd,"%f %f %c\n",&(step[i].angle),&(step[i].Va), &(step[i].capteurs));
			break;
			case '2':
			for (j=0;j<9;j++)
			{
				fscanf(fd,"%d ",&(step[i].servos[j]));
			}
			fscanf(fd,"%d\n", &(step[i].capteurs));
			break;
			case'3':
			fscanf(fd,"%d %c\n",&(step[i].wait),&(step[i].capteurs));
			break;
		}
		c=fgetc(fd);
		// printf("c:%d\n",c);
		i++;
		//delay(100);
	}
	N_steps = i;
	//printf("N_steps: %d\n");
}


void debug_parcours()
{
	int i,j;
	for (i=0;i<N_steps;i++)
	{
		printf("Etape n°: %d",i);
		printf("\tMode %c\n",step[i].mode);
		switch(step[i].mode)
		{
			case '0':
			printf("\tX:%d\tY:%d\tVa:%f\tVd:%d\tcapteurs:%c\n", step[i].X,step[i].Y,step[i].Va,step[i].Vd, step[i].capteurs);
			break;
			case '1':
			printf("\tangle:%d\tVa:%d\tcapteurs:%c\n",step[i].angle,step[i].Va, step[i].capteurs);
			break;
			case '2':
			for (j=0;j<9;j++)
			{
				printf("\t%d ",step[i].servos[j]);
			}
			printf("\ncapteurs:%c\n", step[i].capteurs);
			break;
			case'3':
			printf("\twait:%d\tcapteurs:%c\n",step[i].wait,step[i].capteurs);
			break;
		}
	}
}



////
// Interruption de départ: le robot peut commencer le parcours
////


void start()
{
	activate = 1;
	printf("Début du parcours\n");
}
/*
void stop()
{
	activate = 0;
	printf("Temps écoulé !\n");
	//Détacher les interruptions, activer parasol
}
*/

////
// Calcul des déplacements
////

float calcul_angle(float Xdest, float Ydest)
{
	return (180/PI)*(atan((Ydest - OY)/(Xdest - OX)) - Oangle);
}

float calcul_distance(float Xdest, float Ydest)
{
	return sqrt(pow(Xdest-OX,2)+pow(Ydest-OY,2));
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

	//---------------- Configuration Interuptions

	system("gpio edge 29 falling");
	
	if (wiringPiISR(PINITSTART, INT_EDGE_FALLING, &start)<0){printf("Unable to setup interruption\n");exit(1);}
	else{printf("Interrupt setup\n");}

    pullUpDnControl (PINITSTART, PUD_UP);


	//---------------- Configuration série
    //Connexion des arudinos

	if (Connect_Ardus(BAUDRATE) < 0)
		printf("Unable to connect to Arduinos\n");
	
	//Debug Close Serial
	//serialClose(fd_A1);
	//serialClose(fd_A2);


	//---------------- Configuration Odométrie
    //Fonction dans la librairie Libodo.c
    //A améliorer pour initialiser les variables X, Y et angle

	//config_odo(PINGA, PINGB, PINDA, PINDB);


	
		

	//---------------- Configuration parcours

    //liste_chainee etape=NULL;
	//fonction de lecture dans un fichier à faire

	lecture_parcours();
	printf("\nLu\n");
	debug_parcours();
	printf("\nEcrit\n");
	

	//---------------- Variables
    int reponse;
	//Loop vars
	int i,j,num=0;


	//Debug 
	/*
	set_capteurs('O',fd_A2);
	while(1)
	{
		debug_odo();
		delay(100);
	}
	*/

	printf("Attente du signal de dépat...\n");
	//Tq pas d'interruption de départ
	while(activate==0){delay(1);}
	printf("C'est parti !\n");
	activate=1;
	//LED Running


	//---------------- Boucle principale
	while ((activate==1) && (num<=N_steps))
	{
		printf("Etape %d\n",num);
		printf("mode:%c\n",step[num].mode);
		switch (step[num].mode)
		{
			//On set les capteurs, le fait-on même pour les bras et un wait ? ou les met-on automatiquement à 0?
			//set_capteurs(step[num].capteurs,fd_A2);
			//aller en X,Y
			case '0':
                i=0;reponse=-1;
                while(i<3 && reponse!=2)
                {
                	printf("Tentative angle %d\n",i);
                    reponse = dep_angle(calcul_angle(step[num].X,step[num].Y),step[num].Va,fd_A1);
                    // Est ce qu'on considère les obstacles en déplacement angulaire ?
                    printf("Reponse: %d\n\n",reponse);
                    if (reponse==0)
                    {
                        dep_stop(fd_A1);
                        //attendre un signal de voie libre?
                    }
                    i++;
                }
                i=0;reponse=-1;
                while(i<3 && reponse!=2)
                {
                	printf("Tentative dist %d\n",i);
                    reponse = dep_distance(calcul_distance(step[num].X,step[num].Y),step[num].Vd,fd_A1);
                    printf("Reponse: %d\n\n",reponse);
                    if (reponse==0)
                    {
                        dep_stop(fd_A1);
                        //attendre un signal de voie libre?
                    }
                    i++;
                }
			break;
            
            //Se positionner à l'angle specifié
			case '1':
                i=0;reponse=-1;
                while(i<3 && reponse!=2)
                {
                    reponse = dep_angle(step[num].angle-Oangle,step[num].Va,fd_A1);
                    // Est ce qu'on considère les obstacles en déplacement angulaire ?
                    if (reponse==0)
                    {
                        dep_stop(fd_A1);
                        //attendre un signal de voie libre?
                    }
                }
			break;
            
            //Bouger la pince
			case '2':
				move_pince(step[num].servos,fd_A2);
			break;
            
            //Attendre un delay
			case '3':
				delay(step[num].wait);
			break;
		}
		num++;
	}

	printf("Fin du parcours\n");
	//sup_liste(&etape);
	serialClose(fd_A1);
	serialClose(fd_A2);
	//close(fd1);
	//close(fd2);
	return(0);
}
