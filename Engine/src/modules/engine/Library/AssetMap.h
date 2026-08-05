/**
*   @file AssetMap.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */


#pragma once
#include "ReusableArray.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Memory/HashMap.h"
#include "src/modules/engine/Resources/Contexts.h"

/**
 * @brief Maps asssets to asset contexts
 * @tparam V The Asset Type
 * @tparam C The Context Type. Must be an AssetContext or a derivative.
 */
template<typename V, typename C>
requires std::derived_from<C, AssetContext>
class AssetMap {
private:
    /** @brief Holds the assets */
    ReusableArray<V> data{};

    /** @brief Holds the contexts */
    HashMap<String, C> map{};

    /**
     * @brief Returns a pointer to the asset. Does not increment reference count. Should be used internally.
     * @param key Name of the asset
     * @return Pointer to the asset
     */
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

    /**
     * @brief Returns the Asset array.
     * @return A ReusableArray of assets.
     */
    ReusableArray<V>& getData() { return data; }

    /**
     * @brief Gets the context of an asset
     * @param key Name of the asset
     * @return Context for that asset
     */
    C* getContext(const String &key) {
        if (!map.keyExists(key)) return nullptr;
        return map.getValue(key);
    }

    /**
     * @brief Gets an asset. Increments reference count. Should be used externally.
     * @param key Name of the asset
     * @return Pointer to the asset.
     */
    V* acquireAsset(const String& key) {
        AssetContext* context = getContext(key);
        if (!context) return nullptr;
        ++context->referenceCount;
        return &data.get(context->index);
    }

    //Creates a blank context and asset to be edited.
    //Asset is stored, context must be stored with addAssetEntry().
    /**
     * @brief Creates a new Asset, Context pair. Will reuse freed assets/contexts if available.
     * @param name Name of the asset.
     * @param context OUT created context.
     * @return Pointer to the new asset.
     */
    V* createAsset(String name, C& context) {
        ++context.referenceCount;

        unsigned int index = data.assign();
        context.index = index;
        map.addEntry(name, context);
        return &data.get(index);
    }

    /**
     * @brief Decrements the assets reference count. If the asset has no references remaining, the asset, context pair
     * is cleaned to be reused later.
     * @param name Name of the asset.
     * @param out OUT pointer to the asset.
     * @return True if the asset was cleaned. False if the asset has remaining references.
     */
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
