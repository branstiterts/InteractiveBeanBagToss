#include <TimerOne.h>
#include "LPD6803.h"

int dataPin = 4;
int clockPin = 5;

int btnPin = 3;

// Initialize our LED strip containing 36 pixels
LPD6803 strip = LPD6803(36, dataPin, clockPin);

volatile int circleStart = 31;

const long initialScoreTime = 5000;

const int minimumDelay = 40;

// Set current state of PIR sensor (scored) and current state of push button (btnChanged)
volatile boolean scored = false;
volatile boolean btnChanged = false;

volatile int btnCount = 0;

// Color definitions to be used later
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

  // Method created to animate lights while the PIR sensor is calibrating and let us know when done
  loadingLights();
  Serial.println("done loading");
  
  // Interrupt 0 on Pin 2 for PIR Sensor and Interrupt 1 on Pin 3 for push button
  attachInterrupt(0, interruptPir, RISING);
  attachInterrupt(1, interruptBtn, RISING);
}

void loop() {
  Serial.println("start of loop()");
  // Pushing button changes color of the lights, default is our rainbow cycle
  // Single push of button will change value of btnCount to 1 which changes color to Red
  // Seeing interruptBtn()
  if(btnCount == 0) {
    rainbow(50);
  } else if (btnCount == 1) {
    solidColor(red);
  } else if (btnCount == 2) {
    solidColor(green);
  } else if (btnCount == 3) {
    solidColor(blue);
  } else if (btnCount == 4) {
    solidColor(pink);
  }
  
  // If button is pushed, btnChanged value becomes true, reset btnChanged back to false
  if(btnChanged) {
    btnChanged = false;
  }
  
  // If the interruptPir() is called, value of scored becomes true, run the score() method to animate lights
  // Reset scored back to false
  if(scored) {
    score();
    scored = false;
  }
  Serial.println("end of loop()");
}

// LED Animation for if PIR is triggered (i.e. someone has scored)
void score() {
  // Figure out which animation to run depending on color on the board (btnCount), if rainbow run flashy animation
  // If btnCount is greater than 0 (board color is not flashy just a solid color) so run that animation
  if(btnCount == 0) {
   scoreNormal(); 
  } else if (btnCount > 0) {
   scoreSolidColor(currentColor); 
  }
  scored = false;
}

// LED animation for when color is on rainbow cycle, very flashy and cool
void scoreNormal() {
 int i;
 // Set outside LED strip to every other pixel being blue and the others are off
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
    
    // LED animation for inner LED strip (the circle)
    // LED chase, one pixel at a time, changing colors each time around (rainbow effect)
    for(i = circleStart; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(color % 96));
      strip.show();
      delay(100);
//      strip.setPixelColor(i, Color(0,0,0));
//      strip.show();
//      delay(100);
      color += 5;  // Change this value to a lower number to get more colors, harder to see color change as it is so little though
    }
    
    // Turn all pixels to black (or off) when animation is done
    for(i = circleStart; i < strip.numPixels(); i++) {
//      strip.setPixelColor(i, Wheel(color % 96));
//      strip.show();
//      delay(100);
      strip.setPixelColor(i, Color(0,0,0));
      strip.show();
      delay(100);
      color += 5;  // May not need this line
    }
  }
  
//  for(i = circleStart; i < strip.numPixels(); i++) {
//   strip.setPixelColor(i, Color(0,0,0)); 
//  }
//  strip.show();
  
  // Finish scoring animation by creating a lock in effect
  // Starts at top of the board in the middle and seperates a pixel at a time
  // One to the right, one to the left and finshes by meeting at the bottom in the middle
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

// LED animation for if the board color is a solid color (i.e. red, green, etc.), plain and simple animation
void scoreSolidColor(unsigned int color) {
  int i, j;
  // Wipe from start of circle, clockwise to fill circle with white
  for(i = circleStart; i < strip.numPixels(); i++) {
   strip.setPixelColor(i, Color(255,255,255)); 
   strip.show();
   delay(100);
  }
  delay(10000);
  // Wipe from end of circle, counter-clockwise to fill circle with original color as before
  for(j = strip.numPixels(); j >= circleStart; j--) {
    strip.setPixelColor(j, color);
    strip.show();
    delay(100);
  }
}

// If PIR Sensor detects movement this interrupt is called
void interruptPir() {
 scored = true; 
}

// If push button is pressed, this interrupt is called
void interruptBtn() {
  if(!btnChanged) {
    Serial.println("In the interrupt");
    btnChanged = true;
    btnCount = (btnCount + 1) % 5; 
  }
}

// Loading animation to wait for PIR sensor to be fully calibrated
void loadingLights() {
  int i;
  unsigned long start = millis();
  while(millis() - start < 60000) {  // Run this for 1 minute
    // LED animation to do a red pixel chase, 3 pixels at a time
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
  
  // Once the minute is up, let the players know by lighting outside of board as green
  for(i = circleStart - 1; i >= 0; i--){
    strip.setPixelColor(i, Color(255,0,0));
    //delay(30); 
  }
  delay(50);
  strip.show();
  delay(2500);
}

// Rainbow cycle LED animation
void rainbow(uint8_t wait) {
  int i, j = 0;
  
  while(j < 96 * 5 && !scored && !btnChanged) {  // 5 cycles of all 96 colors in the wheel
//  for (j=0; j < 96 * 5; j++) {    
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( ((i * 96 / strip.numPixels()) + j) % 96) );
    }
    strip.show(); 
    delay(wait);
    j++;
  }
}

// Sets board to specific solid color depending on value of btnCount
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
