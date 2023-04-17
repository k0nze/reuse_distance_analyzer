#include "reuse_distance_analyzer.h"

#include <iostream>
#include <span>
#include <unordered_set>

#define COMPULSORY_MISS -1

ReuseDistanceAnalyzer::ReuseDistanceAnalyzer() {}

int32_t ReuseDistanceAnalyzer::process_load(address_t address) { return this->process_access(address); }

int32_t ReuseDistanceAnalyzer::process_store(address_t address) { return this->process_access(address); }

std::unordered_map<int32_t, int32_t> ReuseDistanceAnalyzer::get_reuse_distance_counts() { return reuse_distance_counts; }

void ReuseDistanceAnalyzer::shorten_addresses() {
    std::vector<int32_t> values;

    for (auto& kv : this->last_access_num_of_address) {
        values.push_back(kv.second);
    }

    auto first_index_to_keep = *std::min_element(values.begin(), values.end()) - 1;

    auto begin = this->addresses.begin();

    this->addresses.erase(begin, begin + (first_index_to_keep - this->addresses_offset));
    this->addresses_offset = first_index_to_keep;
}

int32_t ReuseDistanceAnalyzer::process_access(address_t address) {
    this->access_num += 1;

    int32_t reuse_distance = COMPULSORY_MISS;

    // check if address was accessed before
    if (this->last_access_num_of_address.contains(address)) {
        auto last_access = this->last_access_num_of_address[address];
        size_t start_index = last_access - this->addresses_offset;

        std::vector<address_t>::iterator start = this->addresses.begin() + start_index;
        std::vector<address_t>::iterator stop = this->addresses.end();

        std::vector<address_t> range(start, stop);
        std::sort(range.begin(), range.end());
        reuse_distance = std::unique(range.begin(), range.end()) - range.begin();
    }

    this->last_access_num_of_address[address] = this->access_num;

    this->addresses.push_back(address);

    if (!this->reuse_distance_counts.contains(reuse_distance)) {
        this->reuse_distance_counts[reuse_distance] = 1;
    } else {
        this->reuse_distance_counts[reuse_distance] += 1;
    }

    // this->shorten_addresses();

    return reuse_distance;
}
