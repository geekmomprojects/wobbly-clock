

// Code to run LED based "seven segment" digits controlled by 5 28BYJ-48 stepper motors
// Both the AccelStepper and FastLED libraries use interrupts and will conflict with each other.
// code has been running on an ESP32 (Lolin D32). Will be attempting to use a version of FastLED that
// will eliminate the interrupts.
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#include <AccelStepper.h>

#define LED_PIN             1
#define BRIGHTNESS          64

#define PIXELS_PER_SEGMENT  3  
#define SEGMENTS_PER_DIGIT  7
#define PIXELS_PER_DIGIT    21 //PIXELS_PER_SEGMENT*SEGMENTS_PER_DIGIT
#define NUM_DIGITS          4
#define NUM_DOTS            2
#define NUM_LEDS            86 //PIXELS_PER_DIGIT*NUM_DIGITS + NUM_DOTS
#define DOT_TOP             84 //PIXELS_PER_DIGIT*NUM_DIGITS
#define DOT_BOTTOM          85 //DOT_TOP + 1

CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// Pixel offset (relative to first pixel in each digit) of the pixels which comprise each segment in the seven-segment display
uint8_t segmentOffsets[SEGMENTS_PER_DIGIT][PIXELS_PER_SEGMENT] = {{0,1,2},{3,4,5},{6,7,8},{9,10,11},{12,13,14},{15,16,17},{18,19,20}};


// Bitwise representation of segments in numbers 0-9
byte numSegments[10] = {B00111111, B00000110 , B01011011 , B01001111 , B01100110 , B01101101 , B01111101 , B00000111 , B01111111 , B01101111};

//First pixel of each digit. Dots are wired afterthe digits
uint8_t digits[4] = {0, PIXELS_PER_DIGIT, 2*PIXELS_PER_DIGIT, 3*PIXELS_PER_DIGIT};

boolean bDrawTop = true;
void switchDots() {
  bDrawTop = !bDrawTop;
}

void drawDots(CHSV color, boolean doShow = false) {
  int led1 = bDrawTop ? DOT_TOP : DOT_BOTTOM;
  int led2 = bDrawTop ? DOT_BOTTOM : DOT_TOP;
  leds[led1] = color;
  leds[led2] = CRGB::Black;
  if (doShow) {
    FastLED.show();
  }
}


void fillDigit(uint8_t digit, CHSV color) {
  uint8_t pixelOffset = digits[digit];
  fill_solid(leds + pixelOffset, PIXELS_PER_DIGIT, color);
}

void fillDigit(uint8_t digit, CRGB color) {
  int pixelOffset = digits[digit];
  fill_solid(leds + pixelOffset, PIXELS_PER_DIGIT, color);
}


void drawNumber(uint8_t digit, uint8_t num, CHSV color, boolean clearDigit = true, boolean doShow = false) {
  if (digit > NUM_DIGITS) digit = 0;  //TBD: better error checking
  if (num > 9) num = 0;
  int pixelOffset = digits[digit];
  if (clearDigit) fillDigit (digit, CRGB::Black);
  byte segments = numSegments[num];
  for (int i = 0; i < SEGMENTS_PER_DIGIT; i++) {
    if (segments & 1) {
      fill_solid(leds + pixelOffset + i*PIXELS_PER_SEGMENT, PIXELS_PER_SEGMENT, color);
    }
    segments >>= 1;
  }
  
  if (doShow) FastLED.show();
};





#define ROTATION_STEPS 2038 // the number of steps in one revolution of your motor (28BYJ-48)
// Creates stepper instances
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence

//for Tensy4.1 
AccelStepper min1(AccelStepper::FULL4WIRE, 13, 15, 14, 16);
AccelStepper hr2(AccelStepper::FULL4WIRE, 29, 31 , 30, 32);
AccelStepper min2(AccelStepper::FULL4WIRE, 17, 19, 18, 20);
AccelStepper dots(AccelStepper::FULL4WIRE, 25, 27, 26, 28);
AccelStepper hr1(AccelStepper::FULL4WIRE,  8, 10, 9, 11); 

