# Matrix Display for reminaing print time (Octoprint)

This is a hardware / software project to display the remianing print time for the FreiLab printers

![Closup of display](Hardware/images/closeup.JPG?raw=true "Closeup")

## Overview
The Display is powered by to the Raspberry Pi which controls the 3D Printer via Octoprint. The Adafruit MCU Board MatrixPortal S3 connects via its built in WiFi card to the Octoprint Instance and displays the current hotend and bed temperature as well as the print job progress and remaining time on the matrix display.

## Hardware
The following hardware is used:
- MCU Board: Adafruit MatrixPortal S3: https://www.adafruit.com/product/5778
- LED Matrix: 64x32 RGB LED Matrix - 3mm pitch (IC: usually HUB75); different vendors available, e.g.: https://www.adafruit.com/product/2279
- Power Supply: a USB-C interface capable of 5V, 1.5A is sufficient, for example a raspberry pi USB output port. In normal mode I measured at 5V about 0.15A draw for the display and MCU.
- Wire it according to the adafruit design guide: https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/overview
- A 3D printed overhead mount was used to mount the Display and the MCU board.
- The Display is fixed with 5 M3x12/M3x16 screws
- For the board four M2 heat inserts and four M2 screws are required

## Software
The display uses these significant libraries:
- Adafruit Protomatter (Display driver): https://github.com/adafruit/Adafruit_Protomatter
- OctoPrintAPI to fetch the print progress: https://github.com/chunkysteveo/OctoPrintAPI
- enter the WiFi credentials and Octorint IP, Port and API key in the file "secret.h"

The were some issues with the Octoprint API which locked up the MCU when a print job was started/finished. Instead of fixing the API itself, a simple watchdog was implemented to restart the Adafruit MatrixPortal S3 when the software freezes.
