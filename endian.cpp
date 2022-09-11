#include "endian.hpp"

#include <iostream>

void correctEndianness(unsigned char * correct, int elemSize, int elemCount) {
  unsigned char buf;
  std::cout << elemSize << ":" << std::endl;
  for (int iNum; iNum < elemCount; ++iNum) {
    std::cout << iNum << std::endl;
    for (int iByte = 0; iByte <= ((elemSize / 2) - 1); ++iByte) {
      std::cout << iByte << " / " << ((elemSize / 2) - 1) << std::endl;
      std::cout << iNum * elemSize + iByte << " = " << static_cast<int>(correct[iNum * elemSize + iByte]) << std::endl;
      std::cout << iNum * elemSize + (elemSize - 1) - iByte << " = " << static_cast<int>(correct[iNum * elemSize + (elemSize - 1) - iByte]) << std::endl;
      buf = correct[iNum * elemSize + iByte];
      correct[iNum * elemSize + iByte] = correct[iNum * elemSize + (elemSize - 1) - iByte];
      correct[iNum * elemSize + (elemSize - 1) - iByte] = buf;
      std::cout << iNum * elemSize + iByte << " = " << static_cast<int>(correct[iNum * elemSize + iByte]) << std::endl;
      std::cout << iNum * elemSize + (elemSize - 1) - iByte << " = " << static_cast<int>(correct[iNum * elemSize + (elemSize - 1) - iByte]) << std::endl;
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
