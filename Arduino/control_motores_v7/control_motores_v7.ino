#include <EEPROM.h>

//**************************** Definición de comandos generales *****************************
#define comandoConfig 'C'       // Para utilizar el listado de comandos de configuración.
#define comandoMovimiento 'M'   // Para utilizar el listado de comandos de movimiento.
#define comandoDiagnostico 'D'  // Para utilizar el listado de comandos de diagnóstico.
//*******************************************************************************************
//********************************* Definición de marcadores ********************************
#define finDatos 'Z'
//*******************************************************************************************
//******************************** Definición de pines driver *******************************
#define dirPin 2
#define stepPin 3
#define ms1 6
#define ms2 5
#define ms3 4
//*******************************************************************************************

/*
Los comandos de CONFIGURACIÓN permiten configurar el arduino y por afecta el movimiento del motor.
  > A - asigna el address del motor para guardarlo en el EEPROM (SIEMPRE HACER ESTO PRIMERO).
  > M - asigna el microstepping que se desee (los consecuentes movimientos y sumas de posición van a funcionar
        en base al microstepping establecido).
    > Establecer el driver que se está utilizando:
      > A49 = A4988
      > DRV = DRV8825
      > 249 = TB67S249FTG
      > 128 = TB67S128FTG
    > 01 - full-step
    > 02 - half-step
    > 04 - quarter-step
    > 08 - 1/8 step
    > 16 - 1/16 step
    > 32 - 1/32 step
  > C - establece cero relativo
Los comandos de MOVIMIENTO permiten ordenarle al motor stepper que ejecute los pasos.
  > S - general el número de pasos establecidos en conjunto con el comando.
  > P - 
Los comandos de DIAGNÓSTICO
  > G - retorna el valor de pasos anteriormente ejecutados.
  > P - retorna la posición del motor relativo al cero relativo establecido anteriormente. NO VA A FUNCIONAR BIEN SIN
        HABER ESTABLECIDO UN CERO RELATIVO ANTERIORMENTE.
*/

char comandoEspecifico = 0;
char comandoGeneral = 0;
char eepromVal = 0;

bool recibiendo = false;
static bool datosNuevos = false;
const byte longitud = 64;
char recibido[longitud];
char tempRecibido[longitud];
unsigned int indice = 0;
unsigned int tempIndice = 0;
char primerMarcador = 0;
char ultimoMarcador = 0;
char inData = 0;
byte longitudDatos = 0;
byte cerosNecesarios = 0;

char addressMotor = 0;
char siguienteAddress = 0;
bool motorSelected = false;
char pasosMotor[longitud];
unsigned long pasosMotorLong[longitud];
unsigned long pasosTotales = 0;
unsigned long contadorPasos = 0;
bool pasosGuardados = false;
unsigned long pasosAnteriores = 0;
char lecturaStop = 0;
byte recibidoStop[longitud];
char getPasos[longitud];
byte microsteps = 1;

char dirGiro = 0;

char driverSelect[3];

unsigned int i, j, k;
//unsigned int j = 0;

float posicion = 0.0;
float tempPosicion = 0.0;
unsigned int posEnteros = 0;
char posEnterosChar[longitud];
unsigned int posDecimales = 0;
char posDecimalesChar[longitud];
char direccionActual = '+';
//byte longitudEnteros = 0;
//byte longitudDecimales = 0;

unsigned long multiplos[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(ms3, OUTPUT);
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  recepcionComandos();
}

