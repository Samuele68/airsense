#include <Wire.h>
#include <SPI.h>

#define ADS_RST_PIN             4        // ADS1256 reset pin
#define ADS_CS_PIN              5        // ADS1256 chip select
#define ADS_RDY_PIN             6        // ADS1256 data ready

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println(F("Starting sketch..."));

  //start the spi-bus
  SPI.begin();
  initADS();
}

void loop() {
  long alpha0 = readADS(0);
  long alpha1 = readADS(1);
  long alphadiff = readADSDiff(0, 1);
  
  Serial.print(alpha0);
  Serial.print(",");
  Serial.print(alpha1);
  Serial.print(",");
  Serial.print(alphadiff);
  
  delay(500);
}
