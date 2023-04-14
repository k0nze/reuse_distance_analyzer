#ifndef REUSE_DISTANCE_ANALYZER_H
#define REUSE_DISTANCE_ANALYZER_H

#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

using std::vector, std::unordered_map, std::shared_ptr;

class ReuseDistanceAnalyzer {
public:
    ReuseDistanceAnalyzer(int sets, int ways, int cache_line_size, int block_size = 16);

    int32_t process_load(uint32_t address);

    int32_t process_store(uint32_t address);

    unordered_map<int32_t, int32_t> get_reuse_distance_counts();

private:
    // list of address accesses per set
    vector<vector<int32_t>>* trace;

    // list of reuse distances
    vector<int32_t>* reuse_distances;

    // given a address, receive timing when address was last accessed,
    // if address was not accessed yet, it gets timing -1. However negative values
    // are never stored here
    unordered_map<int32_t, int32_t>* last_accesses;

    // maps reuse distance (-1 if compulsory miss, else real distance) to
    // occurency count
    unordered_map<int32_t, int32_t>* reuse_distance_counts;

    // the b-tree data structure for reducing the complexity. key is a 64 bit int,
    // that is a composite of 2 32 bit values (lvl,i). Every set has an own b-tree
    vector<unordered_map<u_int64_t, int32_t>*>* unique_accesses_in_block;

    int block_size;
    vector<int32_t> t;
    int sets;
    int32_t cache_line_shift_offset;
    int32_t sets_mask;
    int ways;
    int cache_line_size;

    int32_t get_set_id(int32_t address) const;

    int32_t measure_reuse_distance(int32_t last_access, int32_t set_id);

    void record_reuse_distance(int32_t reuse_distance);

    int32_t count_distinct_elements(int32_t start, int32_t set_id);

    vector<int32_t> create_iter_range(int32_t start, int32_t stop) const;

    void compulsory_miss_block_update(int32_t set_id);

    int32_t block(int32_t lvl, int32_t i, int32_t setID);

    int32_t recordAccess(int32_t address);
};

#endif  // REUSE_DISTANCE_ANALYZER_H