void recepcionComandos(){
  while (Serial.available() > 0 && datosNuevos == false){
    inData = Serial.read();
    if (recibiendo == true){
      if (inData != finDatos){
        recibido[indice] = inData;
        indice++;
        tempIndice = indice;
        if (indice > longitud){
          indice = longitud - 1; 
        }
      }
      else if (inData == finDatos){
        tempIndice = indice;
        ultimoMarcador = inData;
        // *************************** Asignación de address ***************************
        if (primerMarcador == comandoConfig && recibido[0] == 'A'){
          comandoGeneral = primerMarcador;
          comandoEspecifico = recibido[0];
          comandoAsignarAddr(0,recibido[1]);
        }
        // *****************************************************************************
        // ++++ Sección de código para comandos que requieren un address al inicio +++++
        else if ((primerMarcador >= 'a' && primerMarcador <= 'z')){
          verificarAddress(0,primerMarcador);     // Esta función compara el valor de primerMarcador con el valor en el address 0 del arduino.
          if (motorSelected == true){             // Este booleano cambia a true cuando la comparación anterior sea cierta.
            comandoGeneral = recibido[0];         // El sistema asume que el primer valor de recibido es el comando general.
            if (comandoGeneral == comandoConfig){ // if general que determinar primero si se encuentra bajo la categoría de config.
              if (recibido[1] == 'M'){            // Verifica si el comando específico bajo la categoría de config es la de microstepping.
                comandoEspecifico = recibido[1];  // Guarda el comando específico de microstepping en una variable.
                driverSelect[0] = recibido[2];
                driverSelect[1] = recibido[3];
                driverSelect[2] = recibido[4];
                comandoMicrostepping(driverSelect, recibido[5], recibido[6]);
              }
              else if (recibido[1] == 'C'){
                posicion = 0;
              }
            }
            else if (comandoGeneral == comandoMovimiento){  // if general que determinar primero si se encuentra bajo la categoría de movimiento.
              if (recibido[1] == 'S'){
                comandoEspecifico = recibido[1];
                dirGiro = recibido[2];
                conversionPasos();                          // Conversión de los datos recibidos de char array a un unsigned long para permitir movimiento del motor.
                comandoStepping(dirGiro);                   // Ejecución del movimiento de los motores con la dirección recibida.
              }
            }
            else if (comandoGeneral == comandoDiagnostico){
              if (recibido[1] == 'G'){
                comandoEspecifico = recibido[1];
                comandoGetPasosAnteriores();
              }
              else if (recibido[1] == 'P'){
                comandoEspecifico = recibido[1];
                comandoGetPosicion();
              }
            }
          }
          // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
          // ********* Else que reenvia el comando si no es el micro correcto ***********
          else if (motorSelected == false){
            Serial.write(primerMarcador);
            for (i = 0; i < tempIndice; i++){
              Serial.write(recibido[i]);
            }
            
            Serial.write(ultimoMarcador);
          }
          // ****************************************************************************         
        }
        // ******* Vaciado del array y preparación para entrada de nuevos datos *********
        recibido[indice] = '\0';
        indice = 0;
        recibiendo = false;
        datosNuevos = true;
        // ******************************************************************************
      }
    }
    else if ((inData == comandoConfig || inData == comandoDiagnostico || (inData >= 'a' && inData <= 'z'))){
      //Serial.write(primerMarcador);
      recibiendo = true;
      primerMarcador = inData;
    }
  }
  Serial.flush();
  datosNuevos = false;
}

void comandoAsignarAddr(char addrPos, char addr){
  if (tempIndice == 2){
    addressMotor = addr;
    EEPROM.write(addrPos,addressMotor);
    siguienteAddress = addressMotor + 1;
    Serial.write(comandoGeneral);
    Serial.write(comandoEspecifico);
    Serial.write(siguienteAddress);
    Serial.write(ultimoMarcador);
  }
}

void comandoMicrostepping(char driverSel[3], char primerValor, char segundoValor){
  if (tempIndice == 7){
    // **************************** Para driver A4988 ***********************************
    if (driverSel[0] == 'A' && driverSel[1] == '4' && driverSel[2] == '9'){
      if (primerValor == '0'){
        if (segundoValor == '1'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 1;
        }
        else if (segundoValor == '2'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 2;
        }
        else if (segundoValor == '4'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, LOW);
          microsteps = 4;
        }
        else if (segundoValor == '8'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, LOW);
          microsteps = 8;
        }
      }
      // COMENTAR ESTA SECCIÓN SI SE ESTÁ UTILIZANDO UN DRV8825 y DESCOMENTAR EL 16 Y 32 de DE ABAJO.
      else if (primerValor == '1'){
        if (segundoValor == '6'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, HIGH);
          microsteps = 16;
        }
      }
    }
    // **********************************************************************************
    // ***************************** Para driver DRV8825 ********************************
    else if (driverSel[0] == 'D' && driverSel[1] == 'R' && driverSel[2] == 'V'){
      if (primerValor == '0'){
        if (segundoValor == '1'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 1;
        }
        else if (segundoValor == '2'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 2;
        }
        else if (segundoValor == '4'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, LOW);
          microsteps = 4;
        }
        else if (segundoValor == '8'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, LOW);
          microsteps = 8;
        }
      }
      else if (primerValor == '1'){
        if (segundoValor == '6'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, HIGH);
          microsteps = 16;
        }
      }
      else if (primerValor == '3'){
        if (segundoValor == '2'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, HIGH);
          microsteps = 32;
        }
      }
    }
    // **********************************************************************************
    // **************************** Para driver TB67S249TFG *****************************
    else if (driverSel[0] == '2' && driverSel[1] == '4' && driverSel[2] == '9'){
      if (primerValor == '0'){
        if (segundoValor == '1'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, HIGH);
          microsteps = 1;
        }
        // Este driver permite algo llamado "circular half-step" y "non-circular half-step". Por default, se programó
        // con el circular half-step.
        else if (segundoValor == '2'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 2;
        }
        else if (segundoValor == '4'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, HIGH);
          microsteps = 4;
        }
        else if (segundoValor == '8'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, HIGH);
          microsteps = 8;
        }
      }
      else if (primerValor == '1'){
        if (segundoValor == '6'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, LOW);
          microsteps = 16;
        }
      }
      else if (primerValor == '3'){
        if (segundoValor == '2'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, HIGH);
          microsteps = 32;
        }
      }
    }
    else if (driverSel[0] == '1' && driverSel[1] == '2' && driverSel[2] == '8'){
      if (primerValor == '0'){
        if (segundoValor == '1'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 1;
        }
        // Este driver permite algo llamado "circular half-step" y "non-circular half-step". Por default, se programó
        // con el circular half-step.
        else if (segundoValor == '2'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, HIGH);
          microsteps = 2;
        }
        else if (segundoValor == '4'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, LOW);
          microsteps = 4;
        }
        else if (segundoValor == '8'){
          digitalWrite(ms1, LOW);
          digitalWrite(ms2, HIGH);
          digitalWrite(ms3, HIGH);
          microsteps = 8;
        }
      }
      else if (primerValor == '1'){
        if (segundoValor == '6'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, LOW);
          microsteps = 16;
        }
      }
      else if (primerValor == '3'){
        if (segundoValor == '2'){
          digitalWrite(ms1, HIGH);
          digitalWrite(ms2, LOW);
          digitalWrite(ms3, HIGH);
          microsteps = 32;
        }
      }
    }
    //Serial.print(driverSel);
    //Serial.print(microsteps);
  }
}

