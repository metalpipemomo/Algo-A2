#include "../include/HashTable.h"

#include <functional>
#include <algorithm>
#include <iterator>

template <typename K, typename V>
HashTable<K, V>::HashTable(unsigned int size = 100, double threshold = 0.7)
	: tableSize(size), loadFactorThreshold(threshold)
{
	// Initialize vector of buckets with table size
	hashTable = std::vector<Bucket>(tableSize);
}

template <typename K, typename V>
typename HashTable<K, V>::Iterator HashTable<K, V>::begin()
{
	// Find first occupied bucket within hashtable vector and return Iterator at that bucket
	for (auto it = hashTable.begin(); it != hashTable.end(); it++)
	{
		if (it->occupied) return Iterator{ it, hashTable.end() };
	}

	return end();
}

template <typename K, typename V>
typename HashTable<K, V>::Iterator HashTable<K, V>::end()
{
	// Return Iterator at the end of the hashtable vector
	return Iterator{ hashTable.end(), hashTable.end() };
}

template <typename K, typename V>
V& HashTable<K, V>::operator[](const K& key)
{
	// Find bucket associated with key and return the value
	for (Bucket& bucket : hashTable)
	{
		if (key == bucket.key) return bucket.value;
	}

	// If no bucket found, create entry with default initialized value
	V newValue{};
	insert(key, newValue);

	// Find the newly created key/value pair and return the value
	for (Bucket& bucket : hashTable)
	{
		if (key == bucket.key) return bucket.value;
	}

	// This shouldn't fail but if it does, throw.
	throw std::runtime_error("Insertion failed.");
}

template <typename K, typename V>
void HashTable<K, V>::updateValueForKey(const K& key, V newValue)
{
	// Call overloaded operator to handle this, self-explanatory
	(*this)[key] = newValue;
}

template <typename K, typename V>
void HashTable<K, V>::insert(const K& key, const V& value)
{
	// Initial index for insertion, hashed by std::hash
	size_t index = std::hash<K>()(key) % hashTable.size();

	// Initialize hopInfo to be passed in and modified by findFreeSlot
	unsigned int hopInfo = 0;

	// If initial index collides with a bucket, use findFreeSlot to find a free slot to insert
	// If no free slot was found, rehash the table and try inserting again
	if (hashTable[index].occupied)
	{
		try
		{
			index = findFreeSlot(hashTable, index, hopInfo);
		}
		catch (std::runtime_error e)
		{
			rehash();
			insert(key, value);
			return;
		}
	}
	else
	{
		// Initial index inserted cleanly, set hopInfo to 0001
		hopInfo = 0b0001;
	}

	// Construct bucket with key, value, and modified hopInfo
	Bucket b{};
	b.key = key;
	b.value = value;
	b.occupied = true;
	b.hopInfo = hopInfo;

	// Insert bucket into hashtable
	hashTable[index] = b;

	// If the load factor of the hashtable exceeds the threshold, rehash
	if (loadFactor() > loadFactorThreshold)
	{
		rehash();
	}
}

template <typename K, typename V>
V* HashTable<K, V>::search(const K& key)
{
	// Initial index for insertion, hashed by std::hash
	size_t index = std::hash<K>()(key) % hashTable.size();
	unsigned int hopInfo = hashTable[index].hopInfo;

	// Check if the key was hashed into the initial index, if it was: viola
	if (hashTable[index].occupied && hashTable[index].key == key)
	{
		return &hashTable[index].value;
	}

	// Use hopInfo and the hop range to search the neighborhood for the desired bucket
	// Return the value of bucket if found
	for (int i = 0; i < hopRange; i++)
	{
		if (hopInfo & (1 << i))
		{
			size_t bucket = (index + i) % hashTable.size();
			if (hashTable[bucket].occupied && hashTable[bucket].key == key)
			{
				return &hashTable[bucket].value;
			}
		}
	}

	// Return a nullptr if key does not exist in table
	return nullptr;
}

