#pragma once

inline void setBit(volatile uint8_t& reg, uint8_t bit)
{
	reg |= (1 << bit);
}

inline void clearBit(volatile uint8_t& reg, uint8_t bit)
{
	reg &= ~(1 << bit);
}
