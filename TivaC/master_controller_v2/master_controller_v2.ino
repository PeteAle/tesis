// *************************** Recepción de datos de Happy ************************

#define comandoStep 'S'
#define ledGreen GREEN_LED
#define ledRed RED_LED

// ------------------------------ Recepción de datos ------------------------------
static bool datosNuevos = false;
char inData = 0;
uint8_t indice = 0;
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
byte j = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   // Comunicación con Happy
  Serial1.begin(115200);  // Comunicación con los arduinos
  pinMode(ledGreen,OUTPUT);
  pinMode(ledRed, OUTPUT);
  digitalWrite(ledGreen,LOW);
  digitalWrite(ledRed,LOW);
  //Serial2.begin(115200);  // Comunicación con Corona
}

void loop() {
  // put your main code here, to run repeatedly: 
  while(Serial.available() > 0 && datosNuevos == false){
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
        enviarComandos();
        //------------------------------------------------------------------------------
        // Estas líneas SIEMPRE dejarlas de último, ya que son lo que resetea el buffer.
        recibido[indice] = '\0';
        indice = 0;
        recibiendo = false;
        datosNuevos = true;
        //------------------------------------------------------------------------------
      }
    }
    else if (inData >= 'a' && inData <= 'z'){
        primerMarcador = inData;
        recibiendo = true;
    }
  }
  Serial1.flush();
  datosNuevos = false;
}

void enviarComandos(){
  Serial1.write(primerMarcador);
  for (i = 0; i < indice; i++){
    Serial1.write(recibido[i]);
  }
  Serial1.write(ultimoMarcador);
}


void distDatosOCR(){
  if (primerMarcador == 'e'){
    digitalWrite(ledGreen,LOW);
    //digitalWrite(ledRed,HIGH);
    if (indice > 1 && indice < 6){
      if (recibido[0] == comandoStep){
        if (recibido[1] == '+' || recibido [1] == '-'){
          for (i = 0; i < indice; i++){
            datosOCR[i] = recibido[0];
          }
          Serial1.write(primerMarcador);
          for (i = 0; i < indice; i++){
            Serial1.write(datosOCR[i]);
          }
          Serial1.write(ultimoMarcador);
        }
      }
    }
  }
}

void pruebaMandar(){
  
}
