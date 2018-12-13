#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    8
#define PIN_RX    7

#define BAUDRATE  9600

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting sketch...");

  SIM800_init();
  
  delay(3000);

  SIM800_connectInternet();
}

void loop(){

  int signalstrength = -1;
  gprs.getSignalStrength(&signalstrength);

  SIM800_sendStatusLine("just an Arduino test");

  delay(5000);
}

