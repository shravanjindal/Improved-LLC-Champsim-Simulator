#include "LFUFinder.h"

void LFUSetFinder::insertSet(int set, int associativity)
{
    CacheEntry entry;
    entry.frequency = 0;
    for (int i = 0; i < associativity; ++i)
    {
        entry.ways.insert(i);
    }
    cacheEntries[set] = entry;
    minHeap.push({0, set}); // Initialize access frequency to 0
}

uint32_t LFUSetFinder::findLeastUsedSet()
{
    int leastUsedSet = INT_MAX;
    int leastAccessFreq = INT_MAX;

    for (const auto &pair : cacheEntries)
    {
        if (pair.second.frequency < leastAccessFreq)
        {
            leastUsedSet = pair.first;
            leastAccessFreq = pair.second.frequency;
        }
    }

    return leastUsedSet;
}

void LFUSetFinder::incrementAccessCount(int set)
{
    cacheEntries[set].frequency++;
}