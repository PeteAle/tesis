// Se definene las librerías a utilizar

const byte longitud = 64;
char recibido [longitud];
char tempChars [longitud];

#define boton 
char inData;
int joystick;
int coord;
char indice = 0;
static boolean recibiendo = false;
// Switch para determinar si el control está en modo manual o modo automático
// Manual --> modo = True
// Auto --> modo = False
static boolean modo = true;
boolean datosNuevos = false;

int addr = 0;
char inicio = '<';
char fin = '>';
char separador = ';';

void setup() {
  // put your setup code here, to run once:
  pinMode()
  Serial.begin(115200);
  Serial.println("Tiva C conectada.");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  enviarDatos(kp1, ki1, kd1, kp2, ki2, kd2, kp3, ki3, kd3, kp4, ki4, kd4, kp5, ki5, kd5, kp6, ki6, kd6);
  if (Serial.available() > 0){
    if (modo = true){ // Si el modo de funcionamiento del sistema es manual
      recepcionMando(joystick);
    }
    else if (modo = false){ // Si el modo de funcionamiento del sistema es auto.
      recepcionVision(coord);
    }
  }
  enviarErrores(0.5);
  Serial.flush();
}

void enviarDatos(float kp1, float ki1, float kd1, float kp2, float ki2, float kd2, float kp3,
                 float ki3, float kd3, float kp4, float ki4, float kd4, float kp5, float ki5,
                 float kd5, float kp6, float ki6, float kd6) {
  // String de inicio del conjunto de datos
  Serial.write(inicio);
  // Constantes PID 1
  Serial.write(kp1);
  Serial.write(ki1);
  Serial.write(kd1);
  Serial.write(separador);
  // Constantes PID 2
  Serial.write(kp2);
  Serial.write(ki2);
  Serial.write(kd2);
  //Constantes PID 3
  Serial.write(kp3);
  Serial.write(ki3);
  Serial.write(kd3);
  // Constantes PID 4
  Serial.write(kp4);
  Serial.write(ki4);
  Serial.write(kd4);
  // Constantes PID 5
  Serial.write(kp5);
  Serial.write(ki5);
  Serial.write(kd5);
  // Constantes PID 6
  Serial.write(kp6);
  Serial.write(ki6);
  Serial.write(kd6);
  // String de finalización del conjunto de datos.
  Serial.write(fin);
}

void recepcionMando(int joystick){
    while(Serial.available() > 0 && datosNuevos == false){
    joystick = Serial.read();
    if (recibiendo == true){
      if ((joystick != fin) || (joystick != separador)){
        recibido[indice] = joystick;
        indice++;
        if (indice > longitud){
          indice = longitud - 1;
        }
      }
       else{
        recibido[indice] = '\0';
        indice = 0;
        recibiendo = false;
        datosNuevos = true;
      }
    }
    else if (joystick == inicio){
      recibiendo = true;
    }
  }
}

void recepcionVision(int coordenadas){
  
}

/*void recepcionVision(int coordX, int coordY, int coordZ){
  
}*/

enviarErrores(float error1){
  
}
