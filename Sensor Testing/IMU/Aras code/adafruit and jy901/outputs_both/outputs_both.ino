#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <JY901.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

double acc_x_BNO = 0, acc_y_BNO = 0, acc_z_BNO = 0;
double acc_x_JY = 0, acc_y_JY = 0, acc_z_JY = 0;

double roll_BNO = 0, pitch_BNO = 0, yaw_BNO = 0;
double roll_JY = 0, pitch_JY = 0, yaw_JY = 0;

double acc_x_corr_BNO = 0, acc_y_corr_BNO = 0, acc_z_corr_BNO = 0;
double acc_x_corr_JY = 0, acc_y_corr_JY = 0, acc_z_corr_JY = 0;
double acc_x_corr = 0, acc_y_corr = 0, acc_z_corr = 0;

double vel_x = 0, vel_y = 0;

double dt = 0, t_1 = 0, t_2 = 0;

void setup(void)
{
  Serial.begin(115200);
  bno.begin();
  JY901.StartIIC();
}

void loop(void)
{
  sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  JY901.GetAngle();
  JY901.GetAcc();

  //BNO values may be out of order and have random negative signs to match JY901
  acc_x_BNO = accelerometerData.acceleration.y;
  acc_y_BNO = -accelerometerData.acceleration.x;
  acc_z_BNO = -accelerometerData.acceleration.z;
  roll_BNO = orientationData.orientation.y;
  pitch_BNO = orientationData.orientation.z;
  yaw_BNO = orientationData.orientation.x;

  roll_JY = (float)JY901.stcAngle.Angle[0]/32768*180;
  pitch_JY = (float)JY901.stcAngle.Angle[1]/32768*180;
  yaw_JY = (float)JY901.stcAngle.Angle[2]/32768*180;
  acc_x_JY = (float)JY901.stcAcc.a[0]/32768*16*9.81;
  acc_y_JY = (float)JY901.stcAcc.a[1]/32768*16*9.81;
  acc_z_JY = (float)JY901.stcAcc.a[2]/32768*16*9.81;

  acc_x_corr_BNO = acc_x_BNO*cos(pitch_BNO*3.14/180) - acc_z_BNO*sin(pitch_BNO*3.14/180);
  acc_y_corr_BNO = acc_y_BNO*cos(roll_BNO*3.14/180) + acc_z_BNO*sin(roll_BNO*3.14/180);
  
  acc_x_corr_JY = acc_x_JY*cos(pitch_JY*3.14/180) - acc_z_JY*sin(pitch_JY*3.14/180);
  acc_y_corr_JY = acc_y_JY*cos(roll_JY*3.14/180) - acc_z_JY*sin(roll_JY*3.14/180);

  t_2 = t_1;
  t_1 = millis();
  dt = t_1 - t_2;

  Serial.println((String)"JY - Acc xyz (m/s^2): "+acc_x_JY+" "+acc_y_JY+" "+acc_z_JY+" | Acc Corr xy (m/s^2) "+acc_x_corr_JY+" "+acc_y_corr_JY+" | RPY (degrees): "+roll_JY+" "+pitch_JY+" "+yaw_JY+" | dt (ms) "+dt);
  Serial.println((String)"BNO - Acc xyz (m/s^2): "+acc_x_BNO+" "+acc_y_BNO+" "+acc_z_BNO+" | Acc Corr xy (m/s^2) "+acc_x_corr_BNO+" "+acc_y_corr_BNO+" | RPY (degrees): "+roll_BNO+" "+pitch_BNO+" "+yaw_BNO);
  Serial.println();  
}
