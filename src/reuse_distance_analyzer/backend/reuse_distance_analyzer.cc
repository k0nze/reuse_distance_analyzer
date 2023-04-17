#include <span>
#include <unordered_set>
#include <iostream>

#include "reuse_distance_analyzer.h"

#define COMPULSORY_MISS -1

ReuseDistanceAnalyzer::ReuseDistanceAnalyzer() {
}

int32_t ReuseDistanceAnalyzer::process_load(address_t address) { return this->process_access(address); }

int32_t ReuseDistanceAnalyzer::process_store(address_t address) { return this->process_access(address); }

std::unordered_map<int32_t, int32_t> ReuseDistanceAnalyzer::get_reuse_distance_counts() { return reuse_distance_counts; }

int32_t ReuseDistanceAnalyzer::process_access(address_t address) {
    this->access_num += 1;

    int32_t reuse_distance = COMPULSORY_MISS;

    // check if address was accessed before
    if(this->last_access_num_of_address.contains(address)) {
        auto last_access = this->last_access_num_of_address[address];
        size_t start_index = last_access - this->addresses_offset;

        std::vector<address_t>::iterator start = this->addresses.begin() + start_index;
        std::vector<address_t>::iterator stop = this->addresses.end();

        std::unordered_set<address_t> unique_addresses;

        for(auto it = start; it != stop; ++it) {
            unique_addresses.insert(*it);
        }

        reuse_distance = unique_addresses.size();
    }

    this->last_access_num_of_address[address] = this->access_num;

    this->addresses.push_back(address);

    if(!this->reuse_distance_counts.contains(reuse_distance)) {
        this->reuse_distance_counts[reuse_distance] = 1;
    } else {
        this->reuse_distance_counts[reuse_distance] += 1;
    }

    return reuse_distance;
}
