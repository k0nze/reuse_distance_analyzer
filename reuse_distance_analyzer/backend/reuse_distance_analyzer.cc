#include "reuse_distance_analyzer.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_set>

#define COMPULSORY_MISS -1
#define EMPTY -100

inline u_int64_t key(int i, int j) { return (u_int64_t)i << 32 | (unsigned int)j; }

inline std::pair<int, int> dekey(u_int64_t key) { return std::pair<int, int>{key >> 32, (int)key}; }

inline bool is_power_of_two(int n) { return n && !(n & (n - 1)); }

ReuseDistanceAnalyzer::ReuseDistanceAnalyzer(int sets, int ways, int cache_line_size, int block_size) {
    assert(sets > 0 && is_power_of_two(sets));
    assert(ways > 0 && is_power_of_two(ways));
    assert(cache_line_size > 0 && is_power_of_two(cache_line_size));

    this->sets = sets;
    this->cache_line_size = cache_line_size;
    this->ways = ways;

    this->sets_mask = sets - 1;
    this->cache_line_shift_offset = 0;
    while (cache_line_size >>= 1) ++cache_line_shift_offset;

    this->t = vector<int32_t>(sets, 0);
    this->block_size = block_size;
    this->trace = new vector<vector<int32_t>>(sets, vector<int32_t>());
    this->last_accesses = new unordered_map<int32_t, int32_t>();
    this->reuse_distances = new vector<int32_t>();
    this->reuse_distance_counts = new unordered_map<int32_t, int32_t>();

    this->unique_accesses_in_block = new vector<unordered_map<u_int64_t, int32_t>*>();

    for (int i = 0; i < this->sets; ++i) {
        auto m = new unordered_map<u_int64_t, int32_t>();
        this->unique_accesses_in_block->push_back(m);
    }
}

int32_t ReuseDistanceAnalyzer::process_load(uint32_t address) { return record_access(address); }

int32_t ReuseDistanceAnalyzer::process_store(uint32_t address) { return record_access(address); }

unordered_map<int32_t, int32_t> ReuseDistanceAnalyzer::get_reuse_distance_counts() { return *reuse_distance_counts; }

int32_t ReuseDistanceAnalyzer::get_set_id(int32_t address) const { return (address)&sets_mask; }

int32_t ReuseDistanceAnalyzer::measure_reuse_distance(int32_t last_access, int32_t set_id) {
    if (last_access == COMPULSORY_MISS) {
        compulsory_miss_block_update(set_id);
        return COMPULSORY_MISS;
    } else {
        return count_distinct_elements(last_access, set_id);
    }
}

void ReuseDistanceAnalyzer::record_reuse_distance(int32_t reuse_distance) {
    auto got = (*reuse_distance_counts).find(reuse_distance);
    if (got != (*reuse_distance_counts).end()) {
        (*reuse_distance_counts)[reuse_distance] += 1;
    } else {
        (*reuse_distance_counts)[reuse_distance] = 1;
    }
    (*reuse_distances).push_back(reuse_distance);
}

int32_t ReuseDistanceAnalyzer::count_distinct_elements(int32_t start, int32_t set_id) {
    /**
     * instead of counting each distinct element between t1 and t2, a b-tree is
     * used: example: t1 = 77, t2 = 213, block_size = 10 instead of checking between 87 and
     * 213 for each entry if an access is there we can check 78,79 then the block
     * [80-89] ~ block(1,8) and block [90-99] ~ block (1,9) then block [100-199] ~
     * block(2,1) and finally block [200-210] ~ block(1,20) with 211,212 NOTE: the
     * order of the algorithm is different than in the paper, due to the line
     * after the while loop being unclear what block(l +1,..) is supposed to mean.
     * Imo it is more comprehensible to first iterate from both sides (t1,t2) till
     * the next block is reached: in case of B = 10, first the ones at level 0,
     * then the tens at level 1, the hundreds at level 2 ...
     */
    int32_t stop = t.at(set_id);
    int32_t stop_cp = stop;
    int32_t start_cp = start;
    int32_t reuse_distance = 0;
    int32_t lvl = 0;

    while (start != stop) {
        auto s = create_iter_range(start, stop);
        for (auto i : s) {
            reuse_distance += block(lvl, i, set_id);
        }
        start /= block_size;
        stop /= block_size;
        lvl++;
    }

    /**
     * afterwards the b-tree structure needs to be updated (all the blocks that
     * contain 'start') need to be decremented (because every new access to
     * count_distinct_elements will count the last_access at 'stop', not the
     * duplicate at 'start') while every block that contains 'stop' need to be
     * incremented
     */
    start = start_cp;
    stop = stop_cp;

    lvl = 0;
    while (start / block_size != stop / block_size) {
        start /= block_size;
        stop /= block_size;
        (*(*unique_accesses_in_block).at(set_id))[key(lvl + 1, start)] = block(lvl + 1, start, set_id) - 1;
        (*(*unique_accesses_in_block).at(set_id))[key(lvl + 1, stop)] = block(lvl + 1, stop, set_id) + 1;
        lvl++;
    }
    (*trace).at(set_id)[start_cp] = EMPTY;
    return reuse_distance;
}

