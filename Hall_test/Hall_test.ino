 #include <Servo.h>
 #include <Wire.h>
 
 Servo frontServo;
 Servo rearServo;
 short frontServoPin = 8; // Servo signal pin
 short rearServoPin = 9; // Servo signal pin
 byte gear = 0;
 byte frontGear[] = {10,10,10,10,10,180,170,170,170,170};
 byte rearGear[] = {10,50,90,130,170,10,50,90,130,170};
 
 unsigned int rpm;
 unsigned int hall1 = 0;
 unsigned int hall2 = 0;
 byte currPos = 0;
 byte currentSensor = 1;
 byte lastSensor = 0;
 byte readRpm = 0;
 unsigned char rpm1 = 0;
 unsigned char rpm2 = 1;
 unsigned char rpm3 = 2;
 unsigned char rpm4 = 3;
 boolean reverse = false;
 boolean stopped = false;
 byte lowerLimit = 50;
 byte upperLimit = 70;
 unsigned int lastShift = 0;
 unsigned int stopDelay = 5000; //milliseconds to wait before assuming rider is stopped

 void setup()
 {
   Wire.begin();
   frontServo.attach(frontServoPin);
   rearServo.attach(rearServoPin);
   frontServo.write(frontGear[gear]);
   rearServo.write(rearGear[gear]);
   attachInterrupt(0, hallHigh1, RISING);
   attachInterrupt(1, hallHigh2, RISING);
   digitalWrite(2, HIGH);
   digitalWrite(3, HIGH);
   rpm = 0;
 }
 
 unsigned char getRpm(){
   if (readRpm >= 4) {
     unsigned int k = 0;
     if(currentSensor = 1) {
       k = hall1 - hall2;
     }
     else{
       k = hall2 - hall1; 
     }
    return (char)(60000/k); 
   }
 }
 
 void storeRpm() {
   unsigned char r = getRpm();
   if (!reverse && r != rpm1 && r != 0 && readRpm >= 4) {
     rpm4 = rpm3;
     rpm3 = rpm2;
     rpm2 = rpm1;
     rpm1 = r;
     Serial.println(rpm1);
   }
 }

 void loop()
 {
   storeRpm();
   if (!reverse && readRpm >= 10 && rpm1 < lowerLimit && rpm2 < lowerLimit && rpm3 < lowerLimit && rpm4 < lowerLimit){
    readRpm = 0;
    downShift(); 
   }
   else if (!reverse && readRpm >= 10 && rpm1 > upperLimit && rpm2 > upperLimit && rpm3 > upperLimit && rpm4 > upperLimit){
    readRpm = 0;
    upShift(); 
   }
   
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(rpm1);              // sends one byte
  Wire.write(reverse);
  Wire.endTransmission();    // stop transmitting   
 }
 
 void upShift() {
  lastShift = millis();
  gear = min(9, gear+1);
  frontServo.write(frontGear[gear]);
  rearServo.write(rearGear[gear]); 
 }
 
 void downShift() {
  lastShift = millis();
  gear = max(0, gear-1);
  frontServo.write(frontGear[gear]);
  rearServo.write(rearGear[gear]); 
 }

 void hallHigh1()
 {
   if (millis() - lastShift > 500) {
    currentSensor = 1;
    stopped = false;
    if (currentSensor == lastSensor) {
      reverse = !reverse;
    }
    readRpm++;
    hall1 = millis();
    lastSensor = 1;
   }
 }
 
  void hallHigh2()
 {  
    if (millis() - lastShift > 500) {
     currentSensor = 2;
     stopped = false;
     if (currentSensor == lastSensor) {
       reverse = !reverse;
     }
     readRpm++;
     hall2 = millis();
     lastSensor = 2;
    }
 }
