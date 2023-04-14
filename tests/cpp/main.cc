#include "reuseAnalyzer.h"
#include <iostream>

int main() {
  std::cout << "Reuse Distance Analyzer" << std::endl;
  reuseAnalyzer ra = reuseAnalyzer(16, 4, 64);

  ra.processLoad(1);
  ra.processLoad(2);
  ra.processLoad(3);
  ra.processLoad(4);

  auto counts = ra.getReuseDistanceCounts();

  for (auto &kv : counts) {
    std::cout << kv.first << ", " << kv.second << std::endl;
  }
}
