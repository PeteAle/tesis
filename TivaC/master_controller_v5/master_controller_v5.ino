#define inicioDatosMando '<'
#define finDatosMando '>'
#define finDatosOCR 'Z'
#define longitud 64

int encoderVal = 0;
int i = 0;
int j = 0;
int k = 0;

//const byte longitud = 64;
char recibido[longitud];
char inData = 0;
static bool datosNuevos = false;
bool recibiendo = false;

// ************************** Variables de funcionamiento del mando *************************
char inDataMando = 0;
char tempRecibidoMando[longitud];
char recibidoMando[longitud];
static bool datosNuevosMando = false;
bool recibiendoMando = false;
byte indiceMando = 0;
byte tempIndiceMando = 0;
char cerosNecesarios = 0;
struct datosMando {
  //char inDataMando;
  //bool datosNuevosMando;
  //bool recibiendoMando;
  //char datosRecibidos[longitud];
  byte marcadorInicio;
  byte modo;
  char modoChar;
  int motor;
  char motorChar;
  unsigned int ticks;
  char ticksArray[longitud];
  char signo[longitud];
  byte marcadorFinal;
};
datosMando datosMando = {};
// ******************************************************************************************
// *************************** Variables de funcionamiento del OCR **************************
char inDataOCR = 0;
char recibidoOCR[longitud];
char tempRecibidoOCR[longitud];
static bool datosNuevosOCR = false;
bool recibiendoOCR = false;
byte indiceOCR = 0;
byte tempIndiceOCR = 0;
char motorSelect = 0;
struct datosOCR {
  //char inDataMando;
  //char datosRecibidosOCR[longitud];
  byte motor;
  byte comando;
  byte signo;
  byte decena;
  int decenaInt;
  byte unidad;
  int unidadInt;
  byte decimal;
  int decimalInt;
  byte marcadorFinal;
  char pasosNecesarios[longitud];
}; 
datosOCR datosOCR;
// ******************************************************************************************
// **************************** Variable de estado/modo del sistema *************************
byte estado = 0;
// ******************************************************************************************
// **************************** Resoluciones para cierto # de grados ************************
/*
 * Para las relaciones de las juntas el valor de steps/mm o steps/graos son los siguientes:
 * > Junta base = 1:1
 * > Junta 2 = 73:1 del brazo, 100:1 de la caja de engranes del motor.
 * > Junta 3 = 73:1 del brazo, 100:1 de la caja de engranes del motor.
 *   > Para las juntas 2 y 3, en base a los cálculos hechos con un microstepping de full-step:
 *     > 0.05° = 200 pasos.
 *     > 0.1° = 400 pasos.
 *     > 1.0° =  4000 pasos.
 *     Estos datos son puramente teóricos por lo que puede variar en la práctica.
 * > Junta 4 = 1:1
 * > Junta 5 = 20:1 del engrane de gusano (4000 steps/360 grados o 11 steps/grado a full-step).
 * > Junta 6 = 1 rev -> 1 mm, 200 steps/mm a full-step.
 * > Junta 7 = 1:1
 * > Junta 8 = 1:1
*/
// ******************************************************************************************
// ************************************* Steps/grado junta 1 ********************************
#define pasos005GradosJ1 2    // Estos valores son asumiendo que la relación es realmente de 73:1.
#define pasos01GradosJ1 4
#define pasos1GradoJ1 41
#define pasos10GradosJ1 410
// ******************************************************************************************
// ******************************** Steps/grados juntas 2 y 3 *******************************
#define pasos005Grados 203  // Variable de pasos para juntas 2 y 3 para rotar 0.05 grados.
#define pasos01Grados 406   // Variable de pasos para juntas 2 y 3 para rotar 0.1 grados.
#define pasos1Grado 4056    // Variable de pasos para juntas 2 y 3 para rotar 1.0 grados.
#define pasos10Grados 40560 // Variable de pasos para juntas 2 y 3 para rotar 10.0 grados.
// ******************************************************************************************
// *********************************** Steps/grados junta 4 *********************************
// #define pasos005GradosJ4
// #define pasos01GradosJ4
// #define pasos1GradoJ4
// #define pasos10GradosJ4 
// ******************************************************************************************
// ************************** Steps/grados y steps/mm juntas 5 y 6 **************************
// La junta 5 del engranaje de gusano va a ir a 1/2 y la junta 6 se va a mantener en full stepping.
#define pasos10GradosWorm 222   // 
#define pasos1GradoWorm 22    // Variable de número de pasos para la junta 5 del engranaje de gusano para 1.0 grado.
#define pasos01GradosWorm 2     // Variable de número de pasos para la junta 5 del engranaje de gusano para 0.1 grados.
#define pasos005GradosWorm 1    // Variable de número de pasos para la junta 5 del engranaje de gusano para 0.05 grados.
#define pasosPor10MM 2000
#define pasosPor1MM 200          // Variable de número de pasos del motor de la junta 6 adjunta al M8 a movimiento traslacional para 1 mm.
#define pasosPor01MM 20         // Variable de 0.1 mm
#define pasosPor005MM 10        // Variable de 0.05 mm.
// ******************************************************************************************
// ********************************** Steps/grado junta 7 y 8 *******************************
// Estos motores van a estar fijos a un microstepping de 1/32 por su relación 1:1.
#define pasos005GradosNema11 1
#define pasos01GradosNema11 2
#define pasos1GradoNema11 20
#define pasos10GradosNema11 200
// ******************************************************************************************
// ************ Variables para guardar los grados y trabajar las conversiones****************
unsigned int gradosDecenas = 0;
unsigned int gradosUnidades = 0;
unsigned int gradosDecimas = 0;
unsigned int gradosCentesimas = 0;
// ******************************************************************************************
char driverDRV[5] = {'C','M','D','R','V'};
char driver249[8] = {'C','M','2','4','9','0','1','Z'};
char driver128[8] = {'C','M','1','2','8','0','1','Z'};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  //HardwareSerial Serial2(2);
  Serial2.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly: 
  //recibirDatos();
  //recibirDatosGeneral(Serial, datosMando);
  recibirDatosMando();
  recibirDatosOCR();
  //Serial.print(driverDRV);
  //delay(500);
}