/* 
//for Lolin D32 
AccelStepper min2(AccelStepper::FULL4WIRE, 13, 14, 12, 27);
AccelStepper min1(AccelStepper::FULL4WIRE, 26, 33, 25, 32);
AccelStepper dots(AccelStepper::FULL4WIRE, 15, 3 , 2, 1);
AccelStepper hr2(AccelStepper::FULL4WIRE, 16, 5, 17, 18);
AccelStepper hr1(AccelStepper::FULL4WIRE,  19, 22, 21, 23); 
*/
/*
//for WemosD32 with Battery
AccelStepper min2(AccelStepper::FULL4WIRE, 32, 25, 33, 36);
AccelStepper min1(AccelStepper::FULL4WIRE, 27, 12, 14, 13);
AccelStepper dots(AccelStepper::FULL4WIRE, 23, 21 ,22, 19);
AccelStepper hr2(AccelStepper::FULL4WIRE, 18, 17, 5, 16);
AccelStepper hr1(AccelStepper::FULL4WIRE, 2, 8, 15, 7); 
*/
AccelStepper *allSteppers[] = {&hr1, &hr2, &dots, &min1, &min2};
uint8_t NUM_STEPPERS = sizeof(allSteppers)/sizeof(allSteppers[0]);


int endpoints[5] = {100, 100, 100, 100, 100};

int dist_max = 4000;
int dist_min = 1000;


void testLEDs() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
    FastLED.show();
  }
}

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
#endif
  
  FastLED.addLeds<WS2812SERIAL, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  delay(2000);

  
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  randomSeed(analogRead(0));
  for (int i = 0; i < NUM_STEPPERS; i++) {
    allSteppers[i]->setMaxSpeed(900);
    allSteppers[i]->setAcceleration(300);
    //allSteppers[i]->setSpeed(200);
    allSteppers[i]->setSpeed(600);
    //endpoints[i] = random(dist_min, dist_max);
    endpoints[i] = (int) random(500, 2000);
    allSteppers[i]->moveTo(endpoints[i]);
  }



}

void rainbow() {
    // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 20);
}

uint8_t prevDigit(uint8_t curDigit) {
  if (curDigit == 0) return NUM_DIGITS - 1;
  else return (curDigit - 1);
}

void fadeDigits(uint8_t fadeAmount = 64) {
   for (int k = 0; k < (NUM_LEDS - NUM_DOTS); k++) {
      
      if (leds[k].getAverageLight() < 10) {
        leds[k] = CRGB::Black;
      } else {
        leds[k].nscale8(32);
      }
   }
}


uint32_t lastLEDUpdateTime = millis();
int updateInterval = 300;
uint8_t curDigit = 0;
uint8_t gCount = 0;
uint8_t curMotor = 0;
uint8_t bUpdateDigit = true;

boolean doClear = false;
void loop() {

  // Only moving one motor at a time for now...
  if (allSteppers[curMotor]->distanceToGo() == 0) {
    if (endpoints[curMotor] <= 0) {
      
      endpoints[curMotor] = (int) random(500, 2000);
      allSteppers[curMotor]->moveTo(endpoints[curMotor]);
      allSteppers[curMotor]->disableOutputs();
      curMotor = (curMotor + 1) % NUM_STEPPERS;
      allSteppers[curMotor]->enableOutputs();
      
    } else {
      endpoints[curMotor] = 0;
      allSteppers[curMotor]->moveTo(endpoints[curMotor]);
    }
    
  } else {
    allSteppers[curMotor]->run();
  }

   
 
    //TBD:  fix Interrupts from FastLED conflicting with Interrupts from AccelStepper    
    if (bUpdateDigit) {   
      uint8_t curDigit = curMotor;
      if (curDigit != 2) {  //Only draw digit that is currently moving
        if (curDigit > 2) curDigit--;
        drawNumber(curDigit, gCount, CHSV(gHue, 255,255), true, true);
        bUpdateDigit = false;
      }
    } 

    // do some periodic updates
    EVERY_N_MILLISECONDS( 40 ) { gHue = (gHue + 1) % 255;  } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(1) { switchDots(); drawDots(CHSV(gHue, 255, 255), true); }
    EVERY_N_MILLISECONDS( 200 ) { gCount = (gCount + 1) % 10; fadeDigits(); bUpdateDigit = true; }
    
}
