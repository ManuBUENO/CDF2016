/*-----------------ROBOTECH LILLE-------------------
Programme de controle des déplacements du robot Coupe de France 2016

Fonctionnement:
la Raspberry envoie ces commandes sur la liaison série:
- "I\n": Demande de l'ID de l'arduino ; renvoie son ID sur la liason série
- "stop\n": Demande d'immobilisation du robot
- "angleXY": Demande de déplacement angulaire du robot (X:float angle à effectuer; Y:float Vitesse angulaire)
- "distanceXY": Demande de déplacement rectiligne du robot (X:float distance à effectuer; Y:float Vitesse)

Pour les commandes stop, angle et distance: Le programme renvoie "success\n" s'il a atteint son objectif, ou "bloque\n" s'il est bloqué


Résumé des entrées/ sorties:
  pin 2: Codeuse gauche signal A    /!\ resistance pull-up entre +5V et signal
  pin 3: Codeuse droite signal A    /!\ resistance pull-up entre +5V et signal
  pin 4: Codeuse gauche signal B    /!\ resistance pull-up entre +5V et signal
  pin 5: Codeuse droite signal B    /!\ resistance pull-up entre +5V et signal
  
  pin 11: PWM moteur gauche
  pin 7: direction moteur gauche
  pin 12: PWM moteur droit
  pin 9: direction moteur droit
  
  pin 10: Tx Interruption pour série Raspberry
*/

// A AMELIORER: Serial received et send float à améliorer !!!! Voir avec IT Genius Martin

#include <SimpleTimer.h>
#include <math.h>
#include <PID.h>

#define TIMEOUT 1000
#define ID '1'

//Réglage commandes moteur
#define min_cmd_G 20
#define min_cmd_D 20
#define max_cmd_G 255
#define max_cmd_D 255

SimpleTimer timer;
int timer_T;

// Paramètres géométriques

const float D_codeuse =7.2;         //(cm)
const float pi = 3.141593;
const float long_entre_cod = 21;    //(cm)




//PIDa: angle  //PIDd: distance  //PIDva: Vitesse angulaire  //PIDcd: VItesse rectiligne
PID PIDa;
PID PIDd;
PID PIDva;
PID PIDvd;


//Variables PID
int long tick_G = 0;// Compteur de tick de la codeuse gauche
int long tick_D = 0;// Compteur de tick de la codeuse droite
int long tick_G_prec = 0;
int long tick_D_prec = 0;


//Définition des pins des signaux A et B des codeuses
int pinA_G = 2; // int0
int pinA_D = 3; // int1
int pinB_G = 4; 
int pinB_D = 5;

//Définitions des pins de commande moteur
int pin_cmd_G = 11;
int pin_rot_G = 7;
int pin_cmd_D = 12;
int pin_rot_D = 9;

//Définition des fréquences des routines
int frequence_echantillonnage = 20;


//Commandes et variables série
int pin_Tx=10;


//Variables simulation
float retour_G = 0;
float retour_D = 0;


