//****************************************************
//**             LHCreate - 2017                    **
//**               Feynman cube                     **
//**   author : Carl - (c)2017                      **
//**   version : 1.0                                **
//**   date : 16/11/2017                            **
//**                                                **
//**   https://github.com/nostradomus/LHCreate-2017 **
//**                                                **
//****************************************************
// v1.0 : original version for the presentation,  
//        with minor improvements (repetitive code replaced by functions)
/*
                                  +-----+
     +----[PWR]-------------------| USB |--+
     |                            +-----+  |
     |           GND/RST2  [ ] [ ]         |
     |         MOSI2/SCK2  [ ] [ ]  SCL[ ] | 
     |            5V/MISO2 [ ] [ ]  SDA[ ] | 
     |                             AREF[ ] |
     |                              GND[ ] |      5V---+\
     | [ ]N/C                    SCK/13[ ] |      GND--+ |8x8 LED dot-matrix  
     | [ ]v.ref                 MISO/12[O] |----DIN----+ |display module with
     | [ ]RST                   MOSI/11[O]~|----CLK----+ |MAX7219CNG chip 
     | [ ]3V3                        10[O]~|----CS-----+/ 
     | [ ]5v                          9[ ]~| 
     | [ ]GND                         8[ ] | 
     | [ ]GND                              |
     | [ ]Vin                         7[ ] |  
     |                                6[O]~|-----[470R]---------------NEO-DATA-IN--------
     | [ ]A0                          5[O]~|--------------------------SOUND-HORN-------5V
     | [ ]A1                          4[I] |----->|--------->|--------FeynmanArrows---GND    
     | [ ]A2                     INT1/3[ ]~| (2 small-signal, low-drop diodes, integrated in the arrows)
     | [ ]A3                     INT0/2[ ] |
     | [ ]A4      RST SCK MISO     TX>1[ ] |  
     | [ ]A5      [ ] [ ] [ ]      RX<0[ ] | 
     |  ARDUINO   [ ] [ ] [ ]              |
     |  UNO_R3    GND MOSI 5V  ____________/
      \_______________________/
*/

//***************************************************
//**             library section                   **
//***************************************************
#include <Adafruit_NeoPixel.h>           // library for the WS2812B LED's (https://github.com/adafruit/Adafruit_NeoPixel)
#include "LedControl.h"                  // librariy for the 8x8 LED matrix display (https://github.com/wayoda/LedControl)

//***************************************************
//**            constants section                  **
//***************************************************
#define FWversion "v1.0"
#define arrowsDetectPIN 4                // detection when the two arrows are correctly positioned. PD4, or physical pin 6 on DIL-28 ATmega328
#define soundHornPIN 5                   // sound horn output for correct energy. PD5, or physical pin 11 on DIL-28 ATmega328
#define dataPIN 6                        // output pin for the neopixel dataline. PD6, or physical pin 12 on DIL-28 ATmega328p

//***************************************************
//**         global variables section              **
//***************************************************
LedControl lc = LedControl(12,11,10,1);  // DIN-pin, CLK-pin, CS-pin, number of connected display modules
Adafruit_NeoPixel strip = Adafruit_NeoPixel(50, dataPIN, NEO_GRB + NEO_KHZ800); // # of pixels, data-out pin, LED-type
                                         // update the above number with the correct number of pixels as installed in the exhibit
int starPosition;                        // helper variables for the twinkling star animation
uint16_t jStarAnimation;                 //                       "
uint8_t bStarAnimation;                  //                       "
int sStarAnimation;                      //                       "
int starColor;                           //                       "
uint32_t wheeledStarColor;               //                       "
uint32_t green = strip.Color(0, 255, 0); // color preset for the LED-strip (winning)
unsigned long lastAnimationChange;       // timecounter for the animation when playing
int currentAnimationFrame = 1;           // currently shown frame of the animation
unsigned long lastNumberChange;          // timecounter for the numberspinner-animation when playing
bool LastArrowsConnectedState;           // flag to detect a falling/rising edge on the arrows-circuit
unsigned long arrowsConnectedTimestamp;  // timestamp (in millis) when the ball was detected
int beeps;                               // 'one beep a second'-counter

byte animationFrame1[] =
{
   B00000000,  // 
   B00000000,  //
   B00000000,  //
   B00011000,  //   ++
   B00011000,  //   ++
   B00000000,  //
   B00000000,  //
   B00000000   //
};

byte animationFrame2[] =
{
   B00000000,  // 
   B00000000,  //
   B00111100,  //  +--+
   B00100100,  //  |  |
   B00100100,  //  |  |
   B00111100,  //  +--+
   B00000000,  //
   B00000000   //
};

