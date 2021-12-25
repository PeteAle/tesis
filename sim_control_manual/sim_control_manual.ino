/* 
 *  Programa que va a intentar simular el control manual de la parte de Corona.
 *  Puede que la manera de funcionamiento sea diferente, esto es para hacer pruebas preliminares.
*/

#include <stdlib.h>

#define encoder A0
#define longitud 4
#define dirPin 3
#define stepPin 2

int valActual = 0;
int valAnterior = 0;

int encoderArray[longitud];
byte piezas[longitud];
char * encoderValConv;
int encoderVal = 0;
int encoderMapped = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(encoder,INPUT);
  pinMode(dirPin,INPUT);
  pinMode(stepPin,INPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  encoderVal = analogRead(encoder);
  encoderMapped = map(encoderVal,0,1024,-10,10);
  valActual = encoderMapped;
  //encoderArray = encoderMapped;
  if (encoderMapped < 0){
    Serial.write('1');
    Serial.write(',');
    Serial.print(encoderMapped);
    Serial.write('Z');
  }
  else if (encoderMapped > 0){
    //Serial.write('+');
    Serial.write('1');
    Serial.write(',');
    Serial.write(encoderMapped+48);
    Serial.write('Z');
  }
  /*Serial.write(encoderMapped+48);
  for (int i = 0; i < 4; i++){
    piezas[i] = valActual[i];
  }*/
  //pulsos();
  //Serial.println(valActual);
  /*itoa(encoderMapped, encoderValConv, 10);
  Serial.write(encoderValConv);
  if (encoderMapped < 0){
    for (int i = 0; i < 4; i++){
      piezas[i] = encoderValConv[i];
      Serial.write(piezas[i]);
      delay(500);
    }
  }
  else if (encoderMapped > 0){
    piezas[0] = '+';
    for (int j = 1; j < 4; j++){
      piezas[j] = encoderValConv[j];
      Serial.write(piezas[j]);
      delay(500);
    }
  }*/
  //Serial.println(encoderMapped);
  delay(100);
}

void pulsos(){
  if (valAnterior < valActual){
    digitalWrite(dirPin,HIGH);
    for (int j = 0; j < (valActual - valAnterior)+1; j++){
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(15000);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(15000);
    }
  }
  else if (valAnterior > valActual){
    digitalWrite(dirPin,LOW);
    for (int j = 0; j < (valAnterior - valActual)+1; j++){
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(15000);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(15000);
    }
  }
  else if (valAnterior == valActual){
    
  }
  valAnterior = valActual;
}

void convertirPasos(){
  
}
