
/*-----------------ROBOTECH LILLE-------------------
Programme d'e controle d'odométrie du robot Coupe de France 2016

Ce programme permet de connaitre la position (x,y,angle) du robot en tout instant

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
	 

add -lwiringPi when compiling

WiringPi:
https://projects.drogon.net/raspberry-pi/wiringpi/


add -lpthread when compiling


//Interruption sur GPIO pour la série //Pas utilisé
http://cs.smith.edu/dftwiki/index.php/Tutorial:_Interrupt-Driven_Event-Counter_on_the_Raspberry_Pi


gcc filename.c -o execname -lpthread -lwiringPi -lm
sudo ./execname

*/


////
// Include files
////
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

//GPIO
#include <wiringSerial.h>


//PWM
#include <wiringPi.h>
#include <softPwm.h>

////
// Constants
////

#define 		PINGA					0
#define			PINGB 					1
#define			PINDA					2
#define			PINDB					3
#define			PINENVOI				4

#define d_d_cod 0.0129
#define d_a_cod 0.0003729
#define d_d_mot 0.031362
#define d_a_mot 0.0014794

#define N_DL 2
#define N_trigo 15



////
// Volatile vars
////

// Coordonnées
volatile double a=0;
volatile double angle=0;
volatile double X=0;
volatile double Y=0;

//Variables odométrie
volatile float cosa=1;
volatile float sina=0;

volatile int ticks_DL=0;
volatile int ticks_trigo=0;
volatile int ticks_G=0;
volatile int ticks_D=0;



/* Calcul d'un développement limité de sin et cos pour alléger la tache */
void calcul_DL()
{
  angle += (ticks_D-ticks_G)*d_a_mot;
  float cosangle = cosa-(angle-a)*sina-(angle-a)*(angle-a)*cosa/2;
  float sinangle = sina+(angle-a)*cosa-(angle-a)*(angle-a)*sina/2;
  X += (ticks_G+ticks_D)*cosangle*d_d_mot;
  Y += (ticks_G+ticks_D)*sinangle*d_d_mot;
  angle += (ticks_D-ticks_G)*d_a_mot;
}

/* Calcul des fonctions trigo */
void calcul_trigo()
{
  a=angle;
  cosa=cos(angle);
  sina=sin(angle);
}

/* Interruption sur tick de la codeuse */
void compteur_gauche()
{
  ticks_DL++;
  ticks_trigo++;
  if (digitalRead(PINGB)==LOW)
  {
    ticks_G--;
  }
  else
  {
    ticks_G++;
  }   
  //Calcul d'un DL après N_DL ticks
  if (ticks_DL>N_DL){calcul_DL();ticks_DL=0;ticks_D=0;ticks_G=0;}
  //Calcul des fcts trigos après N_trigo ticks
  if (ticks_trigo>N_trigo){calcul_trigo();ticks_trigo=0;}
}
 
void compteur_droit()
{
    ticks_DL++;
    ticks_trigo++;
    if (digitalRead(PINDB)==LOW)
    {
      ticks_D++;
    }
    else
    {
      ticks_D--;
    }
    //Calcul d'un DL après N_DL ticks
    if (ticks_DL>N_DL){calcul_DL();ticks_DL=0;ticks_D=0;ticks_G=0;}
    //Calcul des fcts trigos après N_trigo ticks
    if (ticks_trigo>N_trigo){calcul_trigo();ticks_trigo=0;}
 }
      

/* Débugage  */
void envoi()
{
	printf("X: %f\tY: %f\tA: %f\n",X,Y,angle);
	//printf("G: %d\tD: %d\n",ticks_G,ticks_D);
}


////
// Main function
////

int main(void)
{
    //---------------- Configuration WiringPi

    if (wiringPiSetup()<0){printf("Unable to setup wiringPi\n");exit(1);}
    else{printf("Wiring setup\n");}

    //---------------- Haute priorité à ce programme: 99/99
    piHiPri(99);

    //---------------- Configuration Interuptions

    if (wiringPiISR(PINGA, INT_EDGE_RISING, &compteur_gauche)<0){printf("Unable to setup interruption\n");exit(1);}
    else{printf("Interrupt setup\n");}

    if (wiringPiISR(PINDA, INT_EDGE_RISING, &compteur_droit)<0){printf("Unable to setup interruption\n");exit(1);}
    else{printf("Interrupt setup\n");}

    if (wiringPiISR(PINENVOI, INT_EDGE_RISING, &envoi)<0){printf("Unable to setup interruption\n");exit(1);}
    else{printf("Interrupt setup\n");}

    /*Boucle infinie */
    while(1)
    {
    	delay(100);
    	envoi();
    }
}