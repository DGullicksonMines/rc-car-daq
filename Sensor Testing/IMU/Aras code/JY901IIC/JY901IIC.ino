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

double acc_tot = 0;

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

void setup() 
{
  Serial.begin(115200);
  JY901.StartIIC();

  JY901.GetAcc();
  acc_x_init = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  acc_y_init = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  acc_z_init = (float)JY901.stcAcc.a[2]/32768*16*9.81;
  
  gravity = sqrt(sq(acc_x_init)+sq(acc_y_init)+sq(acc_z_init));
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
  acc_z_corr = acc_z*cos(roll*3.14/180)*cos(pitch*3.14/180) + acc_y*sin(roll*3.14/180) - acc_x*sin(pitch*3.14/180);

  // calcs dt
  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;

  // integrates velocity
  // integrates velocity
  if (millis()>1000) {
    vel_x = vel_x + (acc_x_corr)*(dt/1000);
    vel_y = vel_y + (acc_y_corr)*(dt/1000);
    vel_z = vel_z + (acc_z_corr-gravity)*(dt/1000);
  }
  
  // prints everything
  Serial.println((String)"Acc (m/s^2): "+acc_x+" "+acc_y+" "+acc_z+" | Acc Corr (m/s^2) "+acc_x_corr+" "+acc_y_corr+" "+acc_z_corr+" | RPY (degrees): "+roll+" "+pitch+" "+yaw+" | dt (ms) "+dt+" | Vel (m/s): "+vel_x+" "+vel_y+" "+vel_z);

  delay(0);
  
}
