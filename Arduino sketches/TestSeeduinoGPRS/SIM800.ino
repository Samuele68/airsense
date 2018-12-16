#define CONNECTION_TIMEOUT 30000

char buffer[512];
char command[512];
char numbuff[5];
byte retries = 0;
  
bool SIM800_init(void)
{  
  Serial.println(F("SIM800 init..."));
  pinMode(SIM800_POWER_STATUS, INPUT);
  delay(10);
  if (LOW == digitalRead(SIM800_POWER_STATUS)) {
    if (sim900_send_AT() != true) {
      delay(800);
      digitalWrite(SIM800_POWER_PIN, HIGH);
      delay(200);
      digitalWrite(SIM800_POWER_PIN, LOW);
      delay(2000);
      digitalWrite(SIM800_POWER_PIN, HIGH);
      delay(3000);
    }
    while (sim900_send_AT() != true);
    Serial.println(F("power check OK"));
  }
  else {
    Serial.println(F("power check failed!"));
    return false;
  }

  while (!gprs.init()) {
    delay(1000);
    Serial.println(F("init error, retrying..."));
  }
  Serial.println(F("registered on net"));

  return true;
}

bool SIM800_connectInternet(void) {
  // attempt DHCP
  while (!gprs.join(F("payandgo.o2.co.uk"), F("payandgo"), F("password"))) {
    Serial.println(F("gprs join network error, reinitialising..."));
    gprs.powerUpDown(SIM800_POWER_PIN);
    SIM800_init();
  }

  // successful DHCP
  Serial.print(F("connected to the Internet!"));
  return true;
}

bool SIM800_sendLine(char* line) {

  Serial.println(F("connecting to host"));
  delay(200);
  retries = 0;
  while (!gprs.connect(TCP, F("airsense.fr.openode.io"), 80)) {
    if (retries >= 15) return false;
    Serial.println(F("cannot connect to host, retrying..."));
    retries ++;
    delay(1000);
  }
  
  command[0] = "\0";
  numbuff[0] = "\0";
  strcpy(command, "POST /airsense/data HTTP/1.1\r\nHost: airsense.fr.openode.io\r\nContent-Length: ");
  itoa(strlen(line), numbuff, 10);
  strcat(command, numbuff);
  strcat(command, "\r\nContent-Type: text/plain\r\n\r\n");
  strcat(command, line);
  strcat(command, "\r\n");
  
  Serial.println(F("sending:"));
  Serial.println(command);

  retries = 0;
  while(!gprs.send(command, strlen(command))) {
    if (retries >= 15) {
      gprs.close();
      return false;
    }
    Serial.println(F("cannot send data, retrying..."));
  }

  Serial.println(F("sent, now fetching..."));

  buffer[0] = '\0';
  while (true) {
    int ret = gprs.recv(buffer, sizeof(buffer) - 1);
    if (ret <= 0) {
      Serial.println(F("fetch over..."));
      break;
    }
    buffer[ret] = '\0';
    Serial.print(F("received "));
    Serial.print(ret);
    Serial.print(F(" bytes: "));
    Serial.println(buffer);
  }
  gprs.close();
  return true;
}

