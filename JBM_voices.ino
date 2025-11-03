// ESP32 Input Pin Debounce Control
// Input Pin: 35 (detects HIGH to LOW transition)
// Output Pin 27: ON for debounce duration
// Output Pin 23: ON for 5 seconds only

// Global variables - easily adjustable
const unsigned long DEBOUNCE_TIME = 60000; // 3 minutes in milliseconds (3 * 60 * 1000)
const unsigned long PIN23_ON_TIME = 15000;   // 5 seconds in milliseconds

// Pin definitions
const int INPUT_PIN = 35;
const int OUTPUT_PIN_27 = 27;
const int OUTPUT_PIN_23 = 25;

// State tracking variables
int lastInputState = HIGH;
int currentInputState = HIGH;
unsigned long debounceStartTime = 0;
unsigned long pin23StartTime = 0;
bool debounceActive = false;
bool pin23Active = false;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println("ESP32 Debounce Control Started");
  
  // Configure pins
  pinMode(INPUT_PIN, INPUT_PULLUP);  // Input with internal pull-up
  pinMode(OUTPUT_PIN_27, OUTPUT);
  pinMode(OUTPUT_PIN_23, OUTPUT);
  
  // Ensure outputs are LOW initially
  digitalWrite(OUTPUT_PIN_27, LOW);
  digitalWrite(OUTPUT_PIN_23, LOW);
  
  Serial.println("Pin Configuration:");
  Serial.println("Input Pin: 35");
  Serial.println("Output Pin 27: Full debounce duration");
  Serial.println("Output Pin 23: 5 seconds only");
  Serial.printf("Debounce Time: %lu ms (%.1f minutes)\n", DEBOUNCE_TIME, DEBOUNCE_TIME/60000.0);
  Serial.printf("Pin 23 ON Time: %lu ms\n", PIN23_ON_TIME);
}

void loop() {
  currentInputState = digitalRead(INPUT_PIN);
  unsigned long currentTime = millis();
  
  // Detect HIGH to LOW transition
  if (lastInputState == HIGH && currentInputState == LOW && !debounceActive) {
    // State changed from HIGH to LOW - start debounce period
    debounceActive = true;
    pin23Active = true;
    debounceStartTime = currentTime;
    pin23StartTime = currentTime;
    
    // Turn ON both outputs
    digitalWrite(OUTPUT_PIN_27, HIGH);
    digitalWrite(OUTPUT_PIN_23, HIGH);
    
    Serial.println("\n=== Input Triggered (HIGH -> LOW) ===");
    Serial.println("Pin 27: ON");
    Serial.println("Pin 23: ON");
    Serial.printf("Debounce period started for %lu ms\n", DEBOUNCE_TIME);
  }
  
  // Handle Pin 23 timing (5 seconds only)
  if (pin23Active && (currentTime - pin23StartTime >= PIN23_ON_TIME)) {
    digitalWrite(OUTPUT_PIN_23, LOW);
    pin23Active = false;
    Serial.println("Pin 23: OFF (5 seconds elapsed)");
  }
  
  // Handle debounce timing (full duration for Pin 27)
  if (debounceActive && (currentTime - debounceStartTime >= DEBOUNCE_TIME)) {
    digitalWrite(OUTPUT_PIN_27, LOW);
    debounceActive = false;
    Serial.println("Pin 27: OFF (Debounce period completed)");
    Serial.println("=== Ready for next trigger ===\n");
  }
  
  lastInputState = currentInputState;
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}