/*void comandoMicrosteppingDRV(char primerValor, char segundoValor){
  if (tempIndice == 4){
    if (primerValor == '0'){
      if (segundoValor == '1'){
        digitalWrite(ms1, LOW);
        digitalWrite(ms2, LOW);
        digitalWrite(ms3, LOW);
        microsteps = 1;
      }
      else if (segundoValor == '2'){
        digitalWrite(ms1, HIGH);
        digitalWrite(ms2, LOW);
        digitalWrite(ms3, LOW);
        microsteps = 2;
      }
      else if (segundoValor == '4'){
        digitalWrite(ms1, LOW);
        digitalWrite(ms2, HIGH);
        digitalWrite(ms3, LOW);
        microsteps = 4;
      }
      else if (segundoValor == '8'){
        digitalWrite(ms1, HIGH);
        digitalWrite(ms2, HIGH);
        digitalWrite(ms3, LOW);
        microsteps = 8;
      }
    }
    // COMENTAR ESTA SECCIÓN (16 Y 32) SI SE ESTÁ UTILIZANDO UN A4988.
    else if (primerValor == '1'){
      if (segundoValor == '6'){
        digitalWrite(ms1, LOW);
        digitalWrite(ms2, LOW);
        digitalWrite(ms3, HIGH);
        microsteps = 16;
      }
    }
    else if (primerValor == '3'){
      if (segundoValor == '2'){
        digitalWrite(ms1, HIGH);
        digitalWrite(ms2, LOW);
        digitalWrite(ms3, HIGH);
        microsteps = 32;
      }
    }
  }
}*/

void verificarAddress(byte eepromAddrPos, char incomingAddr){
  eepromVal = EEPROM.read(eepromAddrPos);
  if (eepromVal == incomingAddr){
    motorSelected = true;
  }
  else if (eepromVal != incomingAddr){
    motorSelected = false;
  }
}

void conversionPasos(){
  longitudDatos = tempIndice; // 13 es el máximo entonces se resta longitudDatos de 13 para determinar el número de ceros para colocar.
  //Serial.print(longitudDatos);
  /*for (k = 0; k < tempIndice; k++){
    tempRecibido[k] = recibido[k];
  }*/
  // *********************************************************************************
  // Para esta sección, mejor intentar iterarlo en un pizarrón o en una hoja de papel,
  // es demasiado abstracto para cranearselo de forma mental.
  // Haganse el favor y no lo intenten, yo lo intenté.
  for (k = 0; k < 3; k++){
    tempRecibido[k] = recibido[k];
  }
  for (k = 3; k < 3 + (13 - tempIndice); k++){
    tempRecibido[k] = '0';
  }
  for (k = 0; k < tempIndice-3; k++){
    tempRecibido[k + (3 + (13 - tempIndice))] = recibido[k+3];
  }
  for (k = 0; k < 13; k++){
    recibido[k] = tempRecibido[k];
  }
  // *********************************************************************************
  //Serial.print(recibido);
  for (j = 0; j < 10; j++){
    pasosMotor[j] = recibido[j+3]-48;
    pasosMotorLong[j] = pasosMotor[j];
    pasosMotorLong[j] = pasosMotorLong[j]*multiplos[j];
    pasosTotales = pasosTotales + pasosMotorLong[j];
  }
  if (pasosGuardados == false && (pasosAnteriores == 0 || contadorPasos == 0)){
    pasosAnteriores = pasosTotales;
    pasosGuardados = true;
  }
  //Serial.print(pasosTotales, DEC);
  contadorPasos = pasosTotales;
  Serial.print(contadorPasos);
  pasosTotales = 0;
}