//----------------------------------------------------------------------------------------------------
/* Routine d'initialisation */
void setup() 
{   
  
    //Set PWM frequency to 31kHz for D11 & D12 !!!FOR ArduinoMega only
    TCCR1B = TCCR1B & B11111000 | B00000001;
    
    //Set PWM frequency to 31kHz for D11 & D12 !!!FOR ArduinoUno only
    //TCCR1B = TCCR1B & B11111000 | B00000001;
    
    //Initialisation des régulateurs PID
    //Paramètres: (Kp,Ki,Kd,fe,tolérance position, tolérance vitesse)
    PIDa.set(  0.4 ,  0.5  ,  0  ,  frequence_echantillonnage, 1, 1);
    PIDd.set(  0.8  ,  1.2  ,  0.05  ,  frequence_echantillonnage, 1, 1);
    PIDva.set(  0.2  ,  0.2  ,  0  ,  frequence_echantillonnage, 1, 1);
    PIDvd.set(  0.8  ,  0.6 ,  0  ,  frequence_echantillonnage, 1, 1);
       
    
    //Init Communication série
    Serial.begin(9600);
    
    //Definition du Pin permettant de signifier à la raspbe qu'une donnée a été envoyée
    pinMode(pin_Tx,OUTPUT);  //Lors de l'envoi d'une donnée série vers la rapbe, ajouter la ligne Tx(); après pour envoyer une interruption
    digitalWrite(pin_Tx,LOW);
    
    // Définition des pins et des routines d'interruption
    attachInterrupt(0, compteur_gauche, RISING);    // Appel de compteur_gauche sur front montant valeur signal A de la codeuse gauche (interruption 0 = pin2 arduino mega)
    attachInterrupt(1, compteur_droit, RISING);     // Appel de compteur_droit sur front montant valeur signal A de la codeuse droite (interruption 1 = pin3 arduino mega)
    pinMode(pinB_G, INPUT);   // Pin de signal B du la codeuse gauche
    pinMode(pinB_D, INPUT);   // Pin de signal B de la codeuse droite
    
    // Liaison avec les résistances de Pull-up pour les interuptions
    digitalWrite(pinA_G,HIGH);
    digitalWrite(pinA_D,HIGH); 
    digitalWrite(pinB_G,HIGH);
    digitalWrite(pinB_D,HIGH);
    
    pinMode(pin_cmd_G,OUTPUT);
    pinMode(pin_rot_G,OUTPUT);
    pinMode(pin_cmd_D,OUTPUT);
    pinMode(pin_rot_D,OUTPUT);
    
    // Initialisation du timer qui lance le calcul PID (fonction asservissement) à la fréquence d'échantillonage
    timer_T = timer.setInterval(1000/frequence_echantillonnage,asservissement);
    timer.disable(timer_T);
    
    
    //DEBUG
    /*
    attachInterrupt(2, demarrage, RISING);
    digitalWrite(21, HIGH);*/
    
    
}
 
//-----------------------------------------------------------------------------------------------------
/* Fonction principale */
void loop()
{ 
  timer.run();  
}

//-------------------------------------------------------------------------------------------
/* Fonction de gestion de la régulation (appellée par le timer_T) */
void asservissement()
{     
   //Simulation de la rotation des roues
   tick_G+=retour_G;
   tick_D+=retour_D;
  
  //Calcul des mesures
  float mes_d = (tick_G+tick_D)*pi*D_codeuse/744;
  float mes_a = ((-tick_G+tick_D)*180*D_codeuse)/(372*long_entre_cod);
  float mes_vd = ((tick_G-tick_G_prec)+(tick_D-tick_D_prec))*pi*D_codeuse*frequence_echantillonnage/(744);  
  float mes_va = ((-(tick_G-tick_G_prec)+(tick_D-tick_D_prec))*180*D_codeuse*frequence_echantillonnage)/(372*long_entre_cod) ;
  
  
  //Calcul de la correction PID
  float d = PIDd.calcul(  mes_d  );
  float a = PIDa.calcul(  mes_a  );
  float vd = PIDvd.calcul(  mes_vd );
  float va = PIDva.calcul(  mes_va  );
  
  
  PIDva.setcmd(PIDva.gettmp());
  PIDvd.setcmd(PIDvd.gettmp());
  //Calcul des rampes de vitesse (à améliorer/modifier/tester)
  /*
  if (PIDvd.isenabled())
  {
    PIDvd.setcmd(rampe(PIDd.geterror(),PIDvd.getmeasure(),PIDvd.gettmp()));
  }
  if (PIDva.isenabled())
  {
    PIDva.setcmd(rampe(PIDa.geterror(),PIDva.getmeasure(),PIDva.gettmp()));
  }
  */
  
  //Passage de l'asservissement vitesse en position si 90% de la commande est effectuée  (à améliorer)
  if(PIDvd.isenabled() && (PIDd.geterror()/PIDd.getcmd())<0.1)
    {PIDvd.off();PIDd.on();}
    
  if(PIDva.isenabled() && (PIDa.geterror()/PIDa.getcmd())<0.1)
    {PIDva.off();PIDa.on();}
  
  tick_G_prec=tick_G;
  tick_D_prec=tick_D;
  
   
  //Commandes des moteurs:    
  float S_G =(d + vd - a - va);
  float S_D =(d + vd + a + va);
  
  
  
  /*
  //DEBUG
  Serial.write("A:");
  Serial.print(PIDa.isenabled());
  Serial.write('\ ');
  Serial.print(PIDa.geterror());
  Serial.write("\tD:");
  Serial.print(PIDd.isenabled());
  Serial.write('\ ');
  Serial.print(PIDd.geterror());
  Serial.write("\tVA:");
  Serial.print(PIDva.isenabled());
  Serial.write('\ ');
  Serial.print(PIDva.geterror());
  Serial.write("\tVD:");
  Serial.print(PIDvd.isenabled());
  Serial.write('\ ');
  Serial.println(PIDvd.geterror());
  
  */
  /*
  
  Serial.write("g:");
  Serial.print(S_G);
  Serial.write("g:");
  Serial.println(S_D);
  
  */
  /*
  Serial.write("vd:");
  Serial.println(PIDvd.getcmd()); 
  */
 /*
  Serial.print(PIDva.geterror());
  Serial.write("\t");
  Serial.println(PIDa.getderiv_error());
  
  */
  
  //Check si robot bloque pendant plus de TIMEOUT
  //c.a.d tolérance de vitesse atteinte et tolérance de position non atteinte
  if (PIDa.isblocked(TIMEOUT) || PIDd.isblocked(TIMEOUT))
  {
    timer.disable(timer_T);
    Serial.print("bloque\n");
    Tx();
    S_G=0;
    S_D=0;
  }
    
  //Check si but atteind
  //c.a.d tolerance de vitesse et de position atteintes
  if (PIDa.isgoalreached() && PIDd.isgoalreached())
  {
    timer.disable(timer_T); 
    Serial.print("success\n");
    Tx();
    S_G=0;
    S_D=0;
}
    
    //Application de la commande aux moteurs
  envoi_commandes(S_G,S_D);
  
  //Simulation
  retour_G = S_G;
  retour_D = S_D;
}

