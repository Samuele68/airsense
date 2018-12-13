#define SIM800_POWER_PIN        9
#define SIM800_POWER_STATUS     12

#define CONNECTION_TIMEOUT 30000

char IPaddress[20];
char command[512];
char buffer[512];

bool SIM800_init(void)
{  
  Serial.println("SIM800 init...");
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
    Serial.println("power check OK");
  }
  else {
    Serial.println("power check failed!");
    return false;
  }

  while (!gprs.init()) {
    delay(1000);
    Serial.println("init error, retrying...");
  }
  Serial.println("registered on net");

  return true;
}

bool SIM800_connectInternet(void) {
  // attempt DHCP
  while (!gprs.join(F("payandgo.o2.co.uk"), F("payandgo"), F("password"))) {
    Serial.println("gprs join network error, retrying...");
    delay(2000);
  }

  // successful DHCP
  Serial.print("connected to the Internet! IP Address is ");
  strcpy (IPaddress, gprs.getIPAddress());
  Serial.println(IPaddress);
  return true;
}

bool SIM800_sendStatusLine(char* line) {
  if (!gprs.connect(TCP, "airsense.fr.openode.io", 80)) {
    Serial.println("cannot connect to host");
    return false;
  } else {
    Serial.println("connected to host");
  }
  command[0] = '\0';
  buffer[0] = '\0';
  
  int linelen = strlen(line);
  strcat(command, "POST /airsense/status HTTP/1.0\r\nHost: airsense.fr.openode.io\r\nConnection: close\r\nContent-Length: ");
  char numbuff[5];
  itoa(linelen, numbuff, 10);
  strcat(command, numbuff);
  strcat(command, "\r\nContent-Type: text/plain;charset=UTF-8\r\n\r\n");
  strcat(command, line);
  strcat(command, "\r\n");
  
  Serial.print("sending: ");
  Serial.println(command);

  gprs.send(command, sizeof(command) - 1);
  
  while (true) {
    int ret = gprs.recv(buffer, sizeof(buffer) - 1);
    if (ret <= 0) {
      Serial.println("fetch over...");
      break;
    }
    buffer[ret] = '\0';
    Serial.print("received ");
    Serial.print(ret);
    Serial.print(" bytes: ");
    Serial.println(buffer);
  }
  gprs.close();
  return true;
}