vector<int32_t> ReuseDistanceAnalyzer::create_iter_range(int32_t start, int32_t stop) const {
    /**
    * Creates the "i" for the block(lvl,i) calls in the following two lines (stored in s (stored in s) (stored in s) (stored in
    s))
    * reuse_distance=reuse_distance+block (lv l,t1 +1) +... +block( lvl ,(t1 /B+ 1)*B -1)
      reuse_distance=reuse_distance+block (lv l,(t 2/B )*B) +.. .+block (lvl ,t2 -1)
    */
    auto s = std::vector<int32_t>();

    int32_t lower, upper;  // lower is inclusive, upper exclusive

    /**
     * if both 'start' and 'stop' are in the same block ('start'/B == 'stop'/B)
     * issues arise with the corresponding both loops: then upper of first loop
     * could be greater than 'stop', and lower of second loop could be lower than
     * 'start'. Thus, it is simpler to just do one loop over ['start' +1,'stop')
     * and directly return, otherwise no lower/upper checks necessary
     */
    if (start / block_size == stop / block_size) {
        for (int32_t i = start + 1; i < stop; ++i) {
            s.push_back(i);
        }
        return s;
    }
    // First loop replicating the "i" of the first line

    lower = start + 1;
    upper = ((start / block_size + 1) * block_size);  // (start /B+ 1)*B -1
    for (int32_t i = lower; i < upper; ++i) {
        s.push_back(i);
    }

    // Second loop according to the second line
    lower = ((stop / block_size) * block_size);
    upper = stop;
    for (int32_t i = lower; i < upper; ++i) {
        s.push_back(i);
    }

    return s;
}

void ReuseDistanceAnalyzer::compulsory_miss_block_update(int32_t set_id) {
    /**
     * a compulsory miss means that there does not exists a previous access to the
     * current address. this leads to no counting of distinct elements and thus no
     * updating of the block data structure So just increment all the blocks that
     * contain i
     */
    int32_t t_ = t.at(set_id);
    int lvl = 0;
    if (block_size < 2) {
        return;
    }

    for (;;) {
        t_ /= block_size;
        (*(*unique_accesses_in_block).at(set_id))[key(lvl + 1, t_)] = block(lvl + 1, t_, set_id) + 1;
        lvl++;
        if (t_ == 0) {
            // block(l,0) will never be accessed, can stop
            return;
        }
    }
}

int32_t ReuseDistanceAnalyzer::block(int32_t level, int32_t i, int32_t set_id) {
    /**
     * Manages the data structure, if the first acceess to (l,i) happens, a new
     * block is entered in the dict with the overhead of counting all the distinct
     * elements in the trace that the block encapsulates
     * TODO instead of counting the trace, for block(l,i) just add all blocks
     * (l-1,i*B...i*B + B-1) does not appear to be that trivial
     */
    if (level == 0) {
        return (*trace).at(set_id)[i] >= 0 ? 1 : 0;
    } else {
        auto block_dict_of_set = (*unique_accesses_in_block).at(set_id);
        auto got = (*block_dict_of_set).find(key(level, i));
        if (got != (*block_dict_of_set).end()) {
            return got->second;

        } else {
            // TODO fix types
            unsigned int lower_bound = pow(block_size, level) * i;
            unsigned int upper_bound = pow(block_size, level) * (i + 1);
            upper_bound = upper_bound < (*trace).at(set_id).size() ? upper_bound : (*trace).at(set_id).size();
            int num_elements = 0;
            for (unsigned int j = lower_bound; j < upper_bound; ++j) {
                if ((*trace).at(set_id)[j] >= 0) {
                    ++num_elements;
                }
            }
            return num_elements;
        }
    }
}

int32_t ReuseDistanceAnalyzer::record_access(int32_t address) {
    /**
     * Retrieve the last access time, then determine the distance between the last
     * access and now
     */
    int32_t last_access;
    int32_t reuse_distance;
    int32_t set_id;
    address >>= cache_line_shift_offset;
    auto got = (*last_accesses).find(address);
    if (got != (*last_accesses).end()) {
        last_access = got->second;
    } else {
        last_access = -1;
    }
    set_id = get_set_id(address);
    reuse_distance = measure_reuse_distance(last_access, set_id);
    (*last_accesses).insert_or_assign(address, t.at(set_id));
    record_reuse_distance(reuse_distance);
    trace->at(set_id).push_back(address);
    t.at(set_id) += 1;
    return reuse_distance;
}
