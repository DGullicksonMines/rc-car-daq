#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

double acc_x = 0, acc_y = 0, acc_z = 0;
double roll = 0, pitch = 0, yaw = 0;

void setup(void)
{
  Serial.begin(115200);
  bno.begin();
}

void loop(void)
{
  sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  //bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  //bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  //bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  //bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);

  //values are out of order to match the JY901 IMU orientation
  acc_x = accelerometerData.acceleration.y;
  acc_y = accelerometerData.acceleration.x;
  acc_z = accelerometerData.acceleration.z;

  roll = orientationData.orientation.y;
  pitch = orientationData.orientation.z;
  yaw = orientationData.orientation.x;

  Serial.println((String) "Acc (m/s^2): "+acc_x+" "+acc_y+" "+acc_z+" RPY (deg): "+roll+" "+pitch+" "+yaw);
}
