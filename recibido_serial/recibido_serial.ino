char inData = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available() > 0){
    inData = Serial.read();
    Serial.print(inData);
    //delay(200);
  }
}
