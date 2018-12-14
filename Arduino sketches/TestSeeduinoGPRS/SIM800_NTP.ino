/*
 * adapted from https://playground.arduino.cc/Code/NTPclient
 * thanks to Francesco Potortì 2013 - GPLv3 - Revision: 1.13
 *
 * Send an NTP packet and wait for the response, return the Unix time
*/

unsigned long NTPUnixTimestamp(void)
{
  // Only the first four bytes of an outgoing NTP packet need to be set
  // appropriately, the rest can be whatever.
  const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

  while (!gprs.connect(UDP, "pool.ntp.org", 123)) {
    Serial.println("cannot connect to host, retrying...");
    delay(1000);
  }

  // Send an NTP request
  if (! gprs.send((byte *) & ntpFirstFourBytes, 48))
    return 0;       // sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150;   // poll every this many ms
  const byte maxPoll = 15;    // poll up to this many times
  int pktLen = 0;             // received packet length
  int totRead = 0;            // total received bytes
  byte pkt[48];               // where to store the packet
  char ntpbuffer[50];
  
  for (byte i = 0; i < maxPoll; i++) {
    int pktLen = gprs.recv(ntpbuffer, 48);
    if (pktLen > 48) return 0; // this shouldn't happen!

    // Copy buffer into the pkt
    for (byte j = 0; j < pktLen; j++) {
      pkt[totRead + j] = ntpbuffer[j];
    }
    totRead += pktLen;

    if (totRead >= 48) {
      // we have enough data now
      break;
    }
    delay(pollIntv);
  }
  gprs.close();

  // Read the integer part of sending time
  // Discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  unsigned long time = pkt[useless];  // NTP time
  time = time << 8 | pkt[useless + 1];
  time = time << 8 | pkt[useless + 2];
  time = time << 8 | pkt[useless + 3];

  return time - 2208988800ul;   // convert NTP time to Unix time
}

