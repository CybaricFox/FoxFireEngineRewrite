/**
*   @file HashMap.h
 *  @layer Engine
 *  @module Memory
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include <cmath>

#include "DynamicArray.h"
#include "FF_Memory.h"
#include "src/modules/engine/Library/HashUtils.h"

inline constexpr unsigned int TREE_THRESHOLD = 10; //Not implemented

template<typename>
inline constexpr bool alwaysFalse = false;

/**
 * @brief Contains a key and value pair. Does not actually contain the originals.
 * @tparam K Key Type
 * @tparam V Value Type
 */
template<typename K, typename V>
struct KeyValuePair {
    K* key = nullptr;
    V* value = nullptr;
};

//Contains a parallel array of keys and values.
//Hashmap stores a parallel array of hash keys paired to a HashValue since a hash can contain multiple values.
//Each value here is paired to the raw key for lookup and reference purposes.
/**
 * @brief Contains a parallel array of raw keys and values.
 * Hashkeys are paired to HashValues  in the hashmap since keys can have duplicate hashes.
 * @tparam K Key Type
 * @tparam V Value Type
 */
template<typename K, typename V>
struct HashValue {
    /** @brief Array of raw keys */
    DynamicArray<K> keyRefs{};
    /** @brief Array of values */
    DynamicArray<V> values{};

    HashValue() {
        keyRefs.initialize(1, HASHMAP);
        values.initialize(1, HASHMAP);
    }
    ~HashValue() {
        keyRefs.shutdown();
        values.shutdown();
    }
    //TREE GOES HERE

    HashValue(const HashValue&) = delete;
    HashValue& operator=(const HashValue&) = delete;

    HashValue(HashValue&&) noexcept = default;
    HashValue& operator=(HashValue&&) noexcept = default;
};

/**
 * @brief Hashes keys and stores them parallel to their values.
 * This is an unordered hashmap.
 * @tparam K Type to use for the key. Only supports strings atm.
 * @tparam V Type to use for the value.
 */
template<typename K, typename V>
class FOXFIRE_API HashMap {
    static_assert(
        std::is_same_v<std::remove_cvref_t<K>, String>,
        "HashMap currently only supports String keys."
    );
private:
    /** @brief Array of hashkeys. This array is sorted and searched binarily. */
    DynamicArray<unsigned long> keys{};
    /** @brief Array of hashvalues. This array is not sorted. Values are stored in parallel to their raw key. */
    DynamicArray<HashValue<K, V>> values{};
    /** @brief The number of values added to this map. Not the number of hash keys. */
    unsigned int size = 0;

    /**
     * @brief Hashes the key
     * @param key The key to hash
     * @return The hashed key.
     */
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

    /**
     * @brief Checks if the hash exists for the given key. Does not check if the key itself exists.
     * @param key The key to hash and check.
     * @return The index of the hashkey.
     */
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

    /**
     * @brief Checks if the given raw key exists.
     * @param key The key to check.
     * @param index The index of its hash.
     * @return The internal index of the key.
     */
    unsigned int keyExists(const K& key, unsigned int index) const {
        for (int i = 0; i < values[index].keyRefs.getLength(); i++) {
            if (values[index].keyRefs[i] == key) {
                return i;
            }
        }

        return -1;
    }

    /**
     * @brief Returns the index that would keep the Hashkey array sorted
     * @param hash The hashkey to store
     * @return The index to store the hash at.
     */
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
    HashMap() = default;
    ~HashMap() = default;

    HashMap(const HashMap&) = delete;
    HashMap& operator=(const HashMap&) = delete;

    HashMap(HashMap&&) noexcept = default;
    HashMap& operator=(HashMap&&) noexcept = default;

    /**
     * @brief Adds a key, value pair to the hashmap.
     * @param key The key of the value.
     * @param value The value to store.
     */
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

    /**
     * @brief Removes all elements in the hashmap.
     */
    void clearHashMap() {
        size = 0;
        keys.clear();
        values.clear();
    }

    /**
     * @brief Removes a key, value pair from the hashmap.
     * @param key The key of the value.
     */
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

    /**
     * @brief Gets a value from the key
     * @param key The key of the value
     * @return Pointer to the value.
     */
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

    /**
     * @brief Converts the hashmap to key, value pairs and returns them. The Pairs are references.
     * Avoid using pairs for assigning references as this function does not track referenceCount.
     * @return A DynamicArray of KeyValue pairs.
     */
    DynamicArray<KeyValuePair<K, V>> getPairs() {
        DynamicArray<KeyValuePair<K, V>> pairs{size};

        for (auto& pair : values) {
            for (unsigned long i = 0; i < pair.keyRefs.getLength(); i++) {
                if (!pairs.emplace(&pair.keyRefs[i], &pair.values[i])) {
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

    /**
     * @brief Checks if the given hashkey and raw key exists.
     * @param key The key to check.
     * @return True if both exist, False if only the hashkey or neither exist.
     */
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

    /**
     * @brief Sets the capacity of the hashmap
     * @param capacity Capacity to reserve.
     */
    void setCapacity(unsigned int capacity) {
        if (capacity <= keys.getCapacity()) {
            Logger::logWarn("New Capcity is less than or equal to the current capacity of the hashmap. Command ignored.");
            return;
        }

        keys.ensureSize(capacity);
        values.ensureSize(capacity);
    }

    void shutdown() {
        keys.shutdown();
        values.shutdown();
        size = 0;
    }

    void createHashMap(unsigned int initialCapacity = 0) {
        keys.initialize(initialCapacity, HASHMAP);
        values.initialize(initialCapacity, HASHMAP);
    }

    [[nodiscard]] unsigned int getLength() const {return size;}

};