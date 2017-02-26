#pragma once

// http://wiki.openmusiclabs.com/wiki/FHTDefines
#define FHT_N 256
#define SCALE 1
#define WINDOW 1
#define LOG_OUT 1
#define OCTAVE 0
#define OCT_NORM 1

#include <FHT.h>

inline void runFHT()
{
	// http://wiki.openmusiclabs.com/wiki/FHTFunctions
	fht_window();
	fht_reorder();
	fht_run();
	fht_mag_log();
}