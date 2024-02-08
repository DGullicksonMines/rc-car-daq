/*
RPM Test Code - Zander Eggers
2/7/2024
Connect Pin 2 to Hall Effect Sensor, 3 to a push button that pulls down to ground and pin 10 to SD Card Writer
PULLUP RESISTOR ON HALL EFFECT IS NOT REQUIRED
  let my code handle that for you
To Use:
  Run and take data. Press button to stop reading data and give a couple seconds to finish writing if need be.
  Data will be in the form of microseconds since start.
    This has precision of up to 4 microseconds - someone should do some math and see what that means in terms of end RPM calc
    This also means that this code can run for a max of just under 70 minutes
  Do RPM calculations in post
*/

#include <SD.h>

#define HALL 2 // Pin connected to Hall Effect Sensor
#define STOP_BUTTON 3 // Pin connected to Button
#define SD_PIN 10 // Not entirely sure how this works

bool stop = 0;

unsigned long buf1[128], buf2[128];
bool buf1Full = 0;
bool buf2Full = 0;
bool buf1Empty = 1;
bool buf2Empty = 1;
bool newData = 0;

uint8_t writeCount = 0;

File sdCard;

// Hall effect sensor ISR
void H_ISR() {
  static uint8_t i = 0;
  static unsigned long* curBuf = buf1;
  sdCard.println(micros());

}

void StopISR() {
  stop = 1;
}

void setup() {

  Serial.begin(115200);
  SD.begin(SD_PIN);
  // put your setup code here, to run once:
  pinMode(HALL, INPUT_PULLUP);
  pinMode(STOP_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(HALL), H_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(STOP_BUTTON), StopISR, FALLING);

  sdCard = SD.open("RPM.txt", O_RDWR | O_CREAT | O_TRUNC);
}

void loop() {
  if (stop) {
    sdCard.close();
  }
}
