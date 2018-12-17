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

#define CUSTOM_MICS_PIN         A0      //pin of the custom NO2 Mics metaloxide sensor
#define LED_PIN                 2        //pin used for signalling
#define SIM800_PIN_RX           7        //SIM800 serial receive pin
#define SIM800_PIN_TX           8        //SIM800 serial transmit pin
#define SIM800_POWER_PIN        9        //SIM800 power pin
#define SIM800_POWER_STATUS     12       //SIM800 power status pin
#define ADS_RST_PIN             13       //ADS1256 reset pin
#define ADS_RDY_PIN             11       //ADS1256 data ready
#define ADS_CS_PIN              10       //ADS1256 chip select

#define ADS_ALPHA1_CHANNEL      0        // channel used on the ADS for Alpha1
#define ADS_ALPHA2_CHANNEL      1        // channel used on the ADS for Alpha2
#define ADS_SPEC1_CHANNEL       2        // channel used on the ADS for SPEC1
#define ADS_SPEC2_CHANNEL       3        // channel used on the ADS for SPEC2


#define SLEEP_TIME               60000        //sensor sleep time in ms

#define SIM800_BAUDRATE  9600

BME280 bme280;
GPRS gprs(SIM800_PIN_TX, SIM800_PIN_RX, SIM800_BAUDRATE);

unsigned int temperature;
unsigned int humidity;
float pressure;
long alpha1, alpha2, alphadiff, specdiff;
int custom_MICS;

unsigned long lastSample;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting sketch..."));

  pinMode(LED_PIN, OUTPUT);

  if (!bme280.init()) {
    Serial.println(F("Cannot init BME280!"));
    LED(0, 200, true);
  }
  LED(1000, 50, false);

  //start the spi-bus
  SPI.begin();
  initADS();
  LED(1000, 50, false);

  // start the GPRS board
  if (!SIM800_init()) {
    LED(0, 200, true);
  }
  LED(1000, 50, false);
  delay(3000);
  while (!SIM800_connectInternet()) {
    LED(0, 200, true);
  }

  // all OK!
  LED(3000, 50, false);

  lastSample = -SLEEP_TIME;
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

  while ((millis() - lastSample) < (SLEEP_TIME)) {
    delay(10); //waste some time
  }
  lastSample = millis();

  getTempHumPress();
  getAlphaSPEC();
  getCustomMICS();

  sendData();

  LED(4000, 500, false);

}

//Flashes the LED
//time: total signalling time in milliseconds
//period: flashing period in milliseconds
//forever: flashes forever
void LED(int time, int period, boolean forever) {
  unsigned long now = millis();
  while ((millis() - now < time) || (forever)) {
    digitalWrite(LED_PIN, HIGH);
    delay(period);
    digitalWrite(LED_PIN, LOW);
    delay(period);
  }
}

void getTempHumPress() {
  temperature = bme280.getTemperature();
  humidity = bme280.getHumidity();
  pressure = bme280.getPressure();
}

void getAlphaSPEC() {
  // get 10 measurements and average
  alpha1 = 0;
  alpha2 = 0;
  alphadiff = 0;
  specdiff = 0;
  for (int i = 0; i < 10; i++) {
    alpha1 += readADS(ADS_ALPHA1_CHANNEL);
    alpha2 += readADS(ADS_ALPHA2_CHANNEL);
    alphadiff += readADSDiff(ADS_ALPHA1_CHANNEL, ADS_ALPHA2_CHANNEL);
    specdiff += readADSDiff(ADS_SPEC1_CHANNEL, ADS_SPEC2_CHANNEL);
    delay(100);
  }
  alpha1 /= 10;
  alpha2 /= 10;
  alphadiff /= 10;
  specdiff /= 10;
}

void getCustomMICS() {
  // get 10 measurements and average
  unsigned long acc = 0;
  for (int i = 0; i < 10; i++) {
    acc += analogRead(CUSTOM_MICS_PIN);
    delay(100);
  }
  custom_MICS = (int)(acc / 10);
}

static char numberbuff [15];
static char line[200];
const char comma_space[] = ", ";

void sendData() {
  line[0] = "\0";
  numberbuff[0] = "\0";
  strcpy(line, ltoa(millis(), numberbuff, 10) );
  strcat(line, comma_space);
  strcat(line, itoa( freeMemory(), numberbuff, 10));
  strcat(line, ltoa(temperature, numberbuff, 10));
  strcat(line, comma_space);
  strcat(line, ltoa( humidity, numberbuff, 10));
  strcat(line, comma_space);
  strcat(line, dtostrf(pressure, 3, 2, numberbuff));
  strcat(line, comma_space);
  strcat(line, ltoa(alpha1, numberbuff, 10));
  strcat(line, comma_space);
  strcat(line, ltoa(alpha2, numberbuff, 10));
  strcat(line, comma_space);
  strcat(line, ltoa(alphadiff, numberbuff, 10));
  strcat(line, comma_space);
  strcat(line, ltoa(specdiff, numberbuff, 10));
  strcat(s, comma_space);
  strcat(s, itoa(custom_MICS, numbuff, 10));
    
  SIM800_sendLine(line);
}
