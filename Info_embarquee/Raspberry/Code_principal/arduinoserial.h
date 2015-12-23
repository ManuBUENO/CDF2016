#ifndef H_ARDUINOSERIAL
#define H_ARDUINOSERIAL

////
// Volatile vars
////

#define			PINIT1					7
#define			PINIT2					8

//Liste_chaine
#include "listechaine.h"

typedef struct sbuffer
{
    int available;      // Nombre de commandes en stock
    Liste_chainee Lmsgs; //faire une liste chainee plutot
} Sbuffer;

// Identificateurs série
Sbuffer Serial_buffer;
volatile int fd_A1;
volatile int fd_A2;
void RX1();
void RX2();
int Ardu_ID(int);
int Connect_Ardus();
void serialPutfloat(int,float);
void serialPutstringln(int,char*,int);
float serialGetfloat(int);
int dep_distance(float, float,int);
int dep_angle(float, float,int);
int dep_stop(int);
void set_capteurs(unsigned char, int);
void move_pince(int *, int);



#endif