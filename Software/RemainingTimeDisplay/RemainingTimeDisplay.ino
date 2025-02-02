/*******************************************************************
 *  A sketch to display to read out the state of a Octoprint       *
 *  instance and display it onto a matrix LED display.             *
 *                                                                 *
 *  Necessary acces data has to be provided to the secret.h        *
 *  02/2025                                                        *
 *  By Marius Tetard for FREILab Freiburg e.V. https://freilab.de  *
*******************************************************************/


// Octoprint library
#include <OctoPrintAPI.h>

// Wifi Connection
#include <WiFi.h>
#include <WiFiClient.h>

// file for Wifi credentials, octoprint API token and configuration
#include "secret.h"

// library for RGB LED
#include <Adafruit_NeoPixel.h>

// Graphics Library
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




// Configure WiFi
const char* ssid = SECRET_SSID;      // your network SSID (name)
const char* password = SECRET_PASS;  // your network password

WiFiClient client;

// Configure IP adress of octoprint
const char* ip_address = CONFIG_IP;
IPAddress ip(ip_address);

// Setup Octorint Port and API key
/* If you are connecting through a router this will work, but you need a
  // random port forwarded to the OctoPrint server from your router.
  // Enter that port here if you are external
  */
const int octoprint_httpPort = CONFIG_PORT;
String octoprint_apikey = SECRET_API;  //See top of file or GIT Readme about getting API key

// Initialize OctoPrint API
OctoprintApi api(client, ip, octoprint_httpPort, octoprint_apikey);

unsigned long api_mtbs = 5000;   //mean time between api requests (5 seconds)
unsigned long api_lasttime = 0;  //last time api request has been done

// Setup Neopixel (no of neopizel, Pinnumber, type)
Adafruit_NeoPixel pixels(1, 4, NEO_GRB + NEO_KHZ800);



String printerOperational;
String printerPaused;
String printerPrinting;
String printerReady;
String printerText;
String printerHotend;
String printerTarget;
String payload;

/* Todos:
  handle no availabe WiFi
  handle lost wifi connection (reconnect)
  handle no availabe Octoprint instance (reconnect)
  handle state ready
  handle state printing
  handle state paused
*/



void setup() {
  Serial.begin(115200);

  pixels.begin();
  pixels.clear();  // Reset RGB pixels
  // Set NeoPixel
  pixels.setPixelColor(0, pixels.Color(20, 20, 0));  // yellow
  pixels.show();

  // Start LCD
  ProtomatterStatus status = matrix.begin();
  Serial.print("[Matrix] Protomatter begin() status: ");
  Serial.println((int)status);

  delay(3000);  // long pause for serial interface to register

  // Start Wifi
  Serial.println();
  Serial.println();
  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Will try for about 10 seconds (20x 500ms)
  int tryDelay = 500;
  int numberOfTries = 20;

  // Wait for the WiFi event
  while (true) {

    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        Serial.println("[WiFi] SSID not found");
        // Set NeoPixel
        pixels.setPixelColor(0, pixels.Color(20, 0, 0));  // red
        pixels.show();
        break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        // Set NeoPixel
        pixels.setPixelColor(0, pixels.Color(20, 0, 0));  // red
        pixels.show();
        return;
        break;
      case WL_CONNECTION_LOST:
        // Set NeoPixel
        Serial.println("[WiFi] Connection was lost");
        pixels.setPixelColor(0, pixels.Color(20, 0, 0));  // red
        pixels.show();
        break;
      case WL_SCAN_COMPLETED:
        Serial.println("[WiFi] Scan is completed");
        break;
      case WL_DISCONNECTED:
        Serial.println("[WiFi] WiFi is disconnected");
        // Set NeoPixel
        pixels.setPixelColor(0, pixels.Color(20, 0, 0));  // red
        pixels.show();
        break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        // Set NeoPixel
        pixels.setPixelColor(0, pixels.Color(0, 20, 0));  // green
        pixels.show();
        return;
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;
    }
    delay(tryDelay);

    if (numberOfTries <= 0) {
      Serial.print("[WiFi] Failed to connect to WiFi!");
      // Use disconnect function to force stop trying to connect
      WiFi.disconnect();
      // Set NeoPixel
      pixels.setPixelColor(0, pixels.Color(20, 0, 0));  // red
      pixels.show();
      return;
    } else {
      numberOfTries--;
    }
  }
}


