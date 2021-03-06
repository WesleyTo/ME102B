#include "U8glib.h"
#include <Servo.h>

U8GLIB_ST7920_128X64_4X u8g(9,8,7,6);
const float pi = 3.14;
char output3[4] = "000";
char output2[3] = "00";
char gearOutput1[2] = "0";
char gearOutput2[2] = "0";
byte centerCircle[] = {64, 72};
byte maxRPMtoDisplay = 120;
float increment = 125.47/maxRPMtoDisplay;
int radius = 71;
float dotVector[] = {0-centerCircle[0], 42-centerCircle[1]};

 Servo frontServo;
 Servo rearServo;
 short frontServoPin = 10; // Servo signal pin
 short rearServoPin = 5; // Servo signal pin
 byte gear = 0;
 byte frontGear[] = {12,12,12,12,12,168,168,168,168,168};
 byte rearGear[] = {12,51,90,129,168,12,51,90,129,168};
 
 unsigned int hall1 = 0;
 unsigned int hall2 = 0;
 byte currentSensor = 1;
 byte lastSensor = 0;
 byte readRpm = 0;
 unsigned char rpm1 = 0;
 unsigned char rpm2 = 1;
 unsigned char rpm3 = 2;
 unsigned char rpm4 = 3;
 boolean reverse = false;
 boolean stopped = false;
 boolean up_shift = true;
 byte alert = 0;
 String alertString = "";
 byte targetRpm = 40;
 byte lowerLimit = targetRpm-10;
 byte upperLimit = targetRpm+10;
 unsigned int lastShift = 0;
 unsigned int lastUpdate = 0;
 unsigned int lastDraw = 0;
 unsigned int drawDelay = 400;
 unsigned int shiftDelay = 1000; //milliseconds to wait after shifting to begin reading RPM again
 unsigned int stopDelay = 4000; //milliseconds to wait before assuming rider is stopped

void draw(void) {
  if (millis() - lastDraw > drawDelay) {
    //===========
    // Tachometer
    //===========
    u8g.setColorIndex(1);
    float theta = ((rpm1*increment)*pi)/180; 
    double cs = cos(theta);
    double ss = sin(theta);
    float rotVector[] = {dotVector[0]*cs - dotVector[1]*ss + centerCircle[0], dotVector[0]*ss + dotVector[1]*cs + centerCircle[1]+1};
    //u8g.drawFilledEllipse(rotVector[0], rotVector[1], 3, 3);//draw a dot that follows the RPM
    u8g.drawLine(63,63,rotVector[0], rotVector[1]);//draw a line that follows the RPM
    u8g.drawCircle(centerCircle[0],centerCircle[1],radius-1);
    u8g.setColorIndex(0);
    u8g.drawBox(53,27,23,11);
    u8g.setFont(u8g_font_helvR08);
    u8g.setColorIndex(1);
    u8g.drawStr(1, 55, "0");
    u8g.drawStr(110, 55, "120");
    u8g.drawStr(53, 36, "RPM");
    //===========
    // Gear Signals
    //===========    
    if (alert) {
      if (up_shift) {     
        u8g.drawTriangle(64, 9, 54, 27, 74, 27);
      }
      else{
        u8g.drawTriangle(64, 27, 54, 9, 74, 9);
      }
     alert--;
    }
    //===========
    // Gear Indicators
    //===========
    u8g.drawStr(2,10,"Front");
    u8g.drawStr(106,10,"Rear");
    gearOutput1[0] = 49+(gear/5);
    gearOutput2[0] = 49+(gear%5);
    u8g.drawStr(12,19,gearOutput1);
    u8g.drawStr(114,19,gearOutput2);
    //===========
    // RPM Readout
    //===========
    u8g.setFont(u8g_font_helvR24);

    if (stopped) {
      u8g.drawBox(40, 37, 49, 27);
      u8g.setColorIndex(0);
      u8g.setFont(u8g_font_helvR08);
      u8g.drawStr(40, 55, "STOPPED");
    }
    else if (reverse) {
      u8g.drawBox(40, 37, 49, 27);
      u8g.setColorIndex(0);
      u8g.setFont(u8g_font_helvR08);
      u8g.drawStr(40, 55, "REVERSE");
    }
    else if (rpm1 > 99 && !reverse) {
      u8g.drawBox(41, 38, 51, 26);
      u8g.setColorIndex(0);
      output3[0] = 48+(rpm1/100);
      output3[1] = 48+((rpm1%100)/10);
      output3[2] = 48+(rpm1%10);
      u8g.drawStr(39, 63, output3);
    }
    else if (rpm1 < 100 && !reverse) {
      u8g.drawBox(46, 38, 36, 26);
      u8g.setColorIndex(0);
      output2[0] = 48+rpm1/10;
      output2[1] = 48+rpm1%10;
      u8g.drawStr(47, 63, output2);
    }
  }
}

void setup(void) {
   Serial.begin(9600);
   u8g.begin();
   u8g.setColorIndex(1); // pixel on draw
   frontServo.attach(frontServoPin);
   rearServo.attach(rearServoPin);
   frontServo.write(frontGear[gear]);
   rearServo.write(rearGear[gear]);
   attachInterrupt(0, hallHigh1, RISING);
   attachInterrupt(1, hallHigh2, RISING);
   digitalWrite(2, HIGH);
   digitalWrite(3, HIGH);
   rpm1 = 0;
}

 unsigned char getRpm(){
    return (char)(30000/abs(hall1-hall2)); 
 }
 
 void storeRpm() {
   unsigned char r = getRpm();
   if (!reverse && r != 0 && readRpm >= 2) {
     rpm4 = rpm3;
     rpm3 = rpm2;
     rpm2 = rpm1;
     rpm1 = r;
   }
 }

void loop(void) {
   if (millis() - lastDraw > drawDelay) {
     u8g.firstPage();
     do {
       draw();
     } while(u8g.nextPage());
     lastDraw = millis();
   }
   if (!stopped && (millis() - lastUpdate > stopDelay)) {
     stopped = true;
     reverse = false;
     rpm1 = 0;
   }
   if (stopped) {
    gear = 0; 
    up_shift = false;
    changeGear();
    Serial.println("Stopped");
   }
   else if (!reverse && readRpm >= 10 && rpm1 < lowerLimit && rpm2 < lowerLimit && rpm3 < lowerLimit && gear != 0){
    gear--;
    up_shift = false;
    changeGear();
   }
   else if (!reverse && readRpm >= 10 && rpm1 > upperLimit && rpm2 > upperLimit && rpm3 > upperLimit && rpm4 > upperLimit && gear != 9){
    gear++;
    up_shift = true;
    changeGear();
  }
}

 void changeGear() {
  alert = 4;
  readRpm = 0;
  lastShift = millis();
  frontServo.write(frontGear[gear]);
  rearServo.write(rearGear[gear]); 
 }

 void hallHigh1()
 {
    currentSensor = 1;
    stopped = false;
    if (currentSensor == lastSensor) {
      reverse = !reverse;
    }
    readRpm++;
    hall1 = millis();
    lastUpdate = hall1;
    lastSensor = 1;
    storeRpm();
 }
 
  void hallHigh2()
 {  
   currentSensor = 2;
   stopped = false;
   if (currentSensor == lastSensor) {
     reverse = !reverse;
   }
   readRpm++;
   hall2 = millis();
   lastUpdate = hall2;
   lastSensor = 2;
   storeRpm();
 }