byte animationFrame3[] =
{
   B00000000,  //
   B01111110,  // +----+
   B01000010,  // |    |
   B01000010,  // |    |
   B01000010,  // |    |
   B01000010,  // |    |
   B01111110,  // +----+
   B00000000   //
};

byte animationFrame4[] =
{
   B11111111,  //+------+
   B10000001,  //|      |
   B10000001,  //|      |
   B10000001,  //|      |
   B10000001,  //|      |
   B10000001,  //|      |
   B10000001,  //|      |
   B11111111   //+------+
};

byte one[] =
{
   B00111000,
   B01111000,
   B11011000,
   B00011000,
   B00011000,
   B00011000,
   B01111110,
   B01111110
};

byte two[] =
{
   B00111100,
   B01100110,
   B00000110,
   B00001100,
   B00011000,
   B00110000,
   B01111110,
   B01111110
};

byte three[] =
{
   B00111100,  
   B01100110,
   B00000110,
   B00111100,
   B00000110,
   B01100110,
   B00111100,
   B00000000
};

byte four[] =
{
   B00001100,  
   B00011100,
   B00111100,
   B01101100,
   B11001100,
   B11111111,
   B00001100,
   B00001100
};

byte five[] =
{
   B01111110,  
   B01100000,
   B01100000,
   B01111100,
   B00000110,
   B01100110,
   B00111100,
   B00000000
};

byte six[] =
{
   B00111100,  
   B01100110,
   B01100000,
   B01111100,
   B01100110,
   B01100110,
   B00111100,
   B00000000
};

byte seven[] =
{
   B01111111,  
   B00000011,
   B00000110,
   B00001100,
   B00011000,
   B00011000,
   B00011000,
   B00011000
};

byte eight[] =
{
   B00111100,  
   B01100110,
   B01100110,
   B00111100,
   B01100110,
   B01100110,
   B00111100,
   B00000000
};

byte nine[] =
{
   B00111100,  
   B01100110,
   B01100110,
   B00111110,
   B00000110,
   B00000110,
   B01100110,
   B00111100,
};

byte zero[] =
{
   B00111100,  
   B01111110,
   B01100110,
   B01100110,
   B01100110,
   B01100110,
   B01111110,
   B00111100
};

byte dark[] =
{
   B00000000,  
   B00000000,
   B00000000,
   B00000000,
   B00000000,
   B00000000,
   B00000000,
   B00000000
};

byte full[] =
{
   B11111111,  
   B11111111,  
   B11111111,  
   B11111111,  
   B11111111,  
   B11111111,  
   B11111111,  
   B11111111 
};

//***************************************************
//**               setup section                   **
//***************************************************
void setup() {
  Serial.begin(115200);                        // initialize the serial communication
  Serial.println(".");
  Serial.println("*********************************************************");
  Serial.println("**                   LHCreate - (c)2017                **");
  Serial.println("*                       central cube");
  Serial.println("**    Developed by The Nostradomus Engineering Team    **");
  Serial.println("*");
  Serial.println("*  - Serial communication initialized");
  pinMode(arrowsDetectPIN, INPUT_PULLUP);
  pinMode(soundHornPIN, OUTPUT);
  digitalWrite(soundHornPIN, HIGH);
  Serial.println("*  - Digital inputs configured");
  strip.begin();                              // initialize the LED strip
  strip.setBrightness(32);                    // set the LED-pixel brightness 
  strip.show();                               // Initialize all pixels to 'off'
  Serial.println("*  - Led-strip initialised");
  lc.shutdown(0,false);                       // initialise the 8x8 matrix display
  lc.setIntensity(0,1);                       // set initial brightness level
  lc.clearDisplay(0);                         // clear the display
  Serial.println("*  - Code display initialised");
  Serial.print("*  - Controller version    : ");
  Serial.println(ARDUINO); 
  Serial.print("*  - Firmware version      : ");
  Serial.println(FWversion);
  Serial.print("*  - CPU frequency         : ");
  Serial.println(F_CPU);
  Serial.println("*");
  Serial.println("**                   Setup completed...                **");
  Serial.println("*********************************************************");
  Serial.println(" ");
}

