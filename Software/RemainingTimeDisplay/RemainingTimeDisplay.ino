/*******************************************************************
 *  A sketch to display to read out the state of a Octoprint       *
 *  instance and display it onto a matrix LED display.             *
 *                                                                 *
 *  Necessary acces data has to be provided to the secret.h        *
 *  02/2025                                                        *
 *  By Marius Tetard for FREILab Freiburg e.V. https://freilab.de  *
*******************************************************************/


#include <WiFi.h>
#include <WiFiClient.h>
#include "secret.h"
#include <OctoPrintAPI.h>          // For connection to OctoPrint
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

// Constants for Wi-Fi reconnection
const unsigned long CHECK_INTERVAL = 1000;  // Check Wi-Fi every 1 second
unsigned long previousMillis = 0;

// Setup Octoprint
WiFiClient client;
const char* ip_address = CONFIG_IP;
IPAddress ip(ip_address);
const int octoprint_httpPort = CONFIG_PORT;
String octoprint_apikey = SECRET_API;  //See top of file or GIT Readme about getting API key
// Initialize OctoPrint API
OctoprintApi api(client, ip, octoprint_httpPort, octoprint_apikey);


void setup() {
  // Start Serial Interface
  Serial.begin(115200);
  delay(2000);

  // Start LED Matrix
  ProtomatterStatus status = matrix.begin();
  Serial.print("[Matrix] Protomatter begin() status: ");
  Serial.println((int)status);

  // Wifi not yet connected
  displayWiFiOffline();

  // connect to WiFi
  connectToWiFi();
}

void loop() {
  unsigned long currentMillis = millis();

  // Check the Wi-Fi connection every second
  if (currentMillis - previousMillis >= CHECK_INTERVAL) {
    previousMillis = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      displayWiFiOffline();
      Serial.println("[WiFi] Wi-Fi lost. Reconnecting...");
      reconnectWiFi();
    } else {
      Serial.println("[WiFi] Wi-Fi connected. IP address: " + WiFi.localIP().toString());

      // printOctoprintDebug();

      // get latest Octoprint data
      api.getPrinterStatistics();
      api.getPrintJob();

      // State: printer ready (operational + ready)
      if(api.printerStats.printerStateoperational && api.printerStats.printerStateready) {
        // Printer is ready
        displayPrinterReady(api.printerStats.printerTool0TempActual, api.printerStats.printerBedTempActual);
      }
      // State printer printing (operational + printing)
      if(api.printerStats.printerStateoperational && api.printerStats.printerStatePrinting) {
        // Printer is printing
        float progress = (float)api.printJob.progressPrintTime /
            ((float)api.printJob.progressPrintTime + (float)api.printJob.progressPrintTimeLeft);
            
        displayPrinterPrinting(
          api.printJob.progressPrintTimeLeft,
          progress,
          api.printerStats.printerTool0TempActual,
          api.printerStats.printerBedTempActual);
      }
    }
  }
}

/*
api.printerStats.printerStateready
api.printerStats.printerStateoperational
api.printerStats.printerStatePrinting
*/

