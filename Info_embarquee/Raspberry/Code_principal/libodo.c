
/*-----------------ROBOTECH LILLE-------------------
Librairie des fonctions d'odométrie pour le robot principal de la Coupe de France 2016

Ce programme permet de connaitre la position (x,y,angle) du robot en tout instant
Les mesures sont en mm et en radians

Principe:
Pendant la rotation d'une roue, sa codeuse incrémentale associée emet des signaux : des ticks
Un tick correspond à un petit delta angle qu'a effectué la roue (ex: 1024 ticks par tour, donc un tick = 1/1024 tour)
A chaque tick détecté, on ajoute ou soustrait un petit delta de distance qu'a effectué le robot et un petit delta d'angle du robot.
Il suffit ensuite de calculer les deltas sur la position en X et Y du robot en appliquant:
delta X = delta_distance * cos(angle)
delta Y = delta_distance * sin(angle)

Un problème: les calculs de cos() et sin() sont chronophages pour un processeur, on fait donc des développements limités pour libérer du CPU, et on calcule les fonctions trigos tous les N ticks


//--------------Connexions:
Codeuse gauche:
    Signal A: GPIO 0
    Signal B: GPIO 1
Codeuse droite:
    Signal A: GPIO 2
    Signal B: GPIO 3


Header d'E/S sur Raspberry Pi 2:

		OO +5V
	SDA OO
	SCL OO GND
	   OO
	GND OO
	  0 OO 1
	  2 OO
	  3 OO 4
	    0O 5
		OO
	    OO
	 	OO
	 	OO

*/


////
// Include files
////

//Faire du tri !
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

//GPIO
#include <wiringPi.h>

////
// Constants
////

#define     OPINENVOI        4

#define Od_d_cod 0.129
#define Od_a_cod 0.0003729
#define Od_d_mot 0.31362
#define Od_a_mot 0.0014794

#define ON_DL 2
#define ON_trigo 15

////
// Volatile vars
////


// Coordonnées
volatile double Oa=0;
volatile double Oangle=0;
volatile double OX=0;
volatile double OY=0;

//Variables odométrie
volatile float Ocosa=1;
volatile float Osina=0;

volatile int Oticks_DL=0;
volatile int Oticks_trigo=0;
volatile int Oticks_G=0;
volatile int Oticks_D=0;

volatile int OpGA;
volatile int OpGB;
volatile int OpDA;
volatile int OpDB;


/* Calcul d'un développement limité de sin et cos pour alléger la tache */
void calcul_DL()
{
  Oangle += (Oticks_D-Oticks_G)*Od_a_mot;
  float Ocosangle = Ocosa-(Oangle-Oa)*Osina-(Oangle-Oa)*(Oangle-Oa)*Ocosa/2;
  float Osinangle = Osina+(Oangle-Oa)*Ocosa-(Oangle-Oa)*(Oangle-Oa)*Osina/2;
  OX += (Oticks_G+Oticks_D)*Ocosangle*Od_d_mot;
  OY -= (Oticks_G+Oticks_D)*Osinangle*Od_d_mot;
  Oangle += (Oticks_D-Oticks_G)*Od_a_mot;
}

/* Calcul des fonctions trigo */
void calcul_trigo()
{
  Oa=Oangle;
  Ocosa=cos(Oangle);
  Osina=sin(Oangle);
}

/* Interruption sur tick de la codeuse */
void compteur_gauche()
{
  Oticks_DL++;
  Oticks_trigo++;
  if (digitalRead(OpGB)==LOW)
  {
    Oticks_G--;
  }
  else
  {
    Oticks_G++;
  }   
  //Calcul d'un DL après N_DL ticks
  if (Oticks_DL>ON_DL){calcul_DL();Oticks_DL=0;Oticks_D=0;Oticks_G=0;}
  //Calcul des fcts trigos après N_trigo ticks
  if (Oticks_trigo>ON_trigo){calcul_trigo();Oticks_trigo=0;}
}
 
void compteur_droit()
{
    Oticks_DL++;
    Oticks_trigo++;
    if (digitalRead(OpDB)==LOW)
    {
      Oticks_D++;
    }
    else
    {
      Oticks_D--;
    }
    //Calcul d'un DL après N_DL ticks
    if (Oticks_DL > ON_DL){ calcul_DL(); Oticks_DL=0; Oticks_D=0; Oticks_G=0;}
    //Calcul des fcts trigos après N_trigo ticks
    if (Oticks_trigo > ON_trigo){ calcul_trigo(); Oticks_trigo=0;}
 }
      

/* Débugage  */
void debug_odo()
{
	printf("X: %f\tY: %f\tA: %f\n",OX,OY,Oangle);
	//printf("G: %d\tD: %d\n",ticks_G,ticks_D);
}


////
// Config de l'odométrie
////

//A appeller dans le main du programme, en spécifiant les pins des codeuses
int config_odo(int GA, int GB, int DA, int DB)
{
    int success=1;
    //---------------- Attribution des pins GPIO
    OpGA = GA;
    OpGB = GB;
    OpDA = DA;
    OpDB = DB;
    //---------------- Configuration WiringPi

    if (wiringPiSetup()<0){printf("Unable to setup wiringPi\n");success=-1;}
    else{printf("Wiring setup\n");}

    //---------------- Haute priorité à ce programme: 99/99
    piHiPri(99);

    //---------------- Configuration Interuptions ticks

    if (wiringPiISR(GA, INT_EDGE_RISING, &compteur_gauche)<0){printf("Unable to setup interruption\n");success=-1;}
    else{printf("Interrupt setup\n");}

    if (wiringPiISR(DA, INT_EDGE_RISING, &compteur_droit)<0){printf("Unable to setup interruption\n");success=-1;}
    else{printf("Interrupt setup\n");}

    /*
    if (wiringPiISR(PINENVOI, INT_EDGE_RISING, &envoi)<0){printf("Unable to setup interruption\n");exit(1);}
    else{printf("Interrupt setup\n");}
    */

    return success;
}

void reset_X_angle()
{
    OX=0;
    Oangle=0;
}

void reset_Y()
{
    OY=0;
}
