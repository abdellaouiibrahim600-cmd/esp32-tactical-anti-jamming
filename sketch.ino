#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Tactical system pin definitions
#define RF_NOISE_PIN   34   // Noise / Jamming signal simulation (Potentiometer)
#define LED_SECURE     25   // Green LED (Secure Channel)
#define LED_JAMMED     26   // Red LED (Jamming Attack Detected)
#define BUZZER_PIN     27   // Alarm & Alert buzzer

// Frequency Hopping Spread Spectrum (FHSS) variables
int currentChannel = 1;
unsigned long lastHopTime = 0;
const unsigned long hopInterval = 1500; // Hop channel every 1.5s during attack

void setup() {
  Serial.begin(115200);

  pinMode(LED_SECURE, OUTPUT);
  pinMode(LED_JAMMED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  Serial.println(F("[TACTICAL_EW] Anti-Jamming & FHSS Node Initialized. Scanning spectrum..."));
}

void loop() {
  // Read noise or jamming level from the potentiometer
  int noiseLevel = analogRead(RF_NOISE_PIN);
  
  // Jamming detection threshold
  bool isJammed = (noiseLevel > 2500);

  unsigned long currentMillis = millis();

  if (isJammed) {
    if (currentMillis - lastHopTime >= hopInterval) {
      lastHopTime = currentMillis;
      currentChannel = (currentChannel % 5) + 1; // Hop across 5 different channels
    }
    
    digitalWrite(LED_JAMMED, HIGH);
    digitalWrite(LED_SECURE, LOW);
    
    // Audio alert using tone (2000Hz frequency)
    tone(BUZZER_PIN, 2000); 

    // Print attack telemetry packet in JSON format
    Serial.print("{\"event\":\"JAMMING_DETECTED\",\"noise_rssi\":");
    Serial.print(noiseLevel);
    Serial.print(",\"fhss_active\":true,\"hopped_to_channel\":");
    Serial.print(currentChannel);
    Serial.println(",\"status\":\"EVASION_MODE\"}");

    // Update display with emergency and hopping status
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("!! JAMMING ALERT !l"));
    display.setCursor(0, 20);
    display.print(F("Noise RSSI: ")); display.println(noiseLevel);
    display.setCursor(0, 40);
    display.print(F("FHSS Channel: CH-")); display.println(currentChannel);
    display.display();

  } else {
    digitalWrite(LED_JAMMED, LOW);
    digitalWrite(LED_SECURE, HIGH);
    
    // Stop audio alert in secure state
    noTone(BUZZER_PIN);

    // Print secure state telemetry packet
    Serial.print("{\"event\":\"SPECTRUM_CLEAR\",\"noise_rssi\":");
    Serial.print(noiseLevel);
    Serial.print(",\"fhss_active\":false,\"channel\":");
    Serial.print(currentChannel);
    Serial.println(",\"status\":\"SECURE_LINK\"}");

    // Update display with secure status
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("=== TACTICAL LINK ==="));
    display.setCursor(0, 20);
    display.print(F("Status: SECURE"));
    display.setCursor(0, 40);
    display.print(F("Active CH: CH-")); display.println(currentChannel);
    display.display();
  }

  delay(200);
}