void recibirDatosOCR(){
  while(Serial.available() > 0 && datosNuevosOCR == false){
    inDataOCR = Serial.read();
    if (recibiendoOCR == true){
      if (inDataOCR != finDatosOCR){
        recibidoOCR[indiceOCR] = inDataOCR;
        indiceOCR++;
        tempIndiceOCR = indiceOCR;
        if (indiceOCR > longitud){
          indiceOCR = longitud - 1;
        }
      }
      else if (inDataOCR == finDatosOCR){
        tempIndiceOCR = indiceOCR;
        datosOCR.marcadorFinal = inDataOCR;
        datosOCR.comando = recibidoOCR[0];
        datosOCR.signo = recibidoOCR[1];
        //Serial.write(datosOCR.signo);
        datosOCR.decena = recibidoOCR[2];
        //Serial.write(datosOCR.decena);
        datosOCR.unidad = recibidoOCR[3];
        //Serial.write(datosOCR.unidad);
        datosOCR.decimal = recibidoOCR[4];
        //Serial.write(datosOCR.decimal);
        //Serial.print("Aqui para");
        // OJO! estado = 1 para acitivar OCR, estado = 0 para mantener mando.
        if (estado == 1){
          //Serial.write("Aqui para");
          conversionDatosOCR();
          enviarComandosOCR();
        }
        recibidoOCR[indiceOCR] = '\0';
        indiceOCR = 0;
        recibiendoOCR= false;
        datosNuevosOCR = true;
      }
    }
    else if (inDataOCR >= 'e' && inDataOCR <= 'h'){
      datosOCR.motor = inDataOCR;
      recibiendoOCR = true;
    }
  }
  datosNuevosOCR = false;
}

void recibirDatosMando(){
  while(Serial1.available() > 0 && datosNuevosMando == false){
    inDataMando = Serial1.read();
    if (recibiendoMando == true){
      if (inDataMando != finDatosMando){
        recibidoMando[indiceMando] = inDataMando;
        indiceMando++;
        tempIndiceMando = indiceMando;
        if (indiceMando > longitud){
          indiceMando = longitud - 1;
        }
      }
      else if (inDataMando == finDatosMando){
        tempIndiceMando = indiceMando;
        datosMando.marcadorFinal = inDataMando;
        separacionDatosMando();
        // ****************** Envío de datos recibidos del mando en formado del comando *********************
        if (estado == 0){
          Serial1.write(datosMando.motorChar);
          Serial1.write('M');
          Serial1.write('S');
          // Para el símbolo es necesario establecer una divisón de nuevo.
          if (datosMando.signo[0] == '+' || datosMando.signo[0] == '-'){
            Serial1.write(datosMando.signo[0]);
          }
          /*for (i = 0; i < cerosNecesarios; i++){
            Serial.write('0');
          }*/
          for (j = 0; j < strlen(datosMando.ticksArray); j++){
            Serial1.write(datosMando.ticksArray[j]);
          }
          //Serial.write(datosMando.ticksChar);
          Serial1.write('Z');
          //Serial.print(datosMando.ticksArray);
          /*for (j = 0; j < strlen(datosMando.ticksArray); j++){
            datosMando.ticksArray[j] = '0';
          }*/
        }
        //delay(200);
        // **************************************************************************************************
        // ************ Vaciado de la variable de recepción y establecer que hay datos nuevos ***************
        recibidoMando[indiceMando] = '\0';
        indiceMando = 0;
        recibiendoMando = false;
        datosNuevosMando = true;
        // **************************************************************************************************
      }
    }
    else if (inDataMando == inicioDatosMando){
      datosMando.marcadorInicio = inDataMando;
      recibiendoMando = true;
    }
  }
  datosNuevosMando = false;
}

