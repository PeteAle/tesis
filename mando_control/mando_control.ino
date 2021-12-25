#define ledGreen GREEN_LED
#define ledRed RED_LED

// ------------------ Datos de recepción del mando de control ----------------

int encoderVal = 0;
int ticks = 0;
char ticksChar = 0;
int valAnterior = 0;
int valActual = 0;
int numMotor = 0;
char motorSelect = 0;
int modo = 0;
char modoChar = 0;

//------------------- Datos de recepción para el sistema OCR -----------------
#define finDatosOCR 'Z'
#define finDatosMando '>'
#define inicioDatos '<'
bool datosNuevos = false;
bool recibiendo = false;
char inDataMando;
char inDataOCR;
byte primerMarcador, primerMarcador2, ultimoMarcador, ultimoMarcador2;
int indiceMando = 0;
int indiceOCR = 0;
int tempIndiceMando = 0;
int tempIndiceOCR = 0;
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
  Serial.begin(115200);
  Serial1.begin(115200);  // Información del mando de control manual
  Serial2.begin(115200);  // Información del sistema OCR
  pinMode(ledRed,OUTPUT);
  pinMode(ledGreen,OUTPUT);
  digitalWrite(ledRed,LOW);
  digitalWrite(ledGreen,LOW);
  Serial.flush();
  Serial1.flush();
  Serial2.flush();
  recibiendo = false;
}

void loop() {
  // put your main code here, to run repeatedly: 
  while(Serial.available() > 0 && datosNuevos == false){
    inDataMando = Serial.read();
    //inDataOCR = Serial2.read();
    if (recibiendo == true){
      if (inDataMando != finDatosMando /*&& inDataMando != inicioDatos*/){
        if (primerMarcador == inicioDatos){
          recibidoMando[indiceMando] = inDataMando;
          indiceMando++;
          if (indiceMando > longitud){
            indiceMando = longitud - 1;
          }
        }
      }
      else if (inDataMando == finDatosMando){
        //Serial.print("STOP");
        if (inDataMando == finDatosMando){
          tempIndiceMando = indiceMando;
          //Serial.print(recibidoMando);
          ultimoMarcador = inDataMando;
          separarDatos();
          //Serial.print("Aqui para");
          if (modo == 0){
            //Serial.write(modo);
            estado = 0;
            enviarComandosMando();
          }
          else if (modo == 1){
            estado = 1;
            Serial.print("estado = 1");
          }
          recibidoMando[indiceMando] = '\0';
          indiceMando = 0;
          recibiendo = false;
          datosNuevos = true;
        }
        //------------------------------------------------------------------------------
        // Estas líneas SIEMPRE dejarlas de último, ya que son lo que resetea el buffer.
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
  Serial1.flush();
  datosNuevos = false;
}

void enviarComandosMando(){
  //Serial.write(numMotor);
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
  //Serial.write(primerMarcador);
  Serial.write(motorSelect);
  Serial.write('M');
  Serial.write('S');
  for(i = 0; i < 10-strlen(ticksChar); i++){
    Serial.write('0');
  }
  Serial.write(ticksChar);
  Serial.write('Z');
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
    modoChar = modo+48;
    
    token = strtok(NULL, ",");
    numMotor = atoi(token);
    //numMotorByte = (byte)numMotor;
    //strcpy(numMotor, token);

    token = strtok(NULL, ",");
    ticks = atoi(token);
    ticksChar = ticks+48;
  }
}
