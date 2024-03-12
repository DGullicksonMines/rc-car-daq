#include <Wire.h>
#include <JY901.h>

double acc_x = 0, acc_y = 0, acc_z = 0;

double acc_x_corr = 0, acc_y_corr = 0, acc_z_corr = 0;

double roll = 0, pitch = 0, yaw = 0;

double t_1 = 0, t_2 = 0, dt = 0;

void setup() 
{
  Serial.begin(115200);
  JY901.StartIIC();
} 

void loop() 
{
  t_2 = t_1;
  t_1 = double(micros())/1000;
  dt = t_1 - t_2;
  
  JY901.GetAngle();
  JY901.GetAcc();
  roll = (float)JY901.stcAngle.Angle[0]/32768*180;
  pitch = (float)JY901.stcAngle.Angle[1]/32768*180;
  yaw = (float)JY901.stcAngle.Angle[2]/32768*180;
  acc_x = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  acc_y = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  acc_z = (float)JY901.stcAcc.a[2]/32768*16*9.81;

  if (acc_z > 0 & pitch > 0){
    pitch = 180 - pitch;
  }
  if (acc_z > 0 & pitch < 0){
    pitch = -180 - pitch;
  }
  
  acc_x_corr = acc_x*cos(pitch*3.14/180) - acc_z*sin(pitch*3.14/180);
  acc_y_corr = acc_y*cos(roll*3.14/180) - acc_z*sin(roll*3.14/180);
  
  Serial.println((String)"Acc (m/s^2): "+acc_x+" "+acc_y+" "+acc_z+" | Acc Corr (m/s^2) "+acc_x_corr+" "+acc_y_corr+" | RPY (degrees): "+roll+" "+pitch+" "+yaw+" | dt (ms) "+dt);
}
