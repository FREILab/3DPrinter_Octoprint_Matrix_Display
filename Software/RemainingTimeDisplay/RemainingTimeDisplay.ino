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

String printerOperational;
String printerPaused;
String printerPrinting;
String printerReady;
String printerText;
String printerHotend;
String printerTarget;
String payload;

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

      // Check if Octoprint is ready
      if (api.getOctoprintVersion()) {
        Serial.println("[Octoprint] Octoprint connected)");

        /*
        ---------States---------
Printer Current State: 
Printer State - closedOrError:  0
Printer State - error:  0
Printer State - operational:  0
Printer State - paused:  0
Printer State - printing:  0
Printer State - ready:  0
Printer State - sdReady:  0
------------------------

------Termperatures-----
Printer Temp - Tool0 (°C):  0.00
Printer State - Tool1 (°C):  0.00
Printer State - Bed (°C):  0.00
        */

        displayPrinterReady(212, 85);
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
          }}



        } else {
          Serial.println("[Octoprint] Octoprint not connected");

          displayOctoprintOffline();
        }
      }
    }
  }

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
