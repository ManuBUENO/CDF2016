#ifndef H_LIBODO
#define H_LIBODO

////
// Volatile vars
////

// Coordonnées
volatile double Oa;
volatile double Oangle;
volatile double OX;
volatile double OY;

//Variables odométrie
volatile float Ocosa;
volatile float Osina;

volatile int Oticks_DL;
volatile int Oticks_trigo;
volatile int Oticks_G;
volatile int Oticks_D;

volatile int OpGA;
volatile int OpGB;
volatile int OpDA;
volatile int OpDB;

void calcul_trigo();
void compteur_gauche();
void compteur_droit();
void debug_odo();
int config_odo(int, int, int, int);
void reset_X_angle();
void reset_Y();


#endif