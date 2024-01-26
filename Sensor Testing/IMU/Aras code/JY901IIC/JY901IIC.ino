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

double acc_x_prev = 0;
double acc_y_prev = 0;
double acc_z_prev = 0;

double roll = 0;
double pitch = 0;
double yaw = 0;

double t_1 = 0;
double t_2 = 0;
double dt = 0;

double vel_x = 0;
double vel_y = 0;
double vel_z = 0;

double gravity = 9.81;

void setup() 
{
  Serial.begin(115200);
  JY901.StartIIC();
} 

void loop() 
{

  // store old acc data
  acc_x_prev = acc_x;
  acc_y_prev = acc_y;
  acc_z_prev = acc_z;

  // gets acc data
  JY901.GetAcc();
  acc_x = (float)JY901.stcAcc.a[0]/32768*16;
  acc_y = (float)JY901.stcAcc.a[1]/32768*16;
  acc_z = (float)JY901.stcAcc.a[2]/32768*16;
  acc_tot = sqrt(sq(acc_x)+sq(acc_y)+sq(acc_z));
  
  // gets RPY data
  JY901.GetAngle();
  roll = (float)JY901.stcAngle.Angle[1]/32768*180;
  pitch = (float)JY901.stcAngle.Angle[0]/32768*180;
  yaw = (float)JY901.stcAngle.Angle[2]/32768*180;

  // calcs dt
  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;

  // integrates velocity
  if (millis()>10) {
    vel_x = vel_x + gravity*dt*(acc_x-acc_x_prev);
    vel_y = vel_y + gravity*dt*(acc_y-acc_y_prev);
    vel_z = vel_z + gravity*dt*(acc_z-acc_z_prev);
  }
  
  // prints everything
  Serial.println((String)"Acc (g): "+acc_x+" "+acc_y+" "+acc_z+" | Total Acc (g) "+acc_tot+" | RPY (degrees): "+roll+" "+pitch+" "+yaw+" | dt (ms) "+dt+" | Vel (m/s): "+vel_x+" "+vel_y+" "+vel_z);

  delay(0);
  
}
