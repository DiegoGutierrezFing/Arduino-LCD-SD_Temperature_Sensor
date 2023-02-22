/* Sensor de temperatura y humedad con reloj y registro temporal en microSD
    Diego Andrés Gutiérrez Berón
    22/01/2021 - Montevideo, Uruguay

    Descripción
    El codigo fuente fue adaptado de varios ejemplos encontrados en Internet: es decir
    forma parte de una idea del autor mencionado mas arriba que para su realizacion recurrio a
    ejemplos de proyectos con sensores por parte de otros autores. Luego de haber estudiando su
    codigo y "juntando las piezas", termino en un dispositivo que integra varias funciones.

    La funcionalidad de este dispositivo se resume en los siguientes puntos:
    - Medicion de temperatura ambiente
    - Medicion de humedad relativa ambiente
    - Registro de los datos en memoria microSD para analisis posterior (funcion Datalogger).

    Fuentes de informacion original (articulos, tutoriales y blogs en los que se basa este codigo):

*/

#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <SD.h>

// Descripcion de pines del sensor DHT11:
// VCC: 5V or 3V
// GND: GND
// DATA: 2

/* Definicion de variables */
#define pinDHT11 2                   // El pin de salida de datos del sensor DHT11 se conecta al pin 2 de la placa arduino
#define chipSelect 10                //
SimpleDHT11 dht11(pinDHT11);         // Objeto que representa al sensor DHT11
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Configuracion LCD 16x2 con I2C: establecer la direccion I2C del LCD a 0x27 para un display de 16 columnas y 2 filas

// Variable que representa un caracter personalizado (icono) con forma de gota de agua en el lcd
byte gota[] = {
  B00100,
  B00110,
  B01110,
  B11111,
  B10111,
  B10111,
  B11011,
  B01110
};


void setup() {
  Serial.begin(9600);
  // inicializar el LCD
  lcd.init();

  lcd.createChar(2, gota);

  // Imprimir mensaje de bienvenida en el LCD
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Termometro LCD");
  lcd.setCursor(0, 1);
  lcd.print("Diego Gutierrez");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("22/01/2020");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(1000);
  lcd.clear();

  if (!SD.begin()) {
    Serial.println("SD: Falla");
    return;
  }
  Serial.println("SD: OK");
}

void loop() {

  tmElements_t tm;  // Variable que almacena los valores de fecha y hora reportados por el modulo RTC
  File myFile;      // Variable utilizada para crear un archivo con los datos a registrar
  String medida;    // Variable utilizada para crear una cadena de texto que almacena el valor de temperatura, humedad relativa, fecha y hora para mostrarlos en pantalla

  Serial.println("");
  if (RTC.read(tm)) {
    Serial.println("RTC: OK");
    Serial.println("DHT11: OK");

    // lectura sin muestras.
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;

    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Falla lectura DHT11, err=");
      Serial.println(SimpleDHTErrCode(err));
      return;
    }

    Serial.println("Muestra OK: ");
    lcd.setCursor(0, 0);

    String T = String((int)temperature);
    String t = print2digits(tm.Hour) + ":" + print2digits(tm.Minute) + ":" + print2digits(tm.Second);
    String date = print2digits(tmYearToCalendar(tm.Year)) + "-" + print2digits(tm.Month) + "-" + print2digits(tm.Day);
    String HR = String((int)humidity);

    medida = "T:" + T + (char)0xDF + "C " + t;

    lcd.print(medida);
    lcd.setCursor(0, 1);

    medida = ":" + HR + (char)0x25 + " " + date;

    lcd.print((char)0x02);
    lcd.print(medida);

    Serial.println(T + "ºC " + HR + "% " + " " + t + " " + date);

    // abrir archivo para almacenar datos
    myFile = SD.open("test.txt", FILE_WRITE);

    // si el archivo se abrio correctamente, escribir datos en el
    if (myFile) {
      Serial.print("Escribiendo archivo test.txt... ");
      // formato ISO 8601 en UTC
      myFile.println(T + ";" + HR + ";" + date + "T" + t + "Z");
      // cerrar el archivo
      myFile.close();
      Serial.println("OK.");
    } else {
      // si el archivo no se abre, imprimir un error
      Serial.println("error abriendo test.txt");
    }

    // DHT11 tasa de muestreo es 1Hz.
    delay(1000);
    //lcd.clear();

  } else {
    if (RTC.chipPresent()) {
      Serial.println("Error RTC: Ejecute SetTime de la libreria RTC para inicializar el tiempo.");
      Serial.println();
    } else {
      Serial.println("Error RTC: RTC conectado?");
      Serial.println();
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Falla sensor.");
    lcd.setCursor(0, 1);
    lcd.print("Reiniciando...");
    delay(9000);
    lcd.clear();
  }
}

String print2digits(int number) {
  if (number >= 0 && number < 10) {
    return ("0" + String(number));
  }
  return String(number);
}
