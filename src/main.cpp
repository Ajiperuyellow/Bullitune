#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

//#include <driver/adc.h>
//#include <Arduino.h> 

//BMP280
Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

#define SEALEVELPRESSURE_HPA (1013.25)

//FUNKTIONEN
String printValues();
void callPhone();
void sendSMS(String TextToSend, String Nummer);

//Display
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
byte Grad[8] = {
  B00100,
  B01010,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
};

//Neigungsmesser
const int ADCPin = 34;
int ADCValue = 0;

//SoftwareSerial
#define D5 (14)
#define D6 (12)

#ifdef ESP32
#define BAUD_RATE 9600
#endif

SoftwareSerial swSer;

//CODE:

void setup() {

  swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);

  analogSetAttenuation((adc_attenuation_t)3);   // -11dB range
  analogSetWidth(10);
  analogSetCycles(20);

  lcd.init();                      // initialize the lcd 
  lcd.createChar(1, Grad);
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  
  lcd.setCursor(1,0);
  lcd.print("Guten Morgen");

  if (!bmp.begin()) 
  {
    delay(10);
    lcd.print(String(bmp.begin()));
    while (1) delay(10);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();    


  //delay(2000);
  //callPhone();
  //delay(10000);
}

void loop() {

  /*for (char ch = ' '; ch <= 'z'; ch++) 
  {
		swSer.write(ch);
	}
	swSer.println("");*/

  String SendString;

  SendString = printValues();
  delay(100);

  int ADCtotal;
  ADCtotal=0;
  for (int i = 0; i < 10; i++) 
  {
    ADCValue = analogRead(ADCPin);
    ADCtotal+=ADCValue;
    delay(50);
  }
  ADCtotal/=10;
  SendString += "Neigung="+String((ADCtotal/400.-1.)*20.)+" Grad. :D";
  if(ADCtotal>500)
  {
    sendSMS(SendString, "015781581259");
    delay(1000);
    //sendSMS(SendString, "015773125452");
    //delay(1000);
    //sendSMS(SendString, "017660909952");
    delay(1000);
    delay(1000);
  }

  lcd.setCursor(0,3);
  lcd.print(String((ADCtotal/400.-1.)*20.));
  lcd.print(" Grad");
}

//#include <SPI.h>
//#define BME_SCK 18
//#define BME_MISO 19
//#define BME_MOSI 23
//#define BME_CS 5
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI


String printValues() {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  String Datastring;
  
  lcd.setCursor(0,0);
  lcd.print("Temp= ");
  String TString = String(temp_event.temperature);
  lcd.print(TString);
  lcd.print(" ");
  lcd.write(1);
  lcd.print("C");
  
  Datastring = "Bulli: T=" + TString + " C, ";
  


  // Convert temperature to Fahrenheit
  /*Serial.print("Temperature = ");
  Serial.print(1.8 * bme.readTemperature() + 32);
  Serial.println(" *F");*/

  lcd.setCursor(0,1);
  lcd.print("Press= ");
  lcd.print(pressure_event.pressure);
  lcd.print(" hPa");

  lcd.setCursor(0,2);
  lcd.print("Alt= ");
  lcd.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  lcd.print(" m");

  Datastring +="Alt=" + String(bmp.readAltitude(SEALEVELPRESSURE_HPA)) + " m, ";
  return Datastring;
}

void sendSMS(String TextToSend, String Nummer)
{
  swSer.println("AT+CMGF=1");
  delay(2000);
  swSer.println("AT+CMGS="+Nummer);
  delay(300);
  swSer.println(TextToSend);
  swSer.write(26);
}

void callPhone()
{
  swSer.println("ATD015781581259");;
}