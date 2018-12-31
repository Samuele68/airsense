#define CONNECTION_TIMEOUT 30000

char command[512];
char numbuff[5];
byte retries = 0;

bool SIM800_connect(void) {
  Serial.println(F("SIM800 init..."));
  
  pinMode(SIM800_POWER_STATUS, INPUT);
  pinMode(SIM800_POWER_PIN, OUTPUT);
  delay(100);
  // first reset the module
  digitalWrite(SIM800_POWER_PIN, LOW);
  delay(2000);
  if (LOW == digitalRead(SIM800_POWER_STATUS)) {
    delay(800);
    digitalWrite(SIM800_POWER_PIN, HIGH);
    delay(200);
    digitalWrite(SIM800_POWER_PIN, LOW);
    delay(2000);
    digitalWrite(SIM800_POWER_PIN, HIGH);
    delay(3000);
    retries = 0;
    while (sim900_send_AT() != true) {
      if (retries >= 20) {
        return false;
      }
      retries++;
    }
    Serial.println(F("power check OK"));
  } else {
    Serial.println(F("power check failed!"));
    return false;
  }
  
  // join network
  retries = 0;
  while (!gprs.init()) {
    if (retries >= 15) {
      return false;
    }
    Serial.println(F("network registration error, retrying..."));
    retries++;
    delay(1000);
  }
  Serial.println(F("registered on the net"));

  // attempt DHCP
  retries = 0;
  while (!gprs.join(F("payandgo.o2.co.uk"), F("payandgo"), F("password"))) {
    if (retries >= 15) {
      Serial.println(F("gprs join network error, reinitialising..."));
      SIM800_connect(); // this may loop forever
    }
    Serial.println(F("gprs join network error, retrying..."));
    retries++;
  }

  // successful DHCP
  Serial.println(F("connected to the Internet!"));
  return true;
}

/*
 * Connects to the server and sends a textual line
 */
bool SIM800_sendLine(char* line) {

  Serial.println(F("connecting to host"));
  delay(200);
  retries = 0;
  while (!gprs.connect(TCP, "airsense.fr.openode.io", 80)) {
    if (retries >= 5) {
      gprs.close();
      // it is possible that we lost connection to the Internet, try resetting the module and starting over
      // if we're lucky, next time it'll work
      SIM800_connect();
      return false;
    }
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

  Serial.println(F("sending..."));
  //Serial.println(command);

  retries = 0;
  while (!gprs.send(command, strlen(command))) {
    if (retries >= 5) {
      gprs.close();
      // try resetting the module and starting over
      SIM800_connect();
      return false;
    }
    Serial.println(F("cannot send data, retrying..."));
    retries ++;
  }
  Serial.println(F("sent, closing connection"));

  delay(1000);
  gprs.close();
  return true;
}

