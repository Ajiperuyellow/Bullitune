#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <string>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include <HardwareSerial.h>


#define RXD2 16
#define TXD2 17


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
void init_gprs();
void send_until_ok(String atCommand);
void send_to_thingspeak(int data);
void send_to_thingspeak(String data);
void print_error_or_ok();
void test_gsm();
String get_temperature();
void wait_until_ready();

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

void setup_test()
{
  Serial2.begin(BAUD_RATE);
  //Serial2.println("Test");
}

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete


void setup() {

  Serial2.begin(BAUD_RATE);

  //

  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(14,HIGH);
  digitalWrite(12,HIGH);

  // ADC init
  //analogSetAttenuation((adc_attenuation_t)3);   // -11dB range
  //analogSetWidth(10);
  //analogSetCycles(20);

  // lcd init
  lcd.init();                      // initialize the lcd 
  lcd.print("Guten Morgen");
  lcd.createChar(1, Grad);
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  

  // BMP280: Temperature sensor
  //if (!bmp.begin()) 
  //{
  //  delay(10);
  //  lcd.print(String(bmp.begin()));
  //  while (1) delay(10);
  //}
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  //bmp_temp->printSensorDetails();    

  // button
  pinMode(PushButton, INPUT);

  // Debug call
  //delay(2000);
  //sendSMS("Bulli test 123", "015781581259");
  //delay(10000);

  lcd.clear();
  lcd.setCursor(0,3);
  lcd.print("wait...");
  delay(1000);


    // Software Serial init
    //swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
  //test_gsm();  
  lcd.clear();

}

void loop() 
{

  wait_until_ready();

  lcd.clear();  
  delay(500);
  lcd.setCursor(0,0);
  lcd.print("AT");
  delay(500);
  send_until_ok("AT");
  delay(1000);

  lcd.clear();
  delay(500);
  lcd.setCursor(0,0);
  lcd.print("ATI");
  delay(500);
  send_until_ok("ATI");
  delay(1000);
  
  lcd.clear();

  while(1)
  {
    String tempString = get_temperature();
    init_gprs();
    delay(1000);
    send_to_thingspeak(tempString);
  }
  
  

  

   
  init_gprs();
  delay(1000); 
  send_to_thingspeak(6);


  lcd.print("D O N E.");
  while(1)
  {}
}

void test()
{
  //init_gprs();
  //send_to_thingspeak(12);
  //lcd.setCursor(0,3);
  //lcd.print("done...");
  //Serial2.println("Test");
  Serial2.println("AT");

  while (Serial2.available()) {
    // get the new byte:
    char inChar = (char)Serial2.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      inputString.trim();
    }
  }
  
  if (stringComplete == true)
  {
    lcd.setCursor(0,1);
    lcd.print(inputString);

    if (inputString=="OK")
    {
      lcd.print("YES");
    }

    inputString = "";
    stringComplete = false;

  }else
  {
    //Serial2.println("AT");
  }
  delay(10);
  
  //if (Serial2.available()>1) 
  //{
  //  c = Serial2.read();
  //}

  //send_until_ok("AT");
  //delay(300);
  //lcd.clear();
  //lcd.setCursor(0,0);
  //lcd.print("ok");
  //delay(1000);

}





void print_error_or_ok()
{
  bool okStatus;

  okStatus = check_ok_status();

  if (okStatus==true)
  {
    lcd.clear();
    lcd.setCursor(0,3);
    lcd.print("done!");
    delay(1);
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0,2);
    lcd.print("error!"); 
    delay(1);  
  }

  delay(1);
}

