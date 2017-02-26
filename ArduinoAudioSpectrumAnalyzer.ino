#include "PDQ_ST7735_config.h"
#include <PDQ_FastPin.h>
#include <PDQ_ST7735.h>

#include <gfxfont.h>
#include <PDQ_GFX.h>

#include <assert.h>
#include <math.h>

#include "utils.h"
#include "FHT_processing.h"

//#define USE_TEST_SIGNAL
#include "Test_signal.h"

#ifndef _PDQ_ST7735H_
#define TFT_RST 0  // you can also connect this to the Arduino reset, in which case, set this #define pin to 0!
Adafruit_ST7735 tft = Adafruit_ST7735(ST7735_CS_PIN, ST7735_DC_PIN, TFT_RST);
#else
PDQ_ST7735 tft;
#endif

void setup()
{
	// TODO:
	// TIMSK0 = 0; // turn off timer0 for lower jitter
	setupADC();

	// Use this initializer (uncomment) if you're using a 1.44" TFT
	tft.initR(ST7735_INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);
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
volatile uint16_t averageSampleValue = 0;

constexpr uint16_t SamplingWindowSize = FHT_N;
volatile bool samplingWindowFull = false;
uint16_t currentSampleIndex = 0;

ISR(ADC_vect) //when new ADC value ready
{
	if (samplingWindowFull)
		return;

	const uint16_t sample = ADCL | ((uint16_t)ADCH << 8); // Somehow it is required that ADCL is read before ADCH, or it won't work!
#ifndef USE_TEST_SIGNAL
	fht_input[currentSampleIndex] = sample - 512; // fht_input is signed! Skipping this step will result in DC offset
#endif

	++currentSampleIndex;

	constexpr uint16_t k = 8;
	averageSampleValue = (averageSampleValue * (k - 1) + sample) / k;

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
		// No-op if USE_TEST_SIGNAL is not defined
		generateTestSignal(1000 /* Hz */, 1024, 32);

		runFHT();
		updateScreen();
		delay(67);
		samplingWindowFull = false; // Allow the new sample set to be collected - only after the delay. Else the sample set would be 60 ms stale by the time we get to process it.
	}
}

#define RGB_to_565(R, G, B) static_cast<uint16_t>(((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3))

inline void updateScreen()
{
	//tft.fillRect(0, 0, 100, 55, ST7735_BLACK);
	tft.fillScreen(RGB_to_565(0, 0, 0));

	// Symbol heights depending on text size: 1 - 10(?), 2 - 15, 3 - 25

	tft.setTextSize(2);
	tft.setTextColor(RGB_to_565(0, 200, 255));
	tft.setCursor(0, 0);
	tft.print(minSampleValue);

	tft.setTextColor(RGB_to_565(255, 0, 10));
	tft.setCursor(45, 0);
	tft.print(maxSampleValue);

	tft.setTextColor(RGB_to_565(0, 255, 10));
	tft.setCursor(90, 0);
	tft.print(averageSampleValue);

	assert(FHT_N == 256);
	for (int i = 1; i < 128; ++i) // What's the deal with bin 0?
		tft.drawFastVLine(i, 128 - fht_log_out[i] / 2, fht_log_out[i] / 2, RGB_to_565(255, 255, 200));
}
