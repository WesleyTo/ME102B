/*
  Written and developed by 
      Wesley To 
  With help from
      Rachit Nanda
  For ME102B, Spring 2014, UC Berkeley
  Under Professor Homayoon Kazerooni
  
  This software uses a modified version of u8glib that 
  has been optimized for a 64x128 pixel ST7920 display.

  This software is provided as-is, and is intended for 
  educational, private, and/or non-profit use only. 
  Commercial use is strictly prohibited.
  
  Private users and educators are free to make any 
  changes or modifications to the software, and to 
  redistribute it, on the condition that an 
  acknowledgement is given to the original authors 
  of the software.
*/

#include "U8glib.h"
#include <Servo.h>

 //=================
 // Screen Variables, assumes a 64x128 screen
 //=================
 U8GLIB_ST7920_128X64_4X u8g(8,7,12,6);
 const float pi = 3.1416;
 char output3[4] = "000"; // 3 character RPM readout
 char output2[3] = "00"; // 2 character RPM readout
 char gearOutput1[2] = "0"; // front gear readout
 char gearOutput2[2] = "0"; // rear gear readout
 const byte centerCircle[] = {64, 72};
 const byte maxRPMtoDisplay = 120;
 const float increment = 125.47/maxRPMtoDisplay;
 const int radius = 71; // radius of the tachometer
 const float dotVector[] = {0-centerCircle[0], 42-centerCircle[1]};
 byte alert = 0; // number of times to draw an alert
 unsigned int lastDraw = 0; // stores the last screen update, in milliseconds
 const unsigned int drawDelay = 400; // delay between screen updates
 //=================
 // Servo Variables
 //=================
 Servo frontServo; // front servo, 0 to 180
 Servo frontComp; // front complementary servo, 180 to 0
 Servo rearServo; // rear servo, 0 to 180
 Servo rearComp; // rear complementary servo, 180 to 0
 const short frontServoPin = 10; // Servo signal pin
 const short frontCompPin = 11;  // Complementary servo signal pin
 const short rearServoPin = 5; // Servo signal pin
 const short rearCompPin = 9; // Complementary servo signal pin
 byte gear = 0; // gear number, 0 - 7 for 8 speed bike
 const byte minAngle = 0;
 const byte maxAngle = 180;
 const int mid1 = 100;
 const int mid2 = 160;
 const byte numGears = 4;
 const byte frontGear[] = {minAngle,minAngle,minAngle,minAngle,maxAngle,maxAngle,maxAngle,maxAngle};
 //const byte rearGear[] = {maxAngle,maxAngle-mid2,maxAngle-mid1,minAngle,maxAngle,maxAngle-mid2,maxAngle-mid1,minAngle};
 const byte rearGear[] = {minAngle, mid1, mid2, maxAngle,minAngle, mid1, mid2, maxAngle};
 //=================
 // Hall/RPM Variables
 //=================
 unsigned long hall1 = 0; // stores the last reading for the first hall sensor in milliseconds
 unsigned long hall2 = 0; // stores the last reading for the second hall sensor in milliseconds
 byte currentSensor = 1; // stores which sensor was just read. Determines rever pedaling
 byte lastSensor = 0; // stores which sensor was previously read. Determines rever pedaling
 byte readRpm = 0; // number of RPM reads that have taken place. Some algorithms rely on having multiple RPM readings.
 unsigned int rpm1 = 0; // stores the last RPM reading
 unsigned int rpm2 = 1; // stores the previous RPM reading
 unsigned int rpm3 = 2; // stores an RPM reading
 unsigned int rpm4 = 3; // stores an RPM reading
 boolean reverse = false; // boolean to determine if user is pedaling in reverse
 boolean stopped = false; // boolean to determine if the user has stopped pedaling
 boolean up_shift = true; // boolean to determine which direction a gear change was
 boolean coast = false; // boolean to determine if a user is coasting
 const byte targetRpm = 60; // ideal user cadence
 const byte lowerLimit = targetRpm-10; // lower limit before down-shifting
 const byte upperLimit = targetRpm+10; // upper limit before up-shifting
 unsigned long lastShift = 0; // when the last gear change occurred, in milliseconds
 unsigned long lastUpdate = 0; // stores the time of the last hall sensor reading, in milliseconds
 const unsigned int shiftDelay = 1000; //milliseconds to wait after shifting to begin reading RPM again
 const unsigned int coastDelay = 5000; //milliseconds to wait before assuming rider is coasting
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
    else if (rpm1 > 99) {
      u8g.drawBox(41, 38, 51, 26);
      u8g.setColorIndex(0);
      output3[0] = 48+(rpm1/100);
      output3[1] = 48+((rpm1%100)/10);
      output3[2] = 48+(rpm1%10);
      u8g.drawStr(39, 63, output3);
    }
    else if (rpm1 < 100) {
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
  frontComp.attach(frontCompPin);
  rearComp.attach(rearCompPin);
  frontServo.write(frontGear[gear]);
  frontComp.write(maxAngle - frontGear[gear]);
  rearServo.write(rearGear[gear]);
  rearComp.write(maxAngle - rearGear[gear]);
  attachInterrupt(0, hallHigh1, RISING);
  attachInterrupt(1, hallHigh2, RISING);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
}

