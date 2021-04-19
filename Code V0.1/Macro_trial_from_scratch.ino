/*
===============================================================================================================
Libraries used that can be difficult to identify just by name : 
Simple Rotary : https://github.com/mprograms/SimpleRotary
Bounce2 : https://github.com/thomasfredericks/Bounce2

===============================================================================================================
To do:
0.Remove long apparent delay between button presses
a.try to remove bounce and just read button state as all have pullups
I. try to usethe BUtton definition from bounce 2 library
1.USB disconnect by pressing a button for safe disconnect
2.Change rotary encoder precision by pressing a button - byte p


History:
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

//#include <Adafruit_DotStar.h>

// There is only one pixel on the board
//#define NUMPIXELS 1 

//Use these pin definitions for the ItsyBitsy M4
//#define DATAPIN    41
//#define CLOCKPIN   40

//Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

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

//used to store scroll factor
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
/*
//finalize the BOUNCE2 Library insert
SW1.attach(2, INPUT);
SW1.interval(5);
SW2.attach(5, INPUT);
SW2.interval(5);
SW3.attach(11, INPUT);
SW3.interval(5); 
SW4.attach(7, INPUT);
SW4.interval(5);
SW5.attach(6, INPUT);
SW5.interval(5);
SW6.attach(A3, INPUT);
SW6.interval(5);*/
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
  //while ( !Serial ) delay(10);   // wait for native usb

  Serial.println("Mass storage device connected and mounted");
  Serial.print("JEDEC ID: "); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.println(flash.size());

  changed = true; // to print contents initially
  //strip.begin(); // Initialize pins for output
  //strip.setBrightness(80);
  //strip.show();  // Turn all LEDs off ASAP
  
}

void loop() {
//Joystick Mouse
  vertValue = analogRead(vertPin) - vertZero;  // read vertical offset
  horzValue = analogRead(horzPin) - horzZero;  // read horizontal offset
  
  if (vertValue != 0)
    Mouse.move(0, -vertValue/sensitivity, 0);  // move mouse on y axis
  if (horzValue != 0)
    Mouse.move(-horzValue/sensitivity, 0, 0);  // move mouse on x axis

  //rainbow(10);             // Flowing rainbow cycle along the whole strip
//check flash contents
 /*if ( changed )
  {
    changed = false;
    
    if ( !root.open("/") )
    {
      Serial.println("open root failed");
      return;
    }

      Serial.println("Flash OK");
    Serial.println("Flash contents:");

    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while ( file.openNext(&root, O_RDONLY) )
    {
      file.printFileSize(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if ( file.isDir() )
      {
        // Indicate a directory.
        Serial.write('/');
      }
      Serial.println();
      file.close();
    }

    root.close();

    Serial.println();
    delay(1000); // refresh every 1 second
  }*/

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

/*
if (encst.risingEdge() && (encClickFlag == 0)) {
  p = SCRL_L_VALUE;
  encClickFlag = 1;
   Serial.println("encst Low Value");
}
else if 
(encst.risingEdge() && (encClickFlag == 1)) {
  p = SCRL_H_VALUE;
  encClickFlag = 0;
   Serial.println("encst High Value");
}

if (encdr.risingEdge() && (encClickFlag == 0)) {
  p = SCRL_H_VALUE;
  encClickFlag = 1;
   Serial.println("encdr High Value");
}
else if 
(encdr.risingEdge() && (encClickFlag == 1)) {
  p = SCRL_L_VALUE;
  encClickFlag = 0;
   Serial.println("encdr Low Value");
}
*/

if (button1.fell()) {
  sdFileToKeyboard1();
 // Serial.println("Bottom Left pressed");
}
else if (button1.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
    Serial.println("Low Left Released");
}


if (button2.fell()) {
 sdFileToKeyboard2();
  //Serial.println("Top Left pressed");
 
}

else if (button2.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
    Serial.println("Top Lefy Released");
}

if (button3.fell()){
  sdFileToKeyboard3();
  //Keyboard.press(KEY_LEFT_CTRL);
  //Keyboard.press(KEY_LEFT_SHIFT);
  //delay(1);
  //Mouse.press(MOUSE_MIDDLE);
  //delay(1);
  
  Serial.println("Middle Top pressed");
}

else if (button3.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
    Serial.println("Middle Top Released");
}

/*
else{
    delay(1);
    Mouse.release();
    delay(1);
    Keyboard.releaseAll();
    
}*/

if (button4.fell()) {
Serial.println("Top Right pressed");
sdFileToKeyboard4();
  
}
else if (button4.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
    Serial.println("Top Right Released");
}

if (button5.fell()) {
sdFileToKeyboard5();
  Serial.println("Bottom Right pressed");
}
else if (button5.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
    Serial.println("Bottom Right Released");
}

if (button6.fell()) {
sdFileToKeyboard6();
  Serial.println("Joystick pressed");
}
else if (button6.rose()){
    Keyboard.releaseAll();
    delay(1);
    Mouse.releaseAll();
    delay(1);
    Serial.println("Joystick Released");
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
/*
// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}
*/
