/*
===============================================================================================================
Libraries used that can be difficult to identify just by name : 
Simple Rotary : https://github.com/mprograms/SimpleRotary
Bounce2 : https://github.com/thomasfredericks/Bounce2

===============================================================================================================

History:
v0.2 - cleaned up the code and released it to Github
v0.1 - functions implemented and text file names changed to sensible values 


*/
//#define BOUNCE_LOCK_OUT
#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <TinyUSB_Mouse_and_Keyboard.h>
#include <SimpleRotary.h>
#include <Bounce2.h>
#include <ctype.h>

//Definition of the variables

String filenameOnCard1 = "leftlow.txt";
String filenameOnCard2 = "lefttop.txt";
String filenameOnCard3 = "top.txt";
String filenameOnCard4 = "righttop.txt";
String filenameOnCard5 = "rightlow.txt";
String filenameOnCard6 = "joystick.txt";
String sleepCommandStartingPoint = "Sleep::";
String commandStartingPoint = "Command::";
int delayBetweenCommands = 1;
      
//Define buttons pins
#define SW1 2
#define SW2 5
#define SW3 11
#define SW4 7
#define SW5 6
#define SW6 A3


// INSTANTIATE A Button OBJECT FROM THE Bounce2 NAMESPACE
Bounce2::Button button1 = Bounce2::Button();
Bounce2::Button button2 = Bounce2::Button();
Bounce2::Button button3 = Bounce2::Button();
Bounce2::Button button4 = Bounce2::Button();
Bounce2::Button button5 = Bounce2::Button();
Bounce2::Button button6 = Bounce2::Button();


//Debounce the rotary encoders pins
Bounce encst = Bounce(9, 10);//enc left button
Bounce encdr = Bounce(3, 10);//enc right button

//Define Keys aliases
char shiftKey = KEY_LEFT_SHIFT;
char ctrlKey = KEY_LEFT_CTRL;

//used to store scroll factor for rotary encoders
byte p = 5;



//used to store scroll value High or Low
int encClickFlag = 0;

// Definition of the rotary encoders (Pin A, Pin B, Button Pin)
SimpleRotary rotary(8,A4,9); //left encoder
SimpleRotary rotary2(4,1,3); // right encoder
Bounce encst1 = Bounce(8, 10);//enc left button
Bounce encst2 = Bounce(A4, 10);//enc right button
Bounce encdr1 = Bounce(4, 10);//enc left button
Bounce encdr2 = Bounce(1, 10);//enc right button

//mass storage flash related
#if defined(FRAM_CS) && defined(FRAM_SPI)
  Adafruit_FlashTransport_SPI flashTransport(FRAM_CS, FRAM_SPI);

#else
  // On-board external flash (QSPI or SPI) macros should already
  // defined in your board variant if supported
  // - EXTERNAL_FLASH_USE_QSPI
  // - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
  #if defined(EXTERNAL_FLASH_USE_QSPI)
    Adafruit_FlashTransport_QSPI flashTransport;

  #elif defined(EXTERNAL_FLASH_USE_SPI)
    Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

  #else
    #error No QSPI/SPI flash are defined on your board variant.h !
  #endif
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

FatFile root;
FatFile file;

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;

// Set to true when PC write to flash
bool changed;
//Joystick Axis and sensitivity
int horzPin = A2;  // Analog output of horizontal joystick pin
int vertPin = A1;  // Analog output of vertical joystick pin

int vertZero, horzZero;  // Stores the initial value of each axis, usually around 512
int vertValue, horzValue;  // Stores current analog output of each axis
const int sensitivity = 100;  // Higher sensitivity value = slower mouse, should be <= about 500