void comandoStepping(char direccion){
  for (int k = 0; k < contadorPasos; k++){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800);  // original 15000
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);  // original 15000
    if (direccion == '+'){
      digitalWrite(dirPin, LOW);
      if (microsteps == 1){
        posicion = posicion + 1.80;
      }
      else if (microsteps == 2){
        posicion = posicion + 0.90;
      }
      else if (microsteps == 4){
        posicion = posicion + 0.45;
      }
      else if (microsteps == 8){
        posicion = posicion + 0.23;
      }
      else if (microsteps == 16){
        posicion = posicion + 0.11;
      }
      // Comentar esta sección si se está usando un A4988.
      else if (microsteps == 32){
        posicion = posicion + 0.06;
      }
    }
    else if (direccion == '-'){
      digitalWrite(dirPin, HIGH);
      if (microsteps == 1){
        posicion = posicion - 1.80;
      }
      else if (microsteps == 2){
        posicion = posicion - 0.90;
      }
      else if (microsteps == 4){
        posicion = posicion - 0.45;
      }
      else if (microsteps == 8){
        posicion = posicion - 0.23;
      }
      else if (microsteps == 16){
        posicion = posicion - 0.11;
      }
      // Comentar esta sección si se está usando un A4988.
      else if (microsteps == 32){
        posicion = posicion - 0.06;
      }
    }
    //Serial.print(posDecimales);
    // ************* Sección que lee constantemente por un comando de STOP *************
    lecturaStop = Serial.read();
    if (lecturaStop >= 'a' && lecturaStop <= 'z'){
      recibidoStop[0] = lecturaStop;
      lecturaStop = Serial.read();
      if (lecturaStop == 'M'){
        recibidoStop[1] = lecturaStop;
        lecturaStop = Serial.read();
        if (lecturaStop == 'P'){
           recibidoStop[2] = lecturaStop;
           condicionStop();
           break;
        }
      }
    }
    //**********************************************************************************
  }
  //Serial.print(pasosTotales);
  //pasosTotales = 0;
  pasosGuardados = false;
  contadorPasos = 0;
}

void comandoCeroRelativo(){
  posicion = 0;
}

void condicionStop(){
  digitalWrite(stepPin, LOW);
}

void comandoGetPasosAnteriores(){
  itoa(pasosAnteriores,getPasos,10);
  //Serial.write(strlen(getPasos));
  Serial.write(primerMarcador);
  Serial.write(comandoGeneral);
  Serial.write(comandoEspecifico);
  for (i = 0; i < strlen(getPasos); i++){
    Serial.write(getPasos[i]);
  }
  Serial.write(ultimoMarcador);
}

void comandoGetPosicion(){
  // ****************** Sección de manejo de la posición negativa ********************
  tempPosicion = posicion;
  if (posicion < 0.0){
    posicion = posicion - 2*(posicion);
    direccionActual = '-';
  }
  else{
    direccionActual = '+';
  }
  posEnteros = (unsigned int)posicion;
  //Serial.print(posEnteros);
  posDecimales = (unsigned int)((posicion - posEnteros) * 100);
  itoa(posEnteros, posEnterosChar, 10);
  itoa(posDecimales, posDecimalesChar, 10);
  // *********************************************************************************
  Serial.write(primerMarcador);
  Serial.write(comandoGeneral);
  Serial.write(comandoEspecifico);
  //Serial.print(posEnteros);
  //Serial.print(posEnterosChar);
  //Serial.print(strlen(posEnterosChar));
  if (strlen(posEnterosChar) == 0){
    //longitudEnteros = 1;
    Serial.write(direccionActual);
    Serial.write('0');
  }
  else{
    //longitudEnteros = strlen(posEnterosChar);
    /*if (posicion < 0.0){
      Serial.write(direccionActual);
    }
    else if (posicion > 0.0){
      Serial.write(direccionActual);
    }*/
    Serial.write(direccionActual);
    for (i = 0; i < strlen(posEnterosChar); i++){
      Serial.write(posEnterosChar[i]);
    }
  }
  Serial.write('.');
  if (strlen(posDecimalesChar) == 0){
    //longitudDecimales = 1;
    Serial.write('0');
  }
  else{
    //longitudDecimales = strlen(posDecimalesChar);
    for (j = 0; j < strlen(posDecimalesChar); j++){
      Serial.write(posDecimalesChar[j]);
    }
  }
  Serial.write(ultimoMarcador);
  posicion = tempPosicion;
}
