/*
   AirSense_2.ino
   Sketch used for the second prototype of AirSense

   Hardware:
   - Arduino UNO
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

/* PROBLEM:
    with 2 devices on the SPI bus, the SD card library fails when calling the begin() function

   SOLUTION:
   you need to do a little change in the standard SD card library
   the problem is that the library is not prepared to work with other SPI devices,
   it assumes you only have the SD card basically
   the change is minimal, in your Arduino IDE installation folder find the SD library
   (something like "C:\Program Files (x86)\Arduino\libraries\SD\src"),
   then in the file named SD.cpp, in function begin() (line 337) add this instruction before the return statement:
     if(root.isOpen()) root.close();
   that should do it.
*/
#include "Seeed_BME280.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <MutichannelGasSensor.h>

#define LED_PIN                  2            //pin used for signalling
#define CUSTOM_MICS_PIN          A0           //pin of the custom NO2 Mics metaloxide sensor
#define SD_CS_PIN                4            //chip select pin of the SD card reader
#define ADS_RST_PIN              8            //ADS1256 reset pin
#define ADS_RDY_PIN              9            //ADS1256 data ready
#define ADS_CS_PIN               10           //ADS1256 chip select
// 11, 12 and 13 are taken by the SD card and the ADS

#define ADS_ALPHA1_CHANNEL       0            // channel used on the ADS for Alpha1
#define ADS_ALPHA2_CHANNEL       1            // channel used on the ADS for Alpha2
#define ADS_SPEC1_CHANNEL        2            // channel used on the ADS for SPEC1
#define ADS_SPEC2_CHANNEL       3            // channel used on the ADS for SPEC2

#define SLEEP_TIME               60000        //sensor sleep time in ms


#define MULTIGAS_SENSOR_ADDR     0X04
#define MULTIGAS_PRE_HEAT_TIME   15           // pre-heat time, 10-30 minutes is recommended
//z#define CALIBRATE_MULTIGAS                    // define this to perform calibration at start

#define filename "airsense.csv"               //filename written on the SD card


BME280 bme280;

unsigned int temperature;
unsigned int humidity;
float pressure;
float R0_NH3, R0_CO, R0_NO2;
float Rs_NH3, Rs_CO, Rs_NO2;
float ratio_NH3, ratio_CO, ratio_NO2;
float nh3, co, no2, c3h8, c4h10, ch4, h2, c2h5Oh;
int custom_MICS;
long alpha1, alpha2, alphadiff, specdiff;

unsigned long lastSample;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting air sense"));

  pinMode(LED_PIN, OUTPUT);

  SPI.begin(); //start the spi-bus
  initADS();


  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Card failed"));
    LED(0, 200, true);
  } else Serial.println(F("Card initialised"));

  File logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.println(F("Cannot open file!"));
    LED(0, 200, true);
  } else {
    logfile.println(F("#timestamp, temperature, humidity, pressure, custom_MICS, ratio_NH3, ratio_CO, ratio_NO2, nh3, co, no2, c3h8, c4h10, ch4, h2, c2h5oh, alpha1, alpha2, alphadiff, specdiff"));
    logfile.close();
  }

  if (!bme280.init()) {
    Serial.println(F("Cannot init BME280!"));
    LED(0, 200, true);
  }
  gas.begin(MULTIGAS_SENSOR_ADDR);

#ifdef CALIBRATE_MULTIGAS
  Serial.print(F("Calibrating gas sensor"));
  for (int i = 60 * MULTIGAS_PRE_HEAT_TIME; i >= 0; i--) {
    Serial.print(F("."));
    delay(1000);
  }
  gas.doCalibrate();
  Serial.println(F("Calibration ok"));
  //gas.display_eeprom();
#endif

  LED(3000, 50, false);

  lastSample = -SLEEP_TIME;
}


void loop() {

  while ((millis() - lastSample) < (SLEEP_TIME)) {
    delay(10); //waste some time
  }
  lastSample = millis();

  getTempHumPress();
  getCustomMICS();
  getMultiGas();
  getAlphaSPEC();

  printToSD();

  Serial.println("-----------------");
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

void getCustomMICS() {
  // get 10 measurements and average
  unsigned long acc = 0;
  for (int i = 0; i < 10; i++) {
    acc += analogRead(CUSTOM_MICS_PIN);
    delay(100);
  }
  custom_MICS = (int)(acc / 10);
}

void getMultiGas() {
  R0_NH3 = gas.getR0(0);
  R0_CO  = gas.getR0(1);
  R0_NO2 = gas.getR0(2);

  Rs_NH3 = gas.getRs(0);
  Rs_CO  = gas.getRs(1);
  Rs_NO2 = gas.getRs(2);

  ratio_NH3 = Rs_NH3 / R0_NH3;
  ratio_CO  = Rs_CO / R0_CO;
  ratio_NO2 = Rs_NH3 / R0_NO2;

  nh3 = gas.measure_NH3();

  co = gas.measure_CO();

  no2 = gas.measure_NO2();

  c3h8 = gas.measure_C3H8();

  c4h10 = gas.measure_C4H10();

  ch4 = gas.measure_CH4();

  h2 = gas.measure_H2();

  c2h5Oh = gas.measure_C2H5OH();
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

//Prints a line of measurements to the SD
static char numbuff [15];
static char s[200];
const char comma_space[] = ", ";

void printToSD() {
  File logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.println(F("Cannot open file!"));
    LED(0, 200, true);
  } else {

    strcpy(s, ltoa(millis(), numbuff, 10) );
    strcat(s, comma_space);
    strcat(s, ltoa(temperature, numbuff, 10));
    strcat(s, comma_space);
    strcat(s, ltoa( humidity, numbuff, 10));
    strcat(s, comma_space);
    strcat(s, dtostrf(pressure, 3, 2, numbuff));
    strcat(s, comma_space);
    strcat(s, itoa( custom_MICS, numbuff, 10));
    strcat(s, comma_space);
    strcat(s, dtostrf(ratio_NH3, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(ratio_CO, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(ratio_NO2, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(nh3, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(co, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(no2, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(c3h8, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(c4h10, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(ch4, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(h2, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, dtostrf(c2h5Oh, 3, 4, numbuff));
    strcat(s, comma_space);
    strcat(s, ltoa(alpha1, numbuff, 10));
    strcat(s, comma_space);
    strcat(s, ltoa(alpha2, numbuff, 10));
    strcat(s, comma_space);
    strcat(s, ltoa(alphadiff, numbuff, 10));
    strcat(s, comma_space);
    strcat(s, ltoa(specdiff, numbuff, 10));

    Serial.print(F("-> "));
    Serial.println(s);
    logfile.println(s);
    logfile.close();
  }
}
