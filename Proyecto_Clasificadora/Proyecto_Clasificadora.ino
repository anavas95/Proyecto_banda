/*
Universidad Mariano Gálvez de Guatemala
Facultad de Ingeniería en Sistemas de Información
Ingeniería en Sistemas
Curso:
Catedratico: 
Alumno: 
Carnet: 
Version: 01
Revisiones:  Mayo, 19 - Pruebas de la banda clasificadora.

*/


//Librerias
#include <Wire.h>                 //Libreria que permite usar el protocolo I2C
#include <Servo.h>                //Libreria que permite usar los servomotores
#include <Adafruit_TCS34725.h>    //Libreria que permite usar el sensor de color TCS74325

//Directivas de preprocesador
#define sensor_derecha   22       //Sensor de descarga del lado derecho
#define sensor_izquierda 23       //Sensor de descarga del lado izquierdo
#define motorBanda_IN1   24       //Control de giro del motor de la banda
#define motorBanda_IN2   25       //Control de giro del motor de la banda
#define motorRampa_IN3   26       //Control de giro del motor de la rampa
#define motorRampa_IN4   27       //Control de giro del motor de la rampa
#define servoDerecho     2        //Servo de la compuerta derecha
#define servoIzquierdo   3        //Servo de la compuerta izquierda

//Constructores
Servo Servo_derecho;
Servo Servo_izquierdo;

//Prototipos
void BandaCW(void);          //Funcion utilizada para mover la banda en el sentido de las agujas del reloj
void BandaCCW(void);         //Funcion utilizada para mover la banda en sentido contrario a las agujas del reloj
void BandaStop(void);        //Funcion utilizada para detener la banda
void config_gpios(void);     //Funcion utilizada para configurar los pines de entrada salida del proyecto
void config_actuadores(void);   //Funcion utilizada para iniciar los servomotores y motores.

//variables
bool state_sensor_derecha;        //debug
bool state_sensor_izquierda;      //debug

String info_recibida;             //debug

int count_procesos =0;
int count_procesos_aux;

void setup() {
  Serial.begin(9600);
  config_actuadores();
  Serial.println("Pruebas del proyecto - hijo Gerson Barahona");
  Serial.print("Numero de procesos validados: ");
  Serial.println(count_procesos);

}

void loop() {
  Servo_derecho.write(0);   //CW
  delay(5000);
  Servo_derecho.write(90);  //Stop
  delay(5000);
  Servo_derecho.write(180);  //CCW
  delay(5000);


  while(Serial.available() > 0)
  {
    info_recibida = Serial.readStringUntil('\n');
    Serial.print("he recibido: ");
    Serial.println(info_recibida);
    count_procesos_aux = info_recibida.toInt();
  }

  if(count_procesos_aux > 0 && count_procesos_aux < 10)
  {

    if(digitalRead(sensor_derecha)==LOW || (digitalRead(sensor_izquierda)==LOW))
    {
      Serial.print("Numero de procesos validados: ");
      count_procesos++;
      Serial.println(count_procesos);
      delay(500);
    }
    
    if(count_procesos == count_procesos_aux)
    {
      Serial.println("termino los procesos");
      Serial.println("abro las compuertas");
      Servo_derecho.write(0);
      delay(5000);
      Servo_derecho.write(90);
      delay(5000);
      //Aqui se agregan las compuertas
      count_procesos = 0;
      count_procesos_aux = 0;
    }
  }
  else
  {
    Serial.println("Unicamente se pueden procesar un maximo de 9 elementos");
    delay(750);
  }
}



//Funciones
void config_gpios(void)
{
  pinMode(sensor_derecha, INPUT);         //Pin22 como entrada
  pinMode(sensor_izquierda, INPUT);       //Pin23 como entrada
  pinMode(motorBanda_IN1, OUTPUT);        //Pin24 como salida
  pinMode(motorBanda_IN2, OUTPUT);        //Pin25 como salida
  pinMode(motorRampa_IN3, OUTPUT);        //Pin26 como salida
  pinMode(motorRampa_IN4, OUTPUT);        //Pin27 como salida
}

void config_actuadores(void)
{
  Servo_derecho.attach(servoDerecho);
  Servo_izquierdo.attach(servoIzquierdo);
}