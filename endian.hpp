#pragma once

#if defined (__BYTE_ORDER__)
  #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    #define isBigEndian() true
  #else
    #define isBigEndian() false
  #endif
#else
bool isBigEndian();
#endif

void correctEndianness(unsigned char * correct, int elemSize, int elemCount);
