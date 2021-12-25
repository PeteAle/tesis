// Tercer intento en el programa de comunicación daisy chain entre motores.

#include <EEPROM.h>
#include "protothreads.h"

// Pin de dirección del stepper, 
// dirPin = 1 --> ccw
// dirPin = 0 --> clockwise
#define dirPin 2
#define stepPin 3
#define ms1 6
#define ms2 5
#define ms3 4

static struct pt pt1;
static int proto1(struct pt *pt){
  
}

// -------------------- Set de comandos --------------------
#define comandoAssignAddress 'A'  // Comando de asignación de nombre/address a cada microcontrolador.
//#define comandoDireccion 'D'      
#define comandoStep 'S'           // Hay que cambiar para que S sea para Steps.
//#define comandoReset 'R' 
#define comandoGetValue 'G'       // Se puede emplear para que el usuario verifique el valor de steps anteriormente enviado al uC deseado.
#define comandoGetPosition 'P'
#define comandoOK 'O'             // Comando para iniciar el OK que cada microcontrolador envia si el programa corrió correctamente.
#define finDatos 'Z'              // Final de transmisión de datos.
#define comandoMicroStepping 'M'  // Comando para hacer microstepping, se tendrá que establecer una lista de velocidades para poder usar esto.
#define comandoSetZero 'C'        // Comando para establecer un cero relativo de posición.
#define comandoEncoderTicks 'T'   // Comando para que el actuador dé cierto número de ticks en base al número recibido.

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
uint8_t siguienteAddress = 0;
//uint8_t addressDeseado = 0;

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

// ---------------------- Microsteps -----------------------
byte microsteps [2];
unsigned int microstepsInt [2];
unsigned int microstepsTotales = 0;
unsigned int microstepActual = 1;

// -------------------- Posición del motor -----------------
int posicion = 0;
char * posicionChar = 0;
byte angulos[6];

