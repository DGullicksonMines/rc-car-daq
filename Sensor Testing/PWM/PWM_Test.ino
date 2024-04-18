#define STPin 3
#define ON_TIME_CONST 2000 // measured in micros
int STValue;

long unsigned int prevTime;

void setup() {
  // Serial.begin(115200);
  pinMode(STPin, OUTPUT);
}



void loop() {
  // for (int i = 0; i <= 100; i++)
  //   STValue = analogWrite(STPin,255/20000*1000);
  //   Serial.println(" Steering: " + String(STValue));

    //delay(1);

  digitalWrite(STPin, HIGH);
  while (micros() < prevTime + ON_TIME_CONST);
  prevTime = micros();
  digitalWrite(STPin, LOW);
  while (micros() < prevTime + (20000 - ON_TIME_CONST));
  prevTime = micros();
}