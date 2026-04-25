// MQ-2 Gas Sensor Monitoring for Oil Press Manager
// This code continuously monitors gas levels and sends an alert over Serial if a threshold is exceeded.

const int MQ2_PIN = A0;             // Analog pin connected to MQ-2
const int THRESHOLD = 360;          // Gas threshold
const int MACHINE_ID = 1;           // The ID of the machine this Arduino is attached to

unsigned long lastAlertTime = 0;
const unsigned long DEBOUNCE_DELAY = 5000; // Minimum time (5 seconds) between sending alerts

void setup() {
  Serial.begin(115200);
  
  // Optional: Allow the sensor to warm up before taking readings
  // delay(20000); 
}

void loop() {
  int gasValue = analogRead(MQ2_PIN);
  
  // -- LIGNE AJOUTÉE POUR LE DEBUG --
  // Affiche la valeur en temps réel dans le moniteur série
  Serial.print("Valeur actuelle du gaz : ");
  Serial.println(gasValue);
  
  // If gas level exceeds the safe threshold
  if (gasValue > THRESHOLD) {
    
    // Check if enough time has passed since the last alert to avoid spamming the Serial port
    if (millis() - lastAlertTime > DEBOUNCE_DELAY) {
      
      // Format expected by Qt application: ALERT_GAS:<ID>
      Serial.print("ALERT_GAS:");
      Serial.println(MACHINE_ID);
      
      lastAlertTime = millis();
    }
  }
  
  // Short delay before the next reading (augmenté un peu pour la lisibilité)
  delay(1000); 
}
