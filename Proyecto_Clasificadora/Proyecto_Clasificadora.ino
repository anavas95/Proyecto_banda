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
#include <Wire.h>               //Libreria que permite usar el protocolo I2C
#include <Servo.h>              //Libreria que permite usar los servomotores
#include <Adafruit_TCS34725.h>  //Libreria que permite usar el sensor de color TCS74325
#include <ColorConverterLib.h>  //Libreria que permite convertir RGB a HSL
#include <Keypad.h>             //Libreria que permite usar el teclado matricial
#include <LiquidCrystal_I2C.h>  //Libreria que permite usar la pantalla I2C

//Directivas de preprocesador
#define sensor_derecha 22    //Sensor de detección del lado derecho
#define sensor_izquierda 23  //Sensor de detección del lado izquierdo
#define sensor_sorter 24     //Sensor de detección del sorteador
#define motorBanda_IN1 25    //Control de giro del motor de la banda
#define motorBanda_IN2 26    //Control de giro del motor de la banda
#define motorRampa_IN3 27    //Control de giro del motor de la rampa
#define motorRampa_IN4 28    //Control de giro del motor de la rampa
#define servoDerecho 2       //Servo de la compuerta derecha
#define servoIzquierdo 3     //Servo de la compuerta izquierda
#define servoSorter 4        //Servo que detiene y mueve las pelotas
#define columns_keypad  4    //Cantidad de columnas del teclado matricial
#define rows_keypad     4    //Cantidad de filas del teclado matricial.
#define columns_lcd    20    //Cantidad de columnas de la LCD   
#define rows_lcd        4    //Cantidad de filas de la LCD
#define lcd_address  0x26    //Direccion i2c de la  LCD

//Constructores
Servo Servo_derecho;
Servo Servo_izquierdo;
Servo Servo_sorter;
Adafruit_TCS34725 sensor_color = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);
LiquidCrystal_I2C lcd_dispensadora(lcd_address, columns_lcd, rows_lcd);
//Prototipos
void BandaCW(void);            //Funcion utilizada para mover la banda en el sentido de las agujas del reloj
void BandaCCW(void);           //Funcion utilizada para mover la banda en sentido contrario a las agujas del reloj
void BandaStop(void);          //Funcion utilizada para detener la banda
void BandaRampaOn(void);       //Funcion utilizada para mover la rampa ascendente.
void RandaRampaOff(void);      //Funcion utilizada para detener la rampa ascendente.
void OpenSorter(void);         //Funcion utiizada para mover el servo del sorteador.
void config_gpios(void);       //Funcion utilizada para configurar los pines de entrada salida del proyecto.
void config_actuadores(void);  //Funcion utilizada para iniciar los servomotores y motores.
float read_Color(void);        //Funcion que devuelve el color utilizado
void setup_sensor(void);       //Funcion que setea la comunicación I2C para el sensor de color
void setup_lcd(void);          //FUncion que setea la comunicacion I2C para la pantalla LCD

//variables
float color_lectura;
String info_recibida;  //debug
int count_procesos = 0;
int count_procesos_aux;

void setup() {
  Serial.begin(9600);
  //setup_lcd();
  +setup_sensor();
  config_actuadores();
  Serial.println("Pruebas del proyecto - hijo Gerson Barahona");
  Serial.print("Numero de procesos validados: ");
  Serial.println(count_procesos);
  Serial.println("Pruebas del servomotor");
  BandaStop();
}

