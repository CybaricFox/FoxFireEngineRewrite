//
// Created by cmorg on 7/30/2026.
//

#pragma once

#include <functional>
#include "src/modules/engine/Memory/DynamicArray.h"

struct BaseEventData {
    BaseEventData() = default;
};

//T is the callback struct chosen by the registerer.
template<typename T>
struct EventData final : BaseEventData{
    //array of functions that use the callback data
    DynamicArray<std::function<void(const T&)>> listeners{};

    EventData() {listeners.initialize();}

    //Subscribes a function to this event
    void subscribe(std::function<void(const T&)> function) {
        listeners.emplace(function);
    };

    void call(T& callback) {
        for (auto& listener : listeners) {
            listener(callback);
        }
    }

    void destroyEvent(){listeners.shutdown();}
};

template<>
struct EventData<void> final : BaseEventData{
    //array of functions that use the callback data
    DynamicArray<std::function<void()>> listeners{};

    EventData() {listeners.initialize();}
    ~EventData() {listeners.shutdown();}

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

