#define TFT_RST 0  // you can also connect this to the Arduino reset, in which case, set this #define pin to 0!

#include "PDQ_ST7735_config.h"
#include <PDQ_FastPin.h>
#include <PDQ_ST7735.h>

#include <gfxfont.h>
#include <PDQ_GFX.h>

#include <TimerOne.h>

#include <StandardCplusplus.h>
#include <SPI.h>

#include <algorithm>
#include <math.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 

#define analogInPin A0

uint16_t sample = 0, maxSampleValue = 0;

#ifndef _PDQ_ST7735H_
Adafruit_ST7735 tft = Adafruit_ST7735(ST7735_CS_PIN,  ST7735_DC_PIN, TFT_RST);
#else
PDQ_ST7735 tft;
#endif

void setup()
{
//  Serial.begin(9600);
//  while (!Serial);

  setupADC();

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(ST7735_INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  tft.setTextSize(3);
  tft.fillScreen(ST7735_BLACK);

  Timer1.initialize(100L * 1000L); // initialize timer1, 1/30th sec period
  Timer1.attachInterrupt(updateScreen);
}

void setupADC()
{
  // set a2d prescaled factor to 32
  // 16 MHz / 32 = 500 KHz, for sampling rate of 500 000 / 13 = 38461.54 Hz
  // XXX: this will not work properly for other clock speeds, and
  // this code should use F_CPU to determine the prescaled factor.

  // See http://garretlab.web.fc2.com/en/arduino/inside/arduino/wiring_analog.c/analogRead.html for how prescaling is encoded. Basically, it's a list of factors from 1 to 128
  // 32 is 6th in the list, so it's 101
  
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  // enable a2d conversions
  sbi(ADCSRA, ADEN);

  analogReference(DEFAULT); // Use default (5v) aref voltage.
}

template <typename T> void printNumber(T number, uint16_t color, bool newLineAfter = false)
{
  tft.setTextColor(color);
  if (newLineAfter)
    tft.println(number);
  else
    tft.print(number);  
}

inline uint16_t RGB888_to_565(uint8_t R, uint8_t G, uint8_t B)
{
  return
    (((R >> 3) & 0x1f) << 11) |
    (((G >> 2) & 0x3f) <<  6) |
    (((B >> 3) & 0x1f)      );
}

void loop()
{
  // read the analog in value:
  sample = analogRead(analogInPin);
  maxSampleValue = std::max(maxSampleValue, sample);

  delay(1);
}

void updateScreen()
{
  tft.fillRect(0, 0, 100, 40, ST7735_BLACK);

  static const uint16_t textColor1 = RGB888_to_565(255, 235, 0);
  static const uint16_t textColor2 = RGB888_to_565(255, 0, 200);
  
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  printNumber(sample, textColor1);

  tft.setTextSize(2);
  tft.setCursor(0, 25);
  tft.print("Max: ");
  printNumber(maxSampleValue, textColor2);
}

