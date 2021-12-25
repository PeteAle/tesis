// Tercer intento en el programa de comunicación daisy chain entre motores.

#include <EEPROM.h>

#define enablePin 13

// -------------------- Set de comandos --------------------
char comandoAssignAddress = 'A';
char comandoSet = 'S';
char comandoReset = 'R';
char comandoGetValue = 'G';
char comandoOK = 'O';
char finDatos = 'Z';

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(enablePin,OUTPUT);
  digitalWrite(enablePin,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  recibirComandos();
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
        setResetLed();
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
    //addressMotor = atoi(addressMotor);
    siguienteAddress = addressMotor + 1;
    Serial.write(primerMarcador);
    Serial.write(siguienteAddress);
    Serial.write(ultimoMarcador);
  }
}

void setResetLed(){
  /*EEPROM.get(0,eepromVal);
  Serial.write(eepromVal);
  Serial.write(primerMarcador);*/
  readCurrentAddress();
  if (primerMarcador >= 'a' && primerMarcador <= 'z'){
    if (eepromVal == primerMarcador){
      if (recibido[0] == comandoSet){
        digitalWrite(enablePin, HIGH);
        sendOK();
      }
      else if (recibido[0] == comandoReset){
        digitalWrite(enablePin, LOW);
        sendOK();
      }
      else if (recibido[0] == comandoGetValue){
        getMotorValue();
      }
    }
    else{
      if (tempIndice > 1){
        Serial.write(primerMarcador);
        Serial.write(recibido[0]);
        Serial.write(recibido[1]);
        Serial.write(ultimoMarcador);
      }
      else{
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

void getMotorValue(){
  estadoLed = digitalRead(enablePin);
  Serial.write(primerMarcador);
  Serial.write(recibido[0]);
  Serial.write(estadoLed+48); // Serial.write envía ASCII entonces se le suma el 48 para que se pueda desplegar.
  Serial.write(finDatos);
}

void sendOK(){
  Serial.write(comandoOK);
  Serial.write('K');
}