void loop(){

  //Si detecto que hay una pelota para clasificar
  if(digitalRead(sensor_sorter)==LOW)
  {
    delay(250);  //Espero 250 milisegundos
    Serial.println("He detectado una pelota");
    color_lectura = read_Color(); //Leo el color de la pelota
  }

  //Si esta en el rango de las naranja
  if(color_lectura > 3.99 && color_lectura <10.99)
  {
    Serial.println("Naranja");
    color_lectura = 0;
    OpenSorter();     //Abro la compuerta
    do
    {
      BandaRampaOn();
      BandaCW();
    }
    while(digitalRead(sensor_derecha)!=LOW);
    BandaStop();
  } 
  //Color roja
  if(color_lectura > 11.00 && color_lectura <40.99)
  {
    Serial.println("Roja");
    color_lectura = 0;
    OpenSorter();     //Abro la compuerta
    do
    {
      BandaRampaOn();
      BandaCCW();
    }
    while(digitalRead(sensor_derecha)!=LOW);
    BandaStop();
  }
  //Color verde
  if(color_lectura > 95.00 && color_lectura <115.00)
  {
    Serial.println("Verde");
    color_lectura = 0;
    OpenSorter();     //Abro la compuerta
    do
    {
      BandaRampaOn();
      BandaCW();
    }
    while(digitalRead(sensor_izquierda)!=LOW);
    BandaStop();
  }

}



//Funciones
void config_gpios(void) {
  pinMode(sensor_derecha, INPUT);    //Pin22 como entrada
  pinMode(sensor_izquierda, INPUT);  //Pin23 como entrada
  pinMode(sensor_sorter, INPUT);     //Pin24 como entrada
  pinMode(motorBanda_IN1, OUTPUT);   //Pin25 como salida
  pinMode(motorBanda_IN2, OUTPUT);   //Pin26 como salida
  pinMode(motorRampa_IN3, OUTPUT);   //Pin27 como salida
  pinMode(motorRampa_IN4, OUTPUT);   //Pin28 como salida
}

void config_actuadores(void) {
  Servo_derecho.attach(servoDerecho);
  Servo_izquierdo.attach(servoIzquierdo);
  Servo_sorter.attach(4);
}

void OpenSorter(void) {
  Servo_sorter.write(0);
  delay(250);
  Servo_sorter.write(90);
  delay(1000);
}

float read_Color(void) {
  unsigned int readRed, readBlue, readGreen, clear;
  delay(50);  //espera de 50 ms para integración de la lectura
  sensor_color.getRawData(&readRed, &readGreen, &readBlue, &clear);
  //Serial.println(readRed);
  //Serial.println(readGreen);
  //Serial.println(readBlue);
  uint32_t sum = clear;
  float r, g, b;
  r = readRed;
  r /= sum;
  g = readGreen;
  g /= sum;
  b = readBlue;
  b /= sum;


  r *= 256;
  g *= 256;
  b *= 256;

  double hue, saturation, value;
  ColorConverter::RgbToHsv(r, g, b, hue, saturation, value);

  float color_devuelta = hue * 360;
  return color_devuelta;
}

void setup_sensor(void) {
  if (sensor_color.begin()) {
    Serial.println("He encontrado el sensor");
  } else {
    Serial.println("Sensor no encontrado, revisa conexiones");
    while (1)
      ;
  }
}

void BandaRampaOn(void)
{
  digitalWrite(motorRampa_IN3, HIGH);
  digitalWrite(motorRampa_IN4, LOW);
}

void BandaCCW(void)
{
  digitalWrite(motorBanda_IN1, LOW);
  digitalWrite(motorBanda_IN2, HIGH);
}

void BandaCW(void)
{
  digitalWrite(motorBanda_IN1, HIGH);
  digitalWrite(motorBanda_IN2, LOW);
}

void BandaStop(void)
{
  digitalWrite(motorBanda_IN1, HIGH);
  digitalWrite(motorBanda_IN2, HIGH);
  digitalWrite(motorRampa_IN3, HIGH);
  digitalWrite(motorRampa_IN4, HIGH);
}

void setup_lcd(void)
{
  lcd_dispensadora.init();   //Inicio la comunicacion I2C con la pantalla LCD.
  lcd_dispensadora.backlight();   //enciendo la retroalimentación de la pantalla LCD.
  lcd_dispensadora.print("Huecos"); 
}