void separacionDatosMando(){
  // Se ejecuta si se guardó el < en la variable del marcadorInicio dentro del struct de datosMando.
  if (datosMando.marcadorInicio == inicioDatosMando){
    // *** Guardado de los datos de la variable recibidoMando a un temp por si se quieren utilizar luego. ***
    char * token;
    for (int k = 0; k < tempIndiceMando; k++){
      tempRecibidoMando[k] = recibidoMando[k];
    }
    // ******************************************************************************************************
    // ******************************** Extracción de variable del modo *************************************
    token = strtok(recibidoMando, ",");
    datosMando.modo = atoi(token);
    //Serial.print(datosMando.modo);
    if(datosMando.modo == 0){
      // Cambiar de vuelta a estado = 0 cuando se terminen las pruebas con el OCR.
      estado = 0;
      //Serial.print("estado0");
    }
    else if (datosMando.modo == 1){
      estado = 1;
      // estado = 1 para activar OCR.
      //Serial.print("estado1");
    }
    datosMando.modoChar = datosMando.modo+48;
    // ******************************************************************************************************
    // ******************************* Extracción de variable del motor *************************************
    token = strtok(NULL, ",");
    datosMando.motor = atoi(token);
    //Serial.print(datosMando.motor);
    reasignarMotor(datosMando.motor);
    datosMando.motorChar = motorSelect;
    // ******************************************************************************************************
    // ********************** Extracción de los ticks para el movimiento del motor **************************
    token = strtok(NULL, ",");
    //datosMando.ticksArray = token;
    datosMando.ticks = atoi(token);
    resMovimientoConvertido();
    //Serial.print(datosMando.ticks);
    //resolucionMovimiento();
    //resMovimientoConvertido();
    //itoa(datosMando.ticks,datosMando.ticksArray,10);
    //datosMando.ticksChar = datosMando.ticks+48;
    // ******************************************************************************************************
    // ********************************* Extracción del signo de giro ***************************************
    token = strtok(NULL, ",");
    strcpy(datosMando.signo,token);
    //datosMando.signo = token;
    //datosMando.signoInt = atoi(token);
    //Serial.write(datosMando.signo);
    // ******************************************************************************************************
    // ******************************** Ajuste de resolución por actuador ***********************************
    // ******************************************************************************************************
  }
}

void reasignarMotor(int numMotor){
  switch (numMotor){
    case 1:
      motorSelect = 'a';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driver128); i++){
        Serial1.write(driver128[i]);
      }*/
      break;
    case 2:
      motorSelect = 'b';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driverDRV); i++){
        Serial1.write(driverDRV[i]);
      }
      Serial1.write('0');
      Serial1.write('1');
      Serial1.write('Z');*/
      break;
    case 3:
      motorSelect = 'c';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driverDRV); i++){
        Serial1.write(driverDRV[i]);
      }
      Serial1.write('0');
      Serial1.write('1');
      Serial1.write('Z');*/
      break;
    case 4:
      motorSelect = 'd';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driver249); i++){
        Serial1.write(driver249[i]);
      }*/
      break;
    case 5:
      motorSelect = 'e';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driverDRV); i++){
        Serial1.write(driverDRV[i]);
      }
      Serial1.write('3');
      Serial1.write('2');
      Serial1.write('Z');*/
      break;
    case 6:
      motorSelect = 'f';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driverDRV); i++){
        Serial1.write(driverDRV[i]);
      }
      Serial1.write('0');
      Serial1.write('2');
      Serial1.write('Z');*/
      break;
    case 7:
      motorSelect = 'g';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driverDRV); i++){
        Serial1.write(driverDRV[i]);
      }
      Serial1.write('3');
      Serial1.write('2');
      Serial1.write('Z');*/
      break;
    case 8:
      motorSelect = 'h';
      /*Serial1.write(motorSelect);
      for (i = 0; i < strlen(driverDRV); i++){
        Serial1.write(driverDRV[i]);
      }
      Serial1.write('3');
      Serial1.write('2');
      Serial1.write('Z');*/
      break;
  }
}

