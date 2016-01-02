
/*-----------------ROBOTECH LILLE-------------------
Ce programme permet de récupérer les angles des servomoteurs du robot et de les stocker dans un fichier de son choix.
Le fichier comportera 8 lignes avec l'ID du servomoteur et son angle
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
// Main function
////

int main(void)
{
	//LEDs: Init, Ready, Running

	//---------------- Configuration WiringPi

	if (wiringPiSetup()<0){printf("Unable to setup wiringPi\n");exit(1);}
	else{printf("Wiring setup\n");}

	//---------------- Demande le nom du fichier dans lequel écrire les angles et le crée
	char filename[20],path[20];
	printf("Entrez le nom du fichier: ");
	scanf("%s",filename);
	FILE* fd = fopen(filename,"w");
	if (fd<0){printf("unable to open/create file\n");exit(-1);}


	//---------------- Connexion à l'arduino
	int connected=0,i=0,se=-1,se_temp;
	while(connected==0 && i<5)
	{
        //Chemin du port Série i
		sprintf(path,"/dev/ttyACM%d",i);
		se_temp = serialOpen(path,BAUDRATE);
        //Si ce Port est connecté
		if (se_temp>0)
		{
            //L'arduino Reset automatiquement, on attend qu'elle soit opérationelle
			delay(2000);
            // On demandee l'ID de l'arduino connectée
			if (Ardu_ID(se_temp) == '2')
			{
                //on attribue le se récupéré à l'ouverture du port au fd qui servira à la communiquer avec l'arduino
				se = se_temp;
				printf("	Arduino 2 connected !\tse: %d\n",se);
			}
		}
		i++;
	}
	if (se<0){printf("Unable to detect arduino, check if /dev/ttyACMx exists and the appropriate program is running on arduino\n");exit(-1);}
	delay(5);
	//On demande les angles à l'arduino
	serialPutstringln(se,"getangles",9);
	delay(5);

	//On lit les réponses et on les stocke dans le fichier
	for(i=0;i<8;i++)
	{
		int ID = serialGetchar(se);
		int position=serialGetfloat(se);
		fprintf(fd,"%d %d\n",ID,position);
	}
	
	serialClose(se);
	return(0);
}