//***************************************************
//**            main application loop              **
//***************************************************
void loop() {
    bool arrowsConnected = !digitalRead(arrowsDetectPIN);
    // detect rising edges on the arrows circuit, and set flags accordingly
    // the small-signal, low-drop diodes which are integrated in the Feynman arrows
    // will make sure that the player places them in the correct direction,
    // before revealing the secret code on the display
    if (arrowsConnected != LastArrowsConnectedState) {
      if (arrowsConnected == true) {
        arrowsConnectedTimestamp = millis();
        beeps = 0;
      } 
    } 
    LastArrowsConnectedState = arrowsConnected;

    if (arrowsConnected) {
      //TODO: obtain the code for the on-going game from the central cube 
      //      (a random digit has currently been hard-coded for the presentation)
      showBitmap(seven,15);
      // color the wiggly line green (at full power), when the arrows have been fit correctly
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, green);
        strip.setBrightness(128);                       // set the LED-pixel brightness 
        strip.show();                                   // Initialize all pixels to 'off'
        delay(30);
      }      
      // the soundhorn will beep ten times, when the arrows are connected correctly
      if (((millis() - arrowsConnectedTimestamp) > (beeps * 1000)) and (beeps < 20)) {
        for(int i=0; i<2; i++) {
          digitalWrite(soundHornPIN, LOW);
          delay(50);
          digitalWrite(soundHornPIN, HIGH);
          delay(50);
        }
        beeps++;
      }
    } else {
      // choose on of the two animations below for the 8x8 LED matrix display
      // this anamition will be shown on the display as long as the enigma has not been solved
      numberSpinner();
      //framesAnimation(); 
      // twinkling star animation for the LED strip
      twinklingStarAnimation();
    }
}

//***************************************************
//** application specific functions and procedures **
//***************************************************

// helper function to send 8 bytes to the display
// each byte contains the info for one row of 8 LEDs
void showBitmap(byte bitmap[], int brightness)
{
  lc.setIntensity(0,brightness);
  for (int i = 0; i < 8; i++)  
  {
    lc.setRow(0,i,bitmap[i]);
  }  
}

// helper function, to animate the display with expanding/shrinking squares
void framesAnimation()
{
  if ((lastAnimationChange == 0) or ((millis() - lastAnimationChange) >= 600)) {
    switch (currentAnimationFrame) {
      case 1:
        showBitmap(animationFrame1,8);
        break;
      case 2:
        showBitmap(animationFrame2,6);
        break;
      case 3:
        showBitmap(animationFrame3,4);
        break;
      case 4:
        showBitmap(animationFrame4,2);
        break;
      case 5:
        showBitmap(animationFrame3,4);
        break;
      case 6:
        showBitmap(animationFrame2,6);
        break;
    }
    currentAnimationFrame++;
    if (currentAnimationFrame > 6) {
      currentAnimationFrame = 1;
    }
    lastAnimationChange = millis();
  }
}

// helper function, to animate the display with random numbers
void numberSpinner()
{
  if ((lastNumberChange == 0) or ((millis() - lastNumberChange) >= 65)) {
    int randomNumber = random(1, 10);
    switch (randomNumber) {
      case 1:
        showBitmap(one,1);
        break;
      case 2:
        showBitmap(two,1);
        break;
      case 3:
        showBitmap(three,1);
        break;
      case 4:
        showBitmap(four,1);
        break;
      case 5:
        showBitmap(five,1);
        break;
      case 6:
        showBitmap(six,1);
        break;
      case 7:
        showBitmap(seven,1);
        break;
      case 8:
        showBitmap(eight,1);
        break;
      case 9:
        showBitmap(nine,1);
        break;
      case 10:
        showBitmap(zero,1);
        break;
    }
    lastNumberChange = millis();
  }    
}

// Helper function for the twinkling star or mutant LED animation
// The "twinkling star" or "mutant LED" function makes one LED pop-up in a fixed color.
// The other LED's will run through the spectrum, untill they find the matching RGB combination.
// Next, another mutant LED, or twinkling star will pop-up as a new challenge...
void twinklingStarAnimation() {
  uint32_t wheeledColor;
   for(uint16_t i=0; i<strip.numPixels(); i++) {
     if (i != starPosition) {
       wheeledColor = Wheel((i+jStarAnimation) & 255);
       strip.setPixelColor(i, wheeledColor); 
       } else {
       strip.setPixelColor(starPosition, wheeledStarColor); 
       if (wheeledColor == wheeledStarColor) {
         starPosition = random(1, strip.numPixels());
         starColor = random(0, 255);
         wheeledStarColor = Wheel((starPosition+starColor) & 255);                  
       }
     }
  }
  strip.setBrightness(32);                        // set the LED-pixel brightness 
  strip.show();                                   // Initialize all pixels to 'off'
  delay(30);
  if (jStarAnimation < 255) {
    jStarAnimation++;
    } else {
      jStarAnimation = 0;
      if (sStarAnimation == 1) {
        if (bStarAnimation < 255) {bStarAnimation += 42;} else {sStarAnimation = -1;}
      } else if (sStarAnimation == -1) {
        if (bStarAnimation > 45) {bStarAnimation -= 42;} else {sStarAnimation = 1;}
      }
    }     
}

// Input a value 0 to 255 to get a color value.
// The colours are transitioning red -> green -> blue -> back to red.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