unsigned int getRpm(){
  //measures half revolutions, thus, 30000ms = 30s
  return (unsigned int)(30000.0/(hall1-hall2)); 
}
 
void storeRpm() {
  unsigned int r = getRpm();
  if (!reverse && r != 0 && readRpm >= 2) {
    // basic swap algorithm
    rpm4 = rpm3;
    rpm3 = rpm2;
    rpm2 = rpm1;
    rpm1 = r;
  }
}

void loop(void) {
  // if the screen needs to be updated
  if (millis() - lastDraw > drawDelay) {
    u8g.firstPage();
    do {
      draw();
    } while(u8g.nextPage());
    lastDraw = millis();
  }
  // if we think the user has stopped
  if (!stopped && (millis() - lastUpdate > stopDelay)) {
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
  // if we think the user is coasting
  else if (!stopped && !coast && (millis() - lastUpdate > coastDelay)) {
    stopped = false;
    coast = true;
    reverse = false;
    rpm1 = 0;
    if (gear != 0) {
      gear--; 
      up_shift = false;
      changeGear();
    }
  }
  // if the last 3 RPM readings are below the lower threshold, down-shift
  // we use 3 RPMs instead of 4 because the slower revolutions will 
  // take longer to complete compared to 4 fast revolutions
  else if (!reverse && readRpm >= 10 && rpm1 < lowerLimit && rpm2 < lowerLimit && rpm3 < lowerLimit && gear != 0){
    gear--;
    up_shift = false;
    changeGear();
  }
  // if the last 4 RPM readings are above the upper threshold, up-shift
  else if (!reverse && readRpm >= 10 && rpm1 > upperLimit && rpm2 > upperLimit && rpm3 > upperLimit && rpm4 > upperLimit && gear != 9){
    gear++;
    up_shift = true;
    changeGear();
  }
}

void changeGear() {
  gear = min(gear, 7);
  alert = 4;
  readRpm = 0;
  lastShift = millis();
  frontServo.write(frontGear[gear]);
  frontComp.write(maxAngle - frontGear[gear]);
  rearServo.write(rearGear[gear]);
  rearComp.write(maxAngle - rearGear[gear]);
}

void hallHigh1() {
  hall1 = millis();
  lastUpdate = hall1;
  stopped = false;
  coast = false;
  currentSensor = 1;
  if (currentSensor == lastSensor) {
    reverse = !reverse;
  }
  readRpm++;
  lastSensor = 1;
  storeRpm();
}
 
void hallHigh2() {
  hall2 = millis();
  lastUpdate = hall2;
  stopped = false;  
  coast = false;
  currentSensor = 2;
  if (currentSensor == lastSensor) {
    reverse = !reverse;
  }
  readRpm++;
  lastSensor = 2;
  storeRpm();
}

