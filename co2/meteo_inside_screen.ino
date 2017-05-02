#include <DHT.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define DHTPIN_IN 3     // temp sensor pin
#define DHTPIN_POWER 4  // pin is used to power temperature sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PHOTO_IN 7      // analog read data from photo resistor
#define BACK_LIGHT 5    // screen backlight
#define BUTTON_PIN 19   // buutton to control backlight
DHT dhtIn(DHTPIN_IN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

unsigned long last_backlight = 0;  // milliseconds from last button press. Used to turn off LCD backlight after 6 sec.

LiquidCrystal lcd(7, 6, 13, 9, 10, 12);  // digital pins to 1602 display
SoftwareSerial co2sensorSerial(A0, A1); // A0 - CO2 TX signal sensor, A1 -  RX signal sensor


void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");  
}  


class Meteo {
  public:
      unsigned int co2ppm;
      float tempInside;
      float tempOutside;
      unsigned int humidityInside;
      unsigned int humidityOutside;
      unsigned long presureOutside;
      float presureTempOutside;
      unsigned int lumenInside;
      unsigned int lumenOutside;
      String toJSON();
      unsigned int readPPM();
      float readTempIn();
      float readTempOut();
      unsigned int readHumidityIn();
      unsigned int readHumidityOut();
      unsigned long readPresureOut();
      float readPresureTempOut();
      unsigned int readLumenIn();
      unsigned int readLumenOut();
      void updateSensors();
};

void Meteo::updateSensors() {
  co2ppm = readPPM();
  tempInside = readTempIn();
  tempOutside = readTempOut();
  humidityInside = readHumidityIn();
  humidityOutside = readHumidityOut();
  presureOutside = readPresureOut();
  presureTempOutside = readPresureTempOut();
  lumenInside = readLumenIn();
  lumenOutside = readLumenOut();
}

float Meteo::readTempIn() {
  return dhtIn.readTemperature();
}
float Meteo::readTempOut() {
    //return dhtOut.readTemperature();
  }
unsigned int Meteo::readHumidityIn() {
  return dhtIn.readHumidity();
  }
unsigned int Meteo::readHumidityOut() {
  //return dhtOut.readHumidity();
  }
unsigned long Meteo::readPresureOut() {
  //return bmp.readPressure();
  }
float Meteo::readPresureTempOut() {
  //return bmp.readTemperature();
  }
unsigned int Meteo::readLumenIn() {
    return 1023 - analogRead(PHOTO_IN);
  }
unsigned int Meteo::readLumenOut() {
    //return analogRead(PHOTO_OUT);
  }

