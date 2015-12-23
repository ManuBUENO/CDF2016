int ticks_D = 10;
  int ticks_G = 10;
  float a =2.15;
  float angle = 4.54;
  float cosa = 0.58455;
  float sina = 0.658546;
  float X = 58.45;
  float Y = 645.56;
  float d_a_mot = 15.15;
  float d_d_mot = 15.15;
  float cosangle;
  float sinangle;
  float difangle;
  int start;
  int fin;


void setup()
{
  Serial.begin(9600);
 
}

void loop()
{
  start = micros();
  //calculs DL
  angle += (ticks_D-ticks_G)*d_a_mot;
  difangle=angle-a;
  cosangle = cosa-difangle*sina-difangle*difangle*cosa*0.5;
  sinangle = sina+difangle*cosa-difangle*difangle*sina*0.5;
  X += (ticks_G+ticks_D)*cosangle*d_d_mot;
  Y += (ticks_G+ticks_D)*sinangle*d_d_mot;
  angle += (ticks_D-ticks_G)*d_a_mot;
  fin = micros();
  Serial.print("DL: ");
  Serial.println(fin-start);
  
  start = micros();
  //calculs cos
  a=angle;
  cosa=cos(angle);
  sina=sin(angle);
  fin = micros();
  Serial.print("fct: ");
  Serial.println(fin-start);
  
  delay(1000);  
}
