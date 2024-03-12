#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <JY901.h>
#include <SD.h>
#include <SPI.h>

File myFile;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

double acc_x_BNO = 0, acc_y_BNO = 0, acc_z_BNO = 0;
double acc_x_JY = 0, acc_y_JY = 0, acc_z_JY = 0;

double roll_BNO = 0, pitch_BNO = 0, yaw_BNO = 0;
double roll_JY = 0, pitch_JY = 0, yaw_JY = 0;

double acc_x_corr_BNO = 0, acc_y_corr_BNO = 0, acc_z_corr_BNO = 0;
double acc_x_corr_JY = 0, acc_y_corr_JY = 0, acc_z_corr_JY = 0;
double acc_x_corr = 0, acc_y_corr = 0, acc_z_corr = 0;

double vel_x_BNO = 0, vel_y_BNO = 0;
double vel_x_JY = 0, vel_y_JY = 0;

double dt = 0, t_1 = 0, t_2 = 0;

const int buttonPin = 3;
int current_button_state = LOW;
int prev_button_state = LOW;
int recording = 1;

void setup(void)
{
  Serial.begin(115200);
  
  bno.begin();
  JY901.StartIIC();
  SD.begin(10);
  
  pinMode(buttonPin, INPUT);

  myFile = SD.open("test.txt", O_RDWR | O_CREAT | O_TRUNC);
  myFile.println("dt (ms),time (sec),acc_x_JY (m/s^2),acc_y_JY (m/s^2),acc_z_JY (m/s^2),acc_x_corr_JY (m/s^2),acc_y_corr_JY (m/s^2),roll_JY (deg),pitch_JY (deg),yaw_JY (deg),vel_x_JY (m/s),vel_y_JY (m/s),acc_x_BNO (m/s^2),acc_y_BNO (m/s^2),acc_z_BNO (m/s^2),vel_x_BNO (m/s),vel_y_BNO (m/s)");
}

void loop(void)
{
  sensors_event_t linearAccelData;
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  JY901.GetAngle();
  JY901.GetAcc();

  //BNO values may be out of order and have random negative signs to match JY901
  acc_x_BNO = linearAccelData.acceleration.y;
  acc_y_BNO = linearAccelData.acceleration.x;
  acc_z_BNO = linearAccelData.acceleration.z;

  roll_JY = (float)JY901.stcAngle.Angle[0]/32768*180;
  pitch_JY = (float)JY901.stcAngle.Angle[1]/32768*180;
  yaw_JY = (float)JY901.stcAngle.Angle[2]/32768*180;
  acc_x_JY = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  acc_y_JY = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  acc_z_JY = (float)JY901.stcAcc.a[2]/32768*16*9.81;
  
  acc_x_corr_JY = acc_x_JY*cos(pitch_JY*3.14/180) - acc_z_JY*sin(pitch_JY*3.14/180);
  acc_y_corr_JY = acc_y_JY*cos(roll_JY*3.14/180) - acc_z_JY*sin(roll_JY*3.14/180);

  if (millis() > 1000) {
    vel_x_BNO = vel_x_BNO + acc_x_BNO*dt/1000;
    vel_y_BNO = vel_y_BNO + acc_y_BNO*dt/1000;
    vel_x_JY = vel_x_JY + acc_x_corr_JY*dt/1000;
    vel_y_JY = vel_y_JY + acc_y_corr_JY*dt/1000;
  }
  
  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;

  prev_button_state = current_button_state;
  current_button_state = digitalRead(buttonPin);

  // prints everything to the txt file
if (recording == 1){
    myFile.print(dt);
    myFile.print(",");
    myFile.print(t_1 / 1000);
    myFile.print(",");
    myFile.print(acc_x_JY);
    myFile.print(",");
    myFile.print(acc_y_JY);
    myFile.print(",");
    myFile.print(acc_z_JY);
    myFile.print(",");
    myFile.print(acc_x_corr_JY);
    myFile.print(",");
    myFile.print(acc_y_corr_JY);
    myFile.print(",");
    myFile.print(roll_JY);
    myFile.print(",");
    myFile.print(pitch_JY);
    myFile.print(",");
    myFile.print(yaw_JY);
    myFile.print(",");
    myFile.print(vel_x_JY);
    myFile.print(",");
    myFile.print(vel_y_JY);
    myFile.print(",");
    myFile.print(acc_x_BNO);
    myFile.print(",");
    myFile.print(acc_y_BNO);
    myFile.print(",");
    myFile.print(acc_z_BNO);
    myFile.print(",");
    myFile.print(vel_x_BNO);
    myFile.print(",");
    myFile.println(vel_y_BNO);
  }

  if (current_button_state == HIGH && prev_button_state == LOW){
    myFile.close();
    recording = 0;
  }
}
