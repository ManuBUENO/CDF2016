/*-----------------ROBOTECH LILLE-------------------
Programme de gestion des capteurs et des actionneurs du robot Coupe de France 2016

Fonctionnement:
La Raspberry envoie ces commandes sur la liaison série:
- 'I': Demande de l'ID de l'arduino ; renvoie son ID ('2') sur la liason série
- 'SXXXXXXXX': X: Commande des servomoteurs du robot : chaque X est un int representant l'angle d'un servo

La Raspberry recoit ces commandes:
- 'O': Le robot a detecté un obstacle


Résumé des entrées/ sorties:
  pin A0: Capteur IR avant gauche
  pin A1: Capteur IR avant droit
  pin A2: Capteur IR arriere gauche
  pin A3: Capteur IR arriere droit
  
  pin 10: Tx Interruption pour série Raspberry
*/

//Include librairies
#include <DynamixelSerial.h>
#include <SimpleTimer.h>

//ID Arduino
#define ID '2'

//déclaration constantes
#define PIN_IR_AvG   A0
#define PIN_IR_AvD   A1
#define PIN_IR_ArG   A2
#define PIN_IR_ArD   A3
#define PIN_TX      10

//Declaration ID Servomoteurs

#define IDServo1      1
#define IDServo2      2
#define IDServo3      3
#define IDServo4      4
#define IDServo5      5
#define IDServo6      6
#define IDServo7      7
#define IDServo8      8

//variables capteurs IR
bool IR_ON[4];
int IR_PIN[4];

//variables servomoteurs
int ID_Servos[8];

//Timer pour check obstacles
SimpleTimer timer;
int timer_IR;

void setup()
{
    //Config série
    Serial.begin(9600);
    
    //Config Servomoteurs
    init_Ax12(1000000);
    
    //Attribution pins capteurs
    IR_PIN[0] = PIN_IR_AvG;
    IR_PIN[1] = PIN_IR_AvD;
    IR_PIN[2] = PIN_IR_ArG;
    IR_PIN[3] = PIN_IR_ArD;
    
    //Definition du Pin permettant de signifier à la raspbe qu'une donnée a été envoyée
    pinMode(PIN_TX,OUTPUT);
    digitalWrite(PIN_TX, LOW);

    //timer pour check_obstacles : toutes les 100ms
    timer_IR = timer.setInterval(100,check_Obstacle);
    timer.disable(timer_IR);
}


//Boucle principale
void loop()
{
    timer.run();
    //Serial.println("Loop");
    //delay(100);
}

void TX()
{
    digitalWrite(PIN_TX, HIGH);
    delay(1);
    digitalWrite(PIN_TX, LOW);
}
    
//verification présence obstacles
void check_Obstacle()
{
    int i;
    for (i=0;i<4;i++)
    {
        if (IR_ON[i])
        {
            if (analogRead(IR_PIN[i]) > 300)
            {
                Serial.write('O');
                TX();
            }
        }
    }
}
        
/* Communication Série (avec Raspberry), cette fct se lance lorsqu'un message est recu sur la liaison série */
void serialEvent()
{
    //On lit le caractère reçu
    char code = Serial.read();
    delay(5);
    switch (code)
    {
        //Demande l'ID de l'arduino
        case 'I':
        {
            Serial.write(ID);
            TX();
            break;
        }
        //Définit les capteurs IR actifs
        case 'C':
        {
           bool state[8];
           while(Serial.available()<1){}
           char c = Serial.read();
           // On fait une conversion char -> binaire; On obtient 0000XXXX, les X correspondent à l'état des capteurs
           byte_to_bool(c,state);
           IR_ON[0] = state[0];
           IR_ON[1] = state[1];
           IR_ON[2] = state[2];
           IR_ON[3] = state[3];    
                       
            //A la première définition de l'état des capteurs, on commence la détection d'obstacles
            if (!timer.isEnabled(timer_IR))
            {
                timer.enable(timer_IR);
            }
            break;
        }
        //Reception des commandes servomoteurs
        case 'S':
        {
            //Recevoir commandes Ax-12 et transferer aux servos
            Read_set_AX12();
            break;
        }
        default:
        {
          break;
        }
    }
}



        
/* Fonction de conversion d'un char en tableau binaire */
void byte_to_bool(char c, bool result[])
{
    int i;
    for (i=7;i>=0;i--)
    {
        if (c >= pow(2,i))
        {
            result[i] = HIGH;
            c-=pow(2,i);
        }
        else
            result[i] = LOW;
            
       //Serial.print(result[i]);
    }
}

/* Fonction de lecture des commandes servo sur la liaison série et d'application de ces commandes*/
void Read_set_AX12()
{
    int i;
    for (i=0;i<8;i++)
    {
        Dynamixel.move(ID_Servos[i],Serialreadint());
    }
}


/* Fonction de lecture d'un int sur la liaison série */
int Serialreadint()
{    
  char BUFFER[4]={NULL};  
  while (Serial.available()<4){}
  for(byte i = 0; i < 4; i++) 
  {  
    BUFFER[i] = Serial.read();  
    delay(5);
  }
  return atoi(BUFFER);
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
    
             
