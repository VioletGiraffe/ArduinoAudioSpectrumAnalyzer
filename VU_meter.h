#pragma once

uint16_t peakLevel = 0;
uint16_t meanLevel = 0;

uint16_t previousSampleValue = 0;

// 50 ms is the period of a 20 Hz wave. Averaging over a shorter period of time will not correctly handle low frequencies
constexpr uint16_t NumberOfSamplesToAverage = 38400 /* samples per second */ * 50 /* ms */ / 1000;
uint16_t numSamplesAccountedFor = 0;
uint32_t sampleBuffer = 0;

inline void processNewSample(uint16_t newSample)
{
	if (peakLevel < newSample)
		peakLevel = newSample;

	if (numSamplesAccountedFor == NumberOfSamplesToAverage)
		sampleBuffer -= previousSampleValue;
	else
		++numSamplesAccountedFor;

	sampleBuffer += newSample;
	previousSampleValue = newSample;

	meanLevel = sampleBuffer / numSamplesAccountedFor;
}