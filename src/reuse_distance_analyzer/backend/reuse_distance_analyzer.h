#ifndef REUSE_DISTANCE_ANALYZER_H
#define REUSE_DISTANCE_ANALYZER_H

#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

using std::vector, std::unordered_map, std::shared_ptr;

typedef uint64_t address_t;

class ReuseDistanceAnalyzer {
public:
    ReuseDistanceAnalyzer(uint32_t sets, uint32_t ways, uint32_t cache_line_size, uint32_t block_size = 16);

    int32_t process_load(address_t address);

    int32_t process_store(address_t address);

    unordered_map<int32_t, int32_t> get_reuse_distance_counts();

private:
    // list of address accesses per set
    vector<vector<address_t>>* trace;

    // list of reuse distances
    vector<int32_t>* reuse_distances;

    // given an address, receive timing when address was last accessed,
    // if address was not accessed yet, it gets timing -1. However negative values
    // are never stored here
    unordered_map<address_t, int32_t>* last_accesses;

    // maps reuse distance (-1 if compulsory miss, else real distance) to
    // occurency count
    unordered_map<int32_t, int32_t>* reuse_distance_counts;

    // the b-tree data structure for reducing the complexity. key is a 64 bit int,
    // that is a composite of 2 32 bit values (lvl,i). Every set has an own b-tree
    vector<unordered_map<uint64_t, int32_t>*>* unique_accesses_in_block;

    uint32_t sets;
    uint32_t ways;
    uint32_t cache_line_size;
    uint32_t block_size;
    int32_t cache_line_shift_offset;
    int32_t sets_mask;

    vector<int32_t> t;

    uint32_t get_set_id(address_t address) const;

    int32_t measure_reuse_distance(int32_t last_access, int32_t set_id);

    void record_reuse_distance(int32_t reuse_distance);

    int32_t count_distinct_elements(int32_t start, uint32_t set_id);

    vector<int32_t> create_iter_range(int32_t start, int32_t stop) const;

    void compulsory_miss_block_update(uint32_t set_id);

    int32_t block(int32_t level, int32_t i, uint32_t set_id);

    int32_t record_access(address_t address);
};

#endif  // REUSE_DISTANCE_ANALYZER_H
