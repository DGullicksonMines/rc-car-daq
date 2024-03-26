#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>


SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&mySerial);

char c;

void setup() {
  Serial.begin(115200);
  GPS.begin(9600);
  GPS.sendCommand("$PMTK251,19200*22");
  mySerial.end();
  GPS.begin(19200);

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  delay(1000);
}

void loop() {

  while (!GPS.newNMEAreceived()) {
    c = GPS.read();
  }

  GPS.parse(GPS.lastNMEA());

  if (GPS.fix) {
    Serial.println((String) "Speed (m/s): "+GPS.speed*0.514444);
  }
}
