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
