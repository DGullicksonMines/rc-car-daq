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

double acc_x_init = 0;
double acc_y_init = 0;
double acc_z_init = 0;

double acc_x_corr = 0;
double acc_y_corr = 0;
double acc_z_corr = 0;

double roll = 0;
double pitch = 0;
double yaw = 0;

double t_1 = 0;
double t_2 = 0;
double dt = 0;

double vel_x = 0;
double vel_y = 0;
double vel_z = 0;

double gravity = 0;

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


    // store old acc data
  JY901.GetAcc();
  acc_x_init = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  acc_y_init = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  acc_z_init = (float)JY901.stcAcc.a[2]/32768*16*9.81;

  gravity = sqrt(sq(acc_x_init)+sq(acc_y_init)+sq(acc_z_init));
  
  myFile = SD.open("test.txt", O_RDWR | O_CREAT | O_TRUNC);
  myFile.println("dt (ms),time (sec),acc_x (m/s^2),acc_y (m/s^2),acc_z (m/s^2),acc_x_corr (m/s^2),acc_y_corr (m/s^2),acc_z_corr (m/s^2),roll (deg),pitch (deg),yaw (deg), vel_x (m/s), vel_y (m/s), vel_z (m/s)");
} 



void loop() 
{

  // gets RPY data
  JY901.GetAngle();
  roll = (float)JY901.stcAngle.Angle[0]/32768*180;
  pitch = (float)JY901.stcAngle.Angle[1]/32768*180;
  yaw = (float)JY901.stcAngle.Angle[2]/32768*180;
  
  // gets acc data
  JY901.GetAcc();
  acc_x = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  acc_y = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  acc_z = (float)JY901.stcAcc.a[2]/32768*16*9.81;

  acc_x_corr = acc_x*cos(pitch*3.14/180) - acc_z*sin(pitch*3.14/180);
  acc_y_corr = acc_y*cos(roll*3.14/180) - acc_z*sin(roll*3.14/180);
  acc_z_corr = acc_z*cos(roll*3.14/180)*cos(pitch*3.14/180) + acc_y*sin(roll*3.14/180) + acc_x*sin(pitch*3.14/180);

  // calcs dt
  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;

  // integrates velocity
  if (millis()>1000) {
    vel_x = vel_x + (acc_x_corr)*(dt/1000);
    vel_y = vel_y + (acc_y_corr)*(dt/1000);
    vel_z = vel_z + (acc_z_corr-gravity)*(dt/1000);
  }

 
  prev_button_state = current_button_state;
  current_button_state = digitalRead(buttonPin);

  // prints everything to the txt file
  if (recording == 1){
    myFile.println((String)dt+","+t_1/1000+","+acc_x+","+acc_y+","+acc_z+","+acc_x_corr+","+acc_y_corr+","+acc_z_corr+","+roll+","+pitch+","+yaw+","+vel_x+","+vel_y+","+vel_z);
  }

  if (current_button_state == HIGH && prev_button_state == LOW){
    myFile.close();
    recording = 0;
  }
  
}
