from typing import Dict


class ReuseDistanceAnalyzer:
    def __init__(self):
        self.addresses = []
        self.access_num = 0
        self.last_access_num_of_address = dict()
        self.reuse_distance_counts = dict()

    def process_load(self, address: int) -> int:
        return self.process_access(address)

    def process_store(self, address: int) -> int:
        return self.process_access(address)

    def process_access(self, address: int) -> int:

        self.access_num += 1

        reuse_distance = -1

        # check if address was accessed before
        if address in self.last_access_num_of_address.keys():
            last_access = self.last_access_num_of_address[address]
            reuse_distance = len(set(self.addresses[last_access:]))

        self.last_access_num_of_address[address] = self.access_num

        self.addresses.append(address)

        if reuse_distance not in self.reuse_distance_counts:
            self.reuse_distance_counts[reuse_distance] = 1
        else:
            self.reuse_distance_counts[reuse_distance] += 1
        return reuse_distance

    def get_reuse_distance_counts(self) -> Dict[int, int]:
        return self.reuse_distance_counts.copy()
