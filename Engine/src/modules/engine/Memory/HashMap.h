//
// Created by cmorg on 7/30/2026.
//

#pragma once

#include <cmath>

#include "DynamicArray.h"
#include "FF_Memory.h"
#include "src/modules/engine/Library/HashUtils.h"

inline constexpr unsigned int TREE_THRESHOLD = 10; //Not implemented

template<typename>
inline constexpr bool alwaysFalse = false;

//Struct for returning keys with their value.
template<typename K, typename V>
struct KeyValuePair {
    K key;
    V value;
};

//Contains a parallel array of keys and values.
//Hashmap stores a parallel array of hash keys paired to a HashValue since a hash can contain multiple values.
//Each value here is paired to the raw key for lookup and reference purposes.
template<typename K, typename V>
struct HashValue {
    DynamicArray<K> keyRefs{};
    DynamicArray<V> values{};

    HashValue() {
        keyRefs.initialize(1, HASHMAP);
        values.initialize(1, HASHMAP);
    }
    //TREE GOES HERE

    HashValue(const HashValue&) = delete;
    HashValue& operator=(const HashValue&) = delete;

    HashValue(HashValue&&) noexcept = default;
    HashValue& operator=(HashValue&&) noexcept = default;
};

//ATTENTION, THIS IS AN UNORDERED HASH MAP!
//THE MAP IS SORTED BY HASHES SO BINARY SEARCH CAN BE UTILIZED INTERNALLY!
//HASHES ARE NOT ORDERED BY THE RAW KEY, SO RAW KEY ORDER WILL CHANGE!
template<typename K, typename V>
class FOXFIRE_API HashMap {
    static_assert(
        std::is_same_v<std::remove_cvref_t<K>, String>,
        "HashMap currently only supports String keys."
    );
private:
    //Keys are stored binarily, this allows for binary searches on the key
    DynamicArray<unsigned long> keys{};
    //Values are stored in parallel to keys. Internal data is unordered and iterated.
    DynamicArray<HashValue<K, V>> values{};//Should become a binary tree when there are MANY values
    //The number of values that have been added to this map (Not the number of hash keys!)
    unsigned int size = 0;

    [[nodiscard]]
    unsigned long hashKey(const K& key) const {
        using KeyType = std::remove_cvref_t<K>;

        //Unique to every hashmap. Only checked once.
        if constexpr (std::same_as<KeyType, String>) {
            return HashUtils::generateStringHash(key);
        } else {
            static_assert(alwaysFalse<KeyType>, "HashMap does not support this key type.");
            return -1;
        }
    }

    void createHashMap() {
        keys.initialize(0, HASHMAP);
        values.initialize(0, HASHMAP);
    }

    unsigned int hashExists(const K& key) const {
        if (keys.getLength() == 0) return -1;

        const unsigned long hash = hashKey(key);

        unsigned int low = 0;
        unsigned int high = keys.getLength();

        while (low < high) {
            const unsigned int middle = low + (high - low) / 2;

            if (keys[middle] < hash) {
                low = middle + 1;
            } else {
                high = middle;
            }
        }

        if (low < keys.getLength() && keys[low] == hash) {
            return low;
        }

        return -1;
    }

    unsigned int keyExists(const K& key, unsigned int index) const {
        for (int i = 0; i < values[index].keyRefs.getLength(); i++) {
            if (values[index].keyRefs[i] == key) {
                return i;
            }
        }

        return -1;
    }

    unsigned int findInsertionIndex(const unsigned long hash) {
        unsigned int low = 0;
        auto high = static_cast<unsigned int>(keys.getLength());

        while (low < high) {
            const unsigned int middle = low + (high - low) / 2;

            if (keys[middle] < hash) {
                low = middle + 1;
            } else {
                high = middle;
            }
        }

        return low;
    }

public:
    HashMap() {createHashMap();}
    ~HashMap() = default;

