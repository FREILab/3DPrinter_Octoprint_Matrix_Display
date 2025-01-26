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
const int octoprint_httpPort = 80;
String octoprint_apikey = SECRET_API; //See top of file or GIT Readme about getting API key

String printerOperational;
String printerPaused;
String printerPrinting;
String printerReady;
String printerText;
String printerHotend;
String printerTarget;
String payload;


// Initialize OctoPrint API
OctoprintApi api(client, ip, octoprint_httpPort, octoprint_apikey);

unsigned long api_mtbs = 60000; //mean time between api requests (60 seconds)
unsigned long api_lasttime = 0;   //last time api request has been done




/* Todos:
  handle no availabe WiFi
  handle lost wifi connection (reconnect)
  handle no availabe Octoprint instance (reconnect)
  handle state ready
  handle state printing
  handle state paused
*/



void setup () {
  Serial.begin(115200);
  delay(3000);      // long pause for serial interface to register

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
      case WL_NO_SSID_AVAIL: Serial.println("[WiFi] SSID not found"); break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return;
        break;
      case WL_CONNECTION_LOST: Serial.println("[WiFi] Connection was lost"); break;
      case WL_SCAN_COMPLETED:  Serial.println("[WiFi] Scan is completed"); break;
      case WL_DISCONNECTED:    Serial.println("[WiFi] WiFi is disconnected"); break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
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
      return;
    } else {
      numberOfTries--;
    }
  }
}


void loop() {

  if (millis() - api_lasttime > api_mtbs || api_lasttime==0) {  //Check if time has expired to go check OctoPrint
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      if(api.getOctoprintVersion()){
        Serial.println("---------Version---------");
        Serial.print("Octoprint API: ");
        Serial.println(api.octoprintVer.octoprintApi);
        Serial.print("Octoprint Server: ");
        Serial.println(api.octoprintVer.octoprintServer);
        Serial.println("------------------------");
      }
      Serial.println();
      if(api.getPrinterStatistics()){
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
    api_lasttime = millis();  //Set api_lasttime to current milliseconds run
  }
}
