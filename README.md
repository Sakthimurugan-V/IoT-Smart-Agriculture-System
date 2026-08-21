# 🌱 IoT-Based Smart Agriculture System

An ESP32-based IoT Smart Agriculture System designed for real-time agricultural monitoring, intelligent irrigation control, and remote monitoring using Blynk IoT.

The system integrates multiple sensors with an ESP32 to monitor soil moisture, water level, water flow, temperature, humidity, and ambient light. An ESP32-CAM is also used for real-time visual monitoring of the agricultural setup.

---

## 📌 Project Overview

The IoT-Based Smart Agriculture System is an embedded IoT system developed to demonstrate smart agriculture and automated irrigation concepts.

The system continuously monitors important agricultural parameters and automatically controls a solenoid valve based on soil moisture conditions.

The system also provides:

- Local monitoring through an OLED display
- Remote monitoring through Blynk IoT
- Manual irrigation control
- Automatic irrigation control
- Water-level monitoring
- Water-flow monitoring
- Environmental monitoring
- ESP32-CAM based real-time visual monitoring
- Offline automatic operation when Wi-Fi is unavailable

---

## 🎯 Objectives

- Monitor soil moisture in real time.
- Detect water level using ultrasonic and float sensors.
- Measure water flow rate.
- Monitor temperature and humidity.
- Measure ambient light using an I2C sensor.
- Automatically control irrigation based on soil moisture.
- Provide manual irrigation control through Blynk.
- Display sensor information using an OLED.
- Provide real-time visual monitoring using ESP32-CAM.
- Maintain automatic irrigation even when Wi-Fi is disconnected.

---

## ✨ Features

- 🌱 Soil moisture monitoring
- 💧 Water-level monitoring
- 📊 Water-flow measurement
- 🌡️ Temperature monitoring
- 💦 Humidity monitoring
- ☀️ Ambient-light monitoring
- 🚰 Automatic irrigation
- 🎛️ Manual relay control
- 📱 Blynk IoT dashboard
- 📺 OLED multi-page display
- 📷 ESP32-CAM real-time monitoring
- 📡 Wi-Fi connectivity
- 🔄 Automatic Wi-Fi reconnection
- 📴 Offline automatic operation
- 🔌 Relay-controlled solenoid valve

---

