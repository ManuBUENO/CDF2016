
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
- Lecture des fichiers contenant les étapes du ROBOT    										        		OK												
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
- Définir des leds d'état (Programme lancé, Initialisations faites, Parcours en cours, Parcours terminé, ...)	OK
- Développer une interface utilisateur (écran tactile ?, carte elec ? ...)                                      A faire
- Améliorer la gestion série                                                                                    En cours



//--------------Connexions:

Codeuse gauche:
    Signal A: GPIO 0
    Signal B: GPIO 1
Codeuse droite:
    Signal A: GPIO 2
    Signal B: GPIO 3

LED Init: GPIO 21
LED Ready: GPIO 22
LED Running: GPIO 23

Interrupt depart: GPIO 29
Interrupt select: GPIO 24
Interrupt valid : GPIO 25



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
	 


Installer la librairie WiringPi !!

https://projects.drogon.net/raspberry-pi/wiringpi/

Usefull for GPIOs, Interrupt, Serial, Timing

Complation: sudo make
Lancement:  sudo ./Code_CDF
*/


////
// Include libs
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

#define 		PINLEDINIT				21
#define			PINLEDREADY				22
#define			PINLEDRUNNING			23
#define			PINITSTART				29
#define         PINSELECT               24
#define         PINVALID                25

#define 		BAUDRATE				9600


////
// Global vars
////


int activate;
int N_steps;



////
// Declaration structure etape
////

// Cette structure contient tous les paramètres d'une étape
//(position à atteindre, Etat capteurs, Commandes pince, delays, etc..)
typedef struct etape
{
	//Mode 	0: Xdest,Ydest,Va,Vd, capteurs  (destination XY, Vitesse angulaire Va, vitesse rectiligne Vd, état des capteurs)
	//		1: angle, Va, capteurs 			(angle à effectuer, Vitesse angulaire Va, état des capteurs)
	//		2: bras, capteurs 				(Position des servomoteurs de la pince, état des capteurs)
	//		3: wait,capteur 				(Temps wait de pause en ms)

	//Strucure du fichier:
	// mode en début de ligne puis paramètres éspacés par un espace simple, retour à la ligne pour étape suivante
	
	char mode;
	int X;					//(mm)
	int Y;					//(mm)
	float angle;				//(radian)
	float Va;					//(rad/s)
	int Vd;					//(mm/s)
	unsigned char capteurs;
	int servos[8];				//(deg)
	int ID_servos[8];
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
	
	FILE* fd = fopen("parcours1.txt","r");
	if (fd<0){printf("unable to open file\n");exit(-1);}
	int i=0,j;
	char c,a,b,d; //
	fscanf(fd,"%c ",&c);
	//Tant qu'on n'est pas en fin de fichier
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
			for (j=0;j<8;j++)
			{
				fscanf(fd,"%d ",&(step[i].servos[j]));
			}
			fscanf(fd,"%c\n", &(step[i].capteurs));
			break;
			case '3':
			fscanf(fd,"%s ",path);
			FILE* fd2=fopen(path,"r");
			printf("path:%s\n",path);
			if (fd2<0){printf("unable to open servo file %s\n",path);exit(-1);}
			else
			{
				for (j=0;j<8;j++)
				{
					printf("%d\n",j);
					fscanf(fd2,"%d %d\n",&(step[i].ID_servos[j]),&(step[i].servos[j]));
				}
			}
			fclose(fd2);
			fscanf(fd,"%c\n", &(step[i].capteurs));
			break;
			case'4':
			fscanf(fd,"%d %c\n",&(step[i].wait),&(step[i].capteurs));
			break;
		}
		c=fgetc(fd);
		// printf("c:%d\n",c);
		i++;
		//delay(100);
	}
	N_steps = i;
	fclose(fd);
	//printf("N_steps: %d\n");
}


void debug_parcours()
{
	int i,j;
	for (i=0;i<N_steps;i++)
	{
		printf("Etape %d",i);
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
			for (j=0;j<8;j++)
			{
				printf("\t%d ",step[i].servos[j]);
			}
			printf("\ncapteurs:%c\n", step[i].capteurs);
			break;
			case '3':
			for (j=0;j<8;j++)
			{
				printf("\t%d ",step[i].servos[j]);
			}
			printf("\ncapteurs:%c\n", step[i].capteurs);
			break;
			case'4':
			printf("\twait:%d\tcapteurs:%c\n",step[i].wait,step[i].capteurs);
			break;
		}
	}
}



////
// Interruption de départ: le robot peut commencer le parcours
////

// Cette fonction est appellée lors d'un front montant sur le pin PINITSTART (voir #defines)
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

