#define ledGreen GREEN_LED
#define ledRed RED_LED

// ------------------ Datos de recepción del mando de control ----------------

int encoderVal = 0;
char ticks = 0;
int valAnterior = 0;
int valActual = 0;
unsigned int numMotor = 0;
char motorSelect = 0;
char modo = 0;

//------------------- Datos de recepción para el sistema OCR -----------------
#define finDatosOCR 'Z'
#define finDatosMando '>'
#define inicioDatos '<'
bool datosNuevos = false;
bool recibiendo = false;
char inDataMando;
char inDataOCR;
byte primerMarcador, primerMarcador2, ultimoMarcador, ultimoMarcador2;
unsigned int indiceMando = 0;
unsigned int indiceOCR = 0;
unsigned int tempIndiceMando = 0;
unsigned int tempIndiceOCR = 0;
const byte longitud = 64;
char recibidoMando[longitud];
char recibidoOCR[longitud];
char tempRecibidoMando[longitud];
char tempRecibidoOCR[longitud];
//----------------------------------------------------------------------------
//-------------------------- Cambio de modo de operación ---------------------
byte estado = 0; // Por default, es estado de operación es el del mando
                 // estado = 1 --> Funciona el modo automático
                 // estado = 0 --> Funciona el modo manual
//------------------------ Variables para iteraciones ------------------------
byte i, j, k;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   // Información del sistema OCR
  Serial1.begin(115200);  // Información del mando de control manual
  Serial2.begin(115200);  // Envío de datos a los microcontroladores
  Serial.flush();
  Serial1.flush();
  Serial2.flush();
}

void loop() {
  // put your main code here, to run repeatedly: 
  while((Serial.available() || Serial2.available()) > 0 && datosNuevos == false){
    inDataMando = Serial1.read();
    inDataOCR = Serial.read();
    if (recibiendo == true){
      if (inDataMando != finDatosMando || inDataOCR != finDatosOCR){
        if (primerMarcador == inicioDatos){
          recibidoMando[indiceMando] = inDataMando;
          indiceMando++;
          if (indiceMando > longitud){
            indiceMando = longitud - 1;
          }
        }
        if (primerMarcador2 >= 'a' && primerMarcador2 <= 'z' ){
          recibidoOCR[indiceOCR] = inDataOCR;
          indiceOCR++;
          if (indiceOCR > longitud){
            indiceOCR = longitud - 1;
          }
        }
      }
      else if (inDataMando == finDatosMando || inDataOCR == finDatosOCR){
        if (inDataMando == finDatosMando){
          ultimoMarcador = inDataMando;
          separarDatos();
          if (modo == 0){
            estado = 0;
            enviarComandosMando();
          }
          else if (modo == 1){
            estado = 1;
          }
          tempIndiceMando = indiceMando;
          recibidoMando[indiceMando] = '\0';
          recibiendo = false;
          datosNuevos = true;
        }
        if (inDataOCR == finDatosOCR){
          ultimoMarcador2 = inDataOCR;
          if (estado == 1){
            enviarComandosOCR();
          }
          tempIndiceOCR = indiceOCR;
          recibidoOCR[indiceOCR] = '\0';
          recibiendo = false;
          datosNuevos = true;
        }
        //------------------------------------------------------------------------------
        // Estas líneas SIEMPRE dejarlas de último, ya que son lo que resetea el buffer.
        /*recibido[indice] = '\0';
        indice = 0;
        recibiendo = false;
        datosNuevos = true;*/
        //------------------------------------------------------------------------------
      }
    }
    else{
      if (inDataMando == inicioDatos){
        primerMarcador = inDataMando;
      }
      if (inDataOCR >= 'a' && inDataOCR <= 'z'){
        primerMarcador2 = inDataOCR;
      }        
      recibiendo = true;
    }
  }
  Serial.flush();
  datosNuevos = false;
}

void enviarComandosOCR(){
  if (primerMarcador2 >= 'a' && primerMarcador2 <= 'z'){
    Serial.write(primerMarcador2);
  }
  for (i = 0; i < 2; i++){
    Serial.write(recibidoOCR[i]);
  }
  Serial.write('0');
  Serial.write('0');
  // Para hacer las pruebas de dígitos, si se quiere 5 dígitos, descomentar las líneas anteriores.
  // Para hacer las pruebas con 3 dígitos, comentar las líneas anteriores.
  for (j = 2; j < indiceOCR; j++){
    Serial.write(recibidoOCR[j]);
  }
  Serial.write(ultimoMarcador2);
}

void enviarComandosMando(){
  Serial1.write(numMotor);
  switch (numMotor){
    case 1:
      motorSelect = 'a'; break;
    case 2:
      motorSelect = 'b'; break;
    case 3:
      motorSelect = 'c'; break;
    case 4:
      motorSelect = 'd'; break;
    case 5:
      motorSelect = 'e'; break;
    case 6:
      motorSelect = 'f'; break;
    case 7:
      motorSelect = 'g'; break;
    case 8:
      motorSelect = 'h'; break;
  }
  Serial1.write(primerMarcador);
  Serial1.write(modo);
  Serial1.write(',');
  Serial1.write(motorSelect);
  Serial1.write(',');
  Serial1.write(ticks);
  Serial1.write(ultimoMarcador);
}

void separarDatos(){
  if (primerMarcador == inicioDatos){
    //---------- Separación de datos ----------
    char * token;
    for (int k = 0; k < tempIndiceMando; k++){
      tempRecibidoMando[k] = recibidoMando[k];
    }
    token = strtok(recibidoMando, ",");
    modo = atoi(token);
    if (modo == 0){
      estado = 0;
    }
    else if (modo == 1){
      estado = 1;
    }
    token = strtok(NULL, ",");
    numMotor = atoi(token);
    //strcpy(numMotor, token);

    token = strtok(NULL, ",");
    ticks = atoi(token);
  }
}

void enviarOK(){
  Serial.write('O');
  Serial.write('K');
}