bool check_ok_status()
{
   
  int okChars, errorChars;
  okChars = 0;
  errorChars = 0;
  //while (okChars<2)
  {

      // Read the data from buffer
//      int len = 0;
      int c;  
      //unsigned long timeout = 700;
      //unsigned long start = millis();
      
      //int buffer[512];
      //memset(buffer, 0, sizeof(buffer));
          
      //while ((millis() - start < timeout)) 

        //while(1)
        {
          while (Serial2.available()) 
          {
            c = Serial2.read();
            //buffer[len++] = c;
            //lcd.write(c);

            // check if O and is sent = ASCI 79
            if (okChars == 0 && c == 79)
            { 
              okChars++;
            }           
            // check if K is sent = ASCI 75
            if (okChars == 1 && c == 75)
            {
              okChars++;
              return true;
            }        

            // check for errorStatus, E
            if (errorChars == 0 && c == 69)
            {  
              errorChars++;
            }
            // check for errorStatus, R
            if (errorChars == 1 && c == 82)
            {  
              errorChars++;
            }
            // check for errorStatus, R
            if (errorChars == 2 && c == 82)
            {  
              errorChars++;
            }
            // check for errorStatus, O
            if (errorChars == 3 && c == 79)
            {  
              return false;
            }
          }
       }
  }
  
  return false;
  
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

void test_gsm()
{
  lcd.setCursor(0,0);
  while(1)
  {
    lcd.print("send AT");
    Serial2.println("AT");
    
    int c;
  
    while (Serial2.available()) 
    {
      c = Serial2.read();
      lcd.write(c);
      lcd.print(c);
    }
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

String get_temperature()
{
  sensors_event_t temp_event;
  bmp_temp->getEvent(&temp_event);
  String TString = String(temp_event.temperature);
  return TString;
}

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

void init_gprs()
{
  // AT+CREG=1
  send_until_ok("AT+CREG=1");
 
  // AT+CGATT=1
  send_until_ok("AT+CGATT=1");

  // AT+CGDCONT=1,"IP","internet.eplus.de"
  send_until_ok("AT+CGDCONT=1,\"IP\",\"internet.eplus.de\"");

  // AT+CGACT=1,1
  send_until_ok("AT+CGACT=1,1");

    lcd.setCursor(0,3);
    lcd.print("GPRS init ok!");
    delay(1000);
}

void send_to_thingspeak(String data)
{

  lcd.clear();

  // AT+CIPSTART="TCP","api.thingspeak.com",80
  send_until_ok("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80");    
    
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(".");
  delay(100);

  // AT+CIPSEND
  Serial2.println("AT+CIPSEND");   
  delay(100);
  lcd.print(".");
  delay(100);

  // AT+CIPSTART="TCP","api.thingspeak.com",80
  String getString;
  getString = "GET https://api.thingspeak.com/update?api_key=EZV17IADITIDG1ZC&field1=";
  getString += data;
  Serial2.println(getString);
  Serial2.write(26);
  lcd.print(".");

  lcd.setCursor(0,1);
  lcd.print("sending...");
  delay(3000);  

  for (int i = 0; i <= 5; i++)
  {
    send_until_ok("AT"); 
    delay(500);
    lcd.clear();
    delay(500);
  }
  delay(3000); 
  for (int i = 0; i <= 5; i++)
  {
    send_until_ok("AT"); 
    delay(500);
    lcd.clear();
    delay(500);
  } 
  send_until_ok("AT+CIPCLOSE");
  send_until_ok("AT"); 

  lcd.clear();
}



void send_to_thingspeak(int data)
{

  lcd.clear();

  // AT+CIPSTART="TCP","api.thingspeak.com",80
  send_until_ok("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80");    
    
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(".");
  delay(100);

  // AT+CIPSEND
  Serial2.println("AT+CIPSEND");   
  delay(100);
  lcd.print(".");
  delay(100);

  // AT+CIPSTART="TCP","api.thingspeak.com",80
  String getString;
  getString = "GET https://api.thingspeak.com/update?api_key=EZV17IADITIDG1ZC&field1=";
  getString += String(data);
  Serial2.println(getString);
  Serial2.write(26);
  lcd.print(".");

  lcd.setCursor(0,1);
  lcd.print("sending...");
  delay(3000);  

  for (int i = 0; i <= 5; i++)
  {
    send_until_ok("AT"); 
    delay(500);
    lcd.clear();
    delay(500);
  }
  delay(3000); 
  for (int i = 0; i <= 5; i++)
  {
    send_until_ok("AT"); 
    delay(500);
    lcd.clear();
    delay(500);
  } 
  send_until_ok("AT+CIPCLOSE");
  send_until_ok("AT"); 

  lcd.clear();
}


void send_until_ok_dep(String atCommand)
{
  Serial2.println(atCommand);
  bool isOK;
  isOK = check_ok_status();
  while (!isOK)
  {
    delay(1);
    Serial2.println(atCommand);
    isOK = check_ok_status();
  }
}

void send_until_ok(String atCommand)
{
  Serial2.println(atCommand);
    
  int cnt = 0;

  bool isOK = false;
  bool doErrorHandling = false;

  while (!isOK)
  {

    stringComplete = false;

    while(stringComplete==false)
    {

      while (Serial2.available() > 0 && stringComplete==false) 
      {
          // get the new byte:
          char inChar = (char)Serial2.read();
          // add it to the inputString:
          inputString += inChar;
          // if the incoming character is a newline, set a flag so the main loop can
          // do something about it:
          if(inChar== 'O')
          {
            inputString = "O";
          }
          if (inChar == 'K') 
          {
            stringComplete = true;
            inputString.trim();
          }
          if (inChar == '\n') 
          {
            stringComplete = true;
            inputString.trim();
          }
          int len = inputString.length();
          if (inputString.substring(len-3) == "OR:")
          {
            doErrorHandling = true;
            stringComplete = true;
          }
      } 
    } 
    
    if (doErrorHandling==true)
    {
      doErrorHandling = false;
      delay(1000);
      
      lcd.setCursor(0,3);
      lcd.print("Error handling..");
      Serial2.println("AT");
      delay(500);
      bool errorAway = false;
      while(!errorAway)
      { 
        while (Serial2.available() > 0) 
        {
          // get the new byte:
          char inChar = (char)Serial2.read();
          if(inChar == 'K')
          {
            errorAway = true;
          }
        }
      }
      
      lcd.setCursor(0,3);
      lcd.print("Error erased.   ");
      delay(2000);
      cnt = 2000;
    }
    else if (inputString=="OK")
    {
      lcd.setCursor(0,3);
      lcd.print("command ok      ");
      isOK = true;
    }
    else
    {
      lcd.setCursor(0,1);
      if( inputString.length() > 16)
      {
        String stringCut = inputString.substring(inputString.length()-16);
        inputString = stringCut;
      }
      lcd.print(inputString);
      isOK = false;
    }

    inputString="";

    delay(1);
    cnt++;
    if (cnt>=2000)
    {
      cnt = 0;
      Serial2.println(atCommand);
    }

  }

  inputString = "";
  stringComplete = false;

  delay(10);

}


void wait_until_ready()
{

 for (int i = 0; i < 1000; i++) 
 {
    Serial2.println("AT");
    while (Serial2.available() > 0) 
    {
      char inChar = (char)Serial2.read();
    }
    delay(1);
 }

  
 
  delay(500);
  bool errorAway = false;
  while(!errorAway)
  { 
    while (Serial2.available() > 0) 
    {
      // get the new byte:
      char inChar = (char)Serial2.read();
      if(inChar == 'K')
      {
        errorAway = true;
      }
      //lcd.print(inChar);
    }
    Serial2.println("AT");
    delay(500);
  }
  //lcd.clear();
  //lcd.print("GSM ready.");
  delay(2000);
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