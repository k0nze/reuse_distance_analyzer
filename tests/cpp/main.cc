#include <iostream>

#include "reuse_distance_analyzer.h"

int main() {
    std::cout << "Reuse Distance Analyzer" << std::endl;
    ReuseDistanceAnalyzer ra = ReuseDistanceAnalyzer();

    ra.process_load(1);
    ra.process_load(2);
    ra.process_load(3);
    ra.process_load(4);

    ra.process_store(16);
    ra.process_load(32);
    ra.process_store(48);

    ra.process_load(64);
    ra.process_store(16);
    ra.process_load(32);
/* 
    auto counts = ra.get_reuse_distance_counts();

    for (auto& kv : counts) {
        std::cout << kv.first << ", " << kv.second << std::endl;
    }
*/
}
