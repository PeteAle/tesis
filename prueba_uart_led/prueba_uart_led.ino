#define ledGreen GREEN_LED
#define ledRed RED_LED
#define finDatos 'Z'
bool datosNuevos = false;
bool recibiendo = false;
char inData;
byte primerMarcador, ultimoMarcador;
int indice = 0;
byte tempIndice = 0;
const byte longitud = 64;
byte recibido[longitud];
byte i, j;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(ledRed,OUTPUT);
  pinMode(ledGreen,OUTPUT);
  digitalWrite(ledRed,LOW);
  digitalWrite(ledGreen,LOW);
  Serial.flush();
  Serial1.flush();
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
