#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

//Aras notes
//even at 10hz, the code only really updates at 1hz which really sucks. maybe there werent enough sattelites when I was testing but idk...
//Normal arduino uno tx and rx pins are rx=0, tx=1. however, this code only works if i plug rx into 2 and tx into 3. i guess this means it doesnt use tx and rx pins???
SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&mySerial);

char c;

void setup() {
  Serial.begin(9600);
  GPS.begin(9600);

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
