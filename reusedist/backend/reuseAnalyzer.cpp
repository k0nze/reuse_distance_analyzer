#include "reuseAnalyzer.h"
#include "cmath"
#include "unordered_set"
#include <fstream>
#include <sstream>

#define COMPULSORY_MISS -1
#define EMPTY -100

inline u_int64_t key(int i, int j) {
  return (u_int64_t)i << 32 | (unsigned int)j;
}

inline std::pair<int, int> dekey(u_int64_t key) {
  return std::pair<int, int>{key >> 32, (int)key};
};

reuseAnalyzer::reuseAnalyzer(int sets, int ways, int cacheLineSize,
                             int blockSize) {
  // TODO assert sets,ways,cachelinesize is power of 2
  this->sets = sets;
  this->cacheLineSize = cacheLineSize;
  this->ways = ways;

  this->setsMask = sets - 1;
  this->cacheLineShiftOffset = 0;
  while (cacheLineSize >>= 1)
    ++cacheLineShiftOffset;

  t = vector<int32_t>(sets, 0);
  B = blockSize;
  trace = new vector<vector<int32_t>>(sets, vector<int32_t>());
  lastAccesses = new unordered_map<int32_t, int32_t>();
  reuseDistances = new vector<int32_t>();
  reuseDistanceCounts = new unordered_map<int32_t, int32_t>();

  uniqueAccessesInBlock = new vector<unordered_map<u_int64_t, int32_t> *>();

  for (int i = 0; i < sets; ++i) {
    auto m = new unordered_map<u_int64_t, int32_t>();
    uniqueAccessesInBlock->push_back(m);
  }
}

reuseAnalyzer::~reuseAnalyzer() {
  delete trace;
  delete lastAccesses;
  delete reuseDistances;
  delete reuseDistanceCounts;
  for (auto d : *uniqueAccessesInBlock) {
    delete d;
  }
  delete uniqueAccessesInBlock;
}

int32_t reuseAnalyzer::processLoad(int32_t address) {
  return recordAccess(address);
}

int32_t reuseAnalyzer::processStore(int32_t address) {
  return recordAccess(address);
}

unordered_map<int32_t, int32_t> reuseAnalyzer::getReuseDistanceCounts() {
  return *reuseDistanceCounts;
}

vector<std::string> split(const std::string &s, char delim) {
  vector<std::string> result;
  std::stringstream ss(s);
  std::string item;

  while (getline(ss, item, delim)) {
    result.push_back(item);
  }

  return result;
}

void reuseAnalyzer::analyzeAcaFile(const std::string &acaFilePath) {
  std::string line;

  std::ifstream is(acaFilePath);

  auto addrs = std::unordered_set<int>();
  while (getline(is, line)) {
    auto splits = split(line, ' ');

    if (splits[2] == "1") {
      // load
      int addr = stoi(splits[splits.size() - 1], nullptr, 16);
      addrs.insert(addr);
      processLoad(addr);
    }
    if (splits[2] == "2") {
      // store
      int addr = stoi(splits[splits.size() - 1], nullptr, 16);
      addrs.insert(addr);
      processStore(addr);
    }
  }
  for (auto a : addrs) {
    printf("%i\n", a);
  }
}

void reuseAnalyzer::printDistanceCounts() {
  for (auto i : *reuseDistanceCounts)
    printf("reuse distance: %i - count: %i\n", i.first, i.second);
}

int32_t reuseAnalyzer::getSetId(int32_t address) const {
  return (address)&setsMask;
}

int32_t reuseAnalyzer::measureReuseDistance(int32_t lastAccess, int32_t setID) {
  if (lastAccess == COMPULSORY_MISS) {
    compulsoryMissBlockUpdate(setID);
    return COMPULSORY_MISS;
  } else {
    return countDistinctElements(lastAccess, setID);
  }
}

void reuseAnalyzer::recordReuseDistance(int32_t reuseDistance) {
  auto got = (*reuseDistanceCounts).find(reuseDistance);
  if (got != (*reuseDistanceCounts).end()) {
    (*reuseDistanceCounts)[reuseDistance] += 1;
  } else {
    (*reuseDistanceCounts)[reuseDistance] = 1;
  }
  (*reuseDistances).push_back(reuseDistance);
}

void reuseAnalyzer::sanityCheckBlockDict() {}

