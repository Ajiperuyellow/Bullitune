#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

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
void printValues();

//Display
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//Neigungsmesser
const int ADCPin = 34;
int ADCValue = 0;

byte Grad[8] = {
  B00100,
  B01010,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
};


//CODE:

void setup() {

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

  printValues();
  delay(200);
}

void loop() {
  int ADCtotal;
  ADCtotal=0;
  for (int i = 0; i < 10; i++) 
  {
    ADCValue = analogRead(ADCPin);
    ADCtotal+=ADCValue;
    delay(50);
  }
  ADCtotal/=10;
  lcd.setCursor(0,3);
  lcd.print(String(ADCtotal));
  
}

//#include <SPI.h>
//#define BME_SCK 18
//#define BME_MISO 19
//#define BME_MOSI 23
//#define BME_CS 5
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI


void printValues() {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);


  
  lcd.setCursor(0,0);
  lcd.print("Temp= ");
  lcd.print(temp_event.temperature);
  lcd.print(" ");
  lcd.write(1);
  lcd.print("C");
  
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


}
