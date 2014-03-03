#include "U8glib.h"
U8GLIB_ST7920_128X64_4X u8g(13, 11, 9, 8);// SPI Com: SCK = en = 13, MOSI = rw = 11, CS = di = 9
//U8GLIB_ST7920_128X64_1X u8g(7, 6, 5, 4, 3, 2, 1, 0, 13, 11, 9);   // 8Bit Com: D0..D7: 8,9,10,11,4,5,6,7 en=18, di=17,rw=16
int rpm = 0;
const float pi = 3.14;
char output[3] = "00";

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here  
  output[0] = 48+(rpm/10);
  output[1] = 48+(rpm%10);
  // 0 = [0,42] 100 = [128,42]
  byte centerCircle[] = {64, 72};
  float increment = 1.2547;
  int radius = 71;
  float dotVector[] = {0-centerCircle[0], 42-centerCircle[1]};
  float theta = ((rpm*increment)*pi)/180; 
  double cs = cos(theta);
  double ss = sin(theta);
  float rotVector[] = {dotVector[0]*cs - dotVector[1]*ss + centerCircle[0], dotVector[0]*ss + dotVector[1]*cs + centerCircle[1]+1};
  u8g.drawCircle(centerCircle[0],centerCircle[1],radius-1);
  u8g.drawFilledEllipse(rotVector[0], rotVector[1], 3, 3);
  u8g.setFont(u8g_font_helvR08);
  u8g.drawStr(53, 36, "RPM");
  u8g.setFont(u8g_font_helvR24);
  u8g.drawStr(47, 63, output);
}

void setup(void) {
  
  // flip screen, if required
  // u8g.setRot180();

  Serial.begin(9600);
  // assign default color value
  u8g.setColorIndex(1); // pixel on draw
}

void loop(void) {
  // picture loop
  u8g.firstPage();
  do {
    draw();
  } while( u8g.nextPage() );
  rpm = (int)random(0,100);
  //rpm++;
  // rebuild the picture after some delay
  delay(1000);
}