// ------------------ Múltiplos para mate ------------------
unsigned int multiplos[] = {10000, 1000, 100, 10, 1};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(dirPin,OUTPUT);
  pinMode(stepPin,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
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
        if (primerMarcador == comandoAssignAddress){
          asignarAddress();
        }
        else if ((primerMarcador >= 'a') && (primerMarcador <= 'z')){
          ejecutarComandos();
        }
        recibido[indice] = '\0';
        indice = 0;
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
          condicionStop(contadorPasos);
          Serial.print(contadorPasos);
        }
        else if (recibido[1] == '+'){
          digitalWrite(dirPin, HIGH);
          convertirPasos();
          contadorPasos = pasosTotales;
          pasosTotales = 0;
          condicionStop(contadorPasos);
          Serial.print(contadorPasos);
        }
      }
      else if (recibido[0] == comandoGetValue){
        Serial.write(primerMarcador);
        Serial.write(recibido[0]);
        Serial.write(pasosAnteriores[0]); // Este comando es para enviar el signo.
        // La condición de i > 6 aplica por si se quiere enviar 5 dígitos, e.g 12345
        // Para enviar 3 dígitos, usar i < 4.
        for (i = 1; i < 6; i++){
          Serial.write(pasosAnteriores[i]);
        }
        Serial.write(finDatos);
      }
      else if (recibido[0] == comandoEncoderTicks){
        ejecutarTicks();
      }
      else if (recibido[0] == comandoMicroStepping){
        setMicroStepping();
      }
      else if (recibido[0] == comandoSetZero){
        setZero();
      }
      else if (recibido[0] == comandoGetPosition){
        getPosition();
      }
    }
    else{
      if (tempIndice > 1 && tempIndice <= 7){
        Serial.write(primerMarcador);
        if (recibido[0] == comandoStep || recibido[0] == comandoGetValue || recibido[0] == comandoMicroStepping || recibido[0] == comandoSetZero || recibido[0] == comandoGetPosition){
          Serial.write(recibido[0]);
        }
        if (recibido[1] == '-' || recibido[1] == '+' || (recibido[1] >= '0' && recibido[1] <= '9')){
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
    //delayMicroseconds(16383);
    delayMicroseconds(15000);   // Este delay funciona si el programa de python contiene un time.sleep(0.02) pero SOLO DE MI PROGRA, NO LA DE HAPPY
    digitalWrite(stepPin, LOW);
    //delayMicroseconds(16383);
    delayMicroseconds(15000);   // Este delay funciona si el programa de python contiene un time.sleep(0.02) pero SOLO DE MI PROGRA, NO LA DE HAPPY
    //pasosTotales = pasosTotales - 1;
    //contadorPasos = contadorPasos - 1;
    int lecturaDir = digitalRead(dirPin);
    if (lecturaDir = HIGH){
      switch (microstepActual){
        // Es preferible trabajar en centígrados de manera que no se involucre el uso de floats.
        // Esto nos ahorra la necesidad de tomar en cuenta el punto decimal y le permite al
        // usuario asumir que siempre van a haber dos decimales.
        // 1.8 grados = 180, 0.9 grados = 090, etc.
        case 1:
          posicion = posicion + 180; // 1.8 -> 180
          break;
        case 2:
          posicion = posicion + 90; // 0.9 -> 090
          break;
        case 4:
          posicion = posicion + 45; // 0.45 -> 045
          break;
        case 8:
          posicion = posicion + 23; // 0.225 -> 023 Aquí es donde la exactitud comienza a sufrir.
          break;
        case 16:
          posicion = posicion + 11; // 0.1125 -> 011
      }
      posicionChar = itoa(posicion, angulos, 10);
      for (i = 0; i < 7; i++){
        angulos[i] = posicionChar[i];
        Serial.write(angulos[i]);
      }
      delay(200);
    }
    /*else if (lecturaDir == LOW){
      switch (microstepActual){
        case 1:
          posicion = posicion - 180;
          break;
        case 2:
          posicion = posicion - 90;
          break;
        case 4:
          posicion = posicion - 45;
          break;
        case 8:
          posicion = posicion - 23;
          break;
        case 16:
          posicion = posicion - 11;
      }
    }*/
    condicionStop(contadorPasos);
  }
}

// Función para detener los pasos del motor una vez estos lleguen a 0.
void condicionStop(uint16_t varMotorDeseado){
  if (varMotorDeseado == 0 || varMotorDeseado == 1){
    digitalWrite(stepPin, LOW);
  }
}

void convertirPasos(){
  // En esta sección, se debe cambiar el valor de i < x dependiendo de cuántos valores va a enviar la tiva
  // es decir, si la Tiva envía aS+00123Z, debe ser i < 7, porque en este caso, i = 0 representa la S, i = 1 representa el +, etc.
  // Entonces, si la tiva envía aS+123Z (ya cuando se empleen ángulos en lugar de pasos), i < 5.
  for (i = 2; i < 7; i++){
    pasosMotor[i-2] = recibido[i];
    pasosMotorInt[i-2] = pasosMotor[i-2] - 48;
    // OJO, la variable de multiplos[] también se tiene que cambiar conforme los datos que se trabajan.
    // En el caso que se trabaje con 5 dígitos (modo de pasos), usar multiplos[i-2].
    // En el caso que se trabaje con 3 dígitos (modo ángulo/error), usar multiplos[i].
    pasosMotorInt[i-2] = pasosMotorInt[i-2]*multiplos[i-2];
    pasosTotales = pasosTotales + pasosMotorInt[i-2];
  }
  if (primerosPasosGuardados == false && pasosTotales != 0){
    primerosPasos = pasosTotales;
    pasosAnteriores[0] = recibido[1];       // Guardado el signo.
    // Para el siguiente for, j < 4 es para guardar 3 dígitos.
    // Para guardar 5 dígitos, j < 6.
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

void setMicroStepping(){
  microsteps[0] = recibido[1];
  microsteps[1] = recibido[2];
  if (microsteps[0] == '0'){
    digitalWrite(ms3, LOW);
    switch (microsteps[1]){
      case '1':
        digitalWrite(ms1, LOW);
        digitalWrite(ms2, LOW);
        microstepActual = 1;
        break;
      case '2':
        digitalWrite(ms1, HIGH);
        digitalWrite(ms2, LOW);
        microstepActual = 2;
        break;
      case '4':
        digitalWrite(ms1, LOW);
        digitalWrite(ms2, HIGH);
        microstepActual = 4;
        break;
      case '8':
        digitalWrite(ms1, HIGH);
        digitalWrite(ms2, HIGH);
        microstepActual = 8;
        break;
      }
  }
  else if (microsteps[0] == '1'){
    digitalWrite(ms3, HIGH);
    if (microsteps[1] == '6'){
      digitalWrite(ms1, HIGH);
      digitalWrite(ms2, HIGH);
      microstepActual = 16;
    }
  }
}

void setZero(){
  posicion = 0;
  Serial.print(posicion);
}

void getPosition(){
  
  Serial.print(posicion);
}

void ejecutarTicks(){
  
}
