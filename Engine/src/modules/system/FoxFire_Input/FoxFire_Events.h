/**
*   @file FoxFire_Events.h
 *  @layer System
 *  @module FoxFire_Input
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include <functional>
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 * @brief Base event class used by the system. Does nothing on its own.
 */
struct BaseEvent {
    BaseEvent() = default;
};

//T is the callback struct chosen by the registerer.
/**
 * @brief An Event
 * @tparam T The callback of this event
 */
template<typename T>
struct Event final : BaseEvent{
    /** @brief array of functions that are listening for this event */
    DynamicArray<std::function<void(const T&)>> listeners{};

    Event() {listeners.initialize();}

    /**
     * @brief Subscribes a function to listen to this event
     * @param function The function that will be called
     */
    void subscribe(std::function<void(const T&)> function) {
        listeners.emplace(function);
    };

    /**
     * @brief Call the event and activate its listeners
     * @param callback The callback object to send to each listener
     */
    void call(T& callback) {
        for (auto& listener : listeners) {
            listener(callback);
        }
    }

    /**
     * @brief Destroys this event and removes its listeners.
     */
    void destroyEvent(){listeners.shutdown();}
};

/**
 * @brief Identical to Event<T> but for no callback.
 */
template<>
struct Event<void> final : BaseEvent{
    //array of functions that use the callback data
    DynamicArray<std::function<void()>> listeners{};

    Event() {listeners.initialize();}
    ~Event() {listeners.shutdown();}

    //Subscribes a function to this event
    void subscribe(const std::function<void()>& function) {
        listeners.emplace(function);
    };

    void call() {
        for (auto& listener : listeners) {
            listener();
        }
    }

    void destroyEvent(){listeners.shutdown();}
};

