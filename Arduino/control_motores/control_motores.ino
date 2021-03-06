// Prueba inicial de código para movimiento de steppers sin biblioteca.
// Peter Yau 17914

#include <EEPROM.h>

// Pin de dirección del stepper, 
// dirPin = 1 --> ccw
// dirPin = 0 --> clockwise
#define dirPin 2
#define stepPin 3
#define enableBit 4

const byte longitud = 128;
byte recibido [longitud];
char tempChars [longitud];
//char motoresDeseados[longitud];
//byte enableInfo;

char inData;
//char motorSelect;
int tempIndex = 0;
char indice = 0;
static boolean recibiendo = false;
char comandoEnable = 'E';
char comandoMotores = 'S';
char inicioDatos = 'V';
char finDatos = '\n';
char separador = ';';
boolean datosNuevos = false;
char datosRecibidos;
boolean primerNumeralGuardado = false;

byte i = 0;
byte j = 0;
char primerDato;       // El primer dato que se recibe desde el momento que se comienze a recibir datos (los comandos a utilizar para los motores).
float siguientesDatos;  // Todos los datos que le siguen al primerDato.
float pasosMotor1;      // pasosMotor1 va a ser la variable que va a recibir el dato del Serial.
float oldData;          // Dato original donde se va a trasladar el dato pasosMotor1.
int contadorPasos = 0;  // Contador que se va a encargar de hacer la cuenta
int numMotores;
bool motorSeleccionado = false;
//char comando;
//int motoresParaActivar = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Comunicación iniciada.");
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enableBit, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  recibirComandos();                  // Función para recibir los valores enviados por Happy y guardarlos en una variable.
  if (datosNuevos == true){
    enableMotors;
    if (primerDato == inicioDatos){
      separarDatos();
    }
    //separarDatos();                   // Separar los datos obtenidos desde la función recibirComandos().
    if (primerNumeralGuardado == false || pasosMotor1 == 0 || contadorPasos == 0){ // Condicional para verificar si la bandera está levantada de haber guardado el primer dato.
      oldData = pasosMotor1;          // Pasar el dato recibido para el primer motor al oldData.
      contadorPasos = oldData;        // Contador utilizado para ir reduciendose cada vez que se de un paso.
      primerNumeralGuardado = true;      // Levantar la bandera que ya se guardó el primer dato en la 
    }
    else if (oldData == 0 && pasosMotor1 != oldData || contadorPasos != oldData){
      oldData = pasosMotor1;
      contadorPasos = oldData;
    }
    /*Serial.print("pasosMotor1: "); Serial.println(pasosMotor1);
    Serial.print("oldData: "); Serial.println(oldData);
    Serial.print("contadorPasos: "); Serial.println(contadorPasos);
    Serial.print("Comando: "); Serial.println(comando);*/
    //datosNuevos = false;
  }
  if (pasosMotor1 > 0){
    digitalWrite(dirPin, HIGH);
    pasoStepper();
    contadorPasos = pasosMotor1 - 1;
    pasosMotor1 = pasosMotor1 - 1;
    condicionStop(pasosMotor1);
    condicionStop(contadorPasos);
  }
  else if (pasosMotor1 < 0){
    digitalWrite(dirPin, LOW);
    pasoStepper();
    pasosMotor1 = pasosMotor1 + 1;
    contadorPasos = pasosMotor1 + 1;
    condicionStop(pasosMotor1);
    condicionStop(contadorPasos);
  }
  //Serial.write((int)pasosMotor1);
  Serial.print("oldData: "); Serial.println(oldData);
  Serial.print("contadorPasos: "); Serial.println(contadorPasos);
  Serial.print("pasosMotor1: "); Serial.println(pasosMotor1);
  Serial.print("tempIndex: "); Serial.println(tempIndex);
  Serial.print("Num de motores: "); Serial.println(primerDato);
  Serial.flush();
  datosNuevos = false;
  }
  /*Serial.flush();
  delay(10);
}*/

void recibirComandos(){
  while(Serial.available() > 0 && datosNuevos == false){
    inData = Serial.read();
    if (recibiendo == true){
      if ((inData != finDatos)){
        recibido[indice] = inData;
        indice++;
        if (indice > longitud){
          indice = longitud - 1;
        }
      }
      else{
        tempIndex = indice;
        recibido[indice] = '\0';
        indice = 0;
        recibiendo = false;
        datosNuevos = true;
      }
    }
    else if (inData == inicioDatos || inData == comandoEnable){
      recibiendo = true;
      primerDato = inData;
    }
  }
}

void enableMotors(){
  if (primerDato == comandoEnable){
    char * token;
    token = strtok(recibido, ",");
    numMotores = atoi(token);
    if (numMotores > 1 && numMotores <= 6){
      digitalWrite(enableBit,HIGH);
      motorSeleccionado = true;
      if (motorSeleccionado = true){
        numMotores = numMotores - 1;
        if (numMotores < 1){
          Serial.write("Motores listos.");
          motorSeleccionado = false;
        }
        else{
          Serial.write("E");
          Serial.write(numMotores);
          motorSeleccionado = false;
        }
      }
    }
  }
}

void separarDatos(){
  if (primerDato == inicioDatos){
    char * token;
  
  token = strtok(recibido, separador);
  pasosMotor1 = atof(token);

  token = strtok(NULL, separador);
  siguientesDatos = atof(token);
  
  /*token = strtok(NULL, ",");
  tercerServo = atoi(token);

  token = strtok(NULL, ",");
  cuartoServo = atoi(token);

  token = strtok(NULL, ",");
  pasos = atoi(token);

  token = strtok(NULL, ",");
  negativo = atoi(token);*/ 
  }
}

void pasoStepper(){
  digitalWrite(stepPin, HIGH);
  delay(5);
  digitalWrite(stepPin, LOW);
  delay(5);
}

void condicionStop(float varMotorDeseado){
  if (varMotorDeseado == 0){
    digitalWrite(stepPin, LOW);
  }
}
