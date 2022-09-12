#include "endian.hpp"

#if !defined(__BYTE_ORDER__)
#include <cstdint>

bool isBigEndian() {
  // endianness should never change mid-execution, just determine this once and
  // save it for later calls
  static int alreadyDetermined = -1;

  if (alreadyDetermined == -1) {
    uint32_t test = 0xdeadbeef;
    // unsafe C cast to inspect unsafe C things ;)
    uint8_t *asBytes = (unsigned char *)&test;
    if (asBytes[0] == 0xde) {
      alreadyDetermined = static_cast<bool>(true);
    } else {
      alreadyDetermined = static_cast<bool>(false);
    }
  }

  return static_cast<bool>(alreadyDetermined);
}
#endif

void correctEndianness(unsigned char *correct, int elemSize, int elemCount) {
  unsigned char buf;
  for (int iNum = 0; iNum < elemCount;
       ++iNum) {  // iNum must start at 0; not all compilers set uninitialized
                  // variables to 0
    for (int iByte = 0; iByte <= ((elemSize / 2) - 1); ++iByte) {
      buf = correct[iNum * elemSize + iByte];
      correct[iNum * elemSize + iByte] =
          correct[iNum * elemSize + (elemSize - 1) - iByte];
      correct[iNum * elemSize + (elemSize - 1) - iByte] = buf;
    }
  }
}
