/*
   adapted from https://playground.arduino.cc/Code/NTPclient
   thanks to Francesco Potortì 2013 - GPLv3 - Revision: 1.13

   Send an NTP packet and wait for the response, return the Unix time
*/

byte packetBuffer[48];

unsigned long NTPUnixTimestamp(void) {
  while (!gprs.connect(UDP, "time.nist.gov", 123)) {
    Serial.println("cannot connect to host, retrying...");
    delay(1000);
  }
  Serial.println("connected to ntp server");

  // Send an NTP request
  memset(packetBuffer, 0, 48);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  if (! gprs.send(packetBuffer)) {
    Serial.println("cannot send NTP request");
    delay(1000);
    gprs.close();
    return 0;       // sending request failed
  }
  Serial.println("NTP request sent");

  int pktLen = 0;             // received packet length
  int totRead = 0;            // total received bytes
  char ntpbuffer[50];

  for (byte i = 0; i < 15; i++) {
    int pktLen = gprs.recv(ntpbuffer, 10);
    Serial.print("got ");
    Serial.println(pktLen);

    // Copy buffer into the pkt
    for (byte j = 0; j < pktLen; j++) {
      packetBuffer[totRead + j] = ntpbuffer[j];
      Serial.println(ntpbuffer[j]);
    }
    totRead += pktLen;

    if (totRead >= 48) {
      // we have enough data now
      Serial.println("ntp packet fully received");
      break;
    }
    delay(150);
  }
  gprs.close();

  Serial.println("parsing packet");

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  Serial.print("Seconds since Jan 1 1900 = ");
  Serial.println(secsSince1900);

  // now convert NTP time into everyday time:
  Serial.print("Unix time = ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSince1900 - seventyYears;
  // print Unix time:
  Serial.println(epoch);

  return epoch;   // convert NTP time to Unix time
}

