double ReadEncs(int ind)
{ //reading encoders 
  // ind=1 - left motor (X), 2 - right motor(Y) 
  byte MSB=0b00000000;
  byte Second_byte =0b00000000;
  byte Third_byte=0b00000000;
  byte LSB=0b00000000;
  long result=0;
  double DEG;
   
  digitalWrite(OE,HIGH);
  delay(25);
  
  if (ind==1)  digitalWrite(XY,LOW);         //read X (Left motor)
  if (ind==2)  digitalWrite(XY,HIGH);         //read Y (Right motor)

  digitalWrite(SEL1,LOW);     digitalWrite(SEL2,HIGH);     //MSB
  digitalWrite(OE,LOW); //start reading cycle  
  //delay(10);
  MSB = PINA;

   
  digitalWrite(SEL1,HIGH);    digitalWrite(SEL2,HIGH); //2ND Byte
  //delay(10);
  Second_byte = PINA;
  
  digitalWrite(SEL1,LOW);     digitalWrite(SEL2,LOW);   //3RD Byte
  //delay(10);
  Third_byte = PINA;
  
  digitalWrite(SEL1,HIGH);     digitalWrite(SEL2,LOW);   //LSB
  //delay(10);
  LSB = PINA;

  result+=(long)MSB<<24;
  result+=(long)Second_byte<<16;
  result+=(long)Third_byte<<8;
  result+=(long)LSB;

 DEG=(double)result*360/67200;  
 //Serial.println(DEG);
 if (ind==1) return -DEG;
 if (ind==2) return DEG;  
} 


void ResetEncs()
{
    //reset encoders
    digitalWrite(rstx,LOW);
    delay(25);
    digitalWrite(rstx,HIGH);
    
    digitalWrite(rsty,LOW);
    delay(25);
    digitalWrite(rsty,HIGH); 
}

void readBumper()
{
  if (digitalRead(RightBumper)==1) 
  {
    BumperStop=1;
    //Serial.println("RB");
  }
  if (digitalRead(LeftBumper)==1) 
  {
    BumperStop=1;
    //Serial.println("LB");
  }
  if (digitalRead(FrontBumper)==1) 
  {
    BumperStop=1;
    //Serial.println("FB");
  } 
}

double ReadCompass()
{
  compass.read();
  CompassAngle = compass.heading((LSM303::vector<int>){0, 0, -1});
  return CompassAngle;
}

void ReadGyro() 
{
  
  old_t=t;
  t=millis();
  double diffdeg=0;
  gyro.read();  
  if (abs((double)gyro.g.y/1000)>0.1) diffdeg=(double)gyro.g.y/1000;
  double dt=-(t-old_t);
  GyroAngle += diffdeg*dt/100;
  if (GyroAngle>360) GyroAngle=GyroAngle-360;
  if (GyroAngle<0) GyroAngle=GyroAngle+360;
  //Serial.println(GyroAngle);
}

double ReadAngle()
{
  ReadCompass();
  ReadGyro();
  if (GyroAngle>350 && CompassAngle<10 && CompassAngle>0 )
  GyroAngle=GyroAngle-360;
  if (CompassAngle>350 && GyroAngle<10 && GyroAngle>0 )
  CompassAngle=CompassAngle-360;
  
  /*Serial.print("GyroAngle: ");
  Serial.println(GyroAngle);
  Serial.print("CompassAngle: ");
  Serial.println(CompassAngle);
  Serial.print("diff: ");
  Serial.println(GyroAngle-CompassAngle);
  Serial.println("::::::::::::::::________________________________________::::::::::::::::");/**/
  int safty=30; //deg
  double d=GyroAngle-CompassAngle;
  if (abs(GyroAngle-CompassAngle)<=safty || abs(CompassAngle)<=10 || abs(GyroAngle)<=10)
  {
    //Serial.println(GyroAngle-CompassAngle);
    Heading=CompassAngle;
    GyroAngle=CompassAngle;
  }
  
  if (abs(GyroAngle-CompassAngle)>safty && abs(CompassAngle)>10 && abs(GyroAngle)>=10)
  {
    Heading=GyroAngle;
    //Serial.println(GyroAngle-CompassAngle);
  }

  return Heading;
}

