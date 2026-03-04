// Final code 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <TinyGPSPlus.h>
#include <MAX30100_PulseOximeter.h>
#include <BluetoothSerial.h>

// Bluetooth
BluetoothSerial SerialBT;

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DHT11 Sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// GPS Module
#define GPS_RX 25
#define GPS_TX 26
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// SIM800L Module
#define SIM_RX 27
#define SIM_TX 14
HardwareSerial sim800l(2);

// MAX30100 Pulse Oximeter
PulseOximeter pox;
#define REPORTING_PERIOD_MS 1000

// SOS Button
#define SOS_BUTTON_PIN 15
bool lastButtonState = HIGH;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;
const long debounceDelay = 50;

// Display mode
int displayMode = 0;
unsigned long lastModeChange = 0;
const long modeInterval = 3000;

// Variables
float temperature = 0;
float humidity = 0;
double latitude = 0;
double longitude = 0;
int satellites = 0;
bool gpsValid = false;
bool sim800lValid = false;
bool max30100Valid = false;
float heartRate = 0;
float spO2 = 0;
uint32_t tsLastReport = 0;

// Alert thresholds (LOWEST and HIGHEST)
#define TEMP_HIGH_THRESHOLD 38.0
#define TEMP_LOW_THRESHOLD 35.0
#define HUMID_HIGH_THRESHOLD 80.0
#define HUMID_LOW_THRESHOLD 20.0
#define HR_HIGH_THRESHOLD 120
#define HR_LOW_THRESHOLD 50
#define SPO2_HIGH_THRESHOLD 100
#define SPO2_LOW_THRESHOLD 90

// Alert flags to prevent spam
bool tempAlertSent = false;
bool humidAlertSent = false;
bool hrAlertSent = false;
bool spo2AlertSent = false;

// Message display variables
String receivedMessage = "";
bool displayingMessage = false;
unsigned long messageDisplayStart = 0;
const long MESSAGE_DISPLAY_DURATION = 5000;

void onBeatDetected() {
  Serial.println("Beat detected!");
}

void scanI2C() {
  Serial.println("\n=== I2C Device Scanner ===");
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      if (address == 0x27) Serial.print(" (LCD)");
      if (address == 0x57) Serial.print(" (MAX30100)");
      
      Serial.println();
      nDevices++;
    }
  }
  
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("=== Scan Complete ===\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Complete Health Monitor with Bluetooth ===");
  
  // Initialize I2C
  Wire.begin(22, 21);
  Wire.setClock(100000);
  delay(100);
  
  scanI2C();
  
  // Initialize LCD
  Serial.println("Initializing LCD...");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("System Starting");
  delay(2000);
  
  // Initialize Bluetooth
  Serial.println("Initializing Bluetooth...");
  SerialBT.begin("ESP32_Health");
  Serial.println("Bluetooth: OK (Name: ESP32_Health)");
  lcd.clear();
  lcd.print("Bluetooth: OK");
  lcd.setCursor(0, 1);
  lcd.print("Pair your phone");
  delay(2000);
  
  // Initialize DHT11
  Serial.println("Initializing DHT11...");
  dht.begin();
  lcd.clear();
  lcd.print("DHT11: OK");
  delay(1000);
  
  // Initialize MAX30100
  Serial.println("Initializing MAX30100...");
  lcd.clear();
  lcd.print("MAX30100: Init");
  
  if (pox.begin()) {
    Serial.println("MAX30100: Connected!");
    max30100Valid = true;
    pox.setOnBeatDetectedCallback(onBeatDetected);
    pox.setIRLedCurrent(MAX30100_LED_CURR_50MA);
    lcd.setCursor(0, 1);
    lcd.print("MAX30100: OK");
  } else {
    Serial.println("MAX30100: FAILED!");
    max30100Valid = false;
    lcd.setCursor(0, 1);
    lcd.print("MAX30100: FAIL");
  }
  delay(1500);
  
  // Initialize GPS
  Serial.println("Initializing GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  lcd.clear();
  lcd.print("GPS: Searching");
  delay(1000);
  
  // Initialize SIM800L
  Serial.println("Initializing SIM800L...");
  sim800l.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(1000);
  
  // Initialize SOS Button
  Serial.println("Initializing SOS Button...");
  pinMode(SOS_BUTTON_PIN, INPUT_PULLUP);
  lcd.clear();
  lcd.print("SOS Button: OK");
  delay(1000);
  
  lcd.clear();
  lcd.print("Testing SIM800L");
  
  sim800l.println("AT");
  delay(1000);
  
  String response = "";
  unsigned long timeout = millis();
  while (millis() - timeout < 2000) {
    if (sim800l.available()) {
      response += (char)sim800l.read();
    }
  }
  
  if (response.indexOf("OK") >= 0) {
    Serial.println("SIM800L: Connected");
    sim800lValid = true;
    lcd.setCursor(0, 1);
    lcd.print("SIM: OK");
  } else {
    Serial.println("SIM800L: No response");
    lcd.setCursor(0, 1);
    lcd.print("SIM: Not found");
  }
  delay(1500);
  
  lcd.clear();
  lcd.print("System Ready!");
  delay(1000);
  lcd.clear();
  
  Serial.println("=== System Ready ===");
  Serial.println("Bluetooth: ESP32_Health");
  Serial.println("Waiting for phone connection...");
  Serial.println();
}

