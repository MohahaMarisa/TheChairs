// StepperScript - drive three or four stepper motors through a sequence of poses
//
// Written in 2018 by Garth Zeglin <garthz@cmu.edu>.  To the extent possible
// under law, the author has dedicated all copyright and related and neighboring
// rights to this software to the public domain worldwide.  This software is
// distributed without any warranty.  You should have received a copy of the CC0
// Public Domain Dedication along with this software.  If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.

// This program assumes that:

//  1. three or four A4988 stepper motor drivers are connected following the CNC Shield pin conventions.
//  2. the AccelStepper library is installed
//  3. if the fourth A axis stepper is used, jumpers are installed to connect
//     A-DIR to D13 (also SpinDir) and A-STEP to D12 (also SpinEnable).

// ================================================================================
// Output pin assignments to control the A4988 stepper motor drivers.  The step
// inputs are triggered on a rising edge, with a minimum 1 microsecond HIGH and
// LOW pulse widths.
#define X_AXIS_STEP_PIN 2
#define Y_AXIS_STEP_PIN 3
#define Z_AXIS_STEP_PIN 4
#define A_AXIS_STEP_PIN 12     // requires an optional jumper

#define X_AXIS_DIR_PIN 5
#define Y_AXIS_DIR_PIN 6
#define Z_AXIS_DIR_PIN 7
#define A_AXIS_DIR_PIN 13      // requires an optional jumper

#define STEPPER_ENABLE_PIN 8  // active-low (i.e. LOW turns on the drivers)

// ================================================================================
// Import the third-party AccelStepper library.
#include <AccelStepper.h>
#include <Wire.h>

// Declare three AccelStepper objects to manage the output timing.
AccelStepper rightWheel(AccelStepper::DRIVER, X_AXIS_STEP_PIN, X_AXIS_DIR_PIN);
AccelStepper leftWheel(AccelStepper::DRIVER, Y_AXIS_STEP_PIN, Y_AXIS_DIR_PIN);

AccelStepper zaxis(AccelStepper::DRIVER, Z_AXIS_STEP_PIN, Z_AXIS_DIR_PIN);
AccelStepper aaxis(AccelStepper::DRIVER, A_AXIS_STEP_PIN, A_AXIS_DIR_PIN);

// ================================================================================
/// Configure the hardware once after booting up.  This runs once after powering
/// up the board, pressing reset, or connecting to the console serial port.


int angleOfOtherChair = 90; //default set to 'in front'

unsigned long lastMillis = 0;

long rightStep = 0;
long leftStep = 0;

long rightSpeed = 0;
long leftSpeed = 0;//steps per....idk, loop, but given angular speed per.....also idk
long targetSpeed = 50;

long rightAcceleration = 0;
long leftAcceleration = 0;
long targetAcceleration = 5;

long mood = 1; //modulates how 'eager' the chair might be, to be dealt with later

volatile int receivedValue = 0;

boolean readingIR = false;

int stepsPerRevolution = 3200;

void setup(void)
{
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveCallback);
  
  // set up the CNC Shield I/O
  digitalWrite(STEPPER_ENABLE_PIN, HIGH); // initialize drivers in disabled state
  pinMode(STEPPER_ENABLE_PIN, OUTPUT);

  rightWheel.setMaxSpeed(4000.0);
  rightWheel.setAcceleration(8000.0);

  leftWheel.setMaxSpeed(4000.0);
  leftWheel.setAcceleration(8000.0);

  zaxis.setMaxSpeed(4000.0);
  zaxis.setAcceleration(8000.0);

  aaxis.setMaxSpeed(4000.0);
  aaxis.setAcceleration(8000.0);

  // set up the serial port for debugging output
  Serial.begin(115200);

  // enable the drivers, the motors will remain constantly energized
  digitalWrite(STEPPER_ENABLE_PIN, LOW);
}
/****************************************************************/
/// Call the ``run()`` function for each stepper driver object, which will
/// recalculate speeds and generate step pulses as needed.
void poll_steppers(void)

{
  rightWheel.run();
  leftWheel.run();
  zaxis.run();
  aaxis.run();
}
/// Return true if any one of the drivers are still moving.
bool is_moving(void)
{
  return (rightWheel.isRunning() || leftWheel.isRunning() || zaxis.isRunning() || aaxis.isRunning());
}

/// Move a relative displacement at the current speed, blocking until the move is done.
void move(long x, long y)
{
  //Serial.print("Moving to ");
 // Serial.print(x); Serial.print(", "); Serial.print(y); 
  
  rightWheel.move(x);
  leftWheel.move(y);
 
  do {
    poll_steppers();
  } while(is_moving());
}

