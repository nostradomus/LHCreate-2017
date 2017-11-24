//****************************************************
//**             LHCreate - 2017                    **
//**               Central cube                     **
//**   author : Carl - (c)2017                      **
//**   version : 1.0                                **
//**   date : 16/11/2017                            **
//**                                                **
//**   https://github.com/nostradomus/LHCreate-2017 **
//**                                                **
//****************************************************
// v1.0 : original version for the presentation, with minor (visual) improvements

/*
                                      +-----+
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |           GND/RST2  [ ] [ ]         |
         |         MOSI2/SCK2  [ ] [ ]  SCL[ ] |   
         |            5V/MISO2 [ ] [ ]  SDA[ ] |   
         |                             AREF[ ] |
         |                              GND[ ] |
         | [ ]N/C                    SCK/13[ ] |   
         | [ ]v.ref                 MISO/12[ ] |   
         | [ ]RST                   MOSI/11[ ]~|   
         | [ ]3V3                        10[ ]~|   
         | [ ]5v                          9[ ]~|   
         | [ ]GND                         8[ ] |   
         | [ ]GND                              |
         | [ ]Vin                         7[ ] |   
         |                                6[ ]~|   
         | [ ]A0                          5[O]~|----SOUND-HORN------5V   
         | [ ]A1                          4[I] |----START-BUTTON----GND
         | [ ]A2                     INT1/3[O]~|----TM1640-DATA-----+\
         | [ ]A3                     INT0/2[O] |----TM1640-CLOCK----+ |JY-LM1640
         | [ ]A4      RST SCK MISO     TX>1[ ] |                5V--+ |16-digit display
         | [ ]A5      [ ] [ ] [ ]      RX<0[ ] |                GND-+/
         |  ARDUINO   [ ] [ ] [ ]              |
         |  UNO_R3    GND MOSI 5V  ____________/
          \_______________________/
*/

//***************************************************
//**             library section                   **
//***************************************************
#include <TM1638.h>              // libraries for the 16-digit display based on the tm1640 multiplexer chip
#include <TM1640.h>              // https://github.com/rjbatista/tm1638-library/

//***************************************************
//**            constants section                  **
//***************************************************
#define FWversion "v1.0"
#define TM1640dataPIN 3          // data pin for the TM1640 display. PD3, or physical pin 5 on DIL-28 ATmega328
#define TM1640clockPIN 2         // clock pin for the TM1640 display. PD2, or physical pin 4 on DIL-28 ATmega328
#define startButtonPIN 4         // start button for the countdown clock. PD4, or physical pin 6 on DIL-28 ATmega328
#define soundHornPIN 5           // sound horn output for the last 10 seconds. PD5, or physical pin 11 on DIL-28 ATmega328

//***************************************************
//**         global variables section              **
//***************************************************
TM1640 module(TM1640dataPIN, TM1640clockPIN);  // instantiate a display module(datapin, clockpin)
unsigned long startMillis;                     // start time when the button is pressed
int timer;                                     // integer to keep track of the countdown time (in 1/100 seconds -> 3min = 18000)
char text[17];                                 // textbuffer to send to the display
int alertBlinking;                             // counter to make the alert label blink
unsigned long gameOverMessageTimer;            // counter to show the "game-over" message for some time when reaching the time-out

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
  pinMode(startButtonPIN, INPUT_PULLUP);
  pinMode(soundHornPIN, OUTPUT);
  digitalWrite(soundHornPIN, HIGH);
  Serial.println("*  - Digital inputs and outputs configured");
  startMillis = millis();
  timer = 0;
  Serial.println("*  - Counters initialised");
  module.clearDisplay();
  displayStartMessage(); 
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
  // check the start button, and set the concerning flags
  bool startButtonPressed = !digitalRead(startButtonPIN);
  if (startButtonPressed) {
    timer = 18000;
  }

  // update the display
  int fakeDigit = random(0, 9);                           // fake digit to add for millisecond animation 
  if (timer > 0) {
    if (timer < 100)
    {
      sprintf(text, " TOO LATE   0%u%u",timer,fakeDigit);
      digitalWrite(soundHornPIN, LOW);
      gameOverMessageTimer = millis();
    }
    else if (timer < 1000)                               // during the last 10 seconds, a "danger" message will
    {                                                    // start blinking, together with a sound horn alarm
      if (alertBlinking < 30) {
        sprintf(text, " -DANGER-   %u%u",timer,fakeDigit);
        digitalWrite(soundHornPIN, LOW);
      } else{
        sprintf(text, "            %u%u",timer,fakeDigit);
        digitalWrite(soundHornPIN, HIGH);
        if (alertBlinking > 60) {alertBlinking = 0;}
      }
      alertBlinking++;
    }
    else if (timer < 10000)
    {
      sprintf(text, " COUNTING  %u%u",timer,fakeDigit);
    }
    else
    {
      sprintf(text, " COUNTING %u%u",timer,fakeDigit);
    }
    module.setDisplayToString(text, 0b0000000000001000); // seperate seconds and 1/1000 with a decimal point
    delay(4);                                            // delay, to have the clock run around 1/100 of a second
    timer = timer - 1;
    //module.setDisplayDigit(x, y, true/false);
    // x : digit to display
    // y : position from 0 to 15
    // decimal point
    //module.setDisplayToString(text, 0b0000000000000000);
    // text : text to display
    // 0b0000000000000000 : bits set to 1 will show the decimal point
  } else {
    if ((millis() - gameOverMessageTimer) > 10000 ){
      displayStartMessage();                            // show "start new game" message when idle
    } else {
      displayGameOverMessage();                         // show "game-over" message for 10 seconds at the end of the game
      digitalWrite(soundHornPIN, HIGH);                 // switch off the sound horn when the game has ended
    }  
  }
}

//***************************************************
//** application specific functions and procedures **
//***************************************************

void displayStartMessage()
{
    sprintf(text, " PUSH TO START  ");
    module.setDisplayToString(text); 
}

void displayGameOverMessage()
{
    sprintf(text, "  --GAME OVER-- ");
    module.setDisplayToString(text); 
}

