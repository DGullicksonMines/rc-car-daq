#define HALL 2 // Pin connected to Hall Effect Sensor
#define STOP_BUTTON 3 // Pin connected to Button


bool stop = 0;

unsigned long buf1[128], buf2[128];
bool buf1Full = 0;
bool buf2Full = 0;
bool buf1Empty = 0;
bool buf2Empty = 0;
bool newData = 0;

uint8_t writeCount = 0;

// Hall effect sensor ISR
void H_ISR() {
  static uint8_t i = 0;
  static unsigned long* curBuf = buf1;

  if(!stop) {
    curBuf[i] = micros();

    i++;

    if(i == 128) {
      if(curBuf == buf1 && buf2Empty) {
        curBuf = buf2;
        buf1Full = true;
      } else if (curBuf == buf2 && buf1Empty) {
        curBuf = buf1;
        buf2Full = true;
      }
      i = 0;
    }
  }

}

void StopISR() {
  stop = 1;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(HALL, INPUT_PULLUP);
  pinMode(STOP_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(HALL), H_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(STOP_BUTTON), StopISR, FALLING);
}

void loop() {

  if (buf1Full) {
    
  }
}
