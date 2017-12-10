void ControlToPWM(boolean motor, int controlSignal, int contMax, boolean driveMode)
/*Converts ControlSignal from contoller to PWM->drives the motors
  motor=1  maans right motor,motor=0 -left. Used for choosing direction with "controlSignal"
  controlSignal - control from a controller.  used   with m"otor" variable to choose rotation direction of the wheel and PWM duty cycle
  contMax - maximum signal(that prevents integrator windup)  that specific controler can give.Used to calculate PWM duty cycle:100% at when controlSignal=max sigbal
  driveMode : switches between drivebreak and drive coast motor modes, implemented for future use (maybe)
  No coast/break mode implemented.*/
{
  double frac = (double)controlSignal / contMax;
  int pwm = abs(255 * frac); //calculate pwm cycle.
  if (abs(255 * frac)<1 & abs(255 * frac)>0)
  {
    pwm = 1; //smallest value
  }
  if (abs(frac) > 1) //shouldn't occur
  {
    pwm = 255;
  }
  if (motor == true) //right motor
  {
    //Serial.write("R ");
    if (controlSignal >= 0) //will set direction
    {
      digitalWrite(Dir_MotorRIGHTPin, HIGH);
      //Serial.write("C+ ");
    }
    else//controSignal<0 -another direction
    {
      digitalWrite(Dir_MotorRIGHTPin, LOW);
    }
    if (driveMode == true) //enable drive break mode
    {
      digitalWrite(PWML_MotorRIGHTPin, HIGH);
      //Serial.write(" DMtrue ");
    }
    else
    {
      analogWrite(PWML_MotorRIGHTPin, pwm);
    }
    analogWrite(PWMH_MotorRIGHTPin, pwm); //both in drivebreak and drive cast
    //Serial.print(pwm);
  }
  else//for left motor
  {
    // Serial.write("L ");
    if (controlSignal >= 0)
    {
      // Serial.write("C+ ");
      digitalWrite(Dir_MotorLEFTPin, HIGH);
    }
    else//controSignal<0
    {
      //Serial.write("C- ");
      digitalWrite(Dir_MotorLEFTPin, LOW);
    }
    if (driveMode == true)
    {
      digitalWrite(PWML_MotorLEFTPin, HIGH);
      // Serial.write(" DMtrue ");
    }
    else
    {
      analogWrite(PWML_MotorLEFTPin, pwm);
      //Serial.write(" DMfalse ");
    }
    analogWrite(PWMH_MotorLEFTPin, pwm);
    //Serial.print(pwm);
  }
}


