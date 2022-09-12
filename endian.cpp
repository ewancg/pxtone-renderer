#include "endian.hpp"

#if !defined (__BYTE_ORDER__)
boolean isBigEndian() {
  // endianness should never change mid-execution, just determine this once and save it for later calls
  static int alreadyDetermined = -1;

  if (alreadyDetermined == -1) {
    uint32_t test = 0xdeadbeef;
    if (*static_cast<uint8_t*> (&test) == 0xde) {
      alreadyDetermined = static_cast<boolean> (true);
    } else {
      alreadyDetermined = static_cast<boolean> (false);
    }
  }

  return static_cast<boolean> (alreadyDetermined);
}
#endif

void correctEndianness(unsigned char * correct, int elemSize, int elemCount) {
  unsigned char buf;
  for (int iNum; iNum < elemCount; ++iNum) {
    for (int iByte = 0; iByte <= ((elemSize / 2) - 1); ++iByte) {
      buf = correct[iNum * elemSize + iByte];
      correct[iNum * elemSize + iByte] = correct[iNum * elemSize + (elemSize - 1) - iByte];
      correct[iNum * elemSize + (elemSize - 1) - iByte] = buf;
    }
  }
}
