

// Code to run LED based "seven segment" digits controlled by 5 28BYJ-48 stepper motors
// Both the AccelStepper and FastLED libraries use interrupts and will conflict with each other.
// code has been running on an ESP32 (Lolin D32). Will be attempting to use a version of FastLED that
// will eliminate the interrupts.

#include <AccelStepper.h>
#include <FastLED.h>
#define LED_PIN 4


#define BRIGHTNESS          64

#define PIXELS_PER_SEGMENT  3  
#define SEGMENTS_PER_DIGIT  7
#define PIXELS_PER_DIGIT    PIXELS_PER_SEGMENT*SEGMENTS_PER_DIGIT
#define NUM_DIGITS          4
#define NUM_DOTS            2
#define NUM_LEDS            PIXELS_PER_DIGIT*NUM_DIGITS + NUM_DOTS
#define DOT_TOP             PIXELS_PER_DIGIT*NUM_DIGITS
#define DOT_BOTTOM          DOT_TOP + 1

CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// Pixel offset (relative to first pixel in each digit) of the pixels which comprise each segment in the seven-segment display
uint8_t segmentOffsets[SEGMENTS_PER_DIGIT][PIXELS_PER_SEGMENT] = {{0,1,2},{3,4,5},{6,7,8},{9,10,11},{12,13,14},{15,16,17},{18,19,20}};


struct Number {
  uint8_t nSegments;
  uint8_t* segments;
};

#define NUM_LETTERS 26
Number letters[NUM_LETTERS];

//Segments comprising each letter for a seven-segment "alphabet"
uint8_t letA[] = {0,1,2,4,5,6};
uint8_t letB[] = {2,3,4,5,6};
uint8_t letC[] = {0,3,4,5};
uint8_t letD[] = {1,2,3,4,6};
uint8_t letE[] = {0,1,3,4,5,6};
uint8_t letF[] = {0,1,4,5,6};
uint8_t letG[] = {0,2,3,4,5};
uint8_t letH[] = {2,4,5,6};
uint8_t letI[] = {2};
uint8_t letJ[] = {1,2,3,4};
uint8_t letK[] = {0,2,4,5,6};
uint8_t letL[] = {3,4,5};
uint8_t letM[] = {0,2,4,6};
uint8_t letN[] = {2,4,6};
uint8_t letO[] = {2,3,4,6};
uint8_t letP[] = {0,1,4,5,6};
uint8_t letQ[] = {0,1,2,5,6};
uint8_t letR[] = {4,6};
uint8_t letS[] = {0,2,3,5,6};
uint8_t letT[] = {3,4,5,6};
uint8_t letU[] = {1,2,3,4,5};
uint8_t letV[] = {2,3,4};
uint8_t letW[] = {1,3,5,6};
uint8_t letX[] = {1,2,4,5,6};
uint8_t letY[] = {1,2,3,5,6};
uint8_t letZ[] = {0,1,3,4,6};

uint8_t* orderedLetters[NUM_LETTERS] = {letA, letB, letC, letD, letE, letF, letG, letH, letI, letJ, letK, letL, letM, letN, letO, letP, letQ, letR, letS, letT, letU, letV, letW, letX, letY, letZ};
void initializeLetters() {
  for (int i = 0; i < NUM_LETTERS; i++) {
    letters[i].segments = orderedLetters[i];
    letters[i].nSegments = sizeof(orderedLetters[i])/sizeof(orderedLetters[i][0]);
  }  
}

#define NUM_NUMBERS 10
Number numbers[NUM_NUMBERS];

//Lists of segments which comprise each number
uint8_t num0[] = {0,1,2,3,4,5}; 
uint8_t num1[] = {1,2}; 
uint8_t num2[] = {0,1,3,4,6}; 
uint8_t num3[] = {0,1,2,3,6}; 
uint8_t num4[] = {1,2,5,6}; 
uint8_t num5[] = {0,2,3,5,6}; 
uint8_t num6[] = {0,2,3,4,5,6}; 
uint8_t num7[] = {0,1,2}; 
uint8_t num8[] = {0,1,2,3,4,5,6}; 
uint8_t num9[] = {0,1,2,5,6}; 

uint8_t* orderedNumbers[NUM_NUMBERS] = {num0, num1, num2, num3, num4, num5, num6, num7, num8, num9};

void initializeNumbers() {
  for (int i = 0; i < NUM_NUMBERS; i++) {
    numbers[i].segments = orderedNumbers[i];
    numbers[i].nSegments = sizeof(orderedNumbers[i])/sizeof(orderedNumbers[i][0]);
  }  
}

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
  int numSegments = numbers[num].nSegments;
  uint8_t *segments = numbers[num].segments;

  if (clearDigit) fillDigit(digit, CRGB::Black);
  for (int i = 0; i < numSegments; i++) { 
    uint8_t* seg = segmentOffsets[segments[i]];
    for (int j = 0; j < PIXELS_PER_SEGMENT; j++) {
      leds[pixelOffset + seg[j]] = color;
    }
  }
  if (doShow) FastLED.show();
};





#define ROTATION_STEPS 2038 // the number of steps in one revolution of your motor (28BYJ-48)
// Creates stepper instances
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
 
//for Lolin D32 
AccelStepper min2(AccelStepper::FULL4WIRE, 13, 14, 12, 27);
AccelStepper min1(AccelStepper::FULL4WIRE, 26, 33, 25, 32);
AccelStepper dots(AccelStepper::FULL4WIRE, 15, 3 , 2, 1);
AccelStepper hr2(AccelStepper::FULL4WIRE, 16, 5, 17, 18);
AccelStepper hr1(AccelStepper::FULL4WIRE,  19, 22, 21, 23); 

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
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  initializeNumbers();
  initializeLetters();
  delay(2000);

  
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  randomSeed(analogRead(0));
  for (int i = 0; i < NUM_STEPPERS; i++) {
    allSteppers[i]->setMaxSpeed(700.0);
    allSteppers[i]->setAcceleration(800);
    //allSteppers[i]->setSpeed(200);
    allSteppers[i]->setSpeed(500);
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
        leds[k].fadeLightBy(fadeAmount);
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
      allSteppers[curMotor]->setSpeed(500);
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
    EVERY_N_MILLISECONDS( 100 ) { gCount = (gCount + 1) % 10; fadeDigits(); bUpdateDigit = true; }
}
