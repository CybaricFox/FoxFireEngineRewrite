//
// Created by cmorg on 8/1/2026.
//

#pragma once
#include "ReusableArray.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Memory/HashMap.h"
#include "src/modules/engine/Resources/Contexts.h"

template<typename V, typename C>
requires std::derived_from<C, AssetContext>
class AssetMap {
private:
    ReusableArray<V> data{};
    HashMap<String, C> map{};

    V* getAsset(const String& key) {
        AssetContext* context = getContext(key);
        if (!context) return nullptr;
        return &data.get(context->index);
    }
public:
    void initialize(unsigned int initialCapacity) {
        data.initialize(initialCapacity);
        map.setCapacity(initialCapacity);
    }

    ReusableArray<V>& getData() { return data; }

    C* getContext(const String &key) {
        if (!map.keyExists(key)) return nullptr;
        return map.getValue(key);
    }

    V* acquireAsset(const String& key) {
        AssetContext* context = getContext(key);
        if (!context) return nullptr;
        ++context->referenceCount;
        return &data.get(context->index);
    }

    //Creates a blank context and asset to be edited.
    //Asset is stored, context must be stored with addAssetEntry().
    V* createAsset(String name, C& context) {
        V* asset;
        ++context.referenceCount;

        unsigned int index = data.assign();
        context.index = index;
        map.addEntry(name, context);
        return &data.get(index);
    }

    bool releaseAsset(String name, V*& out) {
        AssetContext* context = getContext(name);
        if (!context || context->referenceCount == 0) return false;

        --context->referenceCount;

        if (context->referenceCount == 0 && context->bAutoRelease) {
            out = getAsset(name);
            data.release(context->index);
            map.removeValue(name);
            Logger::logDebug(name + " was unloaded from the texture system.");

            return true;
        }

        Logger::logDebug(name + " has one less reference. " + std::to_string(context->referenceCount) + " remains.");
        return false;
    }
};
