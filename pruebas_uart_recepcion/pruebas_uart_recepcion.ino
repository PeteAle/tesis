// Prueba para la communicación UART de recepción.

// Pin de dirección del stepper, 
// dirPin = 1 --> ccw
// dirPin = 0 --> clockwise
#define dirPin 2
#define stepPin 3
#define enableBit 4

const byte longitud = 128;
char indice = 0;
char recibir [longitud];
char inData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("Arduino conectado.");
}

void loop() {
  // put your main code here, to run repeatedly:
  // Recepción de datos
  while (Serial.available() > 0){
    inData = Serial.read();
    delay(100);
    Serial.write(inData);
    delay(1000);
    /*inData = Serial.read();
    recibir[indice] = inData;
    indice++;
    if (indice > longitud){
      indice = indice - 1;
    }
    else{
      recibir[indice] = '\0';
      indice = 0;
    }*/
  }
}
