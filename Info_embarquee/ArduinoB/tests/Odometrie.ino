/*-----------------ROBOTECH LILLE-------------------
Programme d'odométrie du robot Coupe de France 2016

Résumé des entrées/ sorties:
  pin 2: Codeuse gauche signal A    /!\ activer pull-up entre +5V et signal
  pin 3: Codeuse droite signal A    /!\ activer pull-up entre +5V et signal
  pin 4: Codeuse gauche signal B    /!\ activer pull-up entre +5V et signal
  pin 5: Codeuse droite signal B    /!\ activer pull-up entre +5V et signal
*/



#define d_d_cod 0.0129
#define d_a_cod 0.0003729
#define d_d_mot 0.03094
#define d_a_mot 0.0014794



// Coordonnées
double a=0;
double angle=0;
double X=0;
double Y=0;

//Variables odométrie
float cosa=1;
float sina=0;

int ticks_DL=0;
int ticks_trigo=0;
int ticks_G=0;
int ticks_D=0;

//Définition des pins des signaux A et B des codeuses
int pinA_G = 2; // int0
int pinA_D = 3; // int1
int pinB_G = 4; 
int pinB_D = 5;

 
/* Routine d'initialisation */
void setup() 
{   
    //Communication série
    Serial.begin(9600);
    
    //Définition des pins et des routines d'interruption
    pinMode(pinB_D, INPUT);   // Pin de signal B de la codeuse droite
    pinMode(pinB_G, INPUT);   // Pin de signal B de la codeuse droite 
    attachInterrupt(0, compteur_gauche, RISING);    // Appel de compteur_gauche sur front montant valeur signal A de la codeuse gauche (interruption 0 = pin2 arduino mega)
    attachInterrupt(1, compteur_droit, RISING);     // Appel de compteur_droit sur front montant valeur signal A de la codeuse droite (interruption 1 = pin3 arduino mega)   
   
   //debug
    //attachInterrupt(2, envoi, RISING);
    //digitalWrite(21, HIGH); 
    //
    
    // turn on pullup resistors
    digitalWrite(2, HIGH);                          
    digitalWrite(3, HIGH); 
    digitalWrite(pinB_G, HIGH);
    digitalWrite(pinB_D, HIGH);
    
   
}
 
 
 
/* Fonction principale */
void loop()
{
  envoi();
  /*
  Serial.print(ticks_G);
  Serial.write("\t");
  Serial.println(ticks_D);
  
  
  Serial.print(X);
  Serial.write("\t");
  Serial.print(Y);
  Serial.write("\t");
  Serial.println(angle);
  */
  
}

void envoi()
{
  Serial.print(X);
  Serial.write("\t");
  Serial.print(Y);
  Serial.write("\t");
  Serial.println(angle);
}
  
/* Interruption sur tick de la codeuse */
void compteur_gauche()
{
  ticks_DL++;
  ticks_trigo++;
  if (digitalRead(pinB_G)==LOW)
  {
    ticks_G--;
  }
  else
  {
    ticks_G++;
  }   
  if (ticks_DL>5){calcul_DL();ticks_DL=0;ticks_D=0;ticks_G=0;}
  if (ticks_trigo>50){calcul_trigo();ticks_trigo=0;}
}
 
void compteur_droit()
{
    ticks_DL++;
    ticks_trigo++;
    if (digitalRead(pinB_D)==LOW)
    {
      ticks_D++;
    }
    else
    {
      ticks_D--;
    }
    if (ticks_DL>5){calcul_DL();ticks_DL=0;ticks_D=0;ticks_G=0;}
    if (ticks_trigo>50){calcul_trigo();ticks_trigo=0;}
 }
      
void calcul_DL()
{
  angle += (ticks_D-ticks_G)*d_a_mot;
  float cosangle = cosa-(angle-a)*sina-(angle-a)*(angle-a)*cosa/2;
  float sinangle = sina+(angle-a)*cosa-(angle-a)*(angle-a)*sina/2;
  X += (ticks_G+ticks_D)*cosangle*d_d_mot;
  Y += (ticks_G+ticks_D)*sinangle*d_d_mot;
  angle += (ticks_D-ticks_G)*d_a_mot;
}

void calcul_trigo()
{
  a=angle;
  cosa=cos(angle);
  sina=sin(angle);
}

  