/* RAZ des PIDs et demarrage de la fonction asservissement */
void demarrage()
{
  tick_G=0;
  tick_D=0;
  tick_G_prec=0;
  tick_D_prec=0;
  PIDd.reinit();
  PIDa.reinit();
  PIDvd.reinit();
  PIDva.reinit();
  timer.enable(timer_T);
}

  

  
//--------------------------------------------------------------------------------------
/* Communication Série (avec Raspberry), cette fct se lance lorsqu'un message est recu sur la liaison série */
void serialEvent() 
{
  int cmd = Serialreadstring();
  delay(1);
  
  //Demande l'ID de l'arduino
  if(cmd == 73) //somme "I"
  {
      Serial.write(ID);
      //Tx(); Pas besoin de Tx
  }
  
  //Demande l'immobilisation du robot
  else if(cmd == 454)  //somme "stop"
  {
    PIDa.setcmd(0);
    PIDd.setcmd(0);
    PIDa.on();
    PIDd.on();
    PIDva.off();
    PIDvd.off();
    demarrage();
  }
  
  //Demande un mouvement angulaire
  else if(cmd == 519) //somme "angle"
  {
    
    //Réception de l'angle à effectuer
    PIDa.setcmd(Serialreadfloat());
    //Réception de la vitesse angulaire
    PIDva.settmp(Serialreadfloat());
    /*
    //----DEBUG
    PIDa.setcmd(180);
    PIDva.setcmd(10);
    //----DEBUG*/
    PIDd.setcmd(0);
    PIDvd.setcmd(0);
    PIDva.on();  //on
    PIDd.on();    //on
    PIDa.off();    //off
    PIDvd.off();  //off
    demarrage();
  }
   
  //Demande un mouvement rectiligne
  else if(cmd == 843)  //somme "distance"
  {
    //Reception de la distance à effectuer
    PIDd.setcmd(Serialreadfloat());
    //Reception de la vitesse
    PIDvd.settmp(Serialreadfloat());
      
    //Serial.println(PIDd.getcmd());
    //Serial.println(PIDvd.getcmd());
    //Tx();
      
    PIDa.setcmd(0);
    PIDva.setcmd(0);
    PIDvd.on();
    PIDa.on();
    PIDd.off();
    PIDva.off();
    demarrage();
  }
}

