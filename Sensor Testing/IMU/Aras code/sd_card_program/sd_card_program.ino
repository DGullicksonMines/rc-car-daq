#include <SD.h>
#include <SPI.h>

File myFile;


#include <Wire.h>
#include <JY901.h>

/*
Test on Uno R3.
JY901    UnoR3
SDA <---> SDA
SCL <---> SCL
*/
double acc_x = 0;
double acc_y = 0;
double acc_z = 0;

double roll = 0;
double pitch = 0;
double yaw = 0;

double t_1 = 0;
double t_2 = 0;
double dt = 0;

double gravity = 9.81;

const int buttonPin = 3;
int current_button_state = LOW;
int prev_button_state = LOW;
int recording = 1;

void setup() 
{
  Serial.begin(115200);
  JY901.StartIIC();
  SD.begin(10);
  pinMode(buttonPin, INPUT);
  
  myFile = SD.open("test.txt", O_RDWR | O_CREAT | O_TRUNC);
  myFile.println("acc_x (g),acc_y (g),acc_z (g),roll (degrees),pitch (degrees),yaw (degrees),dt (ms)");
} 

void loop() 
{

  // gets acc data
  JY901.GetAcc();
  acc_x = (float)JY901.stcAcc.a[0]/32768*16;
  acc_y = (float)JY901.stcAcc.a[1]/32768*16;
  acc_z = (float)JY901.stcAcc.a[2]/32768*16;
  
  // gets RPY data
  JY901.GetAngle();
  roll = (float)JY901.stcAngle.Angle[1]/32768*180;
  pitch = (float)JY901.stcAngle.Angle[0]/32768*180;
  yaw = (float)JY901.stcAngle.Angle[2]/32768*180;

  // calcs dt
  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;
  
  prev_button_state = current_button_state;
  current_button_state = digitalRead(buttonPin);

  // prints everything to the txt file
  if (recording == 1){
    myFile.println((String)acc_x+","+acc_y+","+acc_z+","+roll+","+pitch+","+yaw+","+dt);
  }

  if (current_button_state == HIGH && prev_button_state == LOW){
    myFile.close();
    recording = 0;
  }
  
}
