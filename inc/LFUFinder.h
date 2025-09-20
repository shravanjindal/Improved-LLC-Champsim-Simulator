#ifndef LFUFINDER_H
#define LFUFINDER_H

#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <climits>
#include <unordered_set>
using namespace std;
extern "C"
{
    class LFUSetFinder
    {
    private:
    struct CacheEntry {
        int frequency;
        std::unordered_set<int> ways;
    };

    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> minHeap; // min-heap of pairs (frequency, set)
    std::unordered_map<int, CacheEntry> cacheEntries; // Map of set to cache entry                                                                       // Set of accessed sets

    public:
        void insertSet(int set, int associativity);

        uint32_t findLeastUsedSet();

        void incrementAccessCount(int set);
    };
}
#endif