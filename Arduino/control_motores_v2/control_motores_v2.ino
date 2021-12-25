// Intento de organizar el código original de los motores en un nuevo .ino

#include "pt.h"

// Pin de dirección del stepper, 
// dirPin = 1 --> ccw
// dirPin = 0 --> clockwise
#define dirPin 2
#define stepPin 3
#define enablePin 13
#define led_pin 4

// -------------- Constantes para counter -----------------

char i = 0;

// -------------- Para recepción de datos -----------------
char inData;
char comandoSeleccionarMotor = 'S';
char comandoEnableMotores = 'E';
char comandoRecepcionVision = 'V';
char comandoRecepcionMando = 'C';
char comandoModoManual = 'M';
char comandoModoAuto = 'A';
char finDatos = 'Z';
const byte longitud = 128;
char recibido [longitud];
char indice = 0;
char tempIndice = 0;
static bool datosNuevos = false;
bool recibiendo = false;
char primerMarcador;
char ultimoMarcador;
// ---------------------------------------------------------

// ------------- Para funcionamiento de motores ------------
char numMotores = 0;
char numMaxMotores = 0;
char motorActual = 0;
char numMotoresSiguientes = 0;
bool motorActivado = false;
static bool motorSeleccionado = false;
struct motores {
  float primerMotor;
  float segundoMotor;
  float tercerMotor;
  float cuartoMotor;
  float quintoMotor;
  float sextoMotor; 
};
typedef struct motores motores;
motores steppers; // Esto define "steppers" como de tipo motores, por lo que las variables se llaman steppers.primerMotor, etc.
// ---------------------------------------------------------

static struct pt pt1;
static int proto1(struct pt *pt){
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin,LOW);
  pinMode(led_pin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  recibirComandosDatos();
  pasosDeMotor();
  delay(10);
  //enableMotors();
}

void recibirComandosDatos(){
  while(Serial.available() > 0 && datosNuevos == false){
    inData = Serial.read();
    if (recibiendo == true){
      if ((inData != finDatos)){
        recibido[indice] = inData;
        indice++;
        tempIndice = indice;
        if (indice > longitud){
          indice = longitud - 1;
        }
      }
      else if (inData == finDatos){
        tempIndice = indice; // OJO! ESTO NO PARECE QUE APLICA SIEMPRE! --> El valor de indice se encuentra en forma de ASCII, por eso se agrega el 48.
        ultimoMarcador = inData;
        recibido[indice] = '\0';
        indice = 0;
        if (primerMarcador == comandoEnableMotores){
          if (tempIndice > 2 || tempIndice < 2){
            Serial.println("\n\rIngrese un número adecuado de motores\n\r");
          }
          else if (tempIndice == 2){
            enableMotors();
            /*Serial.write(primerMarcador);
            Serial.write(recibido);
            Serial.write(ultimoMarcador);*/
          }
        }
        recibiendo = false;
        datosNuevos = true;
      }
    }
    else if (inData == comandoEnableMotores || inData == comandoRecepcionVision || inData == comandoRecepcionMando || inData == comandoSeleccionMotor){
      recibiendo = true;
      primerMarcador = inData;
    }
  }
  Serial.flush();
  datosNuevos = false;
}

/*void enableMotors(){
  if (primerMarcador == comandoEnableMotores){
    numMaxMotores = recibido[0];
    numMotores = recibido[1];
    if ((49 <= numMotores) && (numMotores <= 55)){
      //Serial.write("Sí entra a la condicional");
      digitalWrite(enablePin,HIGH);
      motorSeleccionado = true;
      if (motorSeleccionado = true){
        motorActivado = true;
        numMotoresSiguientes = numMotores - 1;
        Serial.write(primerMarcador);
        Serial.write(numMaxMotores);
        Serial.write(numMotoresSiguientes);
        Serial.write(ultimoMarcador);
        motorSeleccionado = false;
      }
    }
    else if (numMotores == 48){ // 48 porque por alguna razón lo interpreta como un valor en ascii.
      Serial.println(numMotores);
      digitalWrite(enablePin,LOW);
      Serial.write(primerMarcador);
      Serial.write(numMaxMotores);
      Serial.write(numMotores);
      Serial.write(ultimoMarcador);
    }
  }
}*/

void enableMotors(){
  if (primerMarcador == comandoEnableMotores){
    numMotores = recibido[0];
    if (numMotores <= 49 && numMotores <= 56){
      digitalWrite(enablePin,HIGH);
      motorSeleccionado = true;
      if (motorSeleccionado == true){
        motorActual = numMotores;
        numMotoresSiguientes = numMotores + 1;
        Serial.write(primerMarcador);
        Serial.write(numMotoresSiguientes);
        Serial.write(ultimoMarcador);
      }
    }
    else{
      //Serial.write("\n\rIngresar un número adecuado de motores\n\r");
    }
  }
}

void pasosDeMotor(){
  if (primerMarcador == comandoRecepcionVision){
    char * token;

    token = strtok(recibido, ",");
    steppers.primerMotor = atof(token);

    token = strtok(NULL, ",");
    steppers.segundoMotor = atof(token);
    
  }
}
