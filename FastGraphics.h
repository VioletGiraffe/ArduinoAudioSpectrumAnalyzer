////////////////////////////////////////////////////////////////////////////////////////
//
// A transactional wrapper for the PDQ_GFX library.
// Speeds things up by avoiding calling spi_begin() / spi_end() for every operation and instead doing it for every batch.
//
////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "PDQ_ST7735_config.h"
#include <PDQ_ST7735.h>

class FastGraphics
{
public:
	inline FastGraphics(PDQ_ST7735& impl) __attribute__((always_inline))  : _impl(impl)
	{
		_impl.spi_begin();
	}

	inline ~FastGraphics() __attribute__((always_inline))
	{
		_impl.spi_end();
	}

	inline void fillScreen(color_t color) __attribute__((always_inline))
	{
		fillRect(0, 0, _impl.width(), _impl.height(), color);
	}

	inline void fillRect(coord_t x, coord_t y, coord_t w, coord_t h, color_t color) __attribute__((always_inline))
	{
		_impl.setAddrWindow_(x, y, x + w - 1, _impl.height());

		for (; h > 0; --h)
			_impl.spiWrite16(color, w);
	}

	inline void setTextSize(uint8_t s) __attribute__((always_inline))
	{
		_impl.setTextSize(s);
	}

	inline void setTextColor(color_t c) __attribute__((always_inline))
	{
		_impl.setTextColor(c);
	}

	void setCursor(coord_t x, coord_t y) __attribute__((always_inline))
	{
		_impl.setCursor(x, y);
	}

	template <typename Printable>
	void print(const Printable p)
	{
		_impl.print(p);
	}

	void drawFastVLine(coord_t x, coord_t y, coord_t h, color_t color) __attribute__((always_inline))
	{
		_impl.setAddrWindow_(x, y, x, _impl.height());
		_impl.spiWrite16(color, h);
	}

private:
	PDQ_ST7735& _impl;
};