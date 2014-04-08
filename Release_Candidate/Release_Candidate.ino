#include "U8glib.h"
#include <Servo.h>

 U8GLIB_ST7920_128X64_4X u8g(9,8,7,6);
 const float pi = 3.14;
 static char output3[4] = "000";
 static char output2[3] = "00";
 static char gearOutput1[2] = "0";
 static char gearOutput2[2] = "0";
 const byte centerCircle[] = {64, 72};
 const byte maxRPMtoDisplay = 120;
 const float increment = 125.47/maxRPMtoDisplay;
 const int radius = 71;
 const float dotVector[] = {0-centerCircle[0], 42-centerCircle[1]};

 Servo frontServo;
 Servo rearServo;
 const short frontServoPin = 10; // Servo signal pin
 const short rearServoPin = 11; // Servo signal pin
 static byte gear = 0;
 const byte minAngle = 14;
 const byte maxAngle = 166;
 const byte stepAngle = (maxAngle-minAngle)/4;
 const byte frontGear[] = {minAngle,minAngle,minAngle,minAngle,minAngle,maxAngle,maxAngle,maxAngle,maxAngle,maxAngle};
 const byte rearGear[] = {minAngle,minAngle+stepAngle,minAngle+(2*stepAngle),maxAngle-stepAngle,maxAngle,minAngle,minAngle+stepAngle,minAngle+(2*stepAngle),maxAngle-stepAngle,maxAngle};
 
 static unsigned int hall1 = 0;
 static unsigned int hall2 = 0;
 static byte currentSensor = 1;
 static byte lastSensor = 0;
 static byte readRpm = 0;
 static unsigned char rpm1 = 0;
 static unsigned char rpm2 = 1;
 static unsigned char rpm3 = 2;
 static unsigned char rpm4 = 3;
 static boolean reverse = false;
 static boolean stopped = false;
 static boolean up_shift = true;
 static boolean coast = false;
 static byte alert = 0;
 const byte targetRpm = 50;
 const byte lowerLimit = targetRpm-10;
 const byte upperLimit = targetRpm+10;
 static unsigned int lastShift = 0;
 static unsigned int lastUpdate = 0;
 static unsigned int lastDraw = 0;
 const unsigned int drawDelay = 400;
 const unsigned int shiftDelay = 1000; //milliseconds to wait after shifting to begin reading RPM again
 const unsigned int coastDelay = 5000;
 const unsigned int stopDelay = 15000; //milliseconds to wait before assuming rider is stopped

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
    else if (coast) {
      u8g.drawBox(40, 37, 49, 27);
      u8g.setColorIndex(0);
      u8g.setFont(u8g_font_helvR08);
      u8g.drawStr(47, 55, "COAST");
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
   if (!stopped && coast && (millis() - lastUpdate > stopDelay)) {
     stopped = true;
     coast = false;
     reverse = false;
     rpm1 = 0;
     if (gear != 0) {
       gear = 0; 
       up_shift = false;
       changeGear();
     }
   }
   else if (!stopped && !coast && (millis() - lastUpdate > coastDelay)) {
     coast = true;
     reverse = false;
     rpm1 = 0;
     if (gear != 0) {
       gear--; 
       up_shift = false;
       changeGear();
     }
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

void hallHigh1() {
  stopped = false;
  coast = false;
  currentSensor = 1;
  if (currentSensor == lastSensor) {
    reverse = !reverse;
  }
  readRpm++;
  hall1 = millis();
  lastUpdate = hall1;
  lastSensor = 1;
  storeRpm();
}
 
void hallHigh2() {
  stopped = false;  
  coast = false;
  currentSensor = 2;
  if (currentSensor == lastSensor) {
    reverse = !reverse;
  }
  readRpm++;
  hall2 = millis();
  lastUpdate = hall2;
  lastSensor = 2;
  storeRpm();
}
