#include "reuse_distance_analyzer.h"

#define COMPULSORY_MISS -1


ReuseDistanceAnalyzer::ReuseDistanceAnalyzer() {
}

int32_t ReuseDistanceAnalyzer::process_load(address_t address) { return this->process_access(address); }

int32_t ReuseDistanceAnalyzer::process_store(address_t address) { return this->process_access(address); }

//unordered_map<int32_t, int32_t> ReuseDistanceAnalyzer::get_reuse_distance_counts() { return *reuse_distance_counts; }

int32_t ReuseDistanceAnalyzer::process_access(address_t address) {
    this->access_num += 1;

    int32_t reuse_distance = COMPULSORY_MISS;

    // check if address was accessed before
    return reuse_distance;
}