void loop() {

  switch (WiFi.status()) {
    case WL_DISCONNECTED:
      Serial.println("[WiFi] WiFi is disconnected");
      // Set NeoPixel
      pixels.setPixelColor(0, pixels.Color(20, 0, 0));  // red
      pixels.show();
      break;
    case WL_CONNECTED:
      // return;
      break;
    default:
      Serial.print("[WiFi] WiFi Status: ");
      Serial.println(WiFi.status());
      break;
  }

  if (millis() - api_lasttime > api_mtbs || api_lasttime == 0) {  //Check if time has expired to go check OctoPrint
    if (WiFi.status() == WL_CONNECTED) {                          //Check WiFi connection status
      // Set NeoPixel
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));  // off
      pixels.show();
      delay(20);
      pixels.setPixelColor(0, pixels.Color(0, 20, 0));  // green
      pixels.show();
      delay(100);

      // Printer is ready
      displayPrinterReady(212, 85);

      if (api.getOctoprintVersion()) {
        Serial.println("[Octoprint] ---------Version---------");
        Serial.print("[Octoprint] Octoprint API: ");
        Serial.println(api.octoprintVer.octoprintApi);
        Serial.print("[Octoprint] Octoprint Server: ");
        Serial.println(api.octoprintVer.octoprintServer);
        Serial.println("------------------------");
      }
      Serial.println();
      if (api.getPrinterStatistics()) {
        Serial.println("[Octoprint] ---------States---------");
        Serial.print("[Octoprint] Printer Current State: ");
        Serial.println(api.printerStats.printerState);
        Serial.print("[Octoprint] Printer State - closedOrError:  ");
        Serial.println(api.printerStats.printerStateclosedOrError);
        Serial.print("[Octoprint] Printer State - error:  ");
        Serial.println(api.printerStats.printerStateerror);
        Serial.print("[Octoprint] Printer State - operational:  ");
        Serial.println(api.printerStats.printerStateoperational);
        Serial.print("[Octoprint] Printer State - paused:  ");
        Serial.println(api.printerStats.printerStatepaused);
        Serial.print("[Octoprint] Printer State - printing:  ");
        Serial.println(api.printerStats.printerStatePrinting);
        Serial.print("[Octoprint] Printer State - ready:  ");
        Serial.println(api.printerStats.printerStateready);
        Serial.print("[Octoprint] Printer State - sdReady:  ");
        Serial.println(api.printerStats.printerStatesdReady);
        Serial.println("[Octoprint] ------------------------");
        Serial.println();
        Serial.println("[Octoprint] ------Termperatures-----");
        Serial.print("[Octoprint] Printer Temp - Tool0 (°C):  ");
        Serial.println(api.printerStats.printerTool0TempActual);
        Serial.print("[Octoprint] Printer State - Tool1 (°C):  ");
        Serial.println(api.printerStats.printerTool1TempActual);
        Serial.print("[Octoprint] Printer State - Bed (°C):  ");
        Serial.println(api.printerStats.printerBedTempActual);
        Serial.println("[Octoprint] ------------------------");
      }
    }
    if (api.getPrintJob()) {
      if ((api.printJob.printerState == "Printing")) {
        // we are printing something....
        Serial.print("[Octoprint] Progress:\t");
        Serial.print(api.printJob.progressCompletion);
        Serial.println(" %");
        Serial.print("[Octoprint] estimatedPrintTime:\t");
        Serial.print(api.printJob.estimatedPrintTime);
        Serial.println();
        Serial.print("[Octoprint] progressPrintTimeLeft:\t");
        Serial.print(api.printJob.progressPrintTimeLeft);  // time left in seconds
        Serial.println();
      } else if (api.printJob.progressCompletion == 100 && api.printJob.printerState == "Operational") {
        // 100% complete is no longer "Printing" but "Operational"
        Serial.print("[Octoprint] Progress:\t");
        Serial.print(api.printJob.progressCompletion);
        Serial.println(" %");
      } else if (api.printJob.printerState == "Offline" || api.printJob.printerState == "Operational") {
        // we are without working printer.... or the printer is no longer printing.... lights off
      }
    }

    api_lasttime = millis();  //Set api_lasttime to current milliseconds run
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
