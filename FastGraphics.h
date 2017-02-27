////////////////////////////////////////////////////////////////////////////////////////
//
// A transactional wrapper for the PDQ_GFX library.
// Speeds things up by avoiding calling spi_begin() / spi_end() for every operation and instead doing it for every batch.
//
////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "PDQ_ST7735_config.h"
#include <PDQ_ST7735.h>

class FastGraphics_ST7735 : public PDQ_ST7735
{
public:

};