void loop() {
  // Update DHT11
  static unsigned long lastDHTRead = 0;
  if (millis() - lastDHTRead >= 2000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      temperature = t;
      humidity = h;
    }
    lastDHTRead = millis();
  }
  
  // Update MAX30100
  if (max30100Valid) {
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      heartRate = pox.getHeartRate();
      spO2 = pox.getSpO2();
      tsLastReport = millis();
    }
  }
  
  // Update GPS
  updateGPS();
  
  // Check SIM800L
  checkSIM800L();
  
  // Check SOS Button
  checkSOSButton();
  
  // Check thresholds and send alerts ONLY when threshold is hit
  checkHealthAlerts();
  
  // Handle incoming Bluetooth messages
  handleBluetoothMessages();
  
  // Handle message display timeout
  if (displayingMessage && (millis() - messageDisplayStart >= MESSAGE_DISPLAY_DURATION)) {
    displayingMessage = false;
    lcd.clear();
  }
  
  // Normal display cycle only when not displaying a message
  if (!displayingMessage) {
    if (millis() - lastModeChange >= modeInterval) {
      displayMode++;
      if (displayMode > 4) displayMode = 0;
      lastModeChange = millis();
      lcd.clear();
      delay(10);
    }
    
    // Display based on mode
    switch(displayMode) {
      case 0: displayTemperatureHumidity(); break;
      case 1: displayHeartRateSpO2(); break;
      case 2: displayGPS(); break;
      case 3: displaySystemStatus(); break;
      case 4: displayConnectivity(); break;
    }
  }
  
  delay(50);
}

// Send all sensor readings when threshold is hit
void sendAllSensorReadings(String alertType) {
  if (SerialBT.hasClient()) {
    SerialBT.println("");
    SerialBT.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    SerialBT.println("WARNING: " + alertType);
    SerialBT.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    SerialBT.println("");
    SerialBT.println("ALL SENSOR READINGS:");
    SerialBT.println("================================");
    SerialBT.println("Temperature: " + String(temperature, 1) + " C");
    SerialBT.println("Humidity: " + String(humidity, 1) + " %");
    
    if (max30100Valid && heartRate > 0) {
      SerialBT.println("Heart Rate: " + String(heartRate, 0) + " bpm");
      SerialBT.println("SpO2: " + String(spO2, 0) + " %");
    } else {
      SerialBT.println("Heart Rate: N/A");
      SerialBT.println("SpO2: N/A");
    }
    
    if (gpsValid) {
      SerialBT.println("Latitude: " + String(latitude, 6));
      SerialBT.println("Longitude: " + String(longitude, 6));
      SerialBT.println("Satellites: " + String(satellites));
    } else {
      SerialBT.println("GPS: Not locked");
    }
    
    SerialBT.println("================================");
    SerialBT.println("");
    
    Serial.println("WARNING SENT: " + alertType);
  } else {
    Serial.println("WARNING: " + alertType + " (No Bluetooth device connected)");
  }
}