/* Standby: 
17:11:55.919 -> ---------Version---------
17:11:55.919 -> Octoprint API: 0.1
17:11:55.919 -> Octoprint Server: 1.10.3
17:11:55.919 -> ------------------------
17:11:55.919 -> 
17:11:56.182 -> ---------States---------
17:11:56.182 -> Printer Current State: Operational
17:11:56.182 -> Printer State - closedOrError:  0
17:11:56.182 -> Printer State - error:  0
17:11:56.182 -> Printer State - operational:  1
17:11:56.182 -> Printer State - paused:  0
17:11:56.182 -> Printer State - printing:  0
17:11:56.182 -> Printer State - ready:  1
17:11:56.182 -> Printer State - sdReady:  0
17:11:56.182 -> ------------------------
17:11:56.182 -> 
17:11:56.182 -> ------Termperatures-----
17:11:56.182 -> Printer Temp - Tool0 (°C):  16.67
17:11:56.182 -> Printer State - Tool1 (°C):  0.00
17:11:56.182 -> Printer State - Bed (°C):  17.19
17:11:56.182 -> ------------------------
*/

  /* Printer USB disconnected: 
17:11:55.919 -> ---------Version---------
17:11:55.919 -> Octoprint API: 0.1
17:11:55.919 -> Octoprint Server: 1.10.3
17:11:55.919 -> ------------------------
17:11:55.919 -> 
17:17:20.182 -> ---------States---------
17:17:20.182 -> Printer Current State: Operational
17:17:20.182 -> Printer State - closedOrError:  0
17:17:20.182 -> Printer State - error:  0
17:17:20.182 -> Printer State - operational:  1
17:17:20.182 -> Printer State - paused:  0
17:17:20.182 -> Printer State - printing:  0
17:17:20.182 -> Printer State - ready:  1
17:17:20.182 -> Printer State - sdReady:  0
17:17:20.182 -> ------------------------
17:17:20.182 -> 
17:17:20.182 -> ------Termperatures-----
17:17:20.182 -> Printer Temp - Tool0 (°C):  16.67
17:17:20.182 -> Printer State - Tool1 (°C):  0.00
17:17:20.182 -> Printer State - Bed (°C):  17.19
17:17:20.182 -> ------------------------
*/

  /* Printer printing: 
17:11:55.919 -> ---------Version---------
17:11:55.919 -> Octoprint API: 0.1
17:11:55.919 -> Octoprint Server: 1.10.3
17:11:55.919 -> ------------------------
17:11:55.919 -> 
17:20:13.345 -> ---------States---------
17:20:13.345 -> Printer Current State: Printing
17:20:13.345 -> Printer State - closedOrError:  0
17:20:13.345 -> Printer State - error:  0
17:20:13.345 -> Printer State - operational:  1
17:20:13.345 -> Printer State - paused:  0
17:20:13.345 -> Printer State - printing:  1
17:20:13.345 -> Printer State - ready:  0
17:20:13.345 -> Printer State - sdReady:  0
17:20:13.345 -> ------------------------
17:20:13.345 -> 
17:20:13.345 -> ------Termperatures-----
17:20:13.345 -> Printer Temp - Tool0 (°C):  42.00
17:20:13.345 -> Printer State - Tool1 (°C):  0.00
17:20:13.345 -> Printer State - Bed (°C):  19.91
17:17:20.182 -> ------------------------
*/

void connectToWiFi() {
  Serial.println("[WiFi] Connecting to Wi-Fi...");
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  int attempts = 0;
  Serial.print("[WiFi] ");
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi.");
    Serial.println("[WiFi] IP Address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
  }
}

void reconnectWiFi() {
  WiFi.disconnect();
  WiFi.reconnect();

  int attempts = 0;
  Serial.print("[WiFi] ");
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconnected to Wi-Fi.");
  } else {
    Serial.println("\nReconnection attempt failed.");
  }
}


void displayPrinterPrinting(int seconds, float progress, int temp_T0, int temp_T1) {
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

  int blink = 0;
  if ((millis() / 1000) % 2) {
    blink = 0;
  } else {
    blink = 1;
  }

  if (blink == 1) {
    if (bar_max_progress == 3) {bar_max_progress = 4;}
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

// debug: print octoprint API data
void printOctoprintDebug() {
  if (api.getPrinterStatistics()) {
    Serial.println();
    Serial.println("---------States---------");
    Serial.print("Printer Current State: ");
    Serial.println(api.printerStats.printerState);
    Serial.print("Printer State - closedOrError:  ");
    Serial.println(api.printerStats.printerStateclosedOrError);
    Serial.print("Printer State - error:  ");
    Serial.println(api.printerStats.printerStateerror);
    Serial.print("Printer State - operational:  ");
    Serial.println(api.printerStats.printerStateoperational);
    Serial.print("Printer State - paused:  ");
    Serial.println(api.printerStats.printerStatepaused);
    Serial.print("Printer State - printing:  ");
    Serial.println(api.printerStats.printerStatePrinting);
    Serial.print("Printer State - ready:  ");
    Serial.println(api.printerStats.printerStateready);
    Serial.print("Printer State - sdReady:  ");
    Serial.println(api.printerStats.printerStatesdReady);
    Serial.println("------------------------");
    Serial.println();
    Serial.println("------Termperatures-----");
    Serial.print("Printer Temp - Tool0 (°C):  ");
    Serial.println(api.printerStats.printerTool0TempActual);
    Serial.print("Printer State - Tool1 (°C):  ");
    Serial.println(api.printerStats.printerTool1TempActual);
    Serial.print("Printer State - Bed (°C):  ");
    Serial.println(api.printerStats.printerBedTempActual);
    Serial.println("------------------------");
  }
}