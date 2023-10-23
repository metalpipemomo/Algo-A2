#include "../include/HashTable.h"

#include <functional>
#include <algorithm>
#include <iterator>

template <typename K, typename V>
HashTable<K, V>::HashTable(unsigned int size = 100, double threshold = 0.7)
	: tableSize(size), loadFactorThreshold(threshold)
{
	hashTable = std::vector<Bucket>(tableSize);
}

template <typename K, typename V>
typename HashTable<K, V>::Iterator HashTable<K, V>::begin()
{
	for (auto it = hashTable.begin(); it != hashTable.end(); it++)
	{
		if (it->occupied) return Iterator{ it, hashTable.end() };
	}

	return end();
}

template <typename K, typename V>
typename HashTable<K, V>::Iterator HashTable<K, V>::end()
{
	return Iterator{ hashTable.end(), hashTable.end() };
}

template <typename K, typename V>
V& HashTable<K, V>::operator[](const K& key)
{
	for (Bucket& bucket : hashTable)
	{
		if (key == bucket.key) return bucket.value;
	}

	V newValue{};

	insert(key, newValue);

	for (Bucket& bucket : hashTable)
	{
		if (key == bucket.key) return bucket.value;
	}

	throw std::runtime_error("This shit crazy dawg");
}

template <typename K, typename V>
void HashTable<K, V>::updateValueForKey(const K& key, V newValue)
{
	(*this)[key] = newValue;
}

template <typename K, typename V>
void HashTable<K, V>::insert(const K& key, const V& value)
{
	size_t index = std::hash<K>()(key) % hashTable.size();

	unsigned int hopInfo = 0;

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
		hopInfo = 0b0001;
	}

	Bucket b{};
	b.key = key;
	b.value = value;
	b.occupied = true;
	b.hopInfo = hopInfo;

	hashTable[index] = b;

	if (loadFactor() > loadFactorThreshold)
	{
		rehash();
	}
}

template <typename K, typename V>
V* HashTable<K, V>::search(const K& key)
{
	size_t index = std::hash<K>()(key) % hashTable.size();
	unsigned int hopInfo = hashTable[index].hopInfo;

	if (hashTable[index].occupied && hashTable[index].key == key)
	{
		return &hashTable[index].value;
	}

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

	return nullptr;
}

template <typename K, typename V>
bool HashTable<K, V>::remove(const K& key)
{
	for (Bucket& bucket : hashTable)
	{
		if (bucket.key == key && bucket.occupied)
		{
			bucket.occupied = false;
			bucket.hopInfo = 0;
			return true;
		}
	}

	return false;
}

template <typename K, typename V>
void HashTable<K, V>::clear()
{
	hashTable.clear();
	hashTable = std::vector<Bucket>(tableSize);
}

template <typename K, typename V>
unsigned int HashTable<K, V>::size() const
{
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
	return (double)size() / (double)tableSize;
}

template <typename K, typename V>
unsigned int HashTable<K, V>::findFreeSlot(std::vector<Bucket>& table,
	unsigned int startIndex, unsigned int& currentHop)
{
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

	while (true)
	{
		currentBucket = (currentBucket + 1) % table.size();
		if (!table[currentBucket].occupied || currentBucket == startIndex) break;
	}

	int bucketDifference = BucketDifference(currentBucket, startIndex, table.size());

	if (bucketDifference < hopRange)
	{
		currentHop = (1 << bucketDifference) & 0x0F;
		return currentBucket;
	}

	while (bucketDifference >= hopRange)
	{
		unsigned int start = (currentBucket + table.size() - hopRange + 1) % table.size();
		bool swapped = false;
		for (unsigned int i = start; i != currentBucket; i = (i + 1) % table.size())
		{
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

		if (!swapped)
		{
			throw std::runtime_error("This shit aint working dawg");
		}

		bucketDifference = BucketDifference(currentBucket, startIndex, table.size());
	}

	currentHop = (1 << (currentBucket - startIndex)) & 0x0F;
	return currentBucket;
}

template <typename K, typename V>
void HashTable<K, V>::rehash()
{
	HashTable<K, V> hTable{ tableSize * 2, loadFactorThreshold };
	Iterator it = begin();
	for (it; it != end(); ++it)
	{
		hTable.insert((*it)->key, (*it)->value);
	}

	*this = hTable;
}