unsigned int Meteo::readPPM() {
  byte cmdGetPPM[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  byte response[9];
  co2sensorSerial.write(cmdGetPPM, 9);
  memset(response, 0, 9);
  co2sensorSerial.readBytes(response, 9);
  
  byte crc = 0;
  for (int i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;
  unsigned int ppm = 0;
  if ( response[0] == 0xFF && response[1] == 0x86 && response[8] == crc ) {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    ppm = (256*responseHigh) + responseLow;
  } else {
    software_Reset();
  }
  return ppm;
}


String Meteo::toJSON()
{ 
  String json = "{\n";
  json += "\"co2ppm\":" + String(co2ppm) + ",\n";
  json += "\"temperatureInside\":" + String(tempInside) + ",\n";
  json += "\"temperatureOutside\":" + String(tempOutside) + ",\n";
  json += "\"humidityInside\":" + String(humidityInside) + ",\n";
  json += "\"humidityOutside\":" + String(humidityOutside) + ",\n";
  json += "\"presureOutside\":" + String(presureOutside) + ",\n";
  json += "\"presureTempOutside\":" + String(presureTempOutside) + ",\n";
  json += "\"lumenInside\":" + String(lumenInside) + ",\n";
  json += "\"lumenOutside\":" + String(lumenOutside) + "\n";
  json += "}";
  return json;
}

byte degree[8] = {
 0b00110,
 0b01001,
 0b01001,
 0b00110,
 0b00000,
 0b00000,
 0b00000,
 0b00000
};

byte percent[8] = {
 0b11000,
 0b11001,
 0b00010,
 0b00100,
 0b01000,
 0b10011,
 0b00011,
 0b00000
};

byte happy[8] = {
 0b00000,
 0b10001,
 0b10001,
 0b00000,
 0b10001,
 0b01110,
 0b00000,
 0b00000
};

byte netural[8] = {
 0b00000,
 0b10010,
 0b10010,
 0b00000,
 0b11111,
 0b00000,
 0b00000,
 0b00000
};

byte sad[8] = {
 0b00000,
 0b10001,
 0b10001,
 0b00000,
 0b01110,
 0b10001,
 0b00000,
 0b00000
};

byte terrible[8] = {
 0b00000,
 0b10010,
 0b10010,
 0b00000,
 0b00110,
 0b01001,
 0b00110,
 0b00000
};

byte moon[8] = {
 0b01000,
 0b00000,
 0b00100,
 0b10000,
 0b00001,
 0b01000,
 0b00010,
 0b01000
};

byte temp[8] = {
 0b00100,
 0b01010,
 0b01010,
 0b01110,
 0b01110,
 0b11111,
 0b11111,
 0b01110
};

byte hym[8] = {
 0b00100,
 0b00100,
 0b01010,
 0b01010,
 0b10001,
 0b10001,
 0b10001,
 0b01110
};

Meteo meteo;
void setup() {
  pinMode(DHTPIN_POWER, OUTPUT);
  digitalWrite(DHTPIN_POWER, HIGH);
  
  pinMode(BACK_LIGHT, OUTPUT);
  digitalWrite(BACK_LIGHT, HIGH);
  last_backlight = millis();

  pinMode(BUTTON_PIN, INPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Loading ...");
  
  Serial.begin(9600);
  //Serial.println("Start setup");
  co2sensorSerial.begin(9600);
  dhtIn.begin();
  delay(2000); // delay for dht22

  lcd.createChar(1, degree);
  lcd.createChar(2, percent);
  lcd.createChar(3, happy);
  lcd.createChar(4, netural);
  lcd.createChar(5, sad);
  //lcd.createChar(6, terrible);
  lcd.createChar(7, temp);
  lcd.createChar(6, hym);
  Serial.println("End setup");
}

void printSmile(int ppm) {
  lcd.setCursor(0, 0);
  if (ppm < 600) {
    lcd.print("\3");
  } else if ( ppm < 1000) {
    lcd.print("\4");
  //} else if ( ppm < 2000) {
  //  lcd.print("\5");
  } else {
    lcd.print("\5");
  }
}

void loop() 
{
  //Serial.println("In loop");
  meteo.updateSensors();

  char topLine[17];
  sprintf(topLine, "%6dppm %4dLx", int(meteo.co2ppm), meteo.lumenInside/5);
  char bottomLine[17];
  sprintf(bottomLine, "%6d C %6d", int(meteo.tempInside), meteo.humidityInside);

  lcd.setCursor(0, 0);
  lcd.print(topLine);
  lcd.setCursor(0, 1);
  lcd.print(bottomLine);


  lcd.setCursor(2,1);
  lcd.print("\7"); // temp icon
  lcd.setCursor(6, 1);
  lcd.print("\1"); // celsius
  lcd.setCursor(11, 1);
  lcd.print("\6"); // drop
  lcd.setCursor(15, 1);
  lcd.print("\2"); // percentage

  lcd.setCursor(10, 0);
  printSmile(meteo.co2ppm);
  Serial.println(meteo.toJSON());
  for (int i = 0; i < 100; i++){
    if (digitalRead(BUTTON_PIN)==HIGH) {
      Serial.println("LOW");
      last_backlight = millis();
    }
    if (millis() - last_backlight > 6000) {
      digitalWrite(BACK_LIGHT, LOW);
    } else {
      digitalWrite(BACK_LIGHT, HIGH);
    }
    delay(100);
  }
}
