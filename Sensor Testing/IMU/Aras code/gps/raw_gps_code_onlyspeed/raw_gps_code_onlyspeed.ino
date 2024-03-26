#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2); // arduino uno pins TX - 3, RX - 2
Adafruit_GPS GPS(&mySerial);

char c;

double t_1 = 0, t_2 = 0, dt = 0;

void setup() {
  Serial.begin(115200); // serial monitor runs at 115200 baud
  GPS.begin(9600);
  GPS.sendCommand("$PMTK251,19200*22");
  mySerial.end();
  GPS.begin(19200); // gps runs at 19200 baud

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ); // outputs at 10hz
  delay(1000);
}

void loop() {
  
  t_2 = t_1;
  t_1 = double(micros())/1000;
  dt = t_1 - t_2;

  while (!GPS.newNMEAreceived()) {
    c = GPS.read();
  }

  GPS.parse(GPS.lastNMEA());

  if (GPS.fix) {
    Serial.println((String) "Speed (m/s): "+GPS.speed*0.514444+" | Sattelites: "+GPS.satellites+" | dt: "+dt); // prints velocity in m/s and the number of satellites and dt
  }
  else {
    Serial.println("L bozo");
  }
}
