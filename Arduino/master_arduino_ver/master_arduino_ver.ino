// *************************** Recepción de datos de Happy ************************

#define comandoStep 'S'

// ------------------------------ Recepción de datos ------------------------------
static bool datosNuevos = false;
char inData = 0;
uint8_t indice = 0;
uint8_t tempIndice = 0;
bool recibiendo = false;
const byte longitud = 5;
char recibido [longitud];
char finDatos = '\n';
char datosOCR [longitud];

// ----------------------------------- Marcadores ---------------------------------
char primerMarcador;
char ultimoMarcador;

//*********************************************************************************

// ************************** Recepción de datos de Corona ************************
// ------------------------------ Recepción de datos ------------------------------
static bool datosNuevos2 = false;
char inData2 = 0;
uint8_t indice2 = 0;
bool recibiendo2 = false;
char recibido2 [longitud];

// ----------------------------------- Marcadores ---------------------------------
char primerMarcador2;
char ultimoMarcador2;
//*********************************************************************************

byte i = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   // Comunicación con Happy
  //Serial2.begin(115200);  // Comunicación con los arduinos
  //pinMode(ledGreen,OUTPUT);
  //pinMode(ledRed, OUTPUT);
  //digitalWrite(ledGreen,LOW);
  //digitalWrite(ledRed,LOW);
  //Serial3.begin(115200);  // Comunicación con Corona
}

void loop() {
  // put your main code here, to run repeatedly: 
  recibirDatos();
}

void recibirDatos(){
  while (Serial.available() > 0 && datosNuevos == false){
    //digitalWrite(ledRed, HIGH);
    //delay(300);
    inData = Serial.read();
    if (recibiendo == true){
      if (inData != finDatos){
        recibido[indice] = inData;
        indice++;
        if (indice > longitud){
          indice = longitud - 1;
        }
      }
      else if (inData == finDatos){
        ultimoMarcador = inData;
        tempIndice = indice;
        recibido[indice] = '\0';
        indice = 0;
        recibiendo = false;
        datosNuevos = true;
        distDatosOCR();
      }
    }
    else if (inData >= 'a' && inData <= 'z'){
      primerMarcador = inData;
      recibiendo = true;
    }
  }
  /*Serial.write(primerMarcador);
  for (i = 0; i < 5; i++){
    Serial.write(recibido[i]);
  }
  Serial.write('\n');*/
  Serial.flush();
  datosNuevos = false;
}

void distDatosOCR(){
  if (primerMarcador == 'e'){
    //digitalWrite(ledRed,LOW);
    //digitalWrite(ledRed,HIGH);
    if (indice > 1 && indice < 6){
      if (recibido[0] == comandoStep){
        if (recibido[1] == '+' || recibido [1] == '-'){
          for (i = 0; i < indice; i++){
            datosOCR[i] = recibido[0];
          }
          Serial.write(primerMarcador);
          for (i = 0; i < indice; i++){
            Serial.write(datosOCR[i]);
          }
          Serial.write(ultimoMarcador);
        }
      }
    }
  }
}

void pruebaMandar(){
  
}
