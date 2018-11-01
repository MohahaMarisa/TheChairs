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

#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client;

unsigned long lastMillis = 0;

long rightSpeed = 0;
long leftSpeed = 0;
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
  
  
  Bridge.begin();
  Serial.begin(9600);
  client.begin("broker.shiftr.io", net);

  connect();
}

void connect() {
  Serial.print("connecting...");
//  while (!client.connect("arduino", "try", "try")) {//name of client, username and password
//    Serial.print(".");
//  }
  while (!client.connect("amorousChair", "amorousArduino", "amorousChairLoves")) {//name of client, username and password
    Serial.print("CONNECTED TO AMOROUSCHAIR");
  }

  Serial.println("\nconnected!");

  client.subscribe("/amorousLeftMotor");
  client.subscribe("/amorousRightMotor");
   //client.unsubscribe("/example");
}

void loop() {
  client.loop();

  if(!client.connected()) {
    connect();
  }

  // publish a message roughly every second.//REPLACE this with estimates frpm the IR
  if(millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
  move(rightSpeed, leftSpeed);
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
  if (topic == "amorousRightMotor"){
    rightSpeed = payload;
  }else if (topic == "amorousLeftMotor"){
    leftSpeed = payload;
  }
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
  rightWheel.move(rightMove);
  leftWheel.move(leftMove);
  do {
    poll_steppers();
  } while(is_moving());
}

