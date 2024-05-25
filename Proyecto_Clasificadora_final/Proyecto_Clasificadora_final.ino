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
#define f1_keypad      46    //Fila 1 del teclado matricial
#define f2_keypad      48    //Fila 2 del teclado matricial 
#define f3_keypad      50    //Fila 3 del teclado matricial
#define f4_keypad      52    //Fila 4 del teclado matricial
#define c1_keypad      47    //Columna 1 del teclado matricial
#define c2_keypad      49    //Columna 2 del teclado matricial
#define c3_keypad      51    //Columna 3 del teclado matricial
#define c4_keypad      54    //Columna 4 del teclado matricial

//Mapeo de las teclas del teclado matricial
char keys[rows_keypad][columns_keypad] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

//variables utilizadas
float color_lectura;    //variable donde guardo la lectura del color
String numero_ingresado;  //variable donde guardo el valor ingresado por medio del teclado
int count_procesos = 0;     //variable que lleva la cantidad de procesos realizados
int count_procesos_aux;     //variable donde guardo el valor convertido a entero para hacer el ciclo repetitivo
int count_naranjas = 0;     //contador de pelotas naranjas procesadas exitosamente
int count_rojas = 0;        //contador de pelotas rojas procesadas exitosamente
int count_verdes = 0;       //contador de pelotas verdes procesadas exitosamente
byte pins_rows[rows_keypad] = {f1_keypad,f2_keypad,f3_keypad,f4_keypad};        //array utilizada para los pines de las filas
byte pins_columns[columns_keypad] = {c1_keypad, c2_keypad,c3_keypad,c4_keypad}; //array utilizada para los pines de las columnas

//Constructores 
Servo Servo_derecho;            //servo compuerta derecha
Servo Servo_izquierdo;          //servo compuerta izquierda
Servo Servo_sorter;             //servo del sorteador
Adafruit_TCS34725 sensor_color = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);   //sensor de color 
LiquidCrystal_I2C lcd_dispensadora(lcd_address, columns_lcd, rows_lcd);                                 //pantalla LCD I2C 20x4
Keypad teclado_dispensadora = Keypad(makeKeymap(keys), pins_rows, pins_columns, rows_keypad, columns_keypad);

//Prototipos de funciones
void BandaCW(void);            //Funcion utilizada para mover la banda en el sentido de las agujas del reloj
void BandaCCW(void);           //Funcion utilizada para mover la banda en sentido contrario a las agujas del reloj
void BandaStop(void);          //Funcion utilizada para detener la banda
void BandaRampaOn(void);       //Funcion utilizada para mover la rampa ascendente.
void RandaRampaOff(void);      //Funcion utilizada para detener la rampa ascendente.
void OpenSorter(void);         //Funcion utiizada para mover el servo del sorteador.
void OpenGates(void);          //Funcion utilizada para abrir las compuertas de los receptores laterales.
void config_gpios(void);       //Funcion utilizada para configurar los pines de entrada salida del proyecto.
void config_actuadores(void);  //Funcion utilizada para iniciar los servomotores y motores.
float read_Color(void);        //Funcion que devuelve el color utilizado
void setup_sensor(void);       //Funcion que setea la comunicación I2C para el sensor de color
void setup_lcd(void);          //Funcion que setea la comunicacion I2C para la pantalla LCD
void lcd_message_init(void);   //Funcion que imprime los mensajes iniciales
void cleanColor_lcd(void);     //Funcion utilizada para limpiar los espacios 
void lcd_print_retro(void);    //Funcion utilizada para mostrar retroalimentación
void lcd_print_end_process(void); //Funcion utilizada para mostrar clasificacion terminada
void end_process(void);         //Funcion de finalización de procesos


//Configuraciones iniciales
void setup() {
  Serial.begin(9600);       //debug
  setup_lcd();              //inicializo la pantalla LCD I2C
  setup_sensor();           //inicializo el sensor de color TCS34725
  config_actuadores();      //configuracion de los pines gpio de las bandas
  Serial.println("Pruebas del proyecto - Clasificadora");    //debug
  Serial.print("Numero de procesos validados: ");            //debug
  Serial.println(count_procesos);                            //debug
  Servo_sorter.write(90);        //me aseguro que el sortear se encuentre cerrado
  Servo_izquierdo.write(90);     //me aseguro que la compuerta izquierda este cerrada
  Servo_derecho.write(90);       //me aseguro que la compuerta derecha esta cerrada
  BandaStop();                   //bandas inicializan detenidas
  lcd_message_init();            //muestro los valores de pesos RGB, color, contadores, etc
  cleanColor_lcd();              //limpio los pesos de los valores RGB leidos por el sensor de color TCS34725
}


