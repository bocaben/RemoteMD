#include <L3G.h>           //Gyro library.
#include <LSM303.h>        // compass library.
#include <Wire.h>          // used by Gyro & compass.
//---------------------------------GLOBAL VARIABLES----------------------------------
//defines and variables-anything defined here is Global ,if a variable with an identical name is defined in scope {} within that scope the internal variable will be used.



// POSITION CALCULATION VARIABLES:
double Heading=0;                         // current heding.
double forward=0;                         // set empty var to send the path made forward. STOPPED Dinstance: if the robot is stoped befor it finish his way, the arduino send a variable containing the way the robot maid befor stopped.
String right_angles= "";               // set empty string for odiometry calculations. 
String lefy_angles= "";                // set empty string for odiometry calculations. 

// COMUINICATIN VARIABLES:
#define led 13
#define Delay 100
String inputString = "";                // a String to hold incoming data.
//String outputString = "ER 500 EL 200.56 C 150 AC 70\n";   //just for testing the code.


// GYRO VARIABLES:
L3G gyro;
double old_t=0;
double t=0;
double GyroAngle;

//Encoders variables
#define rsty  30
#define rstx  31
#define SEL2  32
#define SEL1  33
#define EN2   34
#define EN1   35
#define OE    36
#define XY    37

double tics_for_deg=360/67200;
double EX; 
double EY; 
char incomingByte;
double ENC_Left;
double ENC_Right;
//End of Encoders variables

// PWM\motor drivers variables
#define Dir_MotorRIGHTPin 12  // Direction input Pin for right motor driver10
#define PWML_MotorRIGHTPin 11 //look  into driver datasheet
#define PWMH_MotorRIGHTPin 10
#define Dir_MotorLEFTPin 7 // Direction input Pin for left  motor driver
#define PWML_MotorLEFTPin 6
#define PWMH_MotorLEFTPin 5
#define Motorpwr A10
boolean L = 0; // command on the left motor
boolean R = 1; // command on the right motor
double leftmotor = 0;
double rightmotor = 0;

// Bumper Sensor Variables
#define LeftBumper 48
#define FrontBumper 49
#define RightBumper 50
bool BumperStop = 0;

//compass vars
#define filterSamples2 12//for filtering compass data
LSM303 compass;
double compassarray[filterSamples2];//array for filter use
double CompassAngle;

bool turnstart = false;

//end compass vars

//Note that pins 4 and 13 on ArduinoMega have 2*freq=2*490Hz=980Hz
// End of PWM/motor drivers variables


//--------------------------------------functions declaration--------------------------------------------------
void ControlToPWM(boolean motor, int controlSignal, int contMax, boolean driveMode);                  //  send PWM signal to the motors.
void send_message(String message);                                                                    //  send message to the computer.
double ReadEncs(int ind);                                                                             //  reads data from encoder.
void ResetEncs();                                                                                     //  reset encoders angle.
double ReadCompass();
void readBumper();
void ReadGyro();
bool control_forward(double dist_ref);
bool control_turn(double theta_ref);
void control_left_mot_angle(double  theta_ref);
bool control_right_mot_angle(double  theta_ref);
/*  void accelarationRead ();     // reads data from the accelaration sensor.
*/
//--------------------------------setup----------------------------------
void setup()
{
  //.........  Comunication setup:  .........
  Serial.begin(115200);           //computer comunication initialization.

  TCCR3B = TCCR3B & 0b11111000;// makes PWM modulaion frequancy 32kHz
  TCCR3B = TCCR3B  | 0b00000001;
  TCCR4B = TCCR4B & 0b11111000;
  TCCR4B = TCCR4B  | 0b00000001;
  TCCR2B = TCCR2B & 0b11111000;// makes PWM modulaion frequancy 32kHz
  TCCR2B = TCCR2B  | 0b00000001;
  pinMode (led, OUTPUT);        //led outputs for tests.


  //......  Motor drivers Pins setup: ........
  pinMode(Motorpwr, OUTPUT);

  //        right motor:
  pinMode(Dir_MotorRIGHTPin, OUTPUT);
  pinMode(Dir_MotorRIGHTPin, OUTPUT);
  pinMode(PWML_MotorRIGHTPin, OUTPUT);

  //        left  motor:
  pinMode(Dir_MotorLEFTPin, OUTPUT);
  pinMode(PWML_MotorLEFTPin, OUTPUT);
  pinMode(PWMH_MotorLEFTPin, OUTPUT);


  //..........  Encoders Setup  ...............
  DDRA = 0b00000000; // set PORTA (digital 22~29) to inputs 
  
  pinMode(OE, OUTPUT); //Enable 1 pin  
  pinMode(SEL1, OUTPUT); //Select 1 pin 
  pinMode(SEL2, OUTPUT); //Select 2 pin
  pinMode(rstx, OUTPUT); //Reset 1 pin 
  pinMode(rsty, OUTPUT); //Reset 2 pin 
  pinMode(EN1, OUTPUT); //Reset 1 pin
  pinMode(EN2, OUTPUT); //Reset 2 pin
  pinMode(XY, OUTPUT); //Reset 1 pin
  
  digitalWrite(OE,HIGH);
  digitalWrite(XY,LOW);
  digitalWrite(rstx,HIGH);
  digitalWrite(rsty,HIGH);
  digitalWrite(SEL1,LOW);     
  digitalWrite(SEL2,HIGH);
    //X4 counting mode:
  digitalWrite(EN1,HIGH);
  digitalWrite(EN2,LOW);
  ResetEncs();
  
  //..........  Bumper Sensor Setup  ...............
  pinMode(LeftBumper, INPUT);
  pinMode(RightBumper, INPUT);
  pinMode(FrontBumper, INPUT);
  
  // ..........  GYRO Setup:  ...............
  Wire.begin();
  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }
  gyro.enableDefault();
  
  // ..........  compass setup: ............
        
        while (!compass.init()) //if the compass doesn't run the robot won't start
        {
          delay(400);
          Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
          Serial.println("-----------------");
        }

        compass.enableDefault();
        compass.m_min = (LSM303::vector<int16_t>) {   +11,  -1365,   -365};
        compass.m_max = (LSM303::vector<int16_t>){ +4230,   -951,  +3266};

  //parking IR pins setup
         // pinMode(IRsensorPin1, INPUT);
        //  pinMode(IRsensorPin2, INPUT);

  ReadCompass();
  GyroAngle=CompassAngle;
}