int32_t reuseAnalyzer::countDistinctElements(int32_t start, int32_t setID) {
  /**
   * instead of counting each distinct element between t1 and t2, a b-tree is
   * used: example: t1 = 77, t2 = 213, B = 10 instead of checking between 87 and
   * 213 for each entry if an access is there we can check 78,79 then the block
   * [80-89] ~ block(1,8) and block [90-99] ~ block (1,9) then block [100-199] ~
   * block(2,1) and finally block [200-210] ~ block(1,20) with 211,212 NOTE: the
   * order of the algorithm is different than in the paper, due to the line
   * after the while loop being unclear what block(l +1,..) is supposed to mean.
   * Imo it is more comprehensible to first iterate from both sides (t1,t2) till
   * the next block is reached: in case of B = 10, first the ones at level 0,
   * then the tens at level 1, the hundreds at level 2 ...
   */
  int32_t stop = t.at(setID);
  int32_t stop_cp = stop;
  int32_t start_cp = start;
  int32_t reuseDist = 0;
  int32_t lvl = 0;

  while (start != stop) {
    auto s = createIterRange(start, stop);
    for (auto i : s) {

      reuseDist += block(lvl, i, setID);
    }
    start /= B;
    stop /= B;
    lvl++;
  }

  /**
   * afterwards the b-tree structure needs to be updated (all the blocks that
   * contain 'start') need to be decremented (because every new access to
   * countDistinctElements will count the lastAccess at 'stop', not the
   * duplicate at 'start') while every block that contains 'stop' need to be
   * incremented
   */
  start = start_cp;
  stop = stop_cp;

  lvl = 0;
  while (start / B != stop / B) {
    start /= B;
    stop /= B;
    (*(*uniqueAccessesInBlock).at(setID))[key(lvl + 1, start)] =
        block(lvl + 1, start, setID) - 1;
    (*(*uniqueAccessesInBlock).at(setID))[key(lvl + 1, stop)] =
        block(lvl + 1, stop, setID) + 1;
    lvl++;
  }
  (*trace).at(setID)[start_cp] = EMPTY;
  return reuseDist;
}

vector<int32_t> reuseAnalyzer::createIterRange(int32_t start,
                                               int32_t stop) const {
  /***
   * Creates the "i" for the block(lvl,i) calls in the following two lines
   * reuseDis=reuseDis+block (lv l,t1 +1) +... +block( lvl ,(t1 /B+ 1)*B -1)
     reuseDis=reuseDis+block (lv l,(t 2/B )*B) +.. .+block (lvl ,t2 -1)
   */
  auto s = std::vector<int32_t>();

  int32_t lower, upper; // lower is inclusive, upper exclusive
  // if both 'start' and 'stop' are in the same block ('start'/B == 'stop'/B)
  // issues arise with the corresponding both loops: then upper of first loop
  // could be greater than 'stop', and lower of second loop could be lower than
  // 'start'. Thus, it is simpler to just do one loop over ['start' +1,'stop')
  // and directly return, otherwise no lower/upper checks necessary
  if (start / B == stop / B) {
    for (int32_t i = start + 1; i < stop; ++i) {
      s.push_back(i);
    }
    return s;
  }
  // First loop replicating the "i" of the first line

  lower = start + 1;
  upper = ((start / B + 1) * B); // (start /B+ 1)*B -1
  for (int32_t i = lower; i < upper; ++i) {
    s.push_back(i);
  }
  // Second loop according to the second line
  lower = ((stop / B) * B);
  upper = stop;
  for (int32_t i = lower; i < upper; ++i) {
    s.push_back(i);
  }

  return s;
}

void reuseAnalyzer::compulsoryMissBlockUpdate(int32_t setID) {

  /**
   * a compulsory miss means that there does not exists a previous access to the
   * current address. this leads to no counting of distinct elements and thus no
   * updating of the block data structure So just increment all the blocks that
   * contain i
   */
  int32_t t_ = t.at(setID);
  int lvl = 0;
  if (B < 2) {
    return;
  }
  for (;;) {
    t_ /= B;
    (*(*uniqueAccessesInBlock).at(setID))[key(lvl + 1, t_)] =
        block(lvl + 1, t_, setID) + 1;
    lvl++;
    if (t_ == 0) {
      // block(l,0) will never be accessed, can stop
      return;
    }
  }
}

int32_t reuseAnalyzer::block(int32_t lvl, int32_t i, int32_t setID) {
  /**
   * Manages the data structure, if the first acceess to (l,i) happens, a new
   * block is entered in the dict with the overhead of counting all the distinct
   * elements in the trace that the block encapsulates
   * TODO instead of counting the trace, for block(l,i) just add all blocks
   * (l-1,i*B...i*B + B-1) does not appear to be that trivial
   */
  if (lvl == 0) {
    return (*trace).at(setID)[i] >= 0 ? 1 : 0;
  } else {
    auto blockDictOfSet = (*uniqueAccessesInBlock).at(setID);
    auto got = (*blockDictOfSet).find(key(lvl, i));
    if (got != (*blockDictOfSet).end()) {
      return got->second;

    } else {
      // TODO fix types
      unsigned int lowerBound = pow(B, lvl) * i;
      unsigned int upperBound = pow(B, lvl) * (i + 1);
      upperBound = upperBound < (*trace).at(setID).size()
                       ? upperBound
                       : (*trace).at(setID).size();
      int elems = 0;
      for (unsigned int j = lowerBound; j < upperBound; ++j) {
        if ((*trace).at(setID)[j] >= 0) {
          ++elems;
        }
      }
      return elems;
    }
  }
}

int32_t reuseAnalyzer::recordAccess(int32_t address) {
  /***
   * Retrieve the last access time, then determine the distance between the last
   * access and now
   */
  int32_t lastAccess;
  int32_t reuseDist;
  int32_t setID;
  address >>= cacheLineShiftOffset;
  auto got = (*lastAccesses).find(address);
  if (got != (*lastAccesses).end()) {
    lastAccess = got->second;
  } else {
    lastAccess = -1;
  }
  setID = getSetId(address);
  reuseDist = measureReuseDistance(lastAccess, setID);
  (*lastAccesses).insert_or_assign(address, t.at(setID));
  recordReuseDistance(reuseDist);
  trace->at(setID).push_back(address);
  t.at(setID) += 1;
  return reuseDist;
}