    void addEntry(const K& key, const V& value) {
        const unsigned long hash = hashKey(key);
        const unsigned int existingIndex = hashExists(key);

        if (existingIndex == -1) {
            const unsigned int index = findInsertionIndex(hash);

            HashValue<K, V> entry{};
            if (!entry.keyRefs.push(key)) {
                Logger::logError("Failed to inset key reference into new hash value!");
                return;
            }
            if (!entry.values.push(value)) {
                Logger::logError("Failed to inset value into new hash value!");
                return;
            }


            if (!keys.insertAt(hash, index)) {
                Logger::logError("Failed to insert hash entry into hashmap!");
                return;
            }
            if (!values.insertAt(std::move(entry), index)) {
                Logger::logError("Failed to insert values into hashmap!");
                keys.pop(index);
                return;
            }
            size++;
        } else {
            unsigned int index = keyExists(key, existingIndex);

            if (index == -1) {
                if (!values[existingIndex].keyRefs.push(key)) {
                    Logger::logError("Failed to insert key reference into hashmap value!");
                    return;
                }
                if (!values[existingIndex].values.push(value)) {
                    Logger::logError("Failed to insert value into hashmap values!");
                    values[existingIndex].keyRefs.popBack();
                    return;
                }
                size++;
            } else {
                values[existingIndex].keyRefs[index] = key;
                values[existingIndex].values[index] = value;
            }
        }
    }

    void clearHashMap() {
        size = 0;
        keys.clear();
        values.clear();
    }

    void removeValue(const K& key) {
        //Check that the key exists
        const unsigned int existingIndex = hashExists(key);
        if (existingIndex == -1) {
            Logger::logWarn("Attempted to remove an entry from a hashmap, but the entry does not exist!");
            return;
        }

        const unsigned int index = keyExists(key, existingIndex);
        if (index == -1) {
            Logger::logWarn("Attempted to remove an entry from a hashmap, but the key does not exist!");
            return;
        }

        values[existingIndex].keyRefs.pop(index);
        values[existingIndex].values.pop(index);

        if (values[existingIndex].keyRefs.getLength() == 0) {
            //Remove the key entry in the main mapping since it is now 0 entries.
            keys.pop(existingIndex);
            values.pop(existingIndex);
        }

        size--;
    }

    V* getValue(const K& key) {
        if (size == 0) {
            Logger::logWarn("Hashmap attempted to get the value of a key in an empty map!");
            return nullptr;
        }

        const unsigned int existingIndex = hashExists(key);
        if (existingIndex == -1) {
            return nullptr;
        }

        unsigned int index = keyExists(key, existingIndex);
        if (index == -1) {
            Logger::logWarn("Hashmap does not contain the given key!");
            return nullptr;
        }

        return &values[existingIndex].values[index];
    }

    DynamicArray<KeyValuePair<K, V>> getPairs() const {
        DynamicArray<KeyValuePair<K, V>> pairs{size};

        for (const HashValue<K, V>& pair : values) {
            for (unsigned long i = 0; i < pair.keyRefs.getLength(); i++) {
                if (!pairs.emplace(pair.keyRefs[i], pair.values[i])) {
                    Logger::logError("Failed to insert key value pair into dynamic array!");
                    break;
                }
            }
        }

        if (pairs.getLength() != size) {
            Logger::logWarn("KeyValuePairs does not contain the same number of pairs as the hashmap reports. Expected: " + std::to_string(size) + " got: " + std::to_string(pairs.getLength()));
        }

        return pairs;
    }

    bool keyExists(const K& key) {
        const unsigned int existingIndex = hashExists(key);
        if (existingIndex == -1) {
            return false;
        }

        for (K& keyValue : values[existingIndex].keyRefs) {
            if (key == keyValue) return true;
        }

        return false;
    }

    void setCapacity(unsigned int capacity) {
        if (capacity <= keys.getCapacity()) {
            Logger::logWarn("New Capcity is less than or equal to the current capacity of the hashmap. Command ignored.");
            return;
        }

        keys.ensureSize(capacity);
        values.ensureSize(capacity);
    }
};