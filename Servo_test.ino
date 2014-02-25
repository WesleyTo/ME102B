
#include <Servo.h>
 
Servo myservo;  // create servo object to control a servo
                // a maximum of eight servo objects can be created
short upState = 0;  // stores the state of the upshift button
short downState = 0; // stores the state of the downshift button
short servoPin = 9; // Servo signal pin
short upSwitch = 1; // Upshift switch input pin
short downSwitch = 2; // Downshift switch input pin
short ledPin = 3; // the LED signal pin
int positions[] = {10, 26, 42, 58, 74, 90, 106, 122, 138, 154, 170}; // position of the servo in degrees of rotation
short minSize = 0;  // minimum index of the positions array
short maxSize = 10; // maximum index of the positions array
short pos = 0;      // variable to store the servo positions index
int preMotor = 75; // delays the LED by this value to sync motor movement and LED
int motorDelay = 350; // delay in between motor movements
boolean manual = false; // boolean to store manual control or not
byte curr[16];
unsigned int rpm[10];
byte rpmIndex = 0;
byte currIndex = 0;
boolean wrap = false;
 
void setup()
{
  Serial.begin(9600);
  myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
  pinMode(ledPin, OUTPUT); 
  pinMode(upSwitch, INPUT); 
  pinMode(downSwitch, INPUT);
  digitalWrite(upSwitch, HIGH); //activate pullup resistor
  digitalWrite(downSwitch, HIGH); //activate pullup resistor
  myservo.write(positions[0]);
}

void downShift() {
  if (pos > minSize) {
    Serial.println("Down Shift");
    pos--;
    myservo.write(positions[pos]);
    delay(preMotor);
    digitalWrite(ledPin, HIGH);  
    delay(motorDelay);
  }
  else{
    // error LED
  }
}

void upShift() {
  if (pos < maxSize) {
    Serial.println("Up Shift");
    pos++;
    myservo.write(positions[pos]);
    delay(preMotor);
    digitalWrite(ledPin, HIGH);  
    delay(motorDelay);
  }
  else{
    // error LED
  }
}

void reset() {
  digitalWrite(ledPin, LOW);
  delay(100); 
}

boolean upSignal() {
  return (digitalRead(upSwitch) == LOW);
}

boolean downSignal() {
  return (digitalRead(downSwitch) == LOW);
}

word getRPM() {
  unsigned int retVal = 0;
  if (wrap) {
    for (int i = 0; i < 16; i++) {
      unsigned int r = rpm[i];
      for (int j = 0; j < 16; j++) {
        retVal += (r >> j) & 0x0001;
      }
    }
  }
  else{
    for (int i = 0; i < rpmIndex-1; i++) {
      unsigned int r = rpm[i];
      for (int j = 0; j < 16; j++) {
        retVal += (r >> j) & 0x0001;
      }
    }
  }
  return retVal;
}
 
void loop()
{
  // manual transmission
  if (manual == true) {
    if (upSignal()) {
      upShift();
    }
    else if (downSignal()) {
      downShift();
    }    
  }
  
  //auto transmission
  else{
    curr[currIndex++] = 1; //read input from hall sensor and encode as 0 or 1
    if (currIndex > 15) {
      currIndex = 0;
      word r = 0;
      if (curr[0]) {r |= 0x0001;}
      if (curr[1]) {r |= 0x0002;}
      if (curr[2]) {r |= 0x0004;}
      if (curr[3]) {r |= 0x0008;}
      if (curr[4]) {r |= 0x0010;}
      if (curr[5]) {r |= 0x0020;}
      if (curr[6]) {r |= 0x0040;}
      if (curr[7]) {r |= 0x0080;}
      if (curr[8]) {r |= 0x0100;}
      if (curr[9]) {r |= 0x0200;}
      if (curr[10]) {r |= 0x0400;}
      if (curr[11]) {r |= 0x0800;}
      if (curr[12]) {r |= 0x1000;}
      if (curr[13]) {r |= 0x2000;}
      if (curr[14]) {r |= 0x4000;}
      if (curr[15]) {r |= 0x8000;}
      rpm[rpmIndex] = r;
      rpmIndex++;
      if (rpmIndex > 10) {
        rpmIndex = 0;
        wrap = true;
      }
      word rpm = getRPM();
      Serial.print("RPM ");
      Serial.println(rpm);
      if (rpm >45) {
        upShift();
        wrap = false;
        rpmIndex = 0;
        currIndex = 0;
      }
      else if (rpm < 30){
        downShift();
        wrap = false;
        rpmIndex = 0;
        currIndex = 0; 
      }
    } 
  }
  reset();
}
