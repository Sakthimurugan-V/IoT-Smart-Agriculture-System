#define BLYNK_TEMPLATE_ID "TMPL3QuQU6TaV"
#define BLYNK_TEMPLATE_NAME "Smart Agriculture"
#define BLYNK_AUTH_TOKEN "InxYt2luFehLLKIZGTanffCnZG1kMhkX"
// Uncomment to see Blynk debug prints on Serial Monitor
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <BH1750.h>              

const char* ssid     = "Wifi_ID";
const char* password = "Wifi_Pass";

// Ultrasonic
#define TRIG_PIN        18
#define ECHO_PIN        19

// Soil moisture 
#define SOIL_PIN        34

// Float switches
#define FLOAT1_PIN      23
#define FLOAT2_PIN      25

// Flow sensor 
#define FLOW_PIN        35

// DHT11
#define DHT_PIN         13
#define DHT_TYPE        DHT11

// Relay / Solenoid valve
#define RELAY_PIN       14

// I2C bus (shared: OLED + ambient light sensor)
#define I2C_SDA         21
#define I2C_SCL         22

// OLED page-navigation buttons
#define SWITCH1_PIN     32   // previous page
#define SWITCH2_PIN     33   // next page

// Wi-Fi status LED
#define WIFI_LED_PIN    2

#define FLOAT1_ACTIVE_LOW true
#define FLOAT2_ACTIVE_LOW true

#define RELAY_ACTIVE_LOW true


#define SOIL_DRY_VALUE   3000   // raw ADC reading in dry condition
#define SOIL_WET_VALUE   1200   // raw ADC reading in wet condition

#define FLOW_CALIBRATION_FACTOR 450.0  // pulses per litre (example only)

// OLED 
#define OLED_WIDTH       128
#define OLED_HEIGHT      64
#define OLED_ADDRESS     0x3C    
#define OLED_RESET       -1

// Timing intervals (ms) 
#define ULTRASONIC_INTERVAL   500
#define SOIL_INTERVAL          500
#define DHT_INTERVAL           2500
#define FLOW_CALC_INTERVAL     1000
#define AMBIENT_INTERVAL        500
#define OLED_REFRESH_INTERVAL   300
#define BLYNK_UPDATE_INTERVAL  1000
#define WIFI_RECONNECT_INTERVAL 5000
#define BUTTON_DEBOUNCE_MS       50

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightMeter;              
BlynkTimer blynkTimer;


// Ultrasonic
float waterDistanceCm = 0;
bool  ultrasonicError = false;

// Soil moisture
int   soilRawValue = 0;
bool  soilIsDry = true;

// Float switches / water level
bool  float1State = false;   
bool  float2State = false;
enum WaterLevel { LEVEL_NONE, LEVEL_HALF, LEVEL_FULL, LEVEL_INVALID };
WaterLevel currentWaterLevel = LEVEL_NONE;

// Flow sensor
volatile unsigned long flowPulseCount = 0;
float flowRateLPM = 0;

// DHT11
float temperatureC = NAN;
float humidityPct   = NAN;
bool  dhtError = false;

// Ambient light
float ambientLux = 0;
bool  ambientSensorOK = false;

// Relay / mode
bool  relayIsOn = false;
bool  modeManual = false;        
bool  effectiveManualMode = false; 
bool  manualRelayCommand = false;  

// Wi-Fi
bool wifiConnected = false;
unsigned long lastWifiAttempt = 0;

// OLED paging
int currentPage = 0;             
const int TOTAL_PAGES = 4;

// Button debounce state
struct ButtonState {
  bool lastReading = HIGH;
  bool stableState = HIGH;
  unsigned long lastChangeTime = 0;
};
ButtonState switch1State, switch2State;

void IRAM_ATTR flowPulseISR() {
  flowPulseCount++;
}

void handleWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConnected) {
      wifiConnected = true;
      Serial.println("[WiFi] Connected.");
    }
  } else {
    if (wifiConnected) {
      wifiConnected = false;
      Serial.println("[WiFi] Disconnected.");
    }

    unsigned long now = millis();
    if (now - lastWifiAttempt >= WIFI_RECONNECT_INTERVAL) {
      lastWifiAttempt = now;
      Serial.println("[WiFi] Attempting reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
}

void updateWiFiLED() {
  digitalWrite(WIFI_LED_PIN, wifiConnected ? HIGH : LOW);
}



// ---- Ultrasonic ------------------------------------------------------------
void readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);

  if (duration == 0) {
    ultrasonicError = true;
  } else {
    ultrasonicError = false;
    waterDistanceCm = duration * 0.0343 / 2.0;  // speed of sound formula
  }
}

// Soil moisture 
void readSoilMoisture() {
  soilRawValue = analogRead(SOIL_PIN);

  if (SOIL_DRY_VALUE >= SOIL_WET_VALUE) {
    int midpoint = (SOIL_DRY_VALUE + SOIL_WET_VALUE) / 2;
    soilIsDry = (soilRawValue >= midpoint);
  } else {
    int midpoint = (SOIL_DRY_VALUE + SOIL_WET_VALUE) / 2;
    soilIsDry = (soilRawValue <= midpoint);
  }
}

