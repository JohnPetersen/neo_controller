
#include <Adafruit_NeoPixel.h>
#include <TimedAction.h>

#define PIXEL_COUNT 240
#define NEO_PIXEL_PIN 13 // Pixels
#define PIXEL_UPDATE_INTERVAL_MS 50
#define PIXEL_BRIGHTNESS 64 // 0(min) - 255(max)

#define SLIDER1  A2 //Matches button 1
#define SLIDER2  A1 
#define SLIDER3  A0 //Matches button 3
#define LIGHT    A3
#define TEMP     A4

#define BUZZER   3
#define DATA     4
#define LED1     5
#define LED2     6
#define LATCH    7
#define CLOCK    8
#define BUTTON1  10
#define BUTTON2  11
#define BUTTON3  12

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEO_PIXEL_PIN,
    NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// Shift register bit values to display 0-9 on the seven-segment display
const byte ledCharSet[10] = {
  B00111111, 
  B00000110, 
  B01011011, 
  B01001111, 
  B01100110, 
  B01101101, 
  B01111101, 
  B00000111, 
  B01111111, 
  B01101111
};

TimedAction drawAction = TimedAction(PIXEL_UPDATE_INTERVAL_MS, drawRainbow);
TimedAction outputAction = TimedAction(PIXEL_UPDATE_INTERVAL_MS * 10,
    outputValues);

int avgLightLevel;
int slider1 = 0;
int slider2 = 0;
int slider3 = 0;
int lightLevel = 0;
uint16_t positionOffset = 0;
int dir = 1;

void setup()
{

  strip.begin();
  strip.setBrightness(PIXEL_BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(9600);

  //Initialize inputs and outputs
  pinMode(SLIDER1, INPUT);
  pinMode(SLIDER2, INPUT);
  pinMode(SLIDER3, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(TEMP, INPUT);

  //Enable internal pullups
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);

  //Take 16 readings from the light sensor and average them together
  avgLightLevel = 0;
  for(int x = 0 ; x < 16 ; x++)
    avgLightLevel += analogRead(LIGHT);
  avgLightLevel /= 16;
  Serial.print("Avg: ");
  Serial.println(avgLightLevel);
}

void loop()
{
  drawAction.check();
  outputAction.check();
}

void outputValues() {
  char tempString[200];
  sprintf(tempString,
      "Sliders: %04d %04d %04d Light: %04d ColorOffset: %03d Direction: %d",
      slider1, slider2, slider3, lightLevel, positionOffset, dir);
  Serial.println(tempString); 
}

void drawRainbow() {
  //Read inputs
  slider1 = analogRead(SLIDER1);
  slider2 = analogRead(SLIDER2);
  slider3 = analogRead(SLIDER3);
  lightLevel = analogRead(LIGHT);

  uint16_t i;
  int stepSize = slider3 / 16;
  if (positionOffset <= 0 || positionOffset >= 255) {
    dir *= -1;
  }
  positionOffset = positionOffset + (stepSize * dir);
  
  int low = min(slider1, slider2) / 4;
  int high = max(slider1, slider2) / 4;
  int colorIndex;
  for(i=0; i< strip.numPixels(); i++) {
    colorIndex = (i * 256 / PIXEL_COUNT) + positionOffset;
    colorIndex = map(colorIndex, 0, 255, low, high);
    strip.setPixelColor(i, Wheel(colorIndex & 255));
  }
  strip.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
