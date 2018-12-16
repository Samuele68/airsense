#include <SPI.h>
#include <SD.h>
#include <dht11.h>

//pins

#define NO2_SPEC_PIN     A0 //pin of the NO2 SPEC chemical sensor
#define NO2_MICS_PIN     A1 //pin of the NO2 Mics metaloxide sensor
#define NO2_ALPHAO2_PIN  A2 //pin of the NO2 Alphasense chemical sensor, opamp2
#define NO2_ALPHAO1_PIN  A3 //pin of the NO2 Alphasense chemical sensor, opamp17
#define MQ2_PIN          A4 //pin fo the M2 sensor
#define DHT11_PIN        2 //pin where the DHT11 is connected
#define SD_CS_PIN        4 //chip select pin of the SD card reader
#define LED_PIN          6 //pin used for signalling
#define DUST_PIN         7 //pin of the dust sensor
// 11, 12 and 13 are taken by the SD card


#define DUST_SAMPLE_TIME  30000 //sampling time in ms
#define SLEEP_TIME        60000 //sensor sleep time in ms

dht11 DHT11;

#define filename "airsense.csv" //filename written on the SD card

int temperature;
int humidity;

int rawno2_MICS;
int rawno2_SPEC;
int rawno2_ALPHAO1;
int rawno2_ALPHAO2;
int rawmq2;
float dustRatio;

unsigned long lastSample;

  
//Flashes the LED
//time: total signalling time in milliseconds
//period: flashing period in milliseconds
//forever: flashes forever
void LED(int time, int period, boolean forever){
  unsigned long now = millis();
  while((millis() - now < time) || (forever)){
     digitalWrite(LED_PIN, HIGH);
     delay(period);
     digitalWrite(LED_PIN, LOW);
     delay(period);
  }
}

//Prints a line of measurements to the SD
void printToSD(){
  File logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.println(F("Cannot open file!"));
    LED(0, 200, true);
  } else {
    
    String s = String ();
    s+= millis();
    s+= " ";
    s+= humidity;
    s+= " ";
    s+= temperature;
    s+= " ";
    s+= rawno2_MICS;
    s+= " ";
    s+= rawno2_SPEC;
    s+= " ";
    s+= rawno2_ALPHAO1;
    s+= " ";
    s+= rawno2_ALPHAO2;
    s+= " ",
    s+= rawmq2;
    s+= " ";
    s+= dustRatio,
    
    Serial.print("-> ");
    Serial.println(s);
    logfile.println(s);
    logfile.close();
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting air sense"));
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(DHT11_PIN, INPUT);
  pinMode(NO2_MICS_PIN, INPUT);
  pinMode(NO2_ALPHAO1_PIN, INPUT);
  pinMode(NO2_ALPHAO2_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  
  SPI.begin(); //start the spi-bus

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Card failed"));
    LED(0, 200, true);
  } else Serial.println(F("Card initialised"));
  
  File logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.println(F("Cannot open file!"));
    LED(0, 200, true);
  } else {
    logfile.println(F("#timestamp humidity temperature rawNO2_Mics rawNO2_SPEC rawNO2_Alpha1 rawNO2_Alpha2 rawmq2 dustratio"));
    logfile.close();
  }
  LED(3000, 50, false);

  lastSample = -SLEEP_TIME;
}

void loop() {
  while((millis() - lastSample) < (SLEEP_TIME)) {
    delay(1000); //waste some time
  }
  lastSample = millis();
  getDust();
  getTempHum();
  getN02_MICS();
  getN02_ALPHA();
  getN02_SPEC();
  getMQ2();
  printToSD();
  LED(4000, 500, false);
}


void getTempHum() {
  if(DHT11.read(DHT11_PIN) == DHTLIB_OK){
    temperature = (float) DHT11.temperature;
    humidity = (float) DHT11.humidity;
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.print(F(", humidity: "));
    Serial.println(humidity);
  } else {
    Serial.print(F("Cannot read DHT11 on pin "));
    Serial.println(DHT11_PIN);
    temperature = -1;
    humidity = -1;
   }
}

int getAndAvg(int pin, int iterations) {
  unsigned long acc = 0;
  for(int i=0; i< iterations; i++){
    acc+= analogRead(pin);
    delay(100);
  }
  return (int)(acc/iterations);
}


void getN02_MICS() {
  rawno2_MICS = getAndAvg(NO2_MICS_PIN, 10);
  Serial.print(F("NO2 Mics raw: "));
  Serial.println(rawno2_MICS);
}

void getN02_ALPHA() {
  rawno2_ALPHAO1 = getAndAvg(NO2_ALPHAO1_PIN, 10);
  rawno2_ALPHAO2 = getAndAvg(NO2_ALPHAO2_PIN, 10);
  Serial.print(F("NO2 alphasense raw: "));
  Serial.print(rawno2_ALPHAO1);
  Serial.print(F(", "));
  Serial.println(rawno2_ALPHAO2);
}

void getN02_SPEC() {
  rawno2_SPEC = getAndAvg(NO2_SPEC_PIN, 10);
  Serial.print(F("NO2 SPEC raw: "));
  Serial.println(rawno2_SPEC);
}

void getMQ2() {
  rawmq2 = getAndAvg(MQ2_PIN, 10);
  Serial.print(F("MQ2 raw: "));
  Serial.println(rawmq2);
}

void getDust() {
  unsigned long duration;
  unsigned long sampletime_ms = DUST_SAMPLE_TIME;
  unsigned long rawdust = 0; 

  unsigned long time = millis();
  while ((millis() - time) < sampletime_ms)
  {
    duration = pulseIn(DUST_PIN, LOW);
    rawdust = rawdust + duration;
  }
  dustRatio = rawdust / (sampletime_ms * 10.0); //in percentage 0 to 100

  Serial.print(F("Dust ratio: "));
  Serial.println(dustRatio);
}
