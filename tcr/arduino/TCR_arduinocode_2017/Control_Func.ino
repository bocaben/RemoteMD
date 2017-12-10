//??????????????????????????????????                  TURN CONTROL:                ???????????????????????????????????????????????????????????????//

bool control_turn(double  theta_ref)
// PID control to turn the robot
{
  bool ES=0;
  double u;
  double theta_i=ReadAngle();
  if ((theta_ref-theta_i)>=180) theta_ref-=360;
  if ((theta_ref-theta_i)<=-180) theta_ref+=360;
  //Serial.println(theta_ref); 
  
  //Serial.println(theta_i);
  double theta_old;
  double theta_now=theta_i;
   
  unsigned long time_i=millis();              //[msec]
  //Serial.println(time_i);                     //[msec]
  unsigned long time_prev=0;                  //[msec]
  unsigned long time_now=time_i;              //[msec]

  double KP = 3;
  double KI = 5.5;
  double KD = 0;
  

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
    //ReadGyro();
    theta_now=ReadAngle();
    if ((theta_ref-theta_now)>=180) theta_ref-=360;
    if ((theta_ref-theta_now)<=-180) theta_ref+=360;
    
    
    //Serial.println(theta_now);  
    time_prev=time_now;                               //[msec]
    time_now=millis();                                //[msec]
    //delta_time = time_now - initial_time;2
    dt = time_now - time_prev;                     //[msec]
   // Serial.println(dt);
    //if (dt < 3)      KD = 0;       

    error = theta_ref - theta_now;
    if (error<-30) error+=360;
    
    if (abs(error)<1) ss_index++;     //stop condiion  
    else ss_index=0;
    
    double P = error*KP;
  //  Serial.print("P:");
  //  Serial.println(P);
    integral += error * dt/1000;
    if (integral>15/KI) integral=15/KI;
    double I = KI * integral;
    //Serial.print("I:");
    //Serial.println(I);
    double derivative = 1000*(pre_error-error) / dt;
    double D = KD * derivative;
  //  Serial.print("D:");
  //  Serial.println(D);
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
      //Serial.println(time_now-time_i); 
      return 0;
      }
    
    //Serial.println(u);
    if (u>120) u=120;
    if (u<-120) u=-120;
    ControlToPWM(L, -(int)u, 255, true);
    ControlToPWM(R, -(int)u, 255, true);
  }
  
  //Serial.println(time_now-time_i);
  
  if (ES==1) return 0;
  if (ES==0) return 1;  
}




//????????????????????????????????????????????????????????            Control Forward             ??????????????????????????????????????????????????????????????????????//
void GO_BACK_XCM(double X)
{
  //Serial.println(X);
  double back= -X; //cm
  double u;  
  const double pi=3.14159;
  const double Wheel_Rad=154.5/(2*10); //[CM]

  double KP = 10 ; 
  double KI = 0.4;
  double KD = 0.1;

  ResetEncs();

  double RWheel_i=ReadEncs(1);
  double LWheel_i=ReadEncs(2);
  double dist_i=((RWheel_i+LWheel_i)/(2))*(pi/180)*Wheel_Rad;
  double dist_old;
  double dist_now=dist_i;
   
  unsigned long time_i=millis();              //[msec]
  //Serial.println(time_i);                     //[msec]
  unsigned long time_prev=0;                  //[msec]
  unsigned long time_now=time_i;              //[msec]
  double error = back - dist_now;
  //Serial.println(error);  
  double dt = 0;
  double integral = 0;
  int ss_index=0;

  while (ss_index<20)
  {
      ReadAngle();
                      // ***Control:***
      double pre_error = error;
      dist_old=dist_now;
      double delta_RWheel=ReadEncs(1)-RWheel_i;
      //Serial.print("delta_RWheel: ");
      //Serial.println(delta_RWheel);
      double delta_LWheel=ReadEncs(2)-LWheel_i;
      //Serial.print("delta_LWheel: ");
      //Serial.println(delta_RWheel);
      dist_now=((delta_RWheel+delta_LWheel)/(2))*(pi/180)*Wheel_Rad;
      //Serial.print("dist_now:");
      //Serial.println(dist_now);  
      time_prev=time_now;                               //[msec]
      time_now=millis();                                //[msec]
      //delta_time = time_now - initial_time;
      dt = time_now - time_prev;                     //[msec]
      // Serial.println(dt);
      
      if (dt < 5)      KD = 0;       
  
      error = back - dist_now;
      //Serial.println(error);
      if (abs(error)<5) ss_index++;
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
      u= P + I + D ;
      
      //Serial.println("u:");
      //Serial.println(u);
      ControlToPWM(R, u, 255, true);
      ControlToPWM(L, -u, 255, true);
  }

}


