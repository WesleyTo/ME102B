// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.


#include <Servo.h> 
 
Servo frontServo; // front servo, 0 to 180
Servo frontComp; // front complementary servo, 180 to 0
Servo rearServo;
Servo rearComp;
const int servoDelay = 15; // delay between servo write updates
const int servoHold = 2000; // amount for servos to hold position after finishing a sweep
            
 
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
  // sweep the front servo forward
  for(pos = 0; pos < 180; pos++){
    delay(servoDelay); 
    frontServo.write(pos);
    frontComp.write(180-pos);
    delay(servoDelay);
  } 
  delay(servoHold);
  // sweep the rear servo forward
  for (pos = 0; pos < 180; pos++) {
    delay(servoDelay);
    rearServo.write(pos);
    rearServo.write(180-pos);
    delay(servoDelay);    
  }
  delay(servoHold);
  // sweep the front servo back
  for(pos = 180; pos>=1; pos--){  
    delay(servoDelay);     
    frontServo.write(pos);
    frontComp.write(180-pos);
    delay(servoDelay);
  } 
  delay(servoHold);
  // sweep the rear servo back
  for (pos = 180; pos>=1; pos--){
    delay(servoDelay);
    rearServo.write(pos);
    rearServo.write(180-pos);
    delay(servoDelay);    
  }
  delay(servoHold);
} 
