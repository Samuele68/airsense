/*
   AirSense_3.ino
   Sketch used for the second prototype of AirSense

   Hardware:
   - Seeeduino GPRS
   - Grove base shield
   - Grove barometer sensor (BME280)
   - Grove multichannel gas sensor
   - An LED
   - A custom made board with a Mics2714
   - ADS1256 board
   - SD card reader

   Connections:
   - Grove barometer and Grove multichannel gas sensors connected to I2C slots
   - LED connected between GND and pin 2
   - custom Mics2714 board connected to pin A0
   - ADS1256 connected to pins: 8 (RST), 9 (RDY), 10 (CS), 11 (DIN), 12 (DOUT), 13 (CLK)
   - SD card reader connected to pins: 4 (CS), 11 (MOSI), 12 (MISO), 13 (SCK)

   Needed libraries:
    - Multichannel gas sensor library (see http://wiki.seeed.cc/Grove-Multichannel_Gas_Sensor/)

*/
#include "Seeed_BME280.h"
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#define PIN_TX    8
#define PIN_RX    7
#define SIM800_POWER_PIN        9
#define SIM800_POWER_STATUS     12

#define SLEEP_TIME               60000        //sensor sleep time in ms

#define BAUDRATE  9600

BME280 bme280;
GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);

unsigned int temperature;
unsigned int humidity;
float pressure;


void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting sketch..."));

  if (!bme280.init()) {
    Serial.println(F("Cannot init BME280!"));
  }

  SIM800_init();
  
  delay(3000);

  SIM800_connectInternet();
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {

  Serial.println("checking SMS");
  int messageIndex = gprs.isSMSunread();
   if (messageIndex > 0) { //At least, there is one UNREAD SMS
      gprs.deleteSMS(messageIndex);
      // if any message is sent, reset the sketch
      Serial.print(F("Got message resetting..."));
      delay(2000);
      resetFunc();
   } else Serial.println(F("no new SMS"));

   getTempHumPress();
  
  String line = String(millis()) + ", " + String(freeMemory());
  
  SIM800_sendLine(line.c_str());

  delay(5000);
}

void getTempHumPress() {
  temperature = bme280.getTemperature();
  humidity = bme280.getHumidity();
  pressure = bme280.getPressure();
}

