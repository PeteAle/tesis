#include "Arduino.h"
#include "controlStepper.h"
#include <EEPROM.h>

controlStepper::controlStepper(int dirPin, int stepPin, int ms1, int ms2, int ms3)
{
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  _dirPin = dirPin;
  _stepPin = stepPin;
  _ms1 = ms1;
  _ms2 = ms2;
  _ms3 = ms3;
}
/*
void controlStepper::asignarAddress()
{
  if (tempIndice == 1){
    addressMotor = recibido[0];
    EEPROM.write(0,addressMotor);
    siguienteAddress = addressMotor + 1;
    Serial.write(primerMarcador);
    Serial.write(siguienteAddress);
    Serial.write(ultimoMarcador);
  }
}
*/