//_____________________________________________LOOP:_______________________________________________

void loop() {
  //ReadEncs(1);
  //ReadEncs(2);
  //readBumper();
  //ReadAngle();
  //compass.read();
  //float heading = compass.heading((LSM303::vector<int>){0, 0, 1});
  //Serial.println(heading);
 // delay(100);
 
  if (Serial.available() > 0)
  {
    inputString = Serial.readStringUntil('\n');

    if (inputString.equals("ON"))
    {
      Serial.flush();
      digitalWrite(led, HIGH);
      Serial.println(inputString + "\n");
      
    }

    if (inputString.equals("OFF"))
    {
      Serial.flush();
      digitalWrite(led, LOW);
      Serial.println(inputString + "\n");
      
    }

    if  (inputString.startsWith("RP"))
    {
      Serial.flush();
      Serial.println(inputString + "\n");
      
    }

    if  (inputString.startsWith("SENS"))
    {
      digitalWrite(led, HIGH);
      ENC_Left=ReadEncs(1);
      ENC_Right=ReadEncs(2);
      String ENC_L_STR=String(ENC_Left,2);
      String ENC_R_STR=String(ENC_Right,2);
      String deg_STR=String(Heading,2);          //DEG USING COMPASS
      String forward_STR=String(forward,2);
      //String deg_STR=String(GyroAngle,2);          //DEG USING GYRO
      Serial.println("EL " + ENC_L_STR+ " ER " + ENC_R_STR + " H " + deg_STR + " D " + forward_STR);
      forward=0;
      
      //Serial.println("EL " + ENC_L_STR);
      //Serial.println("ER " + ENC_R_STR);
      //Serial.println("H " + deg_STR);
            
      //Serial.print("ER ");
      //Serial.print(ENC_Right);
    }

    if  (inputString.startsWith("T"))
    {
      digitalWrite(led, LOW);
      
      //Serial.println(inputString+"\n");
      while (inputString[0] == ' ' || inputString[0] == 'T')
      {
        for (int i = 0; i < inputString.length(); i++)
        {
          inputString[i] = inputString[i + 1];
        }
      }
      double theta_ref = inputString.toDouble();
      //Serial.println(theta_ref);
      bool fin=control_turn(theta_ref);
      if (fin==1)  Serial.println("D"); // done
      if (fin==0)  Serial.println("S"); // stoped
      rightmotor = 0;
      leftmotor = 0;
      inputString="";        
    }


    
    if  (inputString.startsWith("F"))
    {
      while (inputString[0] == ' ' || inputString[0] == 'F')
      {
        for (int i = 0; i < inputString.length(); i++)
        {
          inputString[i] = inputString[i + 1];
        }
      }
      double dist = inputString.toDouble();
      //Serial.println(dist);
      bool fin=control_forward(dist);
      if (fin==1)  Serial.println("D"); // done
      if (fin==0)  Serial.println("S"); // stoped
      
      rightmotor = 0;
      leftmotor = 0;
      inputString="";        
    }
    
    Serial.flush();
    inputString = "";
  }
  ControlToPWM(R, (int)rightmotor, 255, true);
  ControlToPWM(L, (int)leftmotor, 255, true);
}


//-------------function defenition---------------
void send_message(String message)
{
  for (int i = 0; message[i - 1] != '\n'; i++)
  { Serial.flush();
    Serial.print(message[i]);
  }
}

















