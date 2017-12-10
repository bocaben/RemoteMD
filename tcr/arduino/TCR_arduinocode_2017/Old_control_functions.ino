/*/???????????????????????????????????????????      single motor control: ??????????????????????????????????????????????
bool control_right_mot_angle(double  theta_ref)
// PID control to turn the robot
{
  bool ES=0;
  double u;
  double theta_i=ReadEncs(2);
  //Serial.println(theta_i);
  double theta_old;
  double theta_now=theta_i;
   
  unsigned long time_i=millis();              //[msec]
  //Serial.println(time_i);                     //[msec]
  unsigned long time_prev=0;                  //[msec]
  unsigned long time_now=time_i;              //[msec]

  double KP = 3.5;
  double KI = 2.5;
  double KD = 0.2;
  

  double error = theta_ref - theta_i;
  //Serial.println(error);  
  double dt = 0;
  double integral = 0;
  int ss_index=0;
  while (ss_index<20 && ES!=1)
  {
// * * * * * * Control: * * * * * *
    
    double pre_error = error;
    theta_old=theta_now;
    theta_now=ReadEncs(2);
    //Serial.println(theta_now);  
    time_prev=time_now;                               //[msec]
    time_now=millis();                                //[msec]
    //delta_time = time_now - initial_time;
    dt = time_now - time_prev;                     //[msec]
    // Serial.println(dt);
    if (dt < 5)      KD = 0;       

    error = theta_ref - theta_now;
    if (abs(error)<2) ss_index++;             //%%%%%%%%%%%%%%%%  stop condition  %%%%%%%%%%%%%%%%%:
    else ss_index=0;
    
    double P = error*KP;
   // Serial.print("P:");
   // Serial.println(P);
    integral += error * dt/1000;
    if (integral>15/KI) integral=15/KI;
    double I = KI * integral;
    //Serial.print("I:");
    //Serial.println(I);
    double derivative = 1000*(pre_error-error) / dt;
    double D = KD * derivative;
    //Serial.print("D:");
    //Serial.println(D);
    u = P + I + D ;
    
// *****Emergancy stops:*****
    
    if (Serial.available() > 0)
    {
      inputString = Serial.readStringUntil('\n');
      if (inputString.equals("S"))
        {
          u=0;
          ES=1;
          Serial.println("P1"); 
        }     
        Serial.flush();  
    }
       
    //if (digital read of Emergancy botton==0) { u=0; ES=1; }
    readBumper();
    if (BumperStop==1) { u=0; ES=1; }
    //if (digital read of cliff sensor==0) { u=0; ES=1; }
    
    Serial.println(u);
    ControlToPWM(R, (int)u, 255, true);
  }
  
  //Serial.println(time_now-time_i);
  if (ES==1) return 0;
    
  if (ES==0) 
  {
    Serial.println('D');
    return 1;   
  }

}
 


void control_left_mot_angle(double  theta_ref)
// PID control to turn the robot
{
  double u;
  double theta_i=ReadEncs(1);
  Serial.println(theta_i);
  double theta_old;
  double theta_now=theta_i;
   
  unsigned long time_i=millis();              //[msec]
  //Serial.println(time_i);                     //[msec]
  unsigned long time_prev=0;                  //[msec]
  unsigned long time_now=time_i;              //[msec]

  double KP = 2;
  double KI = 5;
  double KD = 0.1;
  

  double error = theta_ref - theta_i;
  //Serial.println(error);  
  double dt = 0;
  double integral = 0;
  int ss_index=0;
  while (ss_index<20)
  {
    double pre_error = error;
    theta_old=theta_now;
    theta_now=ReadEncs(1);
    Serial.println(theta_now);  
    time_prev=time_now;                               //[msec]
    time_now=millis();                                //[msec]
    //delta_time = time_now - initial_time;
    dt = time_now - time_prev;                     //[msec]
    // Serial.println(dt);
    if (dt < 5)      KD = 0;       

    error = theta_ref - theta_now;
    if (abs(error)<0.5) ss_index++;
    else ss_index=0;
    
    double P = error*KP;
   // Serial.print("P:");
   // Serial.println(P);
    integral += error * dt/1000;
    if (integral>15/KI) integral=15/KI;
    double I = KI * integral;
    Serial.print("I:");
    Serial.println(I);
    double derivative = 1000*(pre_error-error) / dt;
    double D = KD * derivative;
    //Serial.print("D:");
    //Serial.println(D);
    u = P + I + D ;
    //Serial.println(u);
    ControlToPWM(L, -(int)u, 255, true);
   
  } 
  Serial.println(time_now-time_i);
}


/*
//??????????????????????????????????                  TURN CONTROL Using Gyro:                ???????????????????????????????????????????????????????????????//

bool control_turn(double  theta_ref)
// PID control to turn the robot
{
  bool ES=0;
  double u;
  double theta_i=GyroAngle;
  //Serial.println(theta_i);
  double theta_old;
  double theta_now=theta_i;
   
  unsigned long time_i=millis();              //[msec]
  //Serial.println(time_i);                     //[msec]
  unsigned long time_prev=0;                  //[msec]
  unsigned long time_now=time_i;              //[msec]

  double KP = 1;
  double KI = 5;
  double KD = 0.1;
  

  double error = theta_ref - theta_i;
  //Serial.println(error);  
  double dt = 0;
  double integral = 0;
  int ss_index=0;
  while (ss_index<20)
  {
    // ***Control:***
    double pre_error = error;
    theta_old=theta_now;
    ReadGyro();
    theta_now=GyroAngle;
    //Serial.println(theta_now);  
    time_prev=time_now;                               //[msec]
    time_now=millis();                                //[msec]
    //delta_time = time_now - initial_time;
    dt = time_now - time_prev;                     //[msec]
    // Serial.println(dt);
    if (dt < 5)      KD = 0;       

    error = theta_ref - theta_now;
    if (abs(error)<0.5) ss_index++;
    else ss_index=0;
    
    double P = error*KP;
   // Serial.print("P:");
   // Serial.println(P);
    integral += error * dt/1000;
    if (integral>15/KI) integral=15/KI;
    double I = KI * integral;
    //Serial.print("I:");
    //Serial.println(I);
    double derivative = 1000*(pre_error-error) / dt;
    double D = KD * derivative;
    //Serial.print("D:");
    //Serial.println(D);
    u = P + I + D ;
    
    // ***Emergancy stops:***
      if (Serial.available() > 0)
      {
        inputString = Serial.readStringUntil('\n');
        if (inputString.equals("S"))
          {
            u=0;
            ES=1;
          }     
          Serial.flush();  
      }
      //if (digital read of Emergancy botton==0) { u=0; ES=1; }
      readBumper();
      if (BumperStop==1) {BumperStop=0; u=0; ES=1; }
      //if (digital read of cliff sensor==0) { u=0; ES=1; }
  
      if (ES==1) 
      {
      Serial.println(time_now-time_i); 
      return 0;
      }
    
    //Serial.println(u);
    ControlToPWM(L, (int)u, 255, true);
    ControlToPWM(R, (int)u, 255, true);
  }
  
  //Serial.println(time_now-time_i);
  
  if (ES==1) return 0;
  if (ES==0) return 1;  
}*/
