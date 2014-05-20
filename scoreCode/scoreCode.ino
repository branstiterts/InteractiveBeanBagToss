#include <TimerOne.h>
#include "LPD6803.h"

int dataPin = 4;
int clockPin = 5;

int btnPin = 3;

LPD6803 strip = LPD6803(36, dataPin, clockPin);

volatile int circleStart = 31;

const long initialScoreTime = 5000;

const int minimumDelay = 40;

volatile boolean scored = false;
volatile boolean btnChanged = false;

volatile int btnCount = 0;

unsigned int red = Color(0, 0, 255);
unsigned int green = Color(255, 0, 0);
unsigned int blue = Color(0, 255, 0);
unsigned int pink = Color(0, 2, 150);
unsigned int currentColor;

void setup() {
  pinMode(btnPin, INPUT);
  
  Serial.begin(9600);
  strip.setCPUmax(90);  // start with 50% CPU usage. up this if the strand flickers or is slow
  strip.begin();

  loadingLights();
  Serial.println("done loading");
  attachInterrupt(0, interruptPir, RISING);
  attachInterrupt(1, interruptBtn, RISING);
}

void loop() {
  Serial.println("start of loop()");
  if(btnCount == 0) {
    rainbow(50);
  } else if (btnCount == 1) {
    solidColor(red);  // RED
  } else if (btnCount == 2) {
    solidColor(green);  // GREEN
  } else if (btnCount == 3) {
    solidColor(blue);  // BLUE
  } else if (btnCount == 4) {
    solidColor(pink);  // PINK
  }
  if(btnChanged) {
    btnChanged = false;
  }
  if(scored) {
    score();
    scored = false;
  }
  Serial.println("end of loop()");
}

void score() {
  if(btnCount == 0) {
   scoreNormal(); 
  } else if (btnCount > 0) {
   scoreSolidColor(currentColor); 
  }
  scored = false;
}

void scoreNormal() {
 int i;
  for(i = 0; i < circleStart; i++) {
    if(i % 2 == 0) {
      strip.setPixelColor(i, Color(0,255,0)); 
    } else {
      strip.setPixelColor(i, Color(0,0,0));
    }
  }
  strip.show();
  
  unsigned long startInner = millis();
  long color = 0;
  while(millis() - startInner < initialScoreTime) {
//    for(i = circleStart; i < strip.numPixels(); i++) {
//      strip.setPixelColor(i, Color(255,255,255));
//    }
//    strip.show();
//    delay(100);
//    for(i = circleStart; i < strip.numPixels(); i++) {
//      strip.setPixelColor(i, Color(0,0,0));
//    }
//    strip.show();
//    delay(100);
    for(i = circleStart; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(color % 96));
      strip.show();
      delay(100);
//      strip.setPixelColor(i, Color(0,0,0));
//      strip.show();
//      delay(100);
      color += 5;
    }
     for(i = circleStart; i < strip.numPixels(); i++) {
//      strip.setPixelColor(i, Wheel(color % 96));
//      strip.show();
//      delay(100);
      strip.setPixelColor(i, Color(0,0,0));
      strip.show();
      delay(100);
      color += 5;
    }
  }
  
  for(i = circleStart; i < strip.numPixels(); i++) {
   strip.setPixelColor(i, Color(0,0,0)); 
  }
  strip.show();
  
  int outerMiddle = circleStart / 2;
  int nextLeds = 0;
  for(i = outerMiddle; i < circleStart; i++) {
    strip.setPixelColor(outerMiddle + nextLeds, Color(0,0,255));
    strip.setPixelColor(outerMiddle - nextLeds, Color(0,0,255));
    strip.show();
    nextLeds++;
    delay(90);
  } 
}

void scoreSolidColor(unsigned int color) {
  int i, j;
  for(i = circleStart; i < strip.numPixels(); i++) {
   strip.setPixelColor(i, Color(255,255,255)); 
   strip.show();
   delay(100);
  }
  delay(10000);
  for(j = strip.numPixels(); j >= circleStart; j--) {
    strip.setPixelColor(j, color);
    strip.show();
    delay(100);
  }
}

void interruptPir() {
 scored = true; 
}

void interruptBtn() {
  if(!btnChanged) {
    Serial.println("In the interrupt");
    btnChanged = true;
    btnCount = (btnCount + 1) % 5; 
  }
}

void loadingLights() {
  int i;
  unsigned long start = millis();
  while(millis() - start < 10000) {
    for(i = circleStart - 3; i >= 0; i -= 3) {
       int a = millis();
       strip.setPixelColor(i, Color(0,0,255));
       strip.setPixelColor(i+1, Color(0,0,255));
       strip.setPixelColor(i+2, Color(0,0,255));
       delay(40);
       strip.show();
       strip.setPixelColor(i, Color(0,0,0));
       strip.setPixelColor(i+1, Color(0,0,0));
       strip.setPixelColor(i+2, Color(0,0,0));
       delay(40);
       strip.show();
    }
  }
  for(i = circleStart - 1; i >= 0; i--){
    strip.setPixelColor(i, Color(255,0,0));
    //delay(30); 
  }
  delay(50);
  strip.show();
  delay(2500);
}

void rainbow(uint8_t wait) {
  int i, j = 0;
  
  while(j < 96 * 5 && !scored && !btnChanged) {
//  for (j=0; j < 96 * 5; j++) {     // 5 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( ((i * 96 / strip.numPixels()) + j) % 96) );
    }
    strip.show();   // write all the pixels out
    delay(wait);
    j++;
  }
}

void solidColor(unsigned int color) {
  int i = 0;
  while(i < strip.numPixels() && !scored && !btnChanged) {
    strip.setPixelColor(i, color);
        currentColor = color;
//    strip.setPixelColor(i, Color(0,2,150));  //PINK COLOR
    i++;
  } 
  strip.show();
  delay(100);
}

unsigned int Color(byte r, byte g, byte b) {
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}

unsigned int Wheel(byte WheelPos)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break; 
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break; 
    case 2:
      b=31- WheelPos % 32;  //blue down 
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break; 
  }
  return(Color(r,g,b));
}
