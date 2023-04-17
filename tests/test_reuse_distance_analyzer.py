import unittest
import timeit
import os
import random

from src.reuse_distance_analyzer import ReuseDistanceAnalyzer


class TestReuseDistanceAnalyzer(unittest.TestCase):
    def test_reuse_distance_counts(self):
        # setting all cache parameters 1 results in the literature defined and cache parameter independent reuse distance
        # rda = ReuseDistanceAnalyzer(sets=1, ways=1, cache_line_size=1, block_size=32)
        rda = ReuseDistanceAnalyzer()

        rda.process_load(1)
        rda.process_load(2)
        rda.process_load(3)
        rda.process_load(4)

        rda.process_store(1)
        rda.process_store(2)
        rda.process_store(3)
        rda.process_store(4)

        reuse_distance_counts = rda.get_reuse_distance_counts()

        self.assertIn(-1, reuse_distance_counts.keys())
        self.assertEqual(reuse_distance_counts[-1], 4)
        self.assertIn(3, reuse_distance_counts.keys())
        self.assertEqual(reuse_distance_counts[3], 4)

        # those access are not distinct, therefore the next access of 3 must
        # have a reuse distance of 1
        rda.process_store(4)
        rda.process_store(4)
        rda.process_store(4)
        rda.process_store(4)
        rda.process_store(4)

        rda.process_store(3)

        reuse_distance_counts = rda.get_reuse_distance_counts()

        self.assertIn(0, reuse_distance_counts.keys())
        self.assertEqual(reuse_distance_counts[0], 5)
        self.assertIn(1, reuse_distance_counts.keys())
        self.assertEqual(reuse_distance_counts[1], 1)

        # there are only two distinct accesses between this access of 2 and the
        # next
        rda.process_load(2)
        # there are only three distinct accesses between this access of 2 and
        # the next
        rda.process_load(1)

        reuse_distance_counts = rda.get_reuse_distance_counts()
        
        self.assertIn(2, reuse_distance_counts.keys())
        self.assertEqual(reuse_distance_counts[2], 1)
        self.assertIn(3, reuse_distance_counts.keys())
        self.assertEqual(reuse_distance_counts[3], 5)

    # @unittest.skipIf(
    #    "RUNTIME_BENCHMARK" in os.environ and os.environ["RUNTIME_BENCHMARK"] == "ON",
    #    "$RUNTIME_BENCHMARK not set to ON",
    # )
    # @unittest.skip
    def test_random_trace_performance(self):
        rda = ReuseDistanceAnalyzer()

        lower_address = 0x1000
        upper_address = 0x2000
        num_addresses = 100_000

        num_runs = 5
        runtimes = []

        for run in range(num_runs):
            start_time = timeit.default_timer()

            for _ in range(num_addresses):
                address = random.randint(lower_address, upper_address)
                rda.process_access(address)

            end_time = timeit.default_timer()
            runtime = end_time - start_time
            runtimes.append(runtime)

            print(f"run {run}: {runtime}")

        print(f"avg. runtime: {sum(runtimes)/len(runtimes)}")
