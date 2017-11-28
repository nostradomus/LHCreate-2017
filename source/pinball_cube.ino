//****************************************************
//**             LHCreate - 2017                    **
//**               Pinball cube                     **
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
         +------------| USB |------------+      5V---+\
         |            +-----+            |      GND--+ |8x8 LED dot-matrix 
         | [ ]D13/SCK        MISO/D12[O] |----DIN----+ |display module with 
         | [ ]3.3V           MOSI/D11[O]~|----CLK----+ |MAX7219CNG chip
         | [ ]V.ref            SS/D10[O]~|----CS-----+/
         | [ ]A0                   D9[ ]~|   
         | [ ]A1                   D8[ ] |   
         | [ ]A2                   D7[ ] |   
         | [ ]A3                   D6[ ]~|   
         | [ ]A4/SDA               D5[O]~|----SOUND-HORN-------5V   
         | [ ]A5/SCL               D4[I] |----Ball-detection---GND   
         | [ ]A6              INT1/D3[ ]~|   
         | [ ]A7              INT0/D2[ ] |   
         | [ ]5V                  GND[ ] |     
         | [ ]RST                 RST[ ] |   
         | [ ]GND   5V MOSI GND   TX1[ ] |   
         | [ ]Vin   [ ] [ ] [ ]   RX1[ ] |   
         |          [ ] [ ] [ ]          |
         | NANO-V3  MISO SCK RST         |
         +-------------------------------+
*/

//***************************************************
//**             library section                   **
//***************************************************
#include "LedControl.h"                  // library for the 8x8 LED matrix display (https://github.com/wayoda/LedControl)

//***************************************************
//**            constants section                  **
//***************************************************
#define FWversion "v1.0"
#define ballDetectPIN 4                  // detection switch for the ball. PD4, or physical pin 2 on ATmega328p-AU (TQFP32 package)
#define soundHornPIN 5                   // sound horn output for correct energy. PD5, or physical pin 9 on ATmega328p-AU (TQFP32 package)
#define codeVisibleTime 10000            // when winning the game, the code will be visible for 10 seconds
 
//***************************************************
//**         global variables section              **
//***************************************************
LedControl lc = LedControl(12,11,10,1);  // DIN-pin, CLK-pin, CS-pin, number of connected display modules
unsigned long lastAnimationChange;       // timecounter for the animation when playing
int currentAnimationFrame = 1;           // currently shown frame of the animation
unsigned long lastNumberChange;          // timecounter for the numberspinner-animation when playing
unsigned long ballDetectedTimestamp;     // timestamp (in millis) when the ball was detected
bool showCodeDigit;                      // player applied correct energy => show code digit for a certain time
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
  Serial.println("*                       pinball cube                    *");
  Serial.println("**    Developed by The Nostradomus Engineering Team    **");
  Serial.println("*");
  Serial.println("*  - Serial communication initialized");
  pinMode(ballDetectPIN, INPUT_PULLUP);
  pinMode(soundHornPIN, OUTPUT);
  digitalWrite(soundHornPIN, HIGH);
  Serial.println("*  - Digital inputs and outputs configured");
  lc.shutdown(0,false);                       // initialise the 8x8 matrix display
  lc.setIntensity(0,1);                       // set initial brightness level
  lc.clearDisplay(0);                         // clear the display
  Serial.println("*  - Display initialised");
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
  // check the ball detection switch, and set the concerning flags
  bool ballDetected = !digitalRead(ballDetectPIN);

  // logic to show code digit for a limited time, when the correct energy had been applied
  if (ballDetected) {
    showCodeDigit = true;
    ballDetectedTimestamp = millis();
    beeps = 0;
  } else {
    if ((millis() - ballDetectedTimestamp) < codeVisibleTime) {
      showCodeDigit = true;
    } else {
      showCodeDigit = false;
    }
  }

  // update the display
  if (showCodeDigit) {
    //TODO: obtain the code for the on-going game from the central cube 
    //      (a random digit has currently been hard-coded for the presentation)
    showBitmap(one,15);
    // the soundhorn will briefly beep every second, as long as the winning code digit is visible
    if ((millis() - ballDetectedTimestamp) > (beeps * 1000)) {
      digitalWrite(soundHornPIN, LOW);
      delay(50);
      digitalWrite(soundHornPIN, HIGH);
      beeps++;
    }
  } else {
    // choose on of the two animations below
    // this anamition will be shown on the display as long as the enigma has not been solved
    numberSpinner();
    //framesAnimation();
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
