#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    8  // exchange TX and RX pins
#define PIN_RX    7

#define SIM800_POWER_PIN        9  // Define SIM800 power status and control pins
#define SIM800_POWER_STATUS     12

//make sure that the baud rate of SIM800 is 9600!
//you can use the AT Command(AT+IPR=9600) to set it through SerialDebug
#define BAUDRATE  9600

char http_cmd[] = "POST /airsense/status HTTP/1.1\r\nHost: airsense.fr.openode.io\r\nConnection: close\r\nContent-Length: 27\r\nContent-Type: text/plain;charset=UTF-8\r\n\r\nRunning a test from Arduino\r\n";
char buffer[512];
GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);

void setup() {
  Serial.begin(9600);
  Serial.print("Starting sketch...\r\n");
  SIM800_PreInit();
  
  // use DHCP
  while(!gprs.init()) {
      delay(1000);
      Serial.print("init error\r\n");
  }
  delay(3000);  
    
  // attempt DHCP
  while(!gprs.join(F("payandgo.o2.co.uk"), F("payandgo"), F("password"))) {
      Serial.println("gprs join network error");
      delay(2000);
  }

  // successful DHCP
  Serial.print("IP Address is ");
  Serial.println(gprs.getIPAddress());

  if(!gprs.connect(TCP, "airsense.fr.openode.io", 80)) {
      Serial.println("connect error");
  }else{
      Serial.println("connect to airsense.fr.openode.io success");
  }

  Serial.println("waiting to fetch...");
  gprs.send(http_cmd, sizeof(http_cmd)-1);
  while (true) {
      int ret = gprs.recv(buffer, sizeof(buffer)-1);
      if (ret <= 0){
          Serial.println("fetch over...");
          break;
      }
      buffer[ret] = '\0';
      Serial.print("Recv: ");
      Serial.print(ret);
      Serial.print(" bytes: ");
      Serial.println(buffer);
  }
  gprs.close();
  gprs.disconnect();
}

void loop(){

}

void SIM800_PreInit(void)
{
    Serial.println("SIM800 PreInit...");
    pinMode(SIM800_POWER_STATUS,INPUT);
    delay(10);
    if(LOW == digitalRead(SIM800_POWER_STATUS))
    {
        if(sim900_send_AT() != true)
        {            
            delay(800);
            digitalWrite(SIM800_POWER_PIN,HIGH);
            delay(200);
            digitalWrite(SIM800_POWER_PIN,LOW);
            delay(2000);
            digitalWrite(SIM800_POWER_PIN,HIGH);
            delay(3000);  
        }
        while(sim900_send_AT() != true);                
        Serial.println("Init O.K!");         
    }
    else
    {
        Serial.println("Power check failed!");  
    }
}
