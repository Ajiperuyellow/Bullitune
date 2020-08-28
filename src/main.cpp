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
bool check_ok_status();

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

// Button
const int PushButton=35;

//SoftwareSerial
#define D5 (14)
#define D6 (12)

#ifdef ESP32
#define BAUD_RATE 9600
#endif

SoftwareSerial swSer;

//CODE:

void setup() {

  // Software Serial init
  swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);

  // ADC init
  analogSetAttenuation((adc_attenuation_t)3);   // -11dB range
  analogSetWidth(10);
  analogSetCycles(20);

  // lcd init
  lcd.init();                      // initialize the lcd 
  lcd.createChar(1, Grad);
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  
  lcd.setCursor(1,0);
  lcd.print("Guten Morgen");

  // BMP280: Temperature sensor
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

  // button
  pinMode(PushButton, INPUT);

  // Debug call
  //delay(2000);
  //sendSMS("Bulli test 123", "015781581259");
  //delay(10000);

  lcd.clear();
  lcd.setCursor(0,3);
  lcd.print("wait...");

}

void loop() 
{
  bool okStatus;

  okStatus = check_ok_status();

  if (okStatus==true)
  {
    lcd.clear();
    lcd.setCursor(0,3);
    lcd.print("done!");
    delay(10000);
  }

  delay(100);
}

bool check_ok_status()
{
   
  int okChars;
  okChars = 0;
  while (okChars<2)
  {
    if (swSer.available()) 
    {
      // Read the data from buffer
      int len = 0;
      int c;  
      unsigned long timeout = 700;
      unsigned long start = millis();
      
      int buffer[512];
      memset(buffer, 0, sizeof(buffer));
          
      while ((millis() - start < timeout)) 
      {
          if (swSer.available()) 
          {
            c = swSer.read();
            buffer[len++] = c;
            //lcd.write(c);
            
            // check if O and is sent = ASCI 79
            if (okChars == 0 && c == 79)
            { 
              okChars++;
            }
            // check if  O is sent 
            if (okChars == 2 && c == 79)
            { 
              okChars=0;
            }            
            // check if K is sent = ASCI 75
            if (okChars == 1 && c == 75)
            {
              okChars++;
            }
            

          }
          yield();
      }
    }
  }
  
  return true;
  
}

void do_data_analysis()
{
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
    //sendSMS(SendString, "015781581259");
    //delay(1000);
    //sendSMS(SendString, "015773125452");
    //delay(1000);
    //sendSMS(SendString, "017660909952");
    //delay(1000);
    //delay(1000);
  }

  lcd.setCursor(0,3);
  lcd.print(String(ADCtotal));
  //lcd.print(String((ADCtotal/400.-1.)*20.));
  lcd.print(" Grad");

  //check button state
  int Push_button_state = digitalRead(PushButton);
  if (Push_button_state == HIGH)
  {
    lcd.setCursor(0,3);
    lcd.print(" AN");
    delay(100);
  }
  else
  {
    lcd.setCursor(0,3);
    lcd.print(" AUS");
    delay(100);
  }
  
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
  lcd.print("Temp = ");
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