//Calcul de la consine en angle à envoyer à l'arduino à partir de la position actuelle (OX,OY) et la destination (Xdest, Ydest)
float calcul_angle(float Xdest, float Ydest)
{
	return (180/PI)*(atan((Ydest - OY)/(Xdest - OX)) - Oangle);
}

//Calcul de la consine en distance à envoyer à l'arduino à partir de la position actuelle (OX,OY) et la destination (Xdest, Ydest)

float calcul_distance(float Xdest, float Ydest)
{
	return sqrt(pow(Xdest-OX,2)+pow(Ydest-OY,2));
}





////
// Main function
////

int main(void)
{	

	//---------------- Configuration WiringPi

	if (wiringPiSetup()<0){printf("Unable to setup wiringPi\n");exit(1);}
	else{printf("Wiring setup\n");}

	//Allumage de la led d'état Init: Le programme a été lancé, le système s'initialise
	pinMode(PINLEDINIT,1);
	pinMode(PINLEDREADY,1);
	pinMode(PINLEDRUNNING,1);
	digitalWrite(PINLEDINIT,HIGH);
	digitalWrite(PINLEDREADY,LOW);
	digitalWrite(PINLEDRUNNING,LOW);

	//---------------- Configuration Interuptions

	
	if (wiringPiISR(PINITSTART, INT_EDGE_FALLING, &start)<0){printf("Unable to setup interruption\n");exit(1);}
	else{printf("Interrupt setup\n");}

    pullUpDnControl (PINITSTART, PUD_UP);


	//---------------- Configuration série
    //Connexion des arudinos
	printf("Able to connect %d Arduino(s)\n",Connect_Ardus(BAUDRATE));
	


	//---------------- Configuration Odométrie
    //Fonction dans la librairie Libodo.c
    //A améliorer pour initialiser les variables X, Y et angle

	//config_odo(PINGA, PINGB, PINDA, PINDB);


	//---------------- Configuration parcours
	//fonction de lecture dans un fichier à faire

	printf("Lecture fichier...\n");
	lecture_parcours();
	//printf("\nLu\n");
	debug_parcours();
	//printf("\nEcrit\n");
	

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
	
	//Allumage de la led d'état READY: Le système est initialisé sans erreurs
	digitalWrite(PINLEDREADY,HIGH);
	printf("Attente du signal de départ...\n");
	//Tq pas d'interruption de départ
	//while(activate==0){}
	activate=1;
	int trash;
	scanf("%d",&trash);
	//Allumage de la led d'état RUNNING: Le parcours est en cours
	digitalWrite(PINLEDRUNNING,HIGH);
	printf("C'est parti !\n");

	//---------------- Boucle principale
	while ((activate==1) && (num<N_steps))
	{
		printf("\nEtape %d\n",num);
		printf("\tmode:%c\n",step[num].mode);

		switch (step[num].mode)
		{
			//aller en X,Y
			case '0':
                i=0;reponse=-1;
                while(i<3 && reponse!=3)
                {
                	//set_capteurs(step[num].capteurs,fd_A2);
                	printf("\tTentative angle %d\n",i);
                    reponse = dep_angle(calcul_angle(step[num].X,step[num].Y),step[num].Va,fd_A1);
                    // Est ce qu'on considère les obstacles en déplacement angulaire ?
                    printf("\tReponse: %d\n\n",reponse);
                    if (reponse==0)
                    {
                        dep_stop(fd_A1);
                        //attendre un signal de voie libre?
                    }
                    i++;
                }
                i=0;reponse=-1;
                while(i<3 && reponse!=3)
                {
                	//set_capteurs(step[num].capteurs,fd_A2);
                	printf("\tTentative dist %d\n",i);
                    reponse = dep_distance(calcul_distance(step[num].X,step[num].Y),step[num].Vd,fd_A1);
                    printf("\tReponse: %d\n\n",reponse);
                    if (reponse==0)
                    {
                        dep_stop(fd_A1);
                        //attendre un signal de voie libre?
                    }
                    i++;
                }
			break;
            
            //Déplacement angulaire
			case '1':
                i=0;reponse=-1;
                while(i<3 && reponse!=3)
                {
                	//set_capteurs(step[num].capteurs,fd_A2);
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
			case '3':
				move_pince(step[num].servos,fd_A2);
			break;

            //Attendre un delay
			case '4':
				delay(step[num].wait);
			break;
		}
		num++;
	}

	printf("Fin du parcours\n");
	digitalWrite(PINLEDINIT,LOW);
	digitalWrite(PINLEDREADY,LOW);
	digitalWrite(PINLEDRUNNING,LOW);

	//sup_liste(&etape);
	serialClose(fd_A1);
	serialClose(fd_A2);
	//close(fd1);
	//close(fd2);
	return(0);
}