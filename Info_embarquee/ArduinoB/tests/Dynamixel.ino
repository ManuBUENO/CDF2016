#include <DynamixelSerial.h>

int Temperature,Voltage,Position; 

void setup(){
Dynamixel.begin(1000000,2);  // Inicialize the servo at 1Mbps and Pin Control 2
pinMode(11,INPUT);
digitalWrite(11,HIGH);
delay(1000);
}

void loop(){
  // Request and Print the Voltage
  while(!digitalRead(11)){}
  
  Position = Dynamixel.readPosition(9);       // Request and Print the Position 
  delay(1000);
  while(!digitalRead(11)){}
  Dynamixel.moveSpeed(9,Position,100);  // Move the Servo radomly from 200 to 800
 
 Dynamixel.end();                 // End Servo Comunication
 Serial.begin(9600);              // Begin Serial Comunication
 
  Serial.println(Position);

  
 Serial.end();                     // End the Serial Comunication
 Dynamixel.begin(1000000,2);         // Begin Servo Comunication
 
delay(1000);

}
