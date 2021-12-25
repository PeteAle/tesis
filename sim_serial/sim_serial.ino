
#define pot 3

unsigned int potVal = 0;
unsigned int prueba = 12345;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pot,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  potVal = analogRead(pot);
  //potVal = map(potVal, 0, 1023, 0, 65534);
  Serial.write('a');
  Serial.write('S');
  Serial.write('+');
  Serial.print(prueba);
  Serial.write('Z');
  delay(1000);
}