## 🧠 System Architecture
```text
                    ┌─────────────────────┐
                    │       ESP32         │
                    │   Main Controller   │
                    └──────────┬──────────┘
                               │
          ┌────────────────────┼────────────────────┐
          │                    │                    │
          ▼                    ▼                    ▼
      Sensors                Display             Control
          │                    │                    │
   ┌──────┼──────┐           OLED              Relay
   │      │      │                               │
   ▼      ▼      ▼                               ▼
Ultrasonic Soil  Float                      Solenoid
Sensor     Sensor Switches                    Valve
   │
   ├── Flow Sensor
   ├── DHT11
   └── I2C Ambient Light Sensor
          
                    │
                    ▼
                 Wi-Fi
                    │
                    ▼
              Blynk IoT
                    │
                    ▼
             Remote Dashboard


              ┌───────────────┐
              │  ESP32-CAM    │
              │ Real-Time     │
              │ Monitoring    │
              └───────────────┘

🔧 Hardware Components
Component            	Quantity	              Purpose
ESP32	                   1	             Main controller
ESP32-CAM	               1	             Real-time visual monitoring
Ultrasonic Sensor	       1	             Water-distance measurement
Soil Moisture Sensor	   1	             Soil moisture monitoring
Float Switch	           2	             Water-level detection
Water Flow Sensor	       1	             Flow-rate measurement
DHT11	                   1	             Temperature and humidity
I2C Ambient Light Sensor 1	             Ambient light measurement
SSD1306 OLED	           1	             Local display
Relay Module	           1	             Solenoid control
Solenoid Valve	         1	             Irrigation control
Push Buttons/Switches	   2	             OLED navigation
LED	                     1	             Wi-Fi status
Power Supply	      As required	         System power

The exact ambient-light sensor model should be specified according to the sensor used in the final hardware implementation.

📌 ESP32 Pin Configuration
Component                 	Function	              ESP32 GPIO
Ultrasonic 	                  TRIG	                  GPIO 18
Ultrasonic	                  ECHO	                  GPIO 19
Soil Moisture	             Analog OUT               	GPIO 34
Float Switch 1	          Digital Input	              GPIO 23
Float Switch 2	          Digital Input	              GPIO 25
Flow Sensor	               Pulse Input	              GPIO 35
DHT11	                        DATA	                  GPIO 13
Relay	                       Control	                GPIO 14
OLED	                        SDA	                    GPIO 21
OLED	                        SCL	                    GPIO 22
Ambient Sensor	              SDA	                    GPIO 21
Ambient Sensor	              SCL	                    GPIO 22
Switch 1	                Previous Page	              GPIO 32
Switch 2	                  Next Page	                GPIO 33
LED	                      Wi-Fi Status	              GPIO 2

⚙️ Operating Modes
1. Automatic Mode

The relay is controlled according to soil moisture.

SOIL DRY
   ↓
RELAY ON
   ↓
SOLENOID ON
SOIL WET
   ↓
RELAY OFF
   ↓
SOLENOID OFF
2. Manual Mode

Manual mode is selected through the Blynk dashboard.

Blynk MODE = ON
        ↓
    MANUAL MODE
        ↓
Blynk Relay Button
        ↓
      Relay
3. Offline Automatic Mode

If Wi-Fi is disconnected, the system automatically returns to local automatic operation.

Wi-Fi Disconnected
        ↓
Automatic Mode
        ↓
Soil Condition
     /       \
   DRY       WET
    ↓         ↓
 Relay ON   Relay OFF

This ensures that irrigation control does not depend completely on the Internet connection.

📺 OLED Interface

The OLED provides four monitoring pages.

Page 1
DIST: xx.x cm
SOIL: DRY
Page 2
LEVEL: HALF
FLOW: x.xx L/min
Page 3
TEMP: xx.x C
HUM: xx %
Page 4
AMBIENT: xxxx
RELAY: ON

The two physical switches are used to navigate between pages.

Switch 1 → Previous page
Switch 2 → Next page
📱 Blynk IoT

The system uses Blynk IoT for remote monitoring and control.

Virtual Pin Configuration
Virtual Pin	Parameter	Direction
V0	Water Distance	ESP32 → Blynk
V1	Soil ADC Value	ESP32 → Blynk
V2	Soil Status	ESP32 → Blynk
V3	Float Switch 1	ESP32 → Blynk
V4	Float Switch 2	ESP32 → Blynk
V5	Water Level	ESP32 → Blynk
V6	Flow Rate	ESP32 → Blynk
V7	Temperature	ESP32 → Blynk
V8	Humidity	ESP32 → Blynk
V9	Relay Status	ESP32 → Blynk
V10	Ambient Light	ESP32 → Blynk
V11	Operating Mode	Blynk → ESP32
V12	Manual Relay Control	Blynk → ESP32
V13	Wi-Fi Status	ESP32 → Blynk

No LDR or LDR-related datastream is used.

📷 ESP32-CAM Real-Time Monitoring

An ESP32-CAM is integrated into the project to provide real-time visual monitoring of the agricultural setup.

The camera can be used to observe:

Irrigation area
Plant growth area
Watering operation
Overall trainer setup
Remote visual status

The ESP32-CAM operates as a visual monitoring unit alongside the main ESP32 controller.

🔄 System Workflow
START
  ↓
Initialize ESP32
  ↓
Initialize Sensors
  ↓
Initialize OLED
  ↓
Initialize Wi-Fi
  ↓
Read Sensors
  ↓
Process Sensor Data
  ↓
Check Wi-Fi
  ↓
┌───────────────────────┐
│ Wi-Fi Connected?      │
└───────────┬───────────┘
        YES │ NO
            │
    ┌───────┴────────┐
    ↓                ↓
Blynk Mode       Automatic
    ↓            Operation
AUTO / MANUAL        ↓
    ↓            Soil Status
Relay Control        ↓
    └───────┬────────┘
            ↓
       Update OLED
            ↓
       Update Blynk
            ↓
         Repeat
🛡️ Safety Considerations
The ESP32 GPIO must not directly drive the solenoid valve.
A suitable relay/driver circuit must be used.
The solenoid should have an appropriate external power supply.
Use proper voltage-level protection for ESP32 inputs.
A 5 V ultrasonic ECHO signal must not be connected directly to an ESP32 GPIO.
Use a voltage divider or suitable level shifter when required.
Use appropriate grounding and power distribution.
Provide suitable protection against inductive switching noise.
💻 Software Requirements
Development Environment
Arduino IDE
ESP32 Board Package
Blynk IoT
Libraries
WiFi.h
BlynkSimpleEsp32.h
Wire.h
Adafruit_GFX.h
Adafruit_SSD1306.h
DHT.h

Additional library:

Ambient-light sensor specific library

The ambient-light library depends on the exact sensor model used.

🚀 Installation
Step 1: Clone the Repository
git clone https://github.com/YOUR_USERNAME/IoT-Smart-Agriculture-Trainer.git
Step 2: Open Arduino IDE

Open:

src/Smart_Agriculture_Trainer/Smart_Agriculture_Trainer.ino
Step 3: Install Required Libraries

Install the required libraries through:

Arduino IDE
→ Sketch
→ Include Library
→ Manage Libraries
Step 4: Configure Credentials

Replace the placeholder values in the source code:

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"


const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
Step 5: Select ESP32 Board

Select the appropriate ESP32 board and COM port.

Step 6: Upload

Compile and upload the firmware.

Step 7: Configure Blynk

Create the required datastreams and dashboard widgets.

Step 8: Calibrate Sensors

Calibrate:

Soil moisture
Flow sensor
Ultrasonic measurement
Ambient light sensor if required
🧪 Testing

The system can be tested using the following conditions:

Test	Expected Result
Dry soil	Relay ON
Wet soil	Relay OFF
Empty tank	NO WATER
Half tank	HALF
Full tank	FULL
Water flow	Flow rate displayed
DHT11 reading	Temperature and humidity displayed
Ambient light	Light value displayed
Blynk connected	Remote monitoring available
Manual mode	Relay controlled from Blynk
Wi-Fi disconnected	Automatic irrigation continues
OLED buttons	Pages change correctly
📊 Results

The developed system provides:

Real-time sensor monitoring
Automated irrigation
Remote IoT monitoring
Manual irrigation control
Local OLED monitoring
Water-level detection
Flow monitoring
Environmental monitoring
ESP32-CAM visual monitoring
Offline automatic operation
📸 Project Images

Project photographs and screenshots are available in the images/ and screenshots/ directories.

Recommended images include:

Complete hardware setup
ESP32 controller
ESP32-CAM
Sensor connections
OLED display pages
Blynk dashboard
Relay and solenoid
Automatic irrigation
Manual control
Final project setup
📚 Documentation

Detailed project documentation is available in:

docs/Project_Manual.pdf

The manual contains:

Project introduction
System architecture
Hardware description
Pin configuration
Wiring
Software requirements
Sensor operation
Irrigation control logic
OLED interface
Blynk configuration
Installation procedure
Testing
Troubleshooting
Results
Conclusion
🔮 Future Improvements

Possible future enhancements include:

AI-based crop/plant monitoring
Automated fertilizer monitoring
Weather API integration
Cloud data logging
Historical sensor graphs
Multiple irrigation zones
Solar-powered operation
Mobile notifications
Plant disease detection using ESP32-CAM
Advanced water-consumption analytics
👨‍💻 Project Author

Sakthimurugan V

Electronics and Communication Engineering

Interested in:

Embedded Systems
IoT
ESP32
Hardware Design
Sensor Interfacing
Automation
VLSI / RTL Design
