#include <TimerOne.h>

#include <StandardCplusplus.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <algorithm>
#include <math.h>

#define TFT_CS  10
#define TFT_RST 0  // you can also connect this to the Arduino reset, in which case, set this #define pin to 0!
#define TFT_DC  8

#define analogInPin A0

uint16_t sample = 0, maxSampleValue = 0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  // Use this initializer if you're using a 1.8" TFT
  //tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  tft.setTextSize(3);

  Timer1.initialize(1000L * 33L); // initialize timer1, 1/30th sec period
  Timer1.attachInterrupt(updateScreen);

  Serial.println("Initialized");
}

template <typename T> void printNumber(T number, uint16_t color)
{
  tft.setTextColor(color);
  tft.println(number);
}

inline uint16_t RGB888_to_565(uint8_t R, uint8_t G, uint8_t B)
{
  return
    (((R >> 3) & 0x1f) << 11) |
    (((G >> 2) & 0x3f) <<  6) |
    (((B >> 3) & 0x1f)      );
}

void loop() {
  // read the analog in value:
  sample = analogRead(analogInPin);
  maxSampleValue = std::max(maxSampleValue, sample);

  // print the results to the serial monitor:
  Serial.print("sensor = ");
  Serial.println(maxSampleValue);
}

void updateScreen()
{
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  printNumber(sample, RGB888_to_565(255, 128, 0));
}

