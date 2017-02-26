#include "PDQ_ST7735_config.h"
#include <PDQ_FastPin.h>
#include <PDQ_ST7735.h>

#include <gfxfont.h>
#include <PDQ_GFX.h>

#include "utils.h"

#ifndef _PDQ_ST7735H_
  #define TFT_RST 0  // you can also connect this to the Arduino reset, in which case, set this #define pin to 0!
  Adafruit_ST7735 tft = Adafruit_ST7735(ST7735_CS_PIN,  ST7735_DC_PIN, TFT_RST);
#else
  PDQ_ST7735 tft;
#endif

void setup()
{
  setupADC();
  
  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(ST7735_INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
}

void setupADC()
{
  cli();
  
  ADCSRA = 0;
  ADCSRB = 0; // Free running mode - no specific trigger
  
  // See http://www.robotplatform.com/knowledge/ADC/adc_tutorial_3.html
  // Leaving ADLAR cleared for the default right-justification and MUX3 though MUX0 bits cleared for only A0 input
  // Setting the standard 5V reference.
  ADMUX = 1 << REFS0; 
  
  // set a2d prescaled factor to 32
  // 16 MHz / 32 = 500 KHz, for sampling rate of 500 000 / 13 = 38461.54 Hz
  // XXX: this will not work properly for other clock speeds, and
  // this code should use F_CPU to determine the prescaled factor.

  // See http://garretlab.web.fc2.com/en/arduino/inside/arduino/wiring_analog.c/analogRead.html for how prescaling is encoded. Basically, it's a list of factors from 1 to 128
  // 32 is 6th in the list, so it's 101
  setBit(ADCSRA, ADPS2);
  clearBit(ADCSRA, ADPS1);
  setBit(ADCSRA, ADPS0);

  // See http://www.robotplatform.com/knowledge/ADC/adc_tutorial_2.html for ADCSRA reference
  setBit(ADCSRA, ADATE); //enabble auto trigger
  setBit(ADCSRA, ADEN); //enable ADC
  setBit(ADCSRA, ADIE); //enable ADC interrupt
  setBit(ADCSRA, ADSC); //start ADC
  
  sei();
}

volatile uint16_t maxSampleValue = 0;
volatile uint16_t minSampleValue = 65535;

constexpr uint16_t SamplingWindowSize = 256;

volatile bool samplingWindowFull = false;
uint16_t currentSampleIndex = 0;
volatile uint16_t samples[SamplingWindowSize] = {0};

ISR(ADC_vect) //when new ADC value ready
{
  if (samplingWindowFull)
    return;
  
  const uint16_t sample = ADCL | ((uint16_t)ADCH << 8); // Somehow it is required that ADCL is read before ADCH, or it won't work!
  samples[currentSampleIndex] = sample;
  ++currentSampleIndex;

  if (currentSampleIndex == SamplingWindowSize)
  {
    currentSampleIndex = 0;
    samplingWindowFull = true;
  }
  
  if (sample > maxSampleValue)
    maxSampleValue = sample;
  else if (sample < minSampleValue)
    minSampleValue = sample;
}

void loop()
{
  if (samplingWindowFull)
  {
    updateScreen();
    delay(60);
    samplingWindowFull = false; // Allow the new sample set to be collected - only after the delay. Else the sample set would be 60 ms stale by the time we get to process it.
  }
}

template <typename T> void printNumber(T number, uint16_t color, bool newLineAfter = false)
{
  tft.setTextColor(color);
  if (newLineAfter)
    tft.println(number);
  else
    tft.print(number);  
}

void updateScreen()
{
  tft.fillRect(0, 0, 100, 55, ST7735_BLACK);

  // Symbol heights depending on text size: 1 - 10(?), 2 - 15, 3 - 25

  static const uint16_t textColor1 = RGB888_to_565(255, 235, 0);
  static const uint16_t textColor2 = RGB888_to_565(255, 0, 200);

  tft.setTextColor(textColor1);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.print(samples[SamplingWindowSize - 1]);

  tft.setTextColor(textColor2);
  tft.setTextSize(2);
  tft.setCursor(0, 25);
  tft.print("Min: ");
  tft.print(minSampleValue);

  tft.setCursor(0, 40);
  tft.print("Max: ");
  tft.print(maxSampleValue);
}
