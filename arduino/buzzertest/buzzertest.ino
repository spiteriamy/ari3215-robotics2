#define BUZZER_PIN 10 // the pin where the buzzer is connected
#define LED_PIN 3    // the pin where the onboard LED is connected

void setup()
{
    Serial.begin(9600);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Buzzer test ready");
}

void loop()
{
    tone(BUZZER_PIN, 500); // Send 5KHz sound signal...
    digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate buzzer is active
    
    delay(1000);
    noTone(BUZZER_PIN);     // Stop sound...
    digitalWrite(LED_PIN, LOW); // Turn off LED to indicate buzzer is inactive
    delay(1000);
}