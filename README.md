# Single Digit Nixie Clock

A small project combining ESP8266 with a vintage Soviet Nixie tube from the 70’ to create an artistic clock.

Huge thank you to **[PCBWay](https://www.pcbway.com)** for sponsoring this project!

![demo](doc/singleDigitNixieClock.gif)

For more details follow project logs on: **[hackaday.io](https://hackaday.io/project/202115-single-digit-nixie-clock)**

Some of the key features:
- Shows current time (hours and minutes) on a single nixie tube.
- Configurable RGB LED backlight lights up the nixie tube every second.
- USB powered.
- Hosts a web server for easy setup & configuration.
- Synchronises time with NTP server.

Operating modes:
- AP mode
  - This mode is hosting a dedicated open wifi network called `NixieClock`. Its main purpose is to use it for first time setup to update wifi configuration. In addition, this mode will be activated when the clock is not able to connect to a configured network. Clock hosts a captive portal. A configuration page will be opened automatically upon connecting to `NixieClock` network.
- Client mode
  - In this mode the clock is connected to a configured wifi network. If there is internet access, the clock will synchronize its RTC with NTP server at startup and then every hour periodically.

In both modes, the configuration page is easily reachable on the following URL: `<HOSTNAME>.local`. There is no need to keep track of the IP address, the clock is hosting Multicast DNS (mDNS) server. mDNS is supported by Chrome and Safari browsers out of the box.

By default, the `HOSTNAME` is set to `mynixieclock`.

## Firmware

Firmware is written in C++ using Arduino.

To simplify project setup, the Arduino core for ESP8266 and all dependencies are moved to be part of the project as git submodules. This way, there is no need to manualy download and setup Arduino IDE and libraries.

### Project setup

To setup the project call the `setup.sh` script. This script will clone all the necessary third party libraries and setup ESP/Arduino environment.
```
cd firmware
./setup.sh
```

### Building and flashing the firmware
Build the project:
```
make
```
Build and flash the project firmware:
```
make flash
```
Build and flash file system:
```
make flash_fs
```
Start serial monitor on the upload port:
```
make monitor
```
A description of all available makefile functions and variables is always available via the following command:
```
make help
```

### Model diagrams

Context view diagram to present the general idea:

![contextView](doc/singleDigitNixieClockContextView.png)

Software system view diagram that shows software components and their relationship:

![softwareSystemView](doc/singleDigitNixieClockSoftwareSystemView.png)

Legend:

![legend](doc/legend.png)

### REST API

| End point | Method | Body (JSON) | Description |
| - | - | - | - |
| /backlight | GET | {<br>&nbsp;&nbsp;&nbsp;&nbsp;“R”: <0-255>,<br>&nbsp;&nbsp;&nbsp;&nbsp;“G”: <0-255>,<br>&nbsp;&nbsp;&nbsp;&nbsp;“B”: <0-255>,<br>&nbsp;&nbsp;&nbsp;&nbsp;“state”: <0-2><br>} | Get color and state of the backlight (RGB LED). |
| /backlight | POST | {<br>&nbsp;&nbsp;&nbsp;&nbsp;“R”: <0-255>,<br>&nbsp;&nbsp;&nbsp;&nbsp;“G”: <0-255>,<br>&nbsp;&nbsp;&nbsp;&nbsp;“B”: <0-255>,<br>&nbsp;&nbsp;&nbsp;&nbsp;“state”: <0-2><br>} | Set color and state of the backlight (RGB LED). |
| /clock/time | POST | {<br>&nbsp;&nbsp;&nbsp;&nbsp; “year”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “month”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “day”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “hour”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “minute”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “second”: \<value><br>} | Set current time. |
| /clock/sleep_info | GET | {<br>&nbsp;&nbsp;&nbsp;&nbsp; “sleep_before”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “sleep_after”: \<value><br>} | Get sleep mode configuration. The time before and after (in minutes) the backlight will be turned off. |
| /clock/sleep_info | POST | {<br>&nbsp;&nbsp;&nbsp;&nbsp; “sleep_before”: \<value>,<br>&nbsp;&nbsp;&nbsp;&nbsp; “sleep_after”: \<value><br>} | Set sleep mode configuration. |
| /wifi | GET | {<br>&nbsp;&nbsp;&nbsp;&nbsp;"hostname": "\<HOSTNAME>",<br>&nbsp;&nbsp;&nbsp;&nbsp;“SSID”: “\<Wifi SSID>”,&nbsp;&nbsp;&nbsp;&nbsp; <br>&nbsp;&nbsp;&nbsp;&nbsp;“password”: “\<base64 encoded password>”<br>} | Get wifi configuration. |
| /wifi | POST | {<br>&nbsp;&nbsp;&nbsp;&nbsp;"hostname": "\<HOSTNAME>",<br>&nbsp;&nbsp;&nbsp;&nbsp;“SSID”: “\<Wifi SSID>”,&nbsp;&nbsp;&nbsp;&nbsp; <br>&nbsp;&nbsp;&nbsp;&nbsp;“password”: “\<base64 encoded password>”<br>} | Get wifi configuration. | Set wifi configuration. |

### Front-end layout & design

Initially, the front end was implemented with jQuery mobile. Reason for choosing this framework was in its simplicity. It provides a simple way to create and control user interface components. One thing that always annoyed me was its low response time. It took too much time to load jQuery, jQueryMobile and theme resources. On top of this, this slow response time caused WDT resets.

One more thing, I just realised that  jQuery mobile project is deprecated since October 7, 2021. Huh, when it comes to web related technologies, it looks like I'm living under a rock...

To solve these issues, I opted to completely re implement the frontend using plain HTML, CSS and JavaScript. The end result looks almost the same as the previous design or even better. Responsiveness and stability increased significantly. The only drawback is now in maintenance and extension. It is required to implement everything manually.

![ui](doc/feDesign.jpg)

## Design

Schematic and PCB are designed with KiCAD 8.0.

PCB views are generated with [PCBWay’s online gerber viewer](https://www.pcbway.com/project/OnlineGerberViewer.html).

### Control board

Control board is basically a re-packaged NodeMCU design connected with DS3231 real time clock and NCH8200HV high voltage boost module board.

The reason for still choosing **ESP8288** instead of more modern **ESP32** variant is simple: I had a couple of unused modules in my toolbox. It was logical to use them. ESP8288 module is perfectly fine for this kind of project. It has a wireless capability and the MCU is good enough.

Maybe in the future I decide to upgrade the project to use **ESP32**.

#### Schematic

![control board schematic](doc/cbSch.jpg)

**IMPORTANT**: For some reason (bug) the wiring of the SMD crystal is wrong. XI and XO pins are located diagonally. To fix this issue after calling "Update PCB from Schematic..."(F8) it is needed to manually change the layout of Y1 to:
![fix](doc/crystalFix.png)

### Display board

Display board uses a CD4028 BCD-to-decimal decoder. Its purpose is to reduce the number of required GPIO pins from 10 to just 4 for showing digits from 0 to 9. Compared to dedicated BCD to decimal decoder nixie drivers (like K511ID1), the CD4028 is a widely spread component and it is available in SMD package. Since CD4028 has a voltage range from 3.0V to 15V it is not compatible with nixie tube’s operating voltage of 170V out of the box. This problem is solved with MMBTA42, a high voltage transistor, rated to more than 200V.

The rest of the display board consists of a WS2812B RGB LED and a pin header used to connect with the control board.

#### Schematic

![display board schematic](doc/dbSch.jpg)

## PCB manufacturing

A representative from **PCBWay** contacted me because he was deeply impressed by my project and offered to sponsor a batch of PCBs to test out their services.

Since I recently moved to KiCad, it was a perfect opportunity to try out the PCBWay Plug-in for KiCad. It provides a simple way to place an order in just a few clicks. There is no need to bother with exporting gerber files, everything is automated. You just need to update the PCB specification and fill in the shipping information on the webpage. That’s it.

The entire journey from placing the order to receiving the package took less than a week. Incredible! PCBWay did a superb job, PCBs are amazing. I am more than impressed.

![PCBs](doc/374A6210.jpg)

![Assembled PCBs](doc/374A6220.jpg)

## Enclosure design

3D model is made with Autodesk Fusion 360.

The end result is a simple box shaped design with holes for the nixie tube and USB connector.

![enclosure model](doc/enclosureModel.png)


## Resources

* [IN-14 Nixie tube datasheet](https://tubehobby.com/datasheets/in14.pdf)
* [NCH8200HV High Voltage DC-DC Booster module](http://omnixie.io/nch8200hv.html)
* [CD4028 datasheet](https://www.renesas.com/en/document/dst/cd4028bms-datasheet?srsltid=AfmBOoqU2NILPd5EzyLmpNgOihDv6LZo1jPMcaXfnR6dCclXbg2DF4TL)
* [NodeMCU](https://circuits4you.com/2017/12/31/nodemcu-pinout/)