//ciclo infinto
void loop(){

  //guardo la tecla presionada
  char tecla = teclado_dispensadora.getKey();

  //Si existe alguna tecla presionada
  if(tecla)
  {
    //Si la tecla presionada es # (enter)
    if(tecla == '#')
    {
      int numero = numero_ingresado.toInt();    //convierto el string donde guarde las teclas presionadas a un entero
      Serial.print("numero ingresado: ");       //debug
      Serial.println(numero);                   //debug
      if(numero >= 0 && numero <=100)           //Si el numero esta entre 0 y 99
      {
          //Mientras el numero ingresado no sea igual al numero de procesos
          while(numero != count_procesos)
          {
            //Leo si hay una pelota para ser leida
            if(digitalRead(sensor_sorter)==LOW)
            {
              delay(250);  //Espero 250 milisegundos
              Serial.println("He detectado una pelota");      //debug
              color_lectura = read_Color(); //Leo el color de la pelota
            }

            //Si esta en el rango de las naranja
            if(color_lectura > 3.99 && color_lectura <10.99)
            {
              Serial.println("Naranja");      //debug
              color_lectura = 0;              //reinicio el valor de la lectura
              OpenSorter();     //Abro la compuerta
              count_naranjas++;   //aumento en uno el contador de pelotas naranjas
              lcd_dispensadora.setCursor(18,2);       //posicion 16 de la tercera fila de la LCD
              lcd_dispensadora.print(count_naranjas);  //imprimo el valor del contador
              lcd_dispensadora.setCursor(12,3);       //posicion 11 de la cuarta fila de la LCD
              lcd_dispensadora.print("naranja ");     //imprimo el color de la pelota detectada
              do
              {
                BandaRampaOn();     //enciendo la rampa en diagonal
                BandaCW();          //giro sentido horario la rampa horizontal
              }
              while(digitalRead(sensor_derecha)!=LOW);      //mientras no detecte el sensor
              count_procesos++;                             //aumento el contador de proceso exitosos realizados
              lcd_dispensadora.setCursor(2,3);              //posicion 2 de la cuarta fila de la LCD
              lcd_dispensadora.print(count_procesos);       //imprimo la cantidad de procesos exitosos
              Serial.print("He procesado: ");               //debug
              Serial.print(count_procesos);                 //debug
              Serial.println(" pelotas");                   //debug
              BandaStop();                                  //paro las bandas
              cleanColor_lcd();                             //limpio los pesos RGB mostrados por la LCD
            } 
            //Color roja
            //NO he comentado las lineas que siguen por el hecho de que realizan 
            //la misma acción que el ciclo anterior
            if(color_lectura > 11.00 && color_lectura <40.99)
            {
              Serial.println("Roja");                       
              color_lectura = 0;                            
              OpenSorter();     //Abro la compuerta
              count_rojas++;    //aumento en uno el contador de pelotas rojas
              lcd_dispensadora.setCursor(18,0);
              lcd_dispensadora.print(count_rojas);
              lcd_dispensadora.setCursor(12,3);
              lcd_dispensadora.print("roja    ");
              do
              {
                BandaRampaOn();
                BandaCCW();
              }
              while(digitalRead(sensor_izquierda)!=LOW);
              count_procesos++;
              lcd_dispensadora.setCursor(2,3);
              lcd_dispensadora.print(count_procesos);
              Serial.print("He procesado: ");
              Serial.print(count_procesos);
              Serial.println(" pelotas");
              BandaStop();
              cleanColor_lcd();
            }
            //Color verde
            if(color_lectura > 95.00 && color_lectura <110.00)
            {
              Serial.println("Verde");
              color_lectura = 0;
              OpenSorter();     //Abro la compuerta
              count_verdes++;   //aumento en uno el contador para pelotas verdes
              lcd_dispensadora.setCursor(18,1);
              lcd_dispensadora.print(count_verdes);
              lcd_dispensadora.setCursor(12,3);
              lcd_dispensadora.print("verde   ");
              do
              {
                BandaRampaOn();
                BandaCCW();
              }
              while(digitalRead(sensor_izquierda)!=LOW);
              count_procesos++;
              lcd_dispensadora.setCursor(2,3);
              lcd_dispensadora.print(count_procesos);
              Serial.print("He procesado: ");
              Serial.print(count_procesos);
              Serial.println(" pelotas");
              BandaStop();
              cleanColor_lcd();
            }
            if(count_procesos == numero)
            {
              Serial.println("Abro las compuertas");
              count_procesos = 0;
              numero = 0;
              numero_ingresado = "";
              Serial.print("numero limpiado:");
              Serial.println("numero_ingresado");
              lcd_dispensadora.setCursor(2,3);
              lcd_dispensadora.print(count_procesos);
              lcd_dispensadora.clear();
              end_process();
              OpenGates();
              delay(2500);
              lcd_dispensadora.clear();
              delay(250);
              lcd_message_init();
            }
          }  
          //Si detecto que hay una pelota para clasificar

        //Aqui va la ejecución de todo el código
      }
    }
    else if(tecla >='0' && tecla <='9')   //Sino, la tecla debera de estar entre 0 y 9 
    {
      //la concateno a un string
        numero_ingresado+=tecla;
    }
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
  Servo_derecho.attach(servoDerecho);       //servo conectado al pin 2
  Servo_izquierdo.attach(servoIzquierdo);   //servo conectado al pin 3
  Servo_sorter.attach(servoSorter);         //servo conectado al pin 4
}

void OpenSorter(void) {
  Servo_sorter.write(0);          //Cierro el servo del sorteador
  delay(250);                     //pausa de 250 milisegundos
  Servo_sorter.write(90);         //Apertura el servo del sorteador
  delay(1000);                    //pausa de 1000 milisegundos
}

float read_Color(void) {
  unsigned int readRed, readBlue, readGreen, clear;
  delay(50);  //espera de 50 ms para integración de la lectura
  sensor_color.getRawData(&readRed, &readGreen, &readBlue, &clear);
  lcd_dispensadora.setCursor(2,0);
  lcd_dispensadora.print(readRed, BIN);
  lcd_dispensadora.setCursor(2,1);
  lcd_dispensadora.print(readGreen,BIN);
  lcd_dispensadora.setCursor(2,2);
  lcd_dispensadora.print(readBlue,BIN);
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
}

void OpenGates(void)
{
  Servo_derecho.write(180);     //abierto
  Servo_izquierdo.write(0);   //abierto
  delay(3000);    
  Servo_derecho.write(90);      //Cerrado
  Servo_izquierdo.write(90);    //Cerrado
  delay(3000);
}

void lcd_message_init(void)
{
  lcd_dispensadora.setCursor(0,0);
  lcd_dispensadora.print("R:");
  lcd_dispensadora.setCursor(15,0);
  lcd_dispensadora.print("CR:");
  lcd_dispensadora.setCursor(18,0);
  lcd_dispensadora.print(count_rojas);
  lcd_dispensadora.setCursor(0,1);
  lcd_dispensadora.print("G:");
  lcd_dispensadora.setCursor(15,1);
  lcd_dispensadora.print("CG:");
  lcd_dispensadora.setCursor(18,1);
  lcd_dispensadora.print(count_verdes);
  lcd_dispensadora.setCursor(0,2);
  lcd_dispensadora.print("B:");
  lcd_dispensadora.setCursor(15,2);
  lcd_dispensadora.print("CN:");
  lcd_dispensadora.setCursor(18,2);
  lcd_dispensadora.print(count_naranjas);
  lcd_dispensadora.setCursor(0,3);
  lcd_dispensadora.print("P:");
  lcd_dispensadora.setCursor(2,3);
  lcd_dispensadora.print(count_procesos);
  lcd_dispensadora.setCursor(5,3);
  lcd_dispensadora.print("Color:");

} 

void cleanColor_lcd(void)
{
   lcd_dispensadora.setCursor(2,0);
   for(int i = 2; i<14; i++)
   {
    lcd_dispensadora.print(" ");
   }
   lcd_dispensadora.setCursor(2,1);
   for(int i = 2; i<14; i++)
   {
    lcd_dispensadora.print(" ");
   }
   lcd_dispensadora.setCursor(2,2);
      for(int i = 2; i<14; i++)
   {
    lcd_dispensadora.print(" ");
   }

   lcd_dispensadora.setCursor(11,3);
   lcd_dispensadora.print("        ");
}

void lcd_print_retro(void)
{
  lcd_dispensadora.clear();
  lcd_dispensadora.setCursor(7,0);
  lcd_dispensadora.print("retro");
  lcd_dispensadora.setCursor(4,1);
  lcd_dispensadora.print("alimentacion");
}

void lcd_print_end_process(void)
{
  lcd_dispensadora.setCursor(4,2);
  lcd_dispensadora.print("Clasificacion");
  lcd_dispensadora.setCursor(6,3);
  lcd_dispensadora.print("Exitosa");
}

void end_process(void)
{
  lcd_print_retro();
  lcd_print_end_process();

}