template <typename K, typename V>
bool HashTable<K, V>::remove(const K& key)
{
	// Find the bucket associated with the key
	for (Bucket& bucket : hashTable)
	{
		if (bucket.key == key && bucket.occupied)
		{
			// Set occupied to false and reset hopInfo
			bucket.occupied = false;
			bucket.hopInfo = 0;
			return true;
		}
	}

	// If bucket associated with key is not found, return false
	return false;
}

template <typename K, typename V>
void HashTable<K, V>::clear()
{
	// Clear hashtable vector
	hashTable.clear();
	// Reinstantiate hashtable vector with tableSize
	hashTable = std::vector<Bucket>(tableSize);
}

template <typename K, typename V>
unsigned int HashTable<K, V>::size() const
{
	// Count occupied buckets and return the count
	unsigned int count = 0;
	for (const Bucket& bucket : hashTable)
	{
		count += static_cast<int>(bucket.occupied);
	}

	return count;
}

template <typename K, typename V>
double HashTable<K, V>::loadFactor() const
{
	// LoadFactor = Num of Occupied Buckets / Table Size
	return (double)size() / (double)tableSize;
}

template <typename K, typename V>
unsigned int HashTable<K, V>::findFreeSlot(std::vector<Bucket>& table,
	unsigned int startIndex, unsigned int& currentHop)
{
	// Lambda function to compute the difference between the current bucket and the start bucket,
	// taking into account wrap-around at the end of the table
	auto BucketDifference = [](unsigned int current, unsigned int start, unsigned int tableSize) -> int
		{
			if (current >= start)
			{
				return current - start;
			} else
			{
				return tableSize - start + current;
			}
		};

	unsigned int currentBucket = startIndex;

	// Find an unoccupied slot
	while (true)
	{
		currentBucket = (currentBucket + 1) % table.size();
		if (!table[currentBucket].occupied || currentBucket == startIndex) break;
	}

	// Get the difference between the current bucket and the start index
	int bucketDifference = BucketDifference(currentBucket, startIndex, table.size());

	// If the bucket difference is within the hop range, update the hop information and return
	// Bitwise shifting is used to set a specific bit in hopInfo based on bucket difference
	if (bucketDifference < hopRange)
	{
		currentHop = (1 << bucketDifference) & 0x0F;
		return currentBucket;
	}

	// While the bucket difference is greater than or equal to hop range, try to swap the elements
	while (bucketDifference >= hopRange)
	{
		unsigned int start = (currentBucket + table.size() - hopRange + 1) % table.size();
		bool swapped = false;
		// Try to find a suitable bucket within hop range to swap with the current bucket
		for (unsigned int i = start; i != currentBucket; i = (i + 1) % table.size())
		{
			// Check if hopInfo has any set bits to see if it is within the hop range
			if (table[i].hopInfo)
			{
				table[currentBucket] = table[i];
				table[i].occupied = false;
				table[i].hopInfo = 0;
				currentBucket = i;
				swapped = true;
				break;
			}
		}

		// If no suitable bucket was found to swap, throw an error
		// Caught in insert, triggers a rehash and a retries the insert
		if (!swapped)
		{
			throw std::runtime_error("This shit aint working dawg");
		}

		bucketDifference = BucketDifference(currentBucket, startIndex, table.size());
	}
	
	// Update hop information based on the final bucket position
	// Use bitwise shift to set the specific bit in hopInfo, tells you the bucket's position relative to the start.
	currentHop = (1 << (currentBucket - startIndex)) & 0x0F;
	return currentBucket;
}

template <typename K, typename V>
void HashTable<K, V>::rehash()
{
	// Creates a new hashtable with double the size of the current one
	HashTable<K, V> hTable{ tableSize * 2, loadFactorThreshold };
	Iterator it = begin();
	// Inserts key/value pairs from old table into new table
	for (it; it != end(); ++it)
	{
		hTable.insert((*it)->key, (*it)->value);
	}

	// Sets new hashtable as current
	*this = hTable;
}