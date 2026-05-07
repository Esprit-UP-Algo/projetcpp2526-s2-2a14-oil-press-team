// Gas Sensor + OLED + Keypad Integration for Oil Press Manager
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

// --- OLED Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Keypad Setup ---
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Adjust to your actual wiring
byte colPins[COLS] = {5, 4, 3, 2}; // Adjust to your actual wiring
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Machine Setup (Using CODE instead of ID) ---
const String MACHINE_CODE = "1234"; // Replace with your actual 4-digit db code
String enteredCode = "";

// --- Gas Sensor Setup ---
const int MQ2_PIN = A0;             // Analog pin connected to MQ-2
const int THRESHOLD = 360;          // Gas threshold
unsigned long lastAlertTime = 0;
const unsigned long DEBOUNCE_DELAY = 5000;

void setup() {
  Serial.begin(115200);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  resetScreen();
}

void loop() {
  // =====================================
  // 1. Gas Sensor Monitoring
  // =====================================
  int gasValue = analogRead(MQ2_PIN);
  
  if (gasValue > THRESHOLD && (millis() - lastAlertTime > DEBOUNCE_DELAY)) {
    // Format expected by Qt application: ALERT_SMOKE:<CODE>
    Serial.print("ALERT_SMOKE:");
    Serial.println(MACHINE_CODE);
    lastAlertTime = millis();
  }
  
  // =====================================
  // 2. Keypad & OLED Interaction
  // =====================================
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') { // Using '#' as an ENTER button
      display.clearDisplay();
      display.setCursor(0, 10);
      
      if (enteredCode == MACHINE_CODE) {
        display.setTextSize(2);
        display.println("GRANTED!");
        Serial.print("AUTH_SUCCESS:");
        Serial.println(MACHINE_CODE);
      } else {
        display.setTextSize(2);
        display.println("DENIED!");
        Serial.print("AUTH_FAILED:");
        Serial.println(enteredCode);
      }
      display.display();
      delay(2000);
      
      enteredCode = ""; // Reset
      resetScreen();
    } 
    else if (key == '*') { // Using '*' as a CLEAR button
      enteredCode = "";
      resetScreen();
    }
    else {
      // Append digit if length is less than 4
      if (enteredCode.length() < 4) {
        enteredCode += key;
        
        display.clearDisplay();
        display.setCursor(0, 10);
        display.setTextSize(1);
        display.println("Enter 4-Digit Code:");
        
        // Show what is being typed
        display.setCursor(0, 30);
        display.setTextSize(2);
        display.println(enteredCode);
        display.display();
      }
    }
  }
}

void resetScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Enter 4-Digit Code:");
  display.display();
}
