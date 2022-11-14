#include "Particle.h"
#include <Wire.h>
#include "Module/MAX30105.h"
#include "Module/heartRate.h"
#include "Module/Adafruit_GFX.h"
#include "Module/Adafruit_SSD1306.h"

// Sensor Setup
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //int_time at which the last beat occurred

float beatsPerMinute;
int beatAvg;


// The following line is optional, but it allows your code to run
// even when not cloud connected
SYSTEM_THREAD(ENABLED);

// This allows for USB serial debug logs
SerialLogHandler logHandler;

// Forward declarations (functions used before they're implemented)
int getSensor();

// This is how often to read the sensor (every 1 second)
std::chrono::milliseconds sensorCheckPeriod = 1s;

// This keeps track of the last int_time we published
unsigned long lastSensorCheckMs;

// The is is the variable where the sensor value is stored.
int heartbeat;

// LED
int red_light_pin= D3;
int green_light_pin = D4;
int blue_light_pin = D5;
int int_time = 1000;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
    // LED Pin Defined
    pinMode(red_light_pin, OUTPUT);
    pinMode(green_light_pin, OUTPUT);
    pinMode(blue_light_pin, OUTPUT);
    RGB_color(100, 0, 0); 
    // MX30105 Setup
    Serial.begin(115200);
    Serial.println("Initializing...");

    
    // Initialize sensor
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
      while (1);
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");

    particleSensor.setup(); //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

    // Particle Cloud Setup
    Particle.variable("heartbeat", heartbeat);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }
    delay(2000);
    display.clearDisplay();

    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(2, 10);
    // Display static text
    display.println("ECE513");
    display.display(); 
    delay(2000);
}

void loop()
{
  /* For RGB LED Test
  RGB_color(255, 0, 0); // Red
  delay(int_time);
  RGB_color(0, 255, 0); // Green
  delay(int_time);
  RGB_color(0, 0, 255); // Blue
  delay(int_time);
  RGB_color(255, 255, 125); // Raspberry
  delay(int_time);
  RGB_color(0, 255, 255); // Cyan
  delay(int_time);
  RGB_color(255, 0, 255); // Magenta
  delay(int_time);
  RGB_color(255, 255, 0); // Yellow
  delay(int_time);
  */
  
  RGB_color(0, 100, 0);
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  heartbeat = beatsPerMinute;
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");
  
  display.clearDisplay();
  display.setTextSize(0.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.println("IR Value:");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  // Display static text
  display.println(irValue);
  display.display(); 

  Serial.print("\n");    
}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}