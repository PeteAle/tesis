#define inicioDatosMando '<'
#define finDatosMando '>'
#define finDatosOCR 'Z'
#define longitud 64

int encoderVal = 0;
byte i = 0;
byte j = 0;

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
  byte ticks;
  char ticksArray[longitud];
  char * signo;
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
  byte unidad;
  byte decimal;
  byte marcadorFinal;
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
// ******************************** Steps/grados juntas 2 y 3 *******************************
unsigned int pasos005Grados = 200;
unsigned int pasos01Grados = 400;
unsigned int pasos1Grado = 4000;
// ******************************************************************************************
// ************************** Steps/grados y steps/mm juntas 5 y 6 **************************
unsigned int pasosPorGrado = 11;
unsigned int pasosPorMM = 200;
// ******************************************************************************************

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly: 
  //recibirDatos();
  //recibirDatosGeneral(Serial, datosMando);
  recibirDatosMando();
  recibirDatosOCR();
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
        Serial.write(datosOCR.signo);
        datosOCR.decena = recibidoOCR[2];
        Serial.write(datosOCR.decena);
        datosOCR.unidad = recibidoOCR[3];
        Serial.write(datosOCR.unidad);
        datosOCR.decimal = recibidoOCR[4];
        Serial.write(datosOCR.decimal);
        //Serial.print("Aqui para");
        if (estado == 0){
          Serial.write("Aqui para");
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
          Serial1.write(datosMando.signo);
          for (i = 0; i < cerosNecesarios; i++){
            Serial1.write('0');
          }
          for (j = 0; j < 10 - cerosNecesarios; j++){
            Serial1.write(datosMando.ticksArray[j]);
          }
          //Serial.write(datosMando.ticksChar);
          Serial1.write('Z');
        }
        //delay(400);
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
      estado = 1;
      Serial.print("estado0");
    }
    else if (datosMando.modo == 1){
      estado = 1;
      Serial.print("estado1");
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
    datosMando.ticks = atoi(token);
    //Serial.print(datosMando.ticks);
    resolucionMovimiento();
    itoa(datosMando.ticks,datosMando.ticksArray,10);
    //datosMando.ticksChar = datosMando.ticks+48;
    // ******************************************************************************************************
    // ********************************* Extracción del signo de giro ***************************************
    token = strtok(NULL, ",");
    datosMando.signo = token;
    //datosMando.signoInt = atoi(token);
    //Serial.write(datosMando.signo);
    // ******************************************************************************************************
  }
}

void reasignarMotor(int numMotor){
  switch (numMotor){
    case 1:
      motorSelect = 'a';
      break;
    case 2:
      motorSelect = 'b';
      break;
    case 3:
      motorSelect = 'c';
      break;
    case 4:
      motorSelect = 'd';
      break;
    case 5:
      motorSelect = 'e';
      break;
    case 6:
      motorSelect = 'f';
      break;
    case 7:
      motorSelect = 'g';
      break;
    case 8:
      motorSelect = 'h';
      break;
  }
}

// Esta función es más para las pruebas, que una vez se agregue el valor de los steps/mm o steps/grado
// el valor de los pasos va a variar fuera del rango de 1 - 100.
void resolucionMovimiento(){
  // Este código es para determinar cuántos ceros son necesarios en base a la resolución enviada por Corona.
  // Cuando el encoder no está siendo girado.
  if (datosMando.ticks == 0){
    cerosNecesarios = 10;
  }
  // Resolución de 1 equivale a 0.1 grados.
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
}

void enviarComandosOCR(){
  Serial1.write(datosOCR.motor);
  Serial1.write('M');
  for (i = 0; i < 5; i++){
    Serial1.write(recibidoOCR[i]);
  }
  Serial1.write('Z');
}
