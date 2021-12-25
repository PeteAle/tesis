// Prueba secundaria para el control de los stepper (con prioridad al UART)

// Pin de dirección del stepper, 
// dirPin = 1 --> ccw
// dirPin = 0 --> clockwise
#define dirPin 2
#define stepPin 3
#define enableBit 4

float myArray[] = { 01.2 , 22.3 , 33.3 };

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Arduino conectado.");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
