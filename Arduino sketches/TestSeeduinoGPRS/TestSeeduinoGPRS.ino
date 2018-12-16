#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    8
#define PIN_RX    7
#define SIM800_POWER_PIN        9
#define SIM800_POWER_STATUS     12

#define BAUDRATE  9600

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting sketch..."));

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
  
  String line = String(millis()) + ", " + String(freeMemory());
  
  SIM800_sendLine(line.c_str());

  delay(5000);
}

