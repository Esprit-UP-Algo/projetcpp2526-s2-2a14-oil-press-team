// MQ-2 Gas Sensor Monitoring & Intervention for Oil Press Manager
// Refined with signal filtering to prevent false positives from cable noise.

const int MQ2_PIN = A0;             
const int THRESHOLD = 360;          
const int MACHINE_ID = 1;           

const int PIN_RELAY  = 6;           
const int PIN_BUZZER = 7;           
const int PIN_RED    = 8;           
const int PIN_GREEN  = 9;           

unsigned long lastAlertTime = 0;
const unsigned long DEBOUNCE_DELAY = 10000; 

// Filtering variables
int highReadingCount = 0;
const int SAMPLES_REQUIRED = 5; // Must be high for 5 consecutive readings (~2.5 seconds)

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RELAY,  OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RED,    OUTPUT);
  pinMode(PIN_GREEN,  OUTPUT);
  
  setSafeState(); 
}

void loop() {
  // 1. SENSING LOGIC with Filtering
  int gasValue = analogRead(MQ2_PIN);
  
  if (gasValue > THRESHOLD) {
    highReadingCount++;
    if (highReadingCount >= SAMPLES_REQUIRED) {
      if (millis() - lastAlertTime > DEBOUNCE_DELAY) {
        Serial.print("ALERT_GAS:");
        Serial.println(MACHINE_ID);
        lastAlertTime = millis();
      }
      highReadingCount = SAMPLES_REQUIRED; 
    }
  } else {
    highReadingCount = 0;
  }
  
  // 2. INTERVENTION LOGIC (Receive from Qt - Database Driven)
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "DANGER") {
      setDangerState();
    } 
    else if (command == "SAFE") {
      setSafeState();
    }
  }
  
  delay(500); 
}

void setDangerState() {
  digitalWrite(PIN_RELAY,  HIGH); 
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_RED,    HIGH);
  digitalWrite(PIN_GREEN,  LOW);
}

void setSafeState() {
  digitalWrite(PIN_RELAY,  LOW);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_RED,    LOW);
  digitalWrite(PIN_GREEN,  HIGH);
}
