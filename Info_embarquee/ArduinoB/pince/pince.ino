/*-----------------ROBOTECH LILLE-------------------
Programme de récupération des positions des pinces

Fonctionnement:
La Raspberry envoie une commande sur la liaison série
L'arduino répond nawak pour signifier qu'elle est bien connectée et envoir les IDs (char) et position de chaque servos (float)


Résumé des entrées/ sorties:
*/

//Include librairies
//#include <DynamixelSerial.h>


//ID Arduino
#define ID '2'


//Declaration ID Servomoteurs

#define IDServo1      1
#define IDServo2      2
#define IDServo3      3
#define IDServo4      4
#define IDServo5      5
#define IDServo6      6
#define IDServo7      7
#define IDServo8      8


//variables servomoteurs
int ID_Servos[8];


void setup()
{
    //Config série
    Serial.begin(9600);
    
    //Config Servomoteurs
     init_Ax12(1000000);
}


//Boucle principale
void loop()
{
}
        
/* Communication Série (avec Raspberry), cette fct se lance lorsqu'un message est recu sur la liaison série */
void serialEvent()
{
    //On lit le caractère reçu
    Serial.write('1');
    delay(5);
    int i;
    float pos;
    for(i=0;i<8;i++)
    {
      Serial.write(ID_Servos[i]);
      pos = i;//Dynamixel.getpos(ID_Servos[i]);
      Serial.println(pos);
    }
}


/* Fonction de paramétrage des Servomoteurs */
void init_Ax12(int bauds)
{
    ID_Servos[0] = IDServo1;
    ID_Servos[1] = IDServo2;
    ID_Servos[2] = IDServo3;
    ID_Servos[3] = IDServo4;
    ID_Servos[4] = IDServo5;
    ID_Servos[5] = IDServo6;
    ID_Servos[6] = IDServo7;
    ID_Servos[7] = IDServo8;
    
    //Begin comm
    //Set couples maxi (Bien pour la consommation courant)
    //Set positions init ? ou laisser faire à la Raspberry
}
    
             
