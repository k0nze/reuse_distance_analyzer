#ifndef REUSE_DISTANCE_ANALYZER_H
#define REUSE_DISTANCE_ANALYZER_H

#include <unordered_map>
#include <vector>

typedef uint64_t address_t;

class ReuseDistanceAnalyzer {
public:
    ReuseDistanceAnalyzer();

    int32_t process_load(address_t address);

    int32_t process_store(address_t address);

    std::unordered_map<int32_t, int32_t> get_reuse_distance_counts();

private:
    std::vector<address_t> addresses;

    uint64_t addresses_offset = 0;
    uint64_t access_num = 0;

    std::unordered_map<address_t, uint64_t> last_access_num_of_address;
    std::unordered_map<int32_t, int32_t> reuse_distance_counts;

    int32_t process_access(address_t address);
    void shorten_addresses();
};

#endif  // REUSE_DISTANCE_ANALYZER_H
