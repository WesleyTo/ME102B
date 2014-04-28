// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.

/* 
This is a modified version of BARRAGAN's Sweep code for the Arduino.
It is written for use of 4 servos arranged in 2 mirrored pairs.
This is suitable for testing the Automatic Bike Transmission servos.
*/

#include <Servo.h> 
 
Servo frontServo; // front servo, 0 to 180
Servo frontComp; // front complementary servo, 180 to 0
Servo rearServo;
Servo rearComp;
const int servoDelay = 15; // delay between servo write updates
const int servoHold = 2000; // amount for servos to hold position after finishing a sweep
const byte minPos = 0;
const byte maxPos = 180;
            
 
int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  frontServo.attach(5);
  frontComp.attach(9);
  rearServo.attach(10);
  rearComp.attach(11);
} 
 
 
void loop() 
{ 
  // sweep the front servos forward
  for(pos = minPos; pos < maxPos; pos++){
    //delay(servoDelay); 
    frontServo.write(pos);
    frontComp.write(maxPos-pos);
    delay(servoDelay);
  } 
  delay(servoHold);
  
  // sweep the rear servos forward
  for (pos = minPos; pos < maxPos; pos++) {
    //delay(servoDelay);
    rearServo.write(pos);
    rearServo.write(maxPos-pos);
    delay(servoDelay);    
  }
  delay(servoHold);
  
  // sweep the front servos back
  for(pos = maxPos; pos > minPos; pos--){  
    //delay(servoDelay);     
    frontServo.write(pos);
    frontComp.write(maxPos-pos);
    delay(servoDelay);
  } 
  delay(servoHold);
  
  // sweep the rear servos back
  for (pos = maxPos; pos > minPos; pos--){
    //delay(servoDelay);
    rearServo.write(pos);
    rearServo.write(maxPos-pos);
    delay(servoDelay);    
  }
  delay(servoHold);
} 
