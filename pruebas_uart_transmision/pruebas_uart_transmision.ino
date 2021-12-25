// Prueba para la communicación UART de transmisión.

char inData;
char inicio = 'E';
char fin = 'E';
const byte longitud = 128;
char recibido [longitud];
char indice = 0;
static bool datosNuevos = false;
bool recibiendo = false;
char primerMarcador;
char ultimoMarcador;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("Arduino conectado.");
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available() > 0 && datosNuevos == false){
    inData = Serial.read();
    if (recibiendo == true){
      if ((inData != fin)){
        recibido[indice] = inData;
        indice++;
        if (indice > longitud){
          indice = longitud - 1;
        }
      }
      else if (inData == fin){
        ultimoMarcador = inData;
        recibido[indice] = '\0';
        indice = 0;
        Serial.write(primerMarcador);
        Serial.write(recibido);
        Serial.write(ultimoMarcador);
        recibiendo = false;
        datosNuevos = true;
      }
    }
    else if (inData == inicio){
      recibiendo = true;
      primerMarcador = inData;
    }
    Serial.flush();
  }
  datosNuevos = false;
}
