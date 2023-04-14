#ifndef REUSEDISTV2_REUSEANALYZER_H
#define REUSEDISTV2_REUSEANALYZER_H

#include <cstdlib>

#include "memory"
#include "unordered_map"
#include "vector"
using std::vector, std::unordered_map, std::shared_ptr;

// Forward Declarations
class SetAssociativeCache;
class ACADLInstructionGenerator;
class reuseAnalyzer {
public:
    reuseAnalyzer(int sets, int ways, int cacheLineSize, int blockSize = 16);
    /*
    explicit reuseAnalyzer(const std::shared_ptr<SetAssociativeCache> &cache,
                           int blockSize = 16);
    ~reuseAnalyzer();
    */

    int32_t processLoad(int32_t address);

    int32_t processStore(int32_t address);

    unordered_map<int32_t, int32_t> getReuseDistanceCounts();

    /*
    void analyzeAcaFile(const std::string &acaFilePath);
    vector<int> analyzeInstructionGenerator(
        const shared_ptr<ACADLInstructionGenerator> &instructionGenerator);
    void printDistanceCounts();
    */

private:
    // list of address accesses per set
    vector<vector<int32_t>>* trace;
    // list of reuse distances
    vector<int32_t>* reuseDistances;
    // given a address, receive timing when address was last accessed,
    // if address was not accessed yet, it gets timing -1. However negative values
    // are never stored here
    unordered_map<int32_t, int32_t>* lastAccesses;
    // maps reuse distance (-1 if compulsory miss, else real distance) to
    // occurency count
    unordered_map<int32_t, int32_t>* reuseDistanceCounts;
    // the b-tree data structure for reducing the complexity. key is a 64 bit int,
    // that is a composite of 2 32 bit values (lvl,i). Every set has an own b-tree
    vector<unordered_map<u_int64_t, int32_t>*>* uniqueAccessesInBlock;
    int B;
    vector<int32_t> t;
    int sets;
    int32_t cacheLineShiftOffset;
    int32_t setsMask;
    int ways;
    int cacheLineSize;

    int32_t getSetId(int32_t address) const;

    int32_t measureReuseDistance(int32_t lastAccess, int32_t setID);

    void recordReuseDistance(int32_t reuseDistance);

    void sanityCheckBlockDict();

    int32_t countDistinctElements(int32_t start, int32_t setID);

    vector<int32_t> createIterRange(int32_t start, int32_t stop) const;

    void compulsoryMissBlockUpdate(int32_t setID);

    int32_t block(int32_t lvl, int32_t i, int32_t setID);

    int32_t recordAccess(int32_t address);
};

#endif  // REUSEDISTV2_REUSEANALYZER_H
