#include <math.h>
float x1,yy1,x2,y2,persistence;//to do with noise
int octaves;
// ================================================================================
// Output pin assignments to control the A4988 stepper motor drivers.  The step
// inputs are triggered on a rising edge, with a minimum 1 microsecond HIGH and
// LOW pulse widths.
#define RIGHT_STEP_PIN 2
#define LEFT_STEP_PIN 3

#define RIGHT_DIR_PIN 4
#define LEFT_DIR_PIN 5
#define STEPPER_ENABLE_PIN 8  // active-low (i.e. LOW turns on the drivers)

// ================================================================================
// Import the third-party AccelStepper library.
#include <AccelStepper.h>

// Declare two AccelStepper objects to manage the output timing.
AccelStepper rightWheel(AccelStepper::DRIVER, RIGHT_STEP_PIN, RIGHT_DIR_PIN);
AccelStepper leftWheel(AccelStepper::DRIVER, LEFT_STEP_PIN, LEFT_DIR_PIN);
// ================================================================================

int NORTH = 1;//in the 'face' of the chair
int SOUTH = 0;//behind the chair
int WEST = 0;//to the left of the chair, leftwheel back, rightwheel forward
int EAST = 0;//to the right, leftwheel forward, right wheel back

unsigned long lastMillis = 0;

long rightStep = 0;
long leftStep = 0;

long rightSpeed = 0;
long leftSpeed = 0;//steps per....idk, loop, but given angular speed per.....also idk
long targetSpeed = 50;

long rightAcceleration = 0;
long leftAcceleration = 0;
long targetAcceleration = 5;

void setup() {
  // set up the CNC Shield I/O
  digitalWrite(STEPPER_ENABLE_PIN, HIGH); // initialize drivers in disabled state
  pinMode(STEPPER_ENABLE_PIN, OUTPUT);

  rightWheel.setMaxSpeed(4000.0);
  rightWheel.setAcceleration(8000.0);

  leftWheel.setMaxSpeed(4000.0);
  leftWheel.setAcceleration(8000.0);

  // enable the drivers, the motors will remain constantly energized
  digitalWrite(STEPPER_ENABLE_PIN, LOW);

  
 //persistence affects the degree to which the "finer" noise is seen
 persistence = 0.25;
 //octaves are the number of "layers" of noise that get computed
 octaves = 3;
 Serial.begin(9600);

  //connect();
}
void loop() {
  checkOrientation();
  rightSpeed = rightSpeed + rightAcceleration;
  leftSpeed = leftSpeed + leftAcceleration;
  rightStep = (rightStep + rightSpeed) % 200;//200 steps in a motor
  leftStep = (leftStep + leftSpeed) % 200;
  move(rightStep, leftStep);
}
void checkOrientation(){
  if (SOUTH == 1){
    rightAcceleration = abs(rightAcceleration) * 0.9 + 0.1*targetAcceleration;
    leftAcceleration = -1 * (0.9*abs(leftAcceleration) + 0.1*targetAcceleration);
  } else if (WEST == 1){
    rightAcceleration = abs(rightAcceleration * 0.9) + 0.1*targetAcceleration;
    leftAcceleration = 0;
    leftSpeed = 0;
  } else if (EAST == 1){
    leftAcceleration = abs(leftAcceleration * 0.9) + 0.1*targetAcceleration;
    rightAcceleration = 0;
    rightSpeed = 0;
  } else {
    rightAcceleration = 0;
    leftAcceleration = 0;
    leftSpeed = 0.2*abs(leftSpeed) + 0.8*targetSpeed;
    rightSpeed = 0.2*abs(rightSpeed) + 0.8*targetSpeed;
  }
}
void spiral(){
  for(int i = 0; i < 100; i++ ){
    rightStep += (rightSpeed + i + 10);
    leftStep += (leftSpeed + i);
    move(rightStep, leftStep);
  }
  rightSpeed = 0;
  leftSpeed = 0;
}

/****************************************************************/
/// Call the ``run()`` function for each stepper driver object, which will
/// recalculate speeds and generate step pulses as needed.
void poll_steppers(void)

{
  rightWheel.run();
  leftWheel.run();
}
/// Return true if any one of the drivers are still moving.
bool is_moving(void)
{
  return (rightWheel.isRunning() || leftWheel.isRunning());
}

/// Move a relative displacement at the current speed, blocking until the move is done.
void move(long rightMove, long leftMove)
{
  Serial.print(rightMove + ",");
  Serial.print(leftMove + " ");
  rightWheel.move(rightMove);
  leftWheel.move(leftMove);
  //poll_steppers();
  //delay(5);
  do {
    poll_steppers();
  } while(is_moving());
}
/******************************PERLIN NOISE STUFF HERE******************************************************/
//using the algorithm from http://freespace.virgin.net/hugo.elias/models/m_perlin.html
// thanks to hugo elias
float Noise2(float x, float y)
{
 long noise;
 noise = x + y * 57;
 noise = pow(noise << 13,noise);
 return ( 1.0 - ( long(noise * (noise * noise * 15731L + 789221L) + 1376312589L) & 0x7fffffff) / 1073741824.0);
}

float SmoothNoise2(float x, float y)
{
 float corners, sides, center;
 corners = ( Noise2(x-1, y-1)+Noise2(x+1, y-1)+Noise2(x-1, y+1)+Noise2(x+1, y+1) ) / 16;
 sides   = ( Noise2(x-1, y)  +Noise2(x+1, y)  +Noise2(x, y-1)  +Noise2(x, y+1) ) /  8;
 center  =  Noise2(x, y) / 4;
 return (corners + sides + center);
}

float InterpolatedNoise2(float x, float y)
{
 float v1,v2,v3,v4,i1,i2,fractionX,fractionY;
 long longX,longY;

 longX = long(x);
 fractionX = x - longX;

 longY = long(y);
 fractionY = y - longY;

 v1 = SmoothNoise2(longX, longY);
 v2 = SmoothNoise2(longX + 1, longY);
 v3 = SmoothNoise2(longX, longY + 1);
 v4 = SmoothNoise2(longX + 1, longY + 1);

 i1 = Interpolate(v1 , v2 , fractionX);
 i2 = Interpolate(v3 , v4 , fractionX);

 return(Interpolate(i1 , i2 , fractionY));
}

float Interpolate(float a, float b, float x)
{
 //cosine interpolations
 return(CosineInterpolate(a, b, x));
}

float LinearInterpolate(float a, float b, float x)
{
 return(a*(1-x) + b*x);
}

float CosineInterpolate(float a, float b, float x)
{
 float ft = x * 3.1415927;
 float f = (1 - cos(ft)) * .5;

 return(a*(1-f) + b*f);
}

float PerlinNoise2(float x, float y, float persistance, int octaves)
{
 float frequency, amplitude;
 float total = 0.0;

 for (int i = 0; i <= octaves - 1; i++)
 {
   frequency = pow(2,i);
   amplitude = pow(persistence,i);

   total = total + InterpolatedNoise2(x * frequency, y * frequency) * amplitude;
 }

 return(total);
}