// Esta función es más para las pruebas, que una vez se agregue el valor de los steps/mm o steps/grado
// el valor de los pasos va a variar fuera del rango de 1 - 100.
/*void resolucionMovimiento(){
  // Este código es para determinar cuántos ceros son necesarios en base a la resolución enviada por Corona.
  // Cuando el encoder no está siendo girado.
  if (datosMando.ticks == 0){
    cerosNecesarios = 10;
  }
  // Resolución de 1 equivale a 0.1 grados.
  // Con los cálculos hechos, para 0.1 grados, se requieren 400 pasos.
  // Esto todavía requiere la conversión a través de steps/mm o steps/grado.
  else if (datosMando.ticks == 1){
    cerosNecesarios = 9;
  }
  // Resolución de 10 equivale a 1.0 grados.
  // Esto todavía requiere la conversión a través de steps/mm o steps/grado.
  else if (datosMando.ticks == 10){
    cerosNecesarios = 8;
  }
  // Resolución de 100 equivale a 10.0 grados.
  // Esto todavía requiere la conversión a través de steps/mm o steps/grado.
  else if (datosMando.ticks == 100){
    cerosNecesarios = 7;
  }
}*/

// Esta función se utiliza si el control final utiliza el método de hacer que el motor gire cada tick.
void resMovimientoConvertido(){
  if (datosMando.motorChar == 'a'){
    if (datosMando.ticks == 0){
      itoa(0,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 5){
      itoa(pasos005GradosJ1,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 10){
      itoa(pasos01GradosJ1,datosMando.ticksArray,10);
      //Serial.print(datosMando.ticksArray);
    }
    else if (datosMando.ticks == 100){
      itoa(pasos1GradoJ1,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 200){
      itoa(pasos10GradosJ1,datosMando.ticksArray,10);
    }
  }
  else if (datosMando.motorChar == 'b' || datosMando.motorChar == 'c'){
    if (datosMando.ticks == 0){
      itoa(0,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 5){
      itoa(pasos005Grados,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 10){
      itoa(pasos01Grados,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 100){
      itoa(pasos1Grado,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 200){
      itoa(pasos10Grados,datosMando.ticksArray,10);
    }
  }
  /*else if (datosMando.motorChar == 'b' || datosMando.motorChar == 'c'){
    if (datosMando.ticks == 5 || datosMando.ticks == 10){
      //cerosNecesarios = 7;
      if (datosMando.ticks == 5){
        itoa(pasos005Grados,datosMando.ticksArray,10);
        //Serial.write(datosMando.ticksArray[0]);
      }
      else if (datosMando.ticks == 10){
        itoa(pasos01Grados,datosMando.ticksArray,10);
      }
    }
    else if (datosMando.ticks == 100){
      //cerosNecesarios = 6;
      itoa(pasos1Grado,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 200){
      //cerosNecesarios = 5;
      itoa(pasos10Grados,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 0){
      //cerosNecesarios = 10;
      itoa(0,datosMando.ticksArray,10);
    }
  }*/
  else if (datosMando.motorChar == 'd'){
    if (datosMando.ticks == 0){
      itoa(0,datosMando.ticksArray,10);
    }
  }
  else if (datosMando.motorChar == 'e'){
    if (datosMando.ticks == 0){
      itoa(0,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 5){
      itoa(pasos005GradosWorm,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 10){
      itoa(pasos01GradosWorm,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 100){
      itoa(pasos1GradoWorm,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 200){
      itoa(pasos10GradosWorm,datosMando.ticksArray,10);
    }
  }
  else if (datosMando.motorChar == 'f'){
    if (datosMando.ticks == 0){
      itoa(0,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 5){
      itoa(pasosPor005MM,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 10){
      itoa(pasosPor01MM,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 100){
      itoa(pasosPor1MM,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 200){
      itoa(pasosPor10MM,datosMando.ticksArray,10);
    }
  }
  else if (datosMando.motorChar == 'g' || datosMando.motorChar == 'h'){
    if (datosMando.ticks == 0){
      itoa(0,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 5){
      itoa(pasos005GradosNema11,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 10){
      itoa(pasos01GradosNema11,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 100){
      itoa(pasos1GradoNema11,datosMando.ticksArray,10);
    }
    else if (datosMando.ticks == 200){
      itoa(pasos10GradosNema11,datosMando.ticksArray,10);
    }
  }
}

// Esta función se utiliza si el control final utiliza el método de enviar un solo valor total de ticks.
void movConvertidoConTotal(){
  
}

void conversionDatosOCR(){
  // El motor 'e' (junta 5) es el motor con relación de 11 steps/grado, o 1 step por 0.1 grados.
  if (datosOCR.motor == 'e'){
    datosOCR.decenaInt = (datosOCR.decena - 48) * 10 * pasos1GradoWorm;
    //Serial.print(datosOCR.decenaInt);
    //Serial.print(',');
    datosOCR.unidadInt = (datosOCR.unidad - 48) * 1 * pasos1GradoWorm;
    //Serial.print(datosOCR.unidadInt);
    //Serial.print(',');
    datosOCR.decimalInt = (datosOCR.decimal - 48) * 1 * pasos01GradosWorm;
    //Serial.print(datosOCR.decimalInt);
    //Serial.print(',');
    //datosOCR.decenaInt = atoi(datosOCR.decena);
    itoa(datosOCR.decenaInt+datosOCR.unidadInt+datosOCR.decimalInt,datosOCR.pasosNecesarios,10);
    //Serial.print(datosOCR.pasosNecesarios);
  }
  // El motor 'f' (junta 6) es el motor con relación de 200 steps/mm.
  else if (datosOCR.motor == 'f'){
    datosOCR.decenaInt = (datosOCR.decena - 48) * 10 * pasosPor1MM;
    datosOCR.unidadInt = (datosOCR.unidad - 48) * 1 * pasosPor1MM;
    datosOCR.decimalInt = (datosOCR.decimal - 48) * 1 * pasosPor01MM;
    itoa(datosOCR.decenaInt+datosOCR.unidadInt+datosOCR.decimalInt,datosOCR.pasosNecesarios,10);
  }
  else if (datosOCR.motor == 'g' || datosOCR.motor == 'h'){
    datosOCR.decenaInt = (datosOCR.decena - 48) * pasos10GradosNema11;
    datosOCR.unidadInt = (datosOCR.unidad - 48) * pasos1GradoNema11;
    datosOCR.decimalInt = (datosOCR.decimal - 48) * pasos01GradosNema11;
    itoa(datosOCR.decenaInt+datosOCR.unidadInt+datosOCR.decimalInt,datosOCR.pasosNecesarios,10);
  }
}

void enviarComandosOCR(){
  Serial1.write(datosOCR.motor);
  Serial1.write('M');
  Serial1.write(datosOCR.comando);
  Serial1.write(datosOCR.signo);
  for (i = 0; i < strlen(datosOCR.pasosNecesarios); i++){
    Serial1.write(datosOCR.pasosNecesarios[i]);
  }
  Serial1.write('Z');
}

void microstepsIndividuales(byte primerValor, byte segundoValor){
  /*Serial1.write();
  Serial1.write();
  Serial1.write();
  Serial1.write();
  Serial1.write();
  Serial1.write();*/
}

void microsteppingPorActuador(char motorSeleccionado, char driver[5]){
  if (motorSeleccionado == 'a'){
    if (datosMando.ticks == 5){
      Serial.write(motorSeleccionado);
      for (i = 0; i < strlen(driver); i++){
        Serial.write(driver[i]);
      }
      Serial.write('0');
      Serial.write('2');
      Serial.write('Z');
    }
    else if (datosMando.ticks == 10){
      Serial.write(motorSeleccionado);
      for (i = 0; i < strlen(driver); i++){
        Serial.write(driver[i]);
      }
      Serial.write('0');
      Serial.write('2');
      Serial.write('Z');
    }
    else if (datosMando.ticks == 100){
      Serial.write(motorSeleccionado);
      for (i = 0; i < strlen(driver); i++){
        Serial.write(driver[i]);
      }
      Serial.write('0');
      Serial.write('2');
      Serial.write('Z');
    }
    else if (datosMando.ticks == 200){
      Serial.write(motorSeleccionado);
      for (i = 0; i < strlen(driver128); i++){
        Serial.write(driver128[i]);
      }
      Serial.write('0');
      Serial.write('2');
      Serial.write('Z');
    }
  }
}
