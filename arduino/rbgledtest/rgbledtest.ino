#include <Adafruit_NeoPixel.h>

#define PIN A2      // The image shows the green wire connected to Pin 7
#define NUMPIXELS 5 // Set this to the actual number of LEDs on your strip

// Setup the NeoPixel library
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool isCycling = false;
uint16_t cyclePos = 0;

void setup()
{
    Serial.begin(9600);
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.show();  // Turn OFF all pixels ASAP
    Serial.println("Ready. Commands:");
    Serial.println("  'start' -> Cycle through spectrum");
    Serial.println("  'stop'  -> Stop cycling");
    Serial.println("  'addr,color' -> Set specific LED (e.g. 1,128)");
}

void loop()
{
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        input.trim(); // Remove whitespace

        if (input.equalsIgnoreCase("stop"))
        {
            isCycling = false;
            Serial.println("Stopped.");
        }
        else if (input.equalsIgnoreCase("start"))
        {
            isCycling = true;
            Serial.println("Starting spectrum cycle.");
        }
        else
        {
            // Try to parse "address,color"
            int commaIndex = input.indexOf(',');
            if (commaIndex != -1)
            {
                int address = input.substring(0, commaIndex).toInt();
                int colorIndex = input.substring(commaIndex + 1).toInt();

                if (address >= 0 && address < NUMPIXELS)
                {
                    isCycling = false; // Stop cycling to show manual color
                    uint32_t color = Wheel(colorIndex & 255);
                    pixels.setPixelColor(address, color);
                    pixels.show();
                    Serial.print("LED ");
                    Serial.print(address);
                    Serial.print(" -> Wheel Pos ");
                    Serial.println(colorIndex);
                }
            }
        }
    }

    if (isCycling)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, Wheel(cyclePos & 255));
        }
        pixels.show();
        cyclePos++;
        // delay(20);
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}