// Float switches / water level 
void readFloatSwitches() {
  int raw1 = digitalRead(FLOAT1_PIN);
  int raw2 = digitalRead(FLOAT2_PIN);

  float1State = FLOAT1_ACTIVE_LOW ? (raw1 == LOW) : (raw1 == HIGH);
  float2State = FLOAT2_ACTIVE_LOW ? (raw2 == LOW) : (raw2 == HIGH);
}

void calculateWaterLevel() {
  if (!float1State && !float2State) {
    currentWaterLevel = LEVEL_NONE;
  } else if (!float1State && float2State) {
    currentWaterLevel = LEVEL_HALF;
  } else if (float1State && float2State) {
    currentWaterLevel = LEVEL_FULL;
  } else { // float1State true, float2State false
    currentWaterLevel = LEVEL_INVALID;
  }
}

const char* waterLevelToString(WaterLevel lvl) {
  switch (lvl) {
    case LEVEL_NONE:    return "NO WATER";
    case LEVEL_HALF:    return "HALF";
    case LEVEL_FULL:    return "FULL";
    case LEVEL_INVALID: return "INVALID";
    default:            return "UNKNOWN";
  }
}

// Flow rate
void calculateFlowRate() {
  
  noInterrupts();
  unsigned long pulses = flowPulseCount;
  flowPulseCount = 0;
  interrupts();

  // litres = pulses / calibration_factor
  float litres = pulses / FLOW_CALIBRATION_FACTOR;
  float minutesElapsed = FLOW_CALC_INTERVAL / 60000.0;
  flowRateLPM = litres / minutesElapsed;
}

// DHT11 
void readDHT11() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    dhtError = true;
    
  } else {
    dhtError = false;
    temperatureC = t;
    humidityPct  = h;
  }
}

// Ambient light sensor 

void initAmbientLightSensor() {
  ambientSensorOK = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  if (!ambientSensorOK) {
    Serial.println("[Ambient] BH1750 not found on I2C bus.");
  }
}

void readAmbientLight() {
  if (!ambientSensorOK) {
    initAmbientLightSensor();
    return;
  }
  float lux = lightMeter.readLightLevel();
  if (lux < 0) {
    ambientSensorOK = false; 
  } else {
    ambientLux = lux;
  }
}

void setRelay(bool on) {
  relayIsOn = on;
  bool pinLevel = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(RELAY_PIN, pinLevel ? HIGH : LOW);
}

void controlRelay() {
 
  effectiveManualMode = wifiConnected && modeManual;

  if (effectiveManualMode) {
    setRelay(manualRelayCommand);
  } else {
    // AUTOMATIC: soil-based control
    setRelay(soilIsDry); // DRY -> ON, WET -> OFF
  }
}

bool debouncedPress(int pin, ButtonState &btn) {
  bool reading = digitalRead(pin);
  bool pressedEdge = false;

  if (reading != btn.lastReading) {
    btn.lastChangeTime = millis();
  }

  if ((millis() - btn.lastChangeTime) > BUTTON_DEBOUNCE_MS) {
    if (reading != btn.stableState) {
      btn.stableState = reading;
      if (btn.stableState == LOW) {
        pressedEdge = true; // fires once per press, not while held
      }
    }
  }

  btn.lastReading = reading;
  return pressedEdge;
}