void loop()
{
  if (!readingIR){
    receivedValue = (receivedValue +180)%360;
  }
  turnToExactAngle(receivedValue);
  Serial.print(" going to ");
  Serial.print(receivedValue);
  long byChance = random(1,11);
  Serial.print(" choosing ");
  int aChoice = (int) byChance;
  switch (aChoice) {
    case 1:
      Serial.print("hihihi");
      hihihi();
      break;
    case 2:
      Serial.print("infinity");
      infinityLoop();
      break;
    case 3:
      Serial.print("fwdDecreasing");
      forwardDecreasing();
      break;
    case 4:
      Serial.print("fwdIncreasing");
      forwardIncreasing();
      break;
    case 5:
      Serial.print("grapevine");
      tentativeGrapevine();//time to look at that
      break;
    case 6:
      Serial.print("arc");
      followArc();
      break; 
    case 7:
      Serial.print("lookAround");
      lookaround();
      break;
    case 8:
      Serial.print("random");
      randomDirection();
      break;
    case 9:
      Serial.print("bigCircle");
      bigCircle();
      break;
    case 10:
      Serial.print("creep");
      creepAlong();
    default:
      Serial.print("creep");
      creepAlong();
      break;         
  }
}
//BASIC MOVES ////////////////////////////////////////////////////
void turnToExactAngle(int angleRecieved){
  int angle = angleRecieved - 90;
  int rDirection = -1;
  if(angleRecieved<90){
    angle = 90 - angleRecieved;
    rDirection = 1;
  }else if (angleRecieved > 270){
    angle = 180 - (angleRecieved - 270);
    rDirection = 1;
  }
  long howFarWheelsGo = angle/360 * 19 * PI;//19 is diameter of two wheel drive
  long exactStep = (howFarWheelsGo/ (6.7 * PI)) * stepsPerRevolution; //6.7 cm diameter for wheels, 200 steps in a motor

  int lDirection = -1 * rDirection;
  move(rDirection * exactStep, lDirection * exactStep);
}
void oneWheelTurnBy(int angleRelative){
  //positive angleRelative means clockwise, as in left wheel moves
  long howFarWheelGoes = angleRelative/360 * 19 * 2 * PI;//19*2 because 19 is now radius
  long exactStep = (howFarWheelGoes/ (6.7 * PI)) * stepsPerRevolution; //6.7 cm diameter for wheels, 200 steps in a motor
  if (angleRelative < 0){
    move(exactStep, 0);
  }else{
    move(0, exactStep);
  }
}

//BIGGER COORDINATED MOVES // //////////////////////////////////////////////////////

void tentativeGrapevine(){
  //half steps that get the other chair roughly always to the side of it
  int howMany = int(random(10,30));
  for(int i = 0; i < howMany; i ++){
    if(i%2 == 1){
      oneWheelTurnBy(180);//degrees clockwise
    }else{
      oneWheelTurnBy(-180);//degrees counterclockwise
    }
    delay(200);
  }
}
void randomDirection(){
  int randomAngle = int(random(0,360));
  turnToExactAngle(randomAngle);
  move(2000,2000);
  delay(2000);
}
void infinityLoop(){
  int howMany = int(random(1,5));
  for(int i = 0; i < howMany; i ++){
    if(i%2 == 1){
      oneWheelTurnBy(360);
    }else{
      oneWheelTurnBy(-360);
    }
    delay(200);
  }
}
void forwardIncreasing(){
  long maxRev = random(5,10);
  for(int i = 0; i < 10; i ++){
     long steps = maxRev*stepsPerRevolution / 10*i;
     move(steps, steps);
     delay(100);
     turnToExactAngle(receivedValue);  
  }
}
void forwardDecreasing(){
  long maxRev = random(5,10);
  long steps = maxRev * stepsPerRevolution;
  for(int i = 10; i > 0; i --){
     move(steps, steps);
     delay(100);
     steps = steps * 0.9;
     turnToExactAngle(receivedValue);  
  }
  delay(maxRev*100);
}
void bigCircle(){
  oneWheelTurnBy(90);
  for(int i = 0; i<180; i++){//180*2 = 360 degrees eventually
    move(400,400);
    oneWheelTurnBy(-2);
  }
  delay(4000);
}
void followArc(){
  for(int i = 0; i < 5; i ++){
    if (receivedValue < 90 and receivedValue > 0){
      long proportion = receivedValue/90;
      move(stepsPerRevolution,stepsPerRevolution*proportion);
    }else if (receivedValue >= 90) {
      long proportion = 90/receivedValue;
      move(stepsPerRevolution*proportion,stepsPerRevolution);
    }
  }
}
void creepAlong(){
  for(int i = 0; i < 15; i ++){
    long stepRandom = random(10,50);
    move(stepRandom, stepRandom);
    if(abs(receivedValue -  90) > 20){
      turnToExactAngle(receivedValue); 
    }else{
      long waitRandom = random(450);
      delay(waitRandom);
    }
  }
}
void lookaround(){
  long turnABit = random(-40,40);
  long waitABit = random(900,3500);
  int repeatABit = int(random(2,6));
  for(int i=0; i<repeatABit; i++){
    turnToExactAngle(90+turnABit);
    delay(waitABit);
    turnToExactAngle(90-turnABit);
    delay(waitABit);
  }
  turnToExactAngle(90);
  delay(waitABit/2);
  if(abs(receivedValue - 90) < 40){
    turnToExactAngle(receivedValue);
    delay(waitABit/2);
    shakeWithExcitement();
  }
}
void shakeWithExcitement(){
  for(int i = 0; i < 10; i++){
    if(i%2 ==1){
       turnToExactAngle(95);
    }else{
      turnToExactAngle(85);
    }
  }
}
void waddle(){
  long stepSize = 300 * random(0.5,1.5);
  for(int i=0; i<10; i++){
    if( i%2==0){
      move(stepSize, 0);
      delay(200);
    }else{
      leftStep = (leftStep + stepSize) % stepsPerRevolution;
      move(0, stepSize);
      delay(200);
    }
  }
}
void hihihi(){//shy step forward and back
  for(int i = 0; i < 5; i++ ){
    long stepSize = random(100,800);//or dependent on mood!!!
    if( i%2==0){
      move(stepSize, stepSize);
      delay(300);
    }else{
      move(-stepSize, -stepSize);
      long waitBeforeSayingHi = random(1000, 5000);
      delay(waitBeforeSayingHi);
    }
  }
}
/****************************************************************/

////// I2C Code to recieve Ints
int receiveCallback(int aCount)
{
  if(aCount == 2){
    receivedValue  = Wire.read() << 8; 
    receivedValue |= Wire.read();
    readingIR = true;
  }
  else
  {
    Serial.print("Unexpected number of bytes received: ");
    Serial.println(aCount);
  }
}
