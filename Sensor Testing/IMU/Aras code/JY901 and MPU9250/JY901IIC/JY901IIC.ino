#include <Wire.h>
#include <JY901.h>
#include <FaBo9Axis_MPU9250.h>

FaBo9Axis MPU;

//901 values
double 901_acc_x = 0;
double 901_acc_y = 0;
double 901_acc_z = 0;

double 901_acc_x_init = 0;
double 901_acc_y_init = 0;
double 901_acc_z_init = 0;

double 901_roll = 0;
double 901_pitch = 0;
double 901_yaw = 0;

//9250 values
double 9250_acc_x = 0;
double 9250_acc_y = 0;
double 9250_acc_z = 0;

double 9250_acc_x_init = 0;
double 9250_acc_y_init = 0;
double 9250_acc_z_init = 0;

double 9250_roll = 0;
double 9250_pitch = 0;
double 9250_yaw = 0;

//global values
double acc_x_corr = 0;
double acc_y_corr = 0;
double acc_z_corr = 0;

double vel_x = 0;
double vel_y = 0;

double t_1 = 0;
double t_2 = 0;
double dt = 0;

void setup() 
{
  Serial.begin(115200);
  JY901.StartIIC();
  MPU.begin();
} 

void loop() 
{

  // gets RPY and acc data
  JY901.GetAngle();
  JY901.GetAcc();
  901_roll = (float)JY901.stcAngle.Angle[0]/32768*180;
  901_pitch = (float)JY901.stcAngle.Angle[1]/32768*180;
  901_yaw = (float)JY901.stcAngle.Angle[2]/32768*180;
  901_acc_x = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  901_acc_y = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  901_acc_z = (float)JY901.stcAcc.a[2]/32768*16*9.81;

  MPU.readAccelXYZ(&9250_acc_x,&9250_acc_y,&9250_acc_z);
  
  acc_x_corr = acc_x*cos(pitch*3.14/180) - acc_z*sin(pitch*3.14/180);
  acc_y_corr = acc_y*cos(roll*3.14/180) - acc_z*sin(roll*3.14/180);
  acc_z_corr = -acc_z*cos(roll*3.14/180)*cos(pitch*3.14/180) - acc_y*sin(roll*3.14/180) + acc_x*sin(pitch*3.14/180);

  // integrates velocity
  if (millis()>1000) {
    vel_x = vel_x + (acc_x_corr)*(dt/1000);
    vel_y = vel_y + (acc_y_corr)*(dt/1000);
  }

  // calcs dt
  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;
  
  // prints everything
  Serial.println((String)"Acc_901  (m/s^2): "+acc_x+" "+acc_y+" "+acc_z+" | Acc Corr (m/s^2) "+acc_x_corr+" "+acc_y_corr+" "+acc_z_corr+" | RPY (degrees): "+roll+" "+pitch+" "+yaw+" | dt (ms) "+dt+" | Vel (m/s): "+vel_x+" "+vel_y);
  Serial.println((String)"Acc_9250 (m/s^2): "+9250_acc_x+" "+9250_acc_y+" "+9250_acc_z)
}
