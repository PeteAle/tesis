// Tercer intento en el programa de comunicación daisy chain entre motores.

#include <EEPROM.h>
#include "protothreads.h"

// Pin de dirección del stepper, 
// dirPin = 1 --> ccw
// dirPin = 0 --> clockwise
#define dirPin 2
#define stepPin 3
#define ledPin 13

static struct pt pt1;
static int proto1(struct pt *pt){
  
}

// -------------------- Set de comandos --------------------
char comandoAssignAddress = 'A';  // Comando de asignación de nombre/address a cada microcontrolador.
char comandoDireccion = 'D';      // Comando para la Dir de los stepper.
char comandoStep = 'S';            // Hay que cambiar para que S sea para Steps.
char comandoReset = 'R'; 
char comandoGetValue = 'G';       // Se puede emplear para que el usuario verifique el valor de steps anteriormente enviado al uC deseado.
char comandoOK = 'O';             // Comando para iniciar el OK que cada microcontrolador envia si el programa corrió correctamente.
char finDatos = 'Z';              // Final de transmisión de datos.

// ---------------------- Marcadores -----------------------
char primerMarcador;
char ultimoMarcador;

// ------------------ Recepción de datos -------------------
uint8_t inData;
const byte longitud = 128;
uint8_t recibido [longitud];
static bool datosNuevos = false;
uint8_t indice = 0;
uint8_t tempIndice = 0;
bool recibiendo = false;
uint8_t addressMotor = 0;
uint8_t addressD1 = 0;
uint8_t addressD2 = 0;
uint8_t siguienteAddress = 0;
uint8_t addressDeseado = 0;
int estadoLed;

// ----------------- Lectura de EEPROM ---------------------
char eepromVal = 0;

// ---------------- Variables de contador ------------------
byte i = 0;
byte j = 0;

// ---------------- Variables para los pasos ---------------
bool primerosPasosGuardados = false;
const byte digitosPasos = 6;
byte pasosMotor [digitosPasos];
unsigned int pasosMotorInt [digitosPasos];
unsigned int pasosTotales = 0;
uint16_t primerosPasos = 0;
unsigned int contadorPasos = 0;
byte pasosAnteriores [digitosPasos] = {0,0,0,0,0,0};

// ------------------ Múltiplos para mate ------------------
int multiplos[] = {10000, 1000, 100, 10, 1};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(dirPin,OUTPUT);
  pinMode(stepPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  recibirComandos();
  pasosStepper();
}


void recibirComandos(){
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
        if (inData == 'K'){
          sendOK();
          recibido[indice] = '\0';
          indice = 0;
          recibiendo = false;
          datosNuevos = true;
        }
      }
      else if (inData == finDatos){
        tempIndice = indice; // OJO! ESTO NO PARECE QUE APLICA SIEMPRE! --> El valor de indice se encuentra en forma de ASCII, por eso se agrega el 48.
        ultimoMarcador = inData;
        recibido[indice] = '\0';
        indice = 0;
        if (primerMarcador == comandoAssignAddress){
          asignarAddress();
        }
        else if ((primerMarcador >= 'a') && (primerMarcador <= 'z')){
          ejecutarComandos();
        }
        recibiendo = false;
        datosNuevos = true;
      }
    }
    else if (inData == comandoAssignAddress || (inData >= 'a' && inData <= 'z') || inData == comandoGetValue || inData == comandoOK){
      recibiendo = true;
      primerMarcador = inData;
    }
  }
  Serial.flush();
  datosNuevos = false;
}

void asignarAddress(){
  if (tempIndice == 1){
    addressMotor = recibido[0];
    EEPROM.write(0,addressMotor);
    siguienteAddress = addressMotor + 1;
    Serial.write(primerMarcador);
    Serial.write(siguienteAddress);
    Serial.write(ultimoMarcador);
  }
}

void ejecutarComandos(){
  readCurrentAddress();
  if (primerMarcador >= 'a' && primerMarcador <= 'z'){
    if (eepromVal == primerMarcador){
      if (recibido[0] == comandoStep){
        if (recibido[1] == '-'){
          digitalWrite(dirPin, LOW);
          convertirPasos();
          contadorPasos = pasosTotales;
          pasosTotales = 0;
          Serial.print(contadorPasos);
        }
        else if (recibido[1] == '+'){
          digitalWrite(dirPin, HIGH);
          convertirPasos();
          contadorPasos = pasosTotales;
          pasosTotales = 0;
          Serial.print(contadorPasos);
        }
      }
      else if (recibido[0] == comandoGetValue){
        Serial.write(primerMarcador);
        Serial.write(recibido[0]);
        Serial.write(pasosAnteriores[0]);
        for (i = 1; i < 6; i++){
          Serial.write(pasosAnteriores[i]);
        }
        Serial.write(finDatos);
      }
    }
    else{
      if (tempIndice > 1 && tempIndice <= 7){
        Serial.write(primerMarcador);
        if (recibido[0] == comandoStep || recibido[0] == comandoGetValue){
          Serial.write(recibido[0]);
        }
        if (recibido[1] == '-' || recibido[1] == '+'){
          Serial.write(recibido[1]);
        }
        if (recibido[2] != finDatos){
          for (i = 2; i < tempIndice; i++){
            Serial.write(recibido[i]);
          }
        }
        Serial.write(ultimoMarcador);
      }
      else if (tempIndice == 1){
        Serial.write(primerMarcador);
        Serial.write(recibido[0]);
        Serial.write(ultimoMarcador);
      }
    }
  }
}

void readCurrentAddress(){
  eepromVal = EEPROM.read(0);
}

void sendOK(){
  Serial.write(comandoOK);
  Serial.write('K');
}

void pasosStepper(){
  if (contadorPasos > 0){
    digitalWrite(stepPin, HIGH);
    delay(5);
    digitalWrite(stepPin, LOW);
    delay(5);
    contadorPasos = contadorPasos - 1;
    condicionStop(contadorPasos);
  }
}

void condicionStop(uint16_t varMotorDeseado){
  if (varMotorDeseado == 0){
    digitalWrite(stepPin, LOW);
  }
}

void ejecutarPasos(){
  if (recibido[0] == comandoStep){
    if (recibido[1] == '-'){
      digitalWrite(dirPin, LOW);
      for (i = 2; i < 7; i++){
        Serial.write(recibido[i]);
        pasosMotor[i-2] = recibido[i]+1;
        Serial.write(',');
        Serial.write(pasosMotor[i-2]);
      }
    }
  }
}

void convertirPasos(){
  for (i = 2; i < 7; i++){
    pasosMotor[i-2] = recibido[i];
    pasosMotorInt[i-2] = pasosMotor[i-2] - 48;
    pasosMotorInt[i-2] = pasosMotorInt[i-2]*multiplos[i-2];
    pasosTotales = pasosTotales + pasosMotorInt[i-2];
  }
  if (primerosPasosGuardados == false && pasosTotales != 0){
    primerosPasos = pasosTotales;
    pasosAnteriores[0] = recibido[1];
    for (j = 1; j < 6; j++){
      pasosAnteriores[j] = pasosMotor[j-1];
    }
    primerosPasosGuardados = true;
  }
  else if (pasosTotales == 0){
    primerosPasosGuardados = false;
    primerosPasos = 0;
  }
}
