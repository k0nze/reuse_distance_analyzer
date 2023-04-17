#include <cassert>
#include <iostream>
#include <vector>

#include "reuse_distance_analyzer.h"

int main() {
    ReuseDistanceAnalyzer rda = ReuseDistanceAnalyzer();

    rda.process_load(1);
    rda.process_load(2);
    rda.process_load(3);
    rda.process_load(4);

    rda.process_store(1);
    rda.process_store(2);
    rda.process_store(3);
    rda.process_store(4);

    auto reuse_distance_counts = rda.get_reuse_distance_counts();

    assert(reuse_distance_counts.contains(-1));
    assert(reuse_distance_counts[-1] == 4);
    assert(reuse_distance_counts.contains(3));
    assert(reuse_distance_counts[-1] == 4);

    // those access are not distinct, therefore the next access of 3 must
    // have a reuse distance of 1
    rda.process_store(4);
    rda.process_store(4);
    rda.process_store(4);
    rda.process_store(4);
    rda.process_store(4);

    rda.process_store(3);

    reuse_distance_counts = rda.get_reuse_distance_counts();

    assert(reuse_distance_counts.contains(0));
    assert(reuse_distance_counts[0] == 5);
    assert(reuse_distance_counts.contains(1));
    assert(reuse_distance_counts[1] == 1);

    // there are only two distinct accesses between this access of 2 and the
    // next
    rda.process_load(2);
    // there are only three distinct accesses between this access of 2 and
    // the next
    rda.process_load(1);

    reuse_distance_counts = rda.get_reuse_distance_counts();

    assert(reuse_distance_counts.contains(2));
    assert(reuse_distance_counts[2] == 1);
    assert(reuse_distance_counts.contains(3));
    assert(reuse_distance_counts[3] == 5);
}
