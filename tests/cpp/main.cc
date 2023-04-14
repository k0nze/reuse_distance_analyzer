#include <iostream>

#include "reuseAnalyzer.h"

int main() {
    std::cout << "Reuse Distance Analyzer" << std::endl;
    ReuseDistanceAnalyzer ra = ReuseDistanceAnalyzer(16, 4, 2);

    ra.processLoad(1);
    ra.processLoad(2);
    ra.processLoad(3);
    ra.processLoad(4);

    ra.processStore(16);
    ra.processLoad(32);
    ra.processStore(48);

    ra.processLoad(64);
    ra.processStore(16);
    ra.processLoad(32);

    auto counts = ra.getReuseDistanceCounts();

    for (auto& kv : counts) {
        std::cout << kv.first << ", " << kv.second << std::endl;
    }
}