void handleOLEDButtons() {
  if (debouncedPress(SWITCH1_PIN, switch1State)) {
    currentPage = (currentPage - 1 + TOTAL_PAGES) % TOTAL_PAGES;
  }
  if (debouncedPress(SWITCH2_PIN, switch2State)) {
    currentPage = (currentPage + 1) % TOTAL_PAGES;
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header: page indicator + mode
  display.setCursor(0, 0);
  display.print("PAGE ");
  display.print(currentPage + 1);
  display.print("/");
  display.print(TOTAL_PAGES);
  display.setCursor(80, 0);
  display.print(effectiveManualMode ? "MANUAL" : "AUTO");
  display.drawLine(0, 10, OLED_WIDTH, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 20);

  switch (currentPage) {
    case 0: { // Page 1: Water distance + Soil status
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print("DIST: ");
      if (ultrasonicError) display.print("ERROR");
      else { display.print(waterDistanceCm, 1); display.print(" cm"); }

      display.setCursor(0, 36);
      display.print("SOIL: ");
      display.print(soilIsDry ? "DRY" : "WET");
      display.setCursor(0, 50);
      display.print("RAW: ");
      display.print(soilRawValue);
      break;
    }
    case 1: { // Page 2: Water level + Flow rate
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print("WATER LEVEL: ");
      display.print(waterLevelToString(currentWaterLevel));

      display.setCursor(0, 36);
      display.print("FLOW: ");
      display.print(flowRateLPM, 2);
      display.print(" L/min");

      display.setCursor(0, 50);
      display.print("F1:");
      display.print(float1State ? "ON " : "OFF");
      display.print(" F2:");
      display.print(float2State ? "ON" : "OFF");
      break;
    }
    case 2: { // Page 3: Temperature + Humidity
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print("TEMP: ");
      if (dhtError) display.print("ERROR");
      else { display.print(temperatureC, 1); display.print(" C"); }

      display.setCursor(0, 36);
      display.print("HUMIDITY: ");
      if (dhtError) display.print("ERROR");
      else { display.print(humidityPct, 1); display.print(" %"); }
      break;
    }
    case 3: { // Page 4: Ambient light + Relay status
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print("AMBIENT: ");
      if (!ambientSensorOK) display.print("ERROR");
      else display.print(ambientLux, 0);

      display.setCursor(0, 36);
      display.print("RELAY: ");
      display.print(relayIsOn ? "ON" : "OFF");

      display.setCursor(0, 50);
      display.print("WiFi: ");
      display.print(wifiConnected ? "CONNECTED" : "DISCONNECTED");
      break;
    }
  }

  display.display();
}

void updateBlynk() {
  if (!wifiConnected || !Blynk.connected()) return;

  Blynk.virtualWrite(V0,  waterDistanceCm);
  Blynk.virtualWrite(V1,  soilRawValue);
  Blynk.virtualWrite(V2,  soilIsDry ? "DRY" : "WET");
  Blynk.virtualWrite(V3,  float1State ? 1 : 0);
  Blynk.virtualWrite(V4,  float2State ? 1 : 0);
  Blynk.virtualWrite(V5,  waterLevelToString(currentWaterLevel));
  Blynk.virtualWrite(V6,  flowRateLPM);
  Blynk.virtualWrite(V7,  dhtError ? -1 : temperatureC);
  Blynk.virtualWrite(V8,  dhtError ? -1 : humidityPct);
  Blynk.virtualWrite(V9,  relayIsOn ? 1 : 0);
  Blynk.virtualWrite(V10, ambientSensorOK ? ambientLux : -1);
  Blynk.virtualWrite(V11, modeManual ? 1 : 0);
  Blynk.virtualWrite(V13, wifiConnected ? "CONNECTED" : "DISCONNECTED");
 
BLYNK_WRITE(V11) {
  modeManual = (param.asInt() == 1);
}

BLYNK_WRITE(V12) {
  manualRelayCommand = (param.asInt() == 1);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // ---- Pin modes ----
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(SOIL_PIN, INPUT);            

  pinMode(FLOAT1_PIN, INPUT_PULLUP);
  pinMode(FLOAT2_PIN, INPUT_PULLUP);

 
  pinMode(FLOW_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false); 

  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);

  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW);

  // Interrupt for flow sensor 
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowPulseISR, FALLING);

  // I2C bus (shared by OLED + ambient light sensor) 
  Wire.begin(I2C_SDA, I2C_SCL);

  // OLED 
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("[OLED] SSD1306 allocation failed.");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Smart Agriculture");
    display.println("Trainer - Booting...");
    display.display();
  }

  // DHT11 
  dht.begin();

  // Ambient light sensor
  initAmbientLightSensor();

  // Wi-Fi (non-blocking start) 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  lastWifiAttempt = millis();

  // Blynk 
  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.println("[Setup] Complete.");
}


void loop() {
  unsigned long now = millis();

  handleWiFi();
  updateWiFiLED();
  if (wifiConnected) {
    Blynk.run();
    blynkTimer.run();
  }

  // Ultrasonic 
  static unsigned long lastUltrasonic = 0;
  if (now - lastUltrasonic >= ULTRASONIC_INTERVAL) {
    lastUltrasonic = now;
    readUltrasonic();
  }

  // Soil moisture 
  static unsigned long lastSoil = 0;
  if (now - lastSoil >= SOIL_INTERVAL) {
    lastSoil = now;
    readSoilMoisture();
  }

  // Float switches / water level
  readFloatSwitches();
  calculateWaterLevel();

  // Flow rate 
  static unsigned long lastFlowCalc = 0;
  if (now - lastFlowCalc >= FLOW_CALC_INTERVAL) {
    lastFlowCalc = now;
    calculateFlowRate();
  }

  // DHT11 
  static unsigned long lastDHT = 0;
  if (now - lastDHT >= DHT_INTERVAL) {
    lastDHT = now;
    readDHT11();
  }

  // ---- Ambient light ----
  static unsigned long lastAmbient = 0;
  if (now - lastAmbient >= AMBIENT_INTERVAL) {
    lastAmbient = now;
    readAmbientLight();
  }

  // ---- Relay control logic (priority rules from Section 30) ----
  controlRelay();

  // ---- OLED page navigation + refresh ----
  handleOLEDButtons();
  static unsigned long lastOLED = 0;
  if (now - lastOLED >= OLED_REFRESH_INTERVAL) {
    lastOLED = now;
    updateOLED();
  }

  // ---- Blynk periodic update ----
  static unsigned long lastBlynkUpdate = 0;
  if (now - lastBlynkUpdate >= BLYNK_UPDATE_INTERVAL) {
    lastBlynkUpdate = now;
    updateBlynk();
  }
}
