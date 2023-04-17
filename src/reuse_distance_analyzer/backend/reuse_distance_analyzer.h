#ifndef REUSE_DISTANCE_ANALYZER_H
#define REUSE_DISTANCE_ANALYZER_H

#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

typedef uint64_t address_t;

class ReuseDistanceAnalyzer {
public:
    ReuseDistanceAnalyzer();

    int32_t process_load(address_t address);

    int32_t process_store(address_t address);

    //unordered_map<int32_t, int32_t> get_reuse_distance_counts();

private:
    int32_t process_access(address_t address);
};

#endif  // REUSE_DISTANCE_ANALYZER_H