void setup() {

//finalize the BOUNCE2 Library insert

encst.attach(9, INPUT);
encst.interval(5);
encdr.attach(3, INPUT);
encdr.interval(5);
encst1.attach(8, INPUT);
encst1.interval(5);
encst2.attach(A4, INPUT);
encst2.interval(5);
encdr1.attach(4, INPUT);
encdr1.interval(5);
encdr2.attach(1, INPUT);
encdr2.interval(5);
//Button setup
button1.attach( SW1, INPUT );
button2.attach( SW2, INPUT );
button3.attach( SW3, INPUT );
button4.attach( SW4, INPUT );
button5.attach( SW5, INPUT );
button6.attach( SW6, INPUT );
// DEBOUNCE INTERVAL IN MILLISECONDS
  button1.interval(5);
  button2.interval(5);
  button3.interval(15);
  button4.interval(5);
  button5.interval(5);
  button6.interval(5);
// INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
  button1.setPressedState(LOW); 
  button2.setPressedState(LOW); 
  button3.setPressedState(LOW); 
  button4.setPressedState(LOW); 
  button5.setPressedState(LOW); 
  button6.setPressedState(LOW); 


  //defining the pins
  pinMode(9, INPUT);//left encoder click
  pinMode(3, INPUT);//right encoder click
  pinMode(1, INPUT); //right enc
  pinMode(4, INPUT); // right enc
  pinMode(A4, INPUT);//left enc
  pinMode(8, INPUT);//left enc
  pinMode(horzPin, INPUT);  // Set both analog pins as inputs
  pinMode(vertPin, INPUT);


//Error delay fix rotary pins standard is 200 , the higher the better to avoid errors at direction change at the cost of quick direction change normallt 0 and 1 work fine for KB
    rotary.setErrorDelay(1); 
    rotary2.setErrorDelay(1); 
//Debounce rotary pins standard is 2 , the higher the better to avoid errors but will be at cost of missing pulses  
    rotary.setDebounceDelay(2);
    rotary2.setDebounceDelay(2);


  //mass storage flash related
  flash.begin();
 

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("DLM", "MacroKB Flash", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size()/512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);

//mount USB Drive to host in order to change config only when Middle Top button is pressed
  if(digitalRead(11)==LOW){
  usb_msc.begin();
  }
  else{
  
  }

  // Init file system on the flash
  fatfs.begin(&flash);
  
  Keyboard.begin();
  Mouse.begin();
  vertZero = analogRead(vertPin);  // get the initial values
  horzZero = analogRead(horzPin);  // Joystick should be in neutral position when reading these

  
  Serial.begin(115200);
  Serial.println("Mass storage device connected and mounted");
  Serial.print("JEDEC ID: "); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.println(flash.size());

  changed = true; // to print contents initially

  
}

void loop() {
//Joystick Mouse
  vertValue = analogRead(vertPin) - vertZero;  // read vertical offset
  horzValue = analogRead(horzPin) - horzZero;  // read horizontal offset
  
  if (vertValue != 0)
    Mouse.move(0, -vertValue/sensitivity, 0);  // move mouse on y axis
  if (horzValue != 0)
    Mouse.move(-horzValue/sensitivity, 0, 0);  // move mouse on x axis

// refresh/update keys status


button1.update();
button2.update();
button3.update();
button4.update();
button5.update();
button6.update();
encst.update();
encdr.update();
encst1.update();
encst2.update();
encdr1.update();
encdr2.update();


if (button1.fell()) {
  sdFileToKeyboard1();

}
else if (button1.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
}


if (button2.fell()) {
 sdFileToKeyboard2();
 
}

else if (button2.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);

}

if (button3.fell()){
  sdFileToKeyboard3();

}

else if (button3.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);

}

if (button4.fell()) {

sdFileToKeyboard4();
  
}
else if (button4.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);

}

if (button5.fell()) {
sdFileToKeyboard5();

}
else if (button5.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);

}

if (button6.fell()) {
sdFileToKeyboard6();

}
else if (button6.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);

}

  
  byte i; //left encoder
  byte x; // right encoder

  // 0 = not turning, 1 = CW, 2 = CCW
  i = rotary.rotate(); //update value left encoder
  x = rotary2.rotate(); // update value right encoder
  if ( i == 1 ) {
    Serial.println("CW");
     Mouse.move(+5*p,0,0);
  }

  if ( i == 2 ) {
    Serial.println("CCW");
     Mouse.move(-5*p, 0,0);
  }
   
   if ( x == 1 ) {
    Serial.println("CW");
     Mouse.move(0,+5*p,0);
  }

  if ( x == 2 ) {
    Serial.println("CCW");
     Mouse.move(0, -5*p,0);
  }
//loop closing below   
}

//mass storage related


// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
 // digitalWrite(LED_BUILTIN, HIGH);

  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // sync with flash
  flash.syncBlocks();

  // clear file system's cache to force refresh
fatfs.cacheClear();


 changed = true;

 // digitalWrite(LED_BUILTIN, LOW);
}
