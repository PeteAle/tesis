#ifndef controlStepper_h
#define controlStepper_h

#include "Arduino.h"

class controlStepper
{
  public:
    controlStepper(int dirPin, int stepPin, int ms1, int ms2, int ms3);
    void asignarAddress();
    void pasosStepper();
    void convertirPasos();
    void condicionStop();
    void setMicroStepping();
  private:
    int _dirPin;
    int _stepPin;
    int _ms1;
    int _ms2;
    int _ms3;
};

#endif
