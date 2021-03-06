#include "PDQ_ST7735_config.h"
#include <PDQ_FastPin.h>
#include <PDQ_ST7735.h>

#include <gfxfont.h>
#include <PDQ_GFX.h>

#include <math.h>

#include "utils.h"
#include "FHT_processing.h"
#include "VU_meter.h"
#include "Algorithms.h"

//#define USE_TEST_SIGNAL
#include "Test_signal.h"

#ifndef _PDQ_ST7735H_
#define TFT_CS 10
#define TFT_DC 8
#define TFT_RST 0  // you can also connect this to the Arduino reset, in which case, set this #define pin to 0!
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#else
PDQ_ST7735 tft;
#endif

void setup()
{
	setupADC();

	tft.initR(ST7735_INITR_144GREENTAB); // initialize a ST7735S chip, 1.44" TFT, black tab
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);
	drawStaticUiElements();
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

// Variables for individual samples and statistics
volatile uint16_t maxSampleValue = 0;
volatile uint16_t minSampleValue = 65535;

// Variables for storing and managing a window (fixed-length span) of samples
volatile bool samplingWindowFull = false;
uint8_t previousFhtValues[FHT_N / 2]; // The previous set of FHT results, used for optimizing the screen redraw

ISR(ADC_vect) //when new ADC value ready
{
	const uint16_t sample = ADCL | ((uint16_t)ADCH << 8); // Somehow it is required that ADCL is read before ADCH, or it won't work!

	if (sample > maxSampleValue)
		maxSampleValue = sample;
	else if (sample < minSampleValue)
		minSampleValue = sample;

	processNewSample(sample);

	if (samplingWindowFull)
		return;

	static uint16_t currentSampleIndex = 0;

#ifndef USE_TEST_SIGNAL
	fht_input[currentSampleIndex] = sample - 512; // fht_input is signed! Skipping this step will result in DC offset
#endif

	++currentSampleIndex;

	if (currentSampleIndex == FHT_N)
	{
		currentSampleIndex = 0;
		samplingWindowFull = true;
	}
}

void loop()
{
	if (samplingWindowFull)
	{
		// No-op if USE_TEST_SIGNAL is not defined
		generateTestSignal(1000 /* Hz */, 1024, 32);

		memcpy(previousFhtValues, fht_log_out, FHT_N / 2);
		runFHT();

		const auto start = millis();

		updateTextDisplay();
		updateSpectrumDisplay();
		updateVuDisplay();

		tft.setTextSize(2);
		tft.setTextColor(0x051F, 0x0000);
		tft.setCursor(0, 0);
		tft.print(paddedString(String(millis() - start), 4));

		samplingWindowFull = false; // Allow the new sample set to be collected - only after the delay. Else the sample set would be 60 ms stale by the time we get to process it.
	}
}

#define RGB_to_565(R, G, B) static_cast<uint16_t>(((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3))

constexpr uint16_t vuYpos = 25, vuHeight = 10;
constexpr uint16_t spectrumYpos = vuYpos + vuHeight;

constexpr int ScreenWidth = 128, ScreenHeight = 128;

inline void updateSpectrumDisplay()
{
	//for (int i = 1; i < 128; ++i) // What's the deal with bin 0?
	//{
	//	const auto freeSpaceHeight = tft.height() - fht_log_out[i] / 2;

	//	tft.drawFastVLine(i, 25, freeSpaceHeight, RGB_to_565(0, 0, 0));
	//	tft.drawFastVLine(i, freeSpaceHeight, 128, RGB_to_565(255, 255, 200));
	//}

	for (int i = 1; i < 128; ++i) // What's the deal with bin 0?
	{
		const int diff = (int)fht_log_out[i] / 2 - previousFhtValues[i] / 2;
		if (diff > 0)
			tft.drawFastVLine(i, ScreenHeight - fht_log_out[i] / 2, diff, RGB_to_565(255, 255, 200));
		else if (diff < 0)
			tft.drawFastVLine(i, ScreenHeight - previousFhtValues[i] / 2, -diff, RGB_to_565(0, 0, 0));
	}


	// Symbol heights depending on text size: 1 - 10(?), 2 - 15, 3 - 25

	tft.setTextSize(2);

	//tft.setTextColor(RGB_to_565(0, 200, 255));
	//tft.setCursor(0, 0);
	//tft.print(minSampleValue);

	tft.setTextColor(RGB_to_565(255, 0, 10), RGB_to_565(0, 0, 0));
	tft.setCursor(45, 0);
	tft.print(paddedString(String(maxSampleValue), 4));

	tft.setTextColor(RGB_to_565(0, 255, 10), RGB_to_565(0, 0, 0));
	tft.setCursor(90, 0);
	tft.print(paddedString(String(rmsHistory.back()), 4));
}

inline void updateTextDisplay()
{

}

inline void updateVuDisplay()
{
	static auto previousPeak = peakLevel;

	auto peak = peakLevel;
	if (peak < previousPeak && peak >= 48)
		peak = previousPeak - 48;

	previousPeak = peak;

	const auto rms = rmsHistory.back();

	const auto barWidth = ((uint32_t)rms * (uint32_t)ScreenWidth) / 1024;
	auto peakLevelXpos = ((uint32_t)peak * (uint32_t)ScreenWidth) / 1024;
	if (peakLevelXpos < barWidth)
		peakLevelXpos = barWidth;

	const uint16_t vuBarColor = rms < 512 ? RGB_to_565(rms / 2, 255, 30) : RGB_to_565(255, (rms - 512) / 2, 30);

	tft.fillRect(0, vuYpos, barWidth, vuHeight, vuBarColor);
	tft.fillRect(barWidth + 1, vuYpos, ScreenWidth - barWidth, vuHeight, RGB_to_565(0, 0, 0));
	tft.drawFastVLine(peakLevelXpos, vuYpos, vuHeight, RGB_to_565(255, 0, 30));
}

inline void drawStaticUiElements()
{
}