// Check SOS Button with debouncing
void checkSOSButton() {
  int reading = digitalRead(SOS_BUTTON_PIN);
  
  // Check if button state changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // If button state is stable for debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If button is pressed (LOW because of INPUT_PULLUP)
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      sendSOSAlert();
    }
    // If button is released
    else if (reading == HIGH && buttonPressed) {
      buttonPressed = false;
    }
  }
  
  lastButtonState = reading;
}

// Send SOS Alert with all readings
void sendSOSAlert() {
  Serial.println("SOS BUTTON PRESSED!");
  
  if (SerialBT.hasClient()) {
    SerialBT.println("");
    SerialBT.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    SerialBT.println("       SOS EMERGENCY ALERT");
    SerialBT.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    SerialBT.println("");
    SerialBT.println("EMERGENCY: SOS button pressed");
    SerialBT.println("Immediate assistance required");
    SerialBT.println("");
    SerialBT.println("ALL SENSOR READINGS:");
    SerialBT.println("================================");
    SerialBT.println("Temperature: " + String(temperature, 1) + " C");
    SerialBT.println("Humidity: " + String(humidity, 1) + " %");
    
    if (max30100Valid && heartRate > 0) {
      SerialBT.println("Heart Rate: " + String(heartRate, 0) + " bpm");
      SerialBT.println("SpO2: " + String(spO2, 0) + " %");
    } else {
      SerialBT.println("Heart Rate: N/A");
      SerialBT.println("SpO2: N/A");
    }
    
    if (gpsValid) {
      SerialBT.println("");
      SerialBT.println("LOCATION:");
      SerialBT.println("Latitude: " + String(latitude, 6));
      SerialBT.println("Longitude: " + String(longitude, 6));
      SerialBT.println("Satellites: " + String(satellites));
      SerialBT.println("Google Maps: https://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6));
    } else {
      SerialBT.println("");
      SerialBT.println("GPS: Not locked - Location unavailable");
    }
    
    SerialBT.println("================================");
    SerialBT.println("");
    
    Serial.println("SOS ALERT SENT via Bluetooth");
  } else {
    Serial.println("SOS PRESSED but no Bluetooth device connected!");
  }
  
  // Show SOS on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!!! SOS SENT !!!");
  lcd.setCursor(0, 1);
  lcd.print("Check phone");
  delay(3000);
  lcd.clear();
}

// Check health thresholds - send alert ONLY when threshold is exceeded
void checkHealthAlerts() {
  // Temperature alerts (LOW or HIGH)
  if (temperature > 0) {
    if (temperature > TEMP_HIGH_THRESHOLD && !tempAlertSent) {
      String alert = "HIGH TEMPERATURE: " + String(temperature, 1) + " C (>" + String(TEMP_HIGH_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("TEMP HIGH!");
      tempAlertSent = true;
    }
    else if (temperature < TEMP_LOW_THRESHOLD && !tempAlertSent) {
      String alert = "LOW TEMPERATURE: " + String(temperature, 1) + " C (<" + String(TEMP_LOW_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("TEMP LOW!");
      tempAlertSent = true;
    }
  }
  // Reset flag when temperature normalizes
  if (temperature >= TEMP_LOW_THRESHOLD && temperature <= TEMP_HIGH_THRESHOLD) {
    tempAlertSent = false;
  }
  
  // Humidity alerts (LOW or HIGH)
  if (humidity > 0) {
    if (humidity > HUMID_HIGH_THRESHOLD && !humidAlertSent) {
      String alert = "HIGH HUMIDITY: " + String(humidity, 1) + " % (>" + String(HUMID_HIGH_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("HUMID HIGH!");
      humidAlertSent = true;
    }
    else if (humidity < HUMID_LOW_THRESHOLD && !humidAlertSent) {
      String alert = "LOW HUMIDITY: " + String(humidity, 1) + " % (<" + String(HUMID_LOW_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("HUMID LOW!");
      humidAlertSent = true;
    }
  }
  if (humidity >= HUMID_LOW_THRESHOLD && humidity <= HUMID_HIGH_THRESHOLD) {
    humidAlertSent = false;
  }
  
  // Heart Rate alerts (only if MAX30100 is working)
  if (max30100Valid && heartRate > 0) {
    if (heartRate > HR_HIGH_THRESHOLD && !hrAlertSent) {
      String alert = "HIGH HEART RATE: " + String(heartRate, 0) + " bpm (>" + String(HR_HIGH_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("HR HIGH!");
      hrAlertSent = true;
    }
    else if (heartRate < HR_LOW_THRESHOLD && !hrAlertSent) {
      String alert = "LOW HEART RATE: " + String(heartRate, 0) + " bpm (<" + String(HR_LOW_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("HR LOW!");
      hrAlertSent = true;
    }
    
    if (heartRate >= HR_LOW_THRESHOLD && heartRate <= HR_HIGH_THRESHOLD) {
      hrAlertSent = false;
    }
  }
  
  // SpO2 alerts
  if (max30100Valid && spO2 > 0) {
    if (spO2 < SPO2_LOW_THRESHOLD && !spo2AlertSent) {
      String alert = "LOW BLOOD OXYGEN: " + String(spO2, 0) + " % (<" + String(SPO2_LOW_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("SPO2 LOW!");
      spo2AlertSent = true;
    }
    else if (spO2 > SPO2_HIGH_THRESHOLD && !spo2AlertSent) {
      String alert = "HIGH BLOOD OXYGEN: " + String(spO2, 0) + " % (>" + String(SPO2_HIGH_THRESHOLD) + ")";
      sendAllSensorReadings(alert);
      showLCDAlert("SPO2 HIGH!");
      spo2AlertSent = true;
    }
    
    if (spO2 >= SPO2_LOW_THRESHOLD && spO2 <= SPO2_HIGH_THRESHOLD) {
      spo2AlertSent = false;
    }
  }
}

// Show alert on LCD briefly
void showLCDAlert(String alertMsg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("! WARNING !");
  lcd.setCursor(0, 1);
  lcd.print(alertMsg);
  delay(2000);
  lcd.clear();
}

// Handle incoming Bluetooth messages
void handleBluetoothMessages() {
  if (SerialBT.available()) {
    String message = SerialBT.readString();
    message.trim();
    
    // Check if it's a command or a message to display
    String upperMsg = message;
    upperMsg.toUpperCase();
    
    if (upperMsg == "SOS") {
      // Trigger SOS alert when "SOS" message is received
      Serial.println("SOS command received via Bluetooth!");
      sendSOSAlert();
    }
    else if (upperMsg == "STATUS") {
      SerialBT.println("=== SYSTEM STATUS ===");
      SerialBT.println("Temperature: " + String(temperature, 1) + " C");
      SerialBT.println("Humidity: " + String(humidity, 1) + " %");
      if (max30100Valid) {
        SerialBT.println("Heart Rate: " + String(heartRate, 0) + " bpm");
        SerialBT.println("SpO2: " + String(spO2, 0) + " %");
      }
      SerialBT.println("GPS: " + String(gpsValid ? "Locked" : "Searching"));
      SerialBT.println("Satellites: " + String(satellites));
      SerialBT.println("====================");
    }
    else if (upperMsg == "GPS") {
      if (gpsValid) {
        SerialBT.println("Latitude: " + String(latitude, 6));
        SerialBT.println("Longitude: " + String(longitude, 6));
      } else {
        SerialBT.println("GPS not locked yet");
      }
    }
    else if (upperMsg == "THRESHOLDS") {
      SerialBT.println("=== ALERT THRESHOLDS ===");
      SerialBT.println("Temp: " + String(TEMP_LOW_THRESHOLD) + " - " + String(TEMP_HIGH_THRESHOLD) + " C");
      SerialBT.println("Humidity: " + String(HUMID_LOW_THRESHOLD) + " - " + String(HUMID_HIGH_THRESHOLD) + " %");
      SerialBT.println("HR: " + String(HR_LOW_THRESHOLD) + " - " + String(HR_HIGH_THRESHOLD) + " bpm");
      SerialBT.println("SpO2: " + String(SPO2_LOW_THRESHOLD) + " - " + String(SPO2_HIGH_THRESHOLD) + " %");
      SerialBT.println("=======================");
    }
    else if (upperMsg == "HELP") {
      SerialBT.println("=== COMMANDS ===");
      SerialBT.println("SOS - Trigger emergency alert");
      SerialBT.println("STATUS - Current readings");
      SerialBT.println("GPS - GPS coordinates");
      SerialBT.println("THRESHOLDS - Alert limits");
      SerialBT.println("HELP - This list");
      SerialBT.println("Any other text will display on LCD for 5 seconds");
      SerialBT.println("================");
    }
    else {
      // It's a message to display on LCD
      receivedMessage = message;
      displayingMessage = true;
      messageDisplayStart = millis();
      
      lcd.clear();
      
      // Display message on LCD (split if longer than 16 chars)
      if (message.length() <= 16) {
        lcd.setCursor(0, 0);
        lcd.print(message);
      } else {
        lcd.setCursor(0, 0);
        lcd.print(message.substring(0, 16));
        lcd.setCursor(0, 1);
        lcd.print(message.substring(16, min((int)message.length(), 32)));
      }
      
      SerialBT.println("Message displayed on LCD");
      Serial.println("Message received: " + message);
    }
  }
}

void updateGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
  
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    gpsValid = true;
  } else {
    gpsValid = false;
  }
  
  if (gps.satellites.isValid()) {
    satellites = gps.satellites.value();
  }
}

void checkSIM800L() {
  if (sim800l.available()) {
    Serial.print("SIM800L: ");
    while (sim800l.available()) {
      Serial.write(sim800l.read());
      delay(2);
    }
    Serial.println();
  }
}

void displayTemperatureHumidity() {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" C   ");
  
  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(humidity, 1);
  lcd.print("%   ");
}

void displayHeartRateSpO2() {
  lcd.setCursor(0, 0);
  lcd.print("HR: ");
  if (max30100Valid && heartRate > 0) {
    lcd.print(heartRate, 0);
    lcd.print(" bpm     ");
  } else {
    lcd.print("--        ");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("SpO2: ");
  if (max30100Valid && spO2 > 0) {
    lcd.print(spO2, 0);
    lcd.print("%      ");
  } else {
    lcd.print("--       ");
  }
}

void displayGPS() {
  lcd.setCursor(0, 0);
  if (gpsValid) {
    lcd.print("Lat:");
    lcd.print(latitude, 4);
    lcd.print("  ");
  } else {
    lcd.print("GPS: Searching  ");
  }
  
  lcd.setCursor(0, 1);
  if (gpsValid) {
    lcd.print("Lon:");
    lcd.print(longitude, 4);
    lcd.print("  ");
  } else {
    lcd.print("Sats: ");
    lcd.print(satellites);
    lcd.print("         ");
  }
}

void displaySystemStatus() {
  lcd.setCursor(0, 0);
  lcd.print("DHT:");
  lcd.print(temperature > 0 ? "OK " : "NO ");
  lcd.print("GPS:");
  lcd.print(gpsValid ? "OK " : "NO ");
  
  lcd.setCursor(0, 1);
  lcd.print("HR:");
  lcd.print(max30100Valid ? "OK " : "NO ");
  lcd.print("SIM:");
  lcd.print(sim800lValid ? "OK" : "NO");
}

void displayConnectivity() {
  lcd.setCursor(0, 0);
  lcd.print("BT: ");
  lcd.print(SerialBT.hasClient() ? "Connected" : "Waiting  ");
  
  lcd.setCursor(0, 1);
  lcd.print("SIM:");
  lcd.print(sim800lValid ? "OK " : "NO ");
  lcd.print("GPS:");
  lcd.print(gpsValid ? "OK" : "NO");
}