bool control_forward(double dist_ref)
// PID controler to make the robot go forward.
// The controller preserve the robot heading while going farward. 
{
  //if(dist_ref>40)   dist_ref=1.15*dist_ref;          // empirical calibration for slips, good for ~ 50-100 cm. 
  bool ES=0;                      // Emergency stop flag.
  double u_forward;               // the control power to moove forward.
  double u_dir;                   // the control power compensation made to control the diration.
  
  const double pi=3.14159;
  const double Wheel_Rad=154.5/(2*10); //[CM]

  double KP = 1 ;
  double KI = 5;
  double KD = 0.3;

  if (dist_ref<0)
  {KP = 1.5; KI=0.4; KD=0.1;  }
  ResetEncs();
  if (dist_ref>70)
  {KP = 0.5; KI=2.5; KD=0.15;  }
  ResetEncs();
    
  double RWheel_i=ReadEncs(1);
  double LWheel_i=ReadEncs(2);
  double dist_i=((RWheel_i+LWheel_i)/(2))*(pi/180)*Wheel_Rad;
  double dist_old;
  double dist_now=dist_i;
   
  unsigned long time_i=millis();              //[msec]
  //Serial.println(time_i);                     //[msec]
  unsigned long time_prev=0;                  //[msec]
  unsigned long time_now=time_i;              //[msec]
  double error = dist_ref - dist_now;
  //Serial.println(error);  
  double dt = 0;
  double integral = 0;
  int ss_index=0;


  double KPdir = 1 ;
  double KIdir = 2;
  double KDdir = 0.2;
  double dir_ininitial=ReadAngle();
  double dir_old=dir_ininitial;
  double dir_now=dir_ininitial;
  double error_dir=0;
  double integral_dir = 0;
  while (ss_index<20 && ES==0)
  {
      ReadAngle();
                      // ***Control:***
      double pre_error = error;
      dist_old=dist_now;
      double delta_RWheel=ReadEncs(1)-RWheel_i;
      //Serial.print("delta_RWheel: ");
      //Serial.println(delta_RWheel);
      double delta_LWheel=ReadEncs(2)-LWheel_i;
      //Serial.print("delta_LWheel: ");
      //Serial.println(delta_RWheel);
      dist_now=((delta_RWheel+delta_LWheel)/(2))*(pi/180)*Wheel_Rad;
      //Serial.print("dist_now:");
      //Serial.println(dist_now);  
      time_prev=time_now;                               //[msec]
      time_now=millis();                                //[msec]
      //delta_time = time_now - initial_time;
      dt = time_now - time_prev;                     //[msec]
      // Serial.println(dt);
      
      if (dt < 5)      KD = 0;       
  
      error = dist_ref - dist_now;
      //Serial.println(error);
      if (abs(error)<5) ss_index++;
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
      u_forward = P + I + D ;
      
                  //direction compensation:
      double pre_error_dir = error_dir;
      dir_old=dir_now;
      dir_now=Heading;
      //Serial.print("dir_now:");
      //Serial.println(dir_now);  
      if (dt < 5)      KDdir = 0;       
  
      error_dir = dir_ininitial - dir_now;
      if (error_dir>300) error_dir-=360;
      if (error_dir<-300) error_dir+=360;
      //Serial.println(error_dir);
          
      double Pdir = error_dir*KPdir;
     // Serial.print("Pdir:");
     // Serial.println(Pdir);
      integral_dir += error_dir * dt/1000;
      if (integral_dir>15/KI) integral_dir=15/KI;
      double Idir = KIdir * integral_dir;
      //Serial.print("I:");
      //Serial.println(I);
      double derivative_dir = 1000*(pre_error_dir-error_dir) / dt;
      double Ddir = KDdir * derivative_dir;
      //Serial.print("Ddir:");
      //Serial.println(Ddir);
      u_dir = Pdir + Idir + Ddir ;

      int u_right=(int)u_forward-u_dir;
      int u_left=-(int)u_forward-u_dir;
      
      // ***Emergancy stops:***
      if (Serial.available() > 0)
      {
        inputString = Serial.readStringUntil('\n');
        if (inputString.equals("S"))
          {
            u_right=0;
            u_left=0;
            ES=1;
          }     
          Serial.flush();  
      }
      //if (digital read of Emergancy botton==0) { u=0; ES=1; }
      //if (digital read of cliff sensor==0) { u=0; ES=1; }
      
      readBumper();
      if (BumperStop==1) 
      {
        
        BumperStop=0;
        u_right=0;
        u_left=0;
        ES=1;
      }
      
  
      //Serial.println("u:");
      //Serial.println(u);
      ControlToPWM(R, u_right, 255, true);
      ControlToPWM(L, u_left, 255, true);
  }
  
  //Serial.println(time_now-time_i); 
  if (ES==1) 
  {
    GO_BACK_XCM(0);
    GO_BACK_XCM(5);
    forward=dist_now-5;
    //Serial.print("forward:");
      //Serial.println(forward);
    return 0;
  }
  
  if (ES==0) 
  {
    forward=dist_now;
    return 1;    
  }
}