//--------------------------------------------------------------------------------------------------------

/* Interruption sur tick des codeuses */
void compteur_gauche()
{
    if (digitalRead(pinB_G)==LOW)
    {
      tick_G--;  // On incrémente le nombre de tick de la codeuse
    }
    else
    {
      tick_G++;
    }
}
void compteur_droit()
{
    if (digitalRead(pinB_D)==LOW)
    {
      tick_D++;  // On incrémente le nombre de tick de la codeuse
    }
    else
    {
      tick_D--;
    }
 }
 
//-----------------------------------------------------------------------------------
/* Signal indiquant à la Raspberry qu'un message série lui a été envoyé */
void Tx()
{
  digitalWrite(pin_Tx,HIGH);
  delay(1);
  digitalWrite(pin_Tx,LOW);
}

//-----------------------------------------------------------------------------------
/* Fonction de lecture d'un float sur la liaison série */
float Serialreadfloat()
{    
  char BUFFER[8]={'0'};  
  while (Serial.available()<8){}
  for(byte i = 0; i < 8; i++) 
  {  
    BUFFER[i] = Serial.read();  
    delay(5);
  }
  return atof(BUFFER);
}



/*  Fonction de lecture d'une chaine de caractère termniéé par \n sur la liaison série
    Renvoie la somme des caractères reçus   */
int Serialreadstring()
{
  int i=0;
  char c=0;
  int string=0;
  while(c!='0')
  {
    while(!Serial.available()){}
    c=Serial.read();
    delay(2);
    if(c!='0'){string+=c;}
    i++;
  }
  return string;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------
/* Gestion de la vitesse du robot */
float rampe(float erreur, float V, float Vmax)
{
  float result;
  float seuil = V;
 
  if (Vmax < 0)
  {
    if (erreur>V)
    {result = Vmax*erreur/seuil;}
    else
    {result = Vmax;}
    result = max(result,Vmax);
    result = min(result,0);
  }
  else
  {
    if (erreur<V)
    {result = Vmax*erreur/seuil;}
    else
    {result = Vmax;}
    result = max(result,0);
    result = min(result,Vmax);
  }
  return result;
}

//------------------------------------------------------------------------------------------------------------------------------------
/* Application de la commande aux moteurs */
void envoi_commandes(float S_G, float S_D)
{
  /*
  //Debug
  Serial.write("g:");
  Serial.print(S_G);
  Serial.write("g:");
  Serial.println(S_D);
  */
  //Mise en forme des signaux pour commande moteur 
    if (S_G>0)
    {
      S_G+=min_cmd_G;
      digitalWrite(pin_rot_G,LOW); ////////////////// !!!!!!!!!!!!/!\ voir signal pour sens rotation /!\
      
      if (S_G > max_cmd_G)
      {
        S_G = max_cmd_G;
      }
    }
      
    if (S_G<0)
    {
      S_G-=min_cmd_G;
      digitalWrite(pin_rot_G,HIGH);
      
      if (S_G < -max_cmd_G)  //  Max moteur: 100 pour l'instant
      {
        S_G=-max_cmd_G;  //  Max moteur: 100 pour l'instant
      }
    }
    analogWrite(pin_cmd_G,abs(S_G));         //analogWrite est deja une PWM  
    
    if (S_D>0)
    {
      S_D+=min_cmd_D;
      digitalWrite(pin_rot_D,LOW);
      
      if (S_D>max_cmd_D)
      {
        S_D=max_cmd_D;
      }
    }
  
    if (S_D<0)
    {
      S_D-=min_cmd_D;
      digitalWrite(pin_rot_D,HIGH);
      
      if (S_D <-max_cmd_D)  
      {
        S_D=-max_cmd_D; 
      }
    }
    analogWrite(pin_cmd_D,abs(S_D));  // analogWrite est deja une PWM
}

