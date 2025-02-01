/* ----------------------------------------------------------------------
"Pixel dust" Protomatter library example. As written, this is
SPECIFICALLY FOR THE ADAFRUIT MATRIXPORTAL with 64x32 pixel matrix.
Change "HEIGHT" below for 64x64 matrix. Could also be adapted to other
Protomatter-capable boards with an attached LIS3DH accelerometer.

PLEASE SEE THE "simple" EXAMPLE FOR AN INTRODUCTORY SKETCH,
or "doublebuffer" for animation basics.
------------------------------------------------------------------------- */

#include <Wire.h>                 // For I2C communication
//#include <Adafruit_LIS3DH.h>      // For accelerometer
//#include <Adafruit_PixelDust.h>   // For sand simulation
#include <Adafruit_Protomatter.h> // For RGB matrix
#include <Fonts/Picopixel.h> // 5px font
#include <Fonts/TomThumb.h> // 5px font breit
#include <Fonts/Org_01.h> // 5px font
#include <Fonts/FreeSans9pt7b.h> // 5px font

#define HEIGHT  32 // Matrix height (pixels) - SET TO 64 FOR 64x64 MATRIX!
#define WIDTH   64 // Matrix width (pixels)
#define MAX_FPS 45 // Maximum redraw rate, frames/second



uint8_t rgbPins[]  = {42, 41, 40, 38, 39, 37};
uint8_t addrPins[] = {45, 36, 48, 35, 21};
uint8_t clockPin   = 2;
uint8_t latchPin   = 47;
uint8_t oePin      = 14;


#if HEIGHT == 16
#define NUM_ADDR_PINS 3
#elif HEIGHT == 32
#define NUM_ADDR_PINS 4
#elif HEIGHT == 64
#define NUM_ADDR_PINS 5
#endif

Adafruit_Protomatter matrix(
  WIDTH, 4, 1, rgbPins, NUM_ADDR_PINS, addrPins,
  clockPin, latchPin, oePin, true);

int16_t  textX;        // Current text position (X)
int16_t  textY;        // Current text position (Y)
char     str[64];      // Buffer to text

void setup(void) {
  Serial.begin(9600);

  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status != PROTOMATTER_OK) {
    for(;;);
  }

}

void loop(void) {
  displayPrinterPrinting(0,1, 0.1);
  delay(3000);
  displayPrinterPrinting(0,12, 0.2);
  delay(3000);
  displayPrinterPrinting(0,120, 0.5);
  delay(3000);
  displayPrinterPrinting(1,0, 0.66);
  delay(3000);
  displayPrinterPrinting(1,12, 1.0);
  delay(3000);
  displayPrinterPrinting(12,23, 0.0);
  delay(3000);
  displayPrinterPrinting(120,23,0.0);
  delay(3000);

  //Serial.print("Refresh FPS = ~");
  //Serial.println(matrix.getFrameCount());
  delay(1000);
}


// void displayPrinterPrinting(int h_ones, int h_tens, int m_tens, int m_ones) {
void displayPrinterPrinting(int h, int min, float progress) {
  // necessary variables
  int h_ones, h_tens, m_tens, m_ones;

  // Extract digits
  h_tens = h / 10; // Extract tens place
  h_ones = h % 10; // Extract ones place

  m_tens = min / 10; // Extract tens place
  m_ones = min % 10; // Extract ones place

  // cap tens places
  if (h_tens>9) {h_tens = 9;}
  if (m_tens>6) {m_tens = 5;}


  // Fill background black
  matrix.fillScreen(0);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  // draw border
  matrix.drawRect(0, 0, 64, 32, matrix.color565(0, 255, 0)); // green

  // Text time (always)
  sprintf(str, "Time");
  textX = 2;
  textY = 2;
  matrix.setTextColor(0xFFFF); // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Minutes Text (always)
  sprintf(str, "m");
  textX = 57;
  textY = 2;
  matrix.setTextColor(0xFFFF); // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Minutes ones (always)
  textX = 51;
  textY = 2;
  matrix.setTextColor(matrix.color565(0, 255, 255)); // bright blue
  matrix.setCursor(textX, textY);
  matrix.println(m_ones);

  // Minutes tens (if h>0 or m_tens>0)
  if( (h > 0) or (m_tens > 0)) {
    textX = 45;
    textY = 2;
    matrix.setTextColor(matrix.color565(0, 255, 255)); // bright blue
    matrix.setCursor(textX, textY);
    matrix.println(m_tens);
  }
  
  // Hours Text (if h>0)
  if( h > 0 ) {
    sprintf(str, "h");
    textX = 38;
    textY = 2;
    matrix.setTextColor(0xFFFF); // white
    matrix.setCursor(textX, textY);
    matrix.println(str);
  }

  // Hours ones (if h>0)
  if( h > 0 ) {
    textX = 32;
    textY = 2;
    matrix.setTextColor(matrix.color565(0, 255, 255)); // bright blue
    matrix.setCursor(textX, textY);
    matrix.println(h_ones);
  }


  // Hours tens (if h>9)
  if( h > 9 ) {
    textX = 26;
    textY = 2;
    matrix.setTextColor(matrix.color565(0, 255, 255)); // bright blue
    matrix.setCursor(textX, textY);
    matrix.println(h_tens);
  }


  // Draw Progressbar outline
  matrix.drawRect(2, 10, 60, 6, matrix.color565(128, 128, 128)); // gray

  // draw the bar length depending on the progress
  for (int i = 3; i<scaleFloatToInteger(progress); i=i+1) {
    matrix.drawRect(i, 11, 1, 4, matrix.color565(0, 255, 0)); // green
  }


  // Update Display
  matrix.show();
}



void displayPrinterOffline() {

  /*#include <Fonts/Picopixel.h> // 5px font
  #include <Fonts/TomThumb.h> // 5px font breit
  #include <Fonts/Org_01.h> // 5px font*/

  sprintf(str, "Drucker 1 offline");
  textX = 0;        // Current text position (X)
  textY = 8;   

  matrix.fillScreen(0); // Fill background black
    matrix.setTextWrap(false);           // Allow text off edge
  matrix.setTextColor(0xFFFF);         // White

  

  // 1
  matrix.setCursor(0, 8);
  matrix.setFont(&Picopixel); // Use nice bitmap font
  matrix.println(str);

  // 2
  matrix.setCursor(0, 16);
  matrix.setFont(&TomThumb); // Use nice bitmap font
  matrix.println(str);

  // 3
  matrix.setCursor(0, 24);
  matrix.setFont(&Org_01); // Use nice bitmap font
  matrix.println(str);
  matrix.show(); // Copy data to matrix buffers
}

// Scale a float between 0 to 1 to a int between 3 and 61
int scaleFloatToInteger(float value) {
  // Ensure the input value stays within the expected range
  value = constrain(value, 0.0, 1.0);
  
  // Map the float to the integer range [3, 61]
  int scaledValue = round(value * (61 - 3) + 3);
  
  return scaledValue;
}
