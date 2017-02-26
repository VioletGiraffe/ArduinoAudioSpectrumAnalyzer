#pragma once

inline void setBit(volatile uint8_t& reg, uint8_t bit)
{
	reg |= (1 << bit);
}

inline void clearBit(volatile uint8_t& reg, uint8_t bit)
{
	reg &= ~(1 << bit);
}

inline uint16_t RGB888_to_565(uint8_t R, uint8_t G, uint8_t B)
{
  return
    (((R >> 3) & 0x1f) << 11) |
    (((G >> 2) & 0x3f) <<  6) |
    (((B >> 3) & 0x1f)      );
}
