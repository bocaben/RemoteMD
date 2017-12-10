/*
The sensor outputs provided by the library are the raw 16-bit values
obtained by concatenating the 8-bit high and low gyro data registers.
They can be converted to units of dps (degrees per second) using the
conversion factors specified in the datasheet for your particular
device and full scale setting (gain).

Example: An L3GD20H gives a gyro X axis reading of 345 with its
default full scale setting of +/- 245 dps. The So specification
in the L3GD20H datasheet (page 10) states a conversion factor of 8.75
mdps/LSB (least significant bit) at this FS setting, so the raw
reading of 345 corresponds to 345 * 8.75 = 3020 mdps = 3.02 dps.
*/

#include <Wire.h>
#include <L3G.h>

L3G gyro;
double old_t=0;
double t=0;
double deg=0;


void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }

  gyro.enableDefault();
}

void loop() {
  
  old_t=t;
  t=millis();
  double diffdeg=0;
  gyro.read();

  //Serial.print("G ");
  //Serial.print("X: ");
  //Serial.print((double)gyro.g.x/1000);
  //Serial.print(" Y: ");
  //Serial.println((double)gyro.g.y/1000);
  //Serial.print(" Z: ");
  //Serial.println((double)gyro.g.z/1000);
  
  if (abs((double)gyro.g.y/1000)>0.1) diffdeg=(double)gyro.g.y/1000;
  double dt=t-old_t;
  deg += diffdeg*dt;
  //Serial.print(" diffdeg: ");
  //Serial.println(diffdeg);
  Serial.print(" dt: ");
  Serial.println(dt);
  //Serial.print(" deg: ");
  //Serial.println(deg);

   
  //delay(100);
}
