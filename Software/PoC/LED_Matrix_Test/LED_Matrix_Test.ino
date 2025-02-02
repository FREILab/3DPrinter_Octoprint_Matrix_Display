/* ----------------------------------------------------------------------
This is the code to display the desired information on the matrix display
------------------------------------------------------------------------- */

#include <Wire.h>  // For I2C communication
#include <Adafruit_Protomatter.h>  // For RGB matrix

#define HEIGHT 32   // Matrix height (pixels) - SET TO 64 FOR 64x64 MATRIX!
#define WIDTH 64    // Matrix width (pixels)
#define MAX_FPS 45  // Maximum redraw rate, frames/second

uint8_t rgbPins[] = { 42, 41, 40, 38, 39, 37 };
uint8_t addrPins[] = { 45, 36, 48, 35, 21 };
uint8_t clockPin = 2;
uint8_t latchPin = 47;
uint8_t oePin = 14;

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

int16_t textX;  // Current text position (X)
int16_t textY;  // Current text position (Y)
char str[64];   // Buffer to text

unsigned long previousMillis = 0;    // Stores the last time the display updated
const unsigned long interval = 100;  // Display data refresh rate in milliseconds

void setup(void) {
  Serial.begin(9600);
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
}

void loop(void) {
  // update display data every 100 milliseconds
  unsigned long currentMillis = millis();

  // Check if 100 ms have passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update the last run time
    // Blink cursorn on odd seconds
    bool blink = 0;
    int run_second = (currentMillis / 1000);
    if (run_second % 2 == 1) {
      blink = 1;
    } else {
      blink = 0;
    }

    // Print job is running
    // displayPrinterPrinting(56640, 1.0, blink, 212, 85);

    // Printer is ready
    displayPrinterReady(212, 85);

    // Octoprint is not connected
    //displayOctoprintOffline();

    // Wifi is not connected
    //displayWiFiOffline();
  }
}

void displayPrinterPrinting(int seconds, float progress, bool blink, int temp_T0, int temp_T1) {
  int h = seconds / 3600;
  int min = (seconds % 3600) / 60;

  // necessary variables
  int h_ones, h_tens, m_tens, m_ones;

  // Extract digits
  h_tens = h / 10;  // Extract tens place
  h_ones = h % 10;  // Extract ones place

  m_tens = min / 10;  // Extract tens place
  m_ones = min % 10;  // Extract ones place

  // cap tens places
  if (h_tens > 9) { h_tens = 9; }
  if (m_tens > 6) { m_tens = 5; }

  // Fill background black
  matrix.fillScreen(0);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  // draw border
  matrix.drawRect(0, 0, 64, 32, matrix.color565(0, 255, 0));  // green

  // Text time (always)
  sprintf(str, "Time");
  textX = 2;
  textY = 3;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Minutes Text (always)
  sprintf(str, "m");
  textX = 57;
  textY = 3;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Minutes ones (always)
  textX = 51;
  textY = 3;
  matrix.setTextColor(matrix.color565(0, 255, 255));  // bright blue
  matrix.setCursor(textX, textY);
  matrix.println(m_ones);

  // Minutes tens (if h>0 or m_tens>0)
  if ((h > 0) or (m_tens > 0)) {
    textX = 45;
    textY = 3;
    matrix.setTextColor(matrix.color565(0, 255, 255));  // bright blue
    matrix.setCursor(textX, textY);
    matrix.println(m_tens);
  }

  // Hours Text (if h>0)
  if (h > 0) {
    sprintf(str, "h");
    textX = 38;
    textY = 3;
    matrix.setTextColor(0xFFFF);  // white
    matrix.setCursor(textX, textY);
    matrix.println(str);
  }

  // Hours ones (if h>0)
  if (h > 0) {
    textX = 32;
    textY = 3;
    matrix.setTextColor(matrix.color565(0, 255, 255));  // bright blue
    matrix.setCursor(textX, textY);
    matrix.println(h_ones);
  }

  // Hours tens (if h>9)
  if (h > 9) {
    textX = 26;
    textY = 3;
    matrix.setTextColor(matrix.color565(0, 255, 255));  // bright blue
    matrix.setCursor(textX, textY);
    matrix.println(h_tens);
  }

  // Draw Progressbar outline
  matrix.drawRect(2, 12, 60, 6, matrix.color565(128, 128, 128));  // gray

  // draw the progress bar length depending on the progress
  int bar_max_progress = scaleFloatToInteger(progress);
  for (int i = 3; i < bar_max_progress; i = i + 1) {
    matrix.drawRect(i, 13, 1, 4, matrix.color565(0, 255, 0));  // green
  }

  // blink the last bar if necesary
  if (blink == 1) {
    matrix.drawRect(bar_max_progress - 1, 13, 1, 4, matrix.color565(0, 0, 0));  // black
  }

  // Line separating Progress and Temperatures
  matrix.drawRect(1, 20, 62, 1, matrix.color565(255, 255, 255));  // white

  // Display Extruder Temperature Text
  textX = 2;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 255, 255));  // white
  matrix.setCursor(textX, textY);
  matrix.println("T:");

  // Display T0
  textX = 15;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 0, 0));  // red
  matrix.setCursor(textX, textY);
  matrix.println(temp_T0);

  // Display Dash
  textX = 34;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 255, 255));  // white
  matrix.setCursor(textX, textY);
  matrix.println("|");

  // Display T1
  textX = 40;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 0, 0));  // red
  matrix.setCursor(textX, textY);
  matrix.println(temp_T1);

  // Update Display
  matrix.show();
}

