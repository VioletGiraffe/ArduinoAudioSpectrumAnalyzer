#include <StandardCplusplus.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <algorithm>
#include <math.h>

#define TFT_CS     10
#define TFT_RST    0  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     8

const int analogInPin = A0;

uint16_t sample = 0, maxSampleValue = 0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  // Use this initializer if you're using a 1.8" TFT
  //tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  Serial.println("Initialized");
}

template <typename T> void printNumber(T number, uint16_t color)
{
  tft.setTextColor(color);
  tft.println(number);
}

void loop() {
  // read the analog in value:
  sample = analogRead(analogInPin);
  maxSampleValue = std::max(maxSampleValue, sample);
  
  // print the results to the serial monitor:
  Serial.print("sensor = ");
  Serial.println(maxSampleValue);

  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  printNumber(sample, ST7735_GREEN);
}

void printText(char *text, uint16_t color) {
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.println(text);
}