void displayPrinterReady(int temp_T0, int temp_T1) {

  // Fill background black
  matrix.fillScreen(0);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);

  // draw border
  matrix.drawRect(0, 0, 64, 32, matrix.color565(255, 255, 0));  // yellow

  // Text time (always)
  sprintf(str, "Ready");
  textX = 2;
  textY = 3;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Draw Progressbar outline
  matrix.drawRect(2, 12, 60, 6, matrix.color565(128, 128, 128));  // gray

  // Line separating Progress and Temperatures
  matrix.drawRect(1, 20, 62, 1, matrix.color565(255, 255, 255));  // white

  // Display Extruder Temperature Text
  textX = 2;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 255, 255));  // white
  matrix.setCursor(textX, textY);
  matrix.println("T:");

  // Display T0
  textX = 15;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 0, 0));  // red
  matrix.setCursor(textX, textY);
  matrix.println(temp_T0);

  // Display Slash
  textX = 34;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 255, 255));  // white
  matrix.setCursor(textX, textY);
  matrix.println("|");

  // Display T1
  textX = 40;
  textY = 23;
  matrix.setTextColor(matrix.color565(255, 0, 0));  // red
  matrix.setCursor(textX, textY);
  matrix.println(temp_T1);

  // Update Display
  matrix.show();
}

void displayOctoprintOffline() {
  // Fill background black
  matrix.fillScreen(0);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  // draw border
  matrix.drawRect(0, 0, 64, 32, matrix.color565(255, 0, 0));  // red

  // Text time (always)
  sprintf(str, "Octoprint");
  textX = 2;
  textY = 3;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  sprintf(str, "offline");
  textX = 2;
  textY = 14;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Update Display
  matrix.show();
}

void displayWiFiOffline() {
  // Fill background black
  matrix.fillScreen(0);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  // draw border
  matrix.drawRect(0, 0, 64, 32, matrix.color565(255, 0, 0));  // red

  // Text time (always)
  sprintf(str, "WiFi");
  textX = 2;
  textY = 3;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  sprintf(str, "offline");
  textX = 2;
  textY = 14;
  matrix.setTextColor(0xFFFF);  // white
  matrix.setCursor(textX, textY);
  matrix.println(str);

  // Update Display
  matrix.show();
}

// Scale a float between 0 to 1 to a int between 3 and 61
int scaleFloatToInteger(float value) {
  // Ensure the input value stays within the expected range
  value = constrain(value, 0.0, 1.0);

  // Map the float to the integer range [3, 61]
  int scaledValue = round(value * (61 - 3) + 3);

  return scaledValue;
}
