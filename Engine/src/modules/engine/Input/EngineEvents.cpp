//
// Created by cmorg on 7/1/2026.
//

#include "EngineEvents.h"

#include <cstring>
#include <functional>

#include "src/modules/engine/Memory/DynamicArray.h"

EngineEvents* EngineEvents::instance = nullptr;

void EngineEvents::initialize() {
    subscribers.initialize(MAX_EVENT);
    for (int i = 0; i < MAX_EVENT; i++) {
        subscribers.emplace();
        subscribers[i].initialize();
    }

    instance = this;
}

void EngineEvents::shutdown() {
    for (int i = 0; i < MAX_EVENT; i++) {
        subscribers[i].shutdown();
    }
    subscribers.shutdown();
}

void EngineEvents::closeApplication() {
    instance->callEvent(QUIT, EngineInputContext{});
}

void EngineEvents::resizeApplication(const EngineInputContext context) {
    instance->callEvent(RESIZED, context);
}

void EngineEvents::subscribe(const EngineEventCode code, const std::function<void(EngineInputContext)>& function, const String& id, Keys key) {
    if (key == MAX_KEYS) {
        subscribers[code].emplace(id, function);
    } else {
        subscribers[code].emplace(id, function, key);
    }
}

void EngineEvents::unsubscribe(const EngineEventCode code, const String& id) {
    DynamicArray<EngineEventCallback>& event = subscribers[code];

    unsigned long index = -1;
    for (unsigned long i = 0; i < event.getLength(); ++i) {
        if (strcmp(event[i].id.c_str(), id.c_str()) == 0) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        event.pop(index);
    }
}

void EngineEvents::callEvent(const EngineEventCode code, const EngineInputContext context) {
    for (const auto& event = subscribers[code]; const EngineEventCallback& callback : event) {
        if (callback.expectedKey == MAX_KEYS || callback.expectedKey == context.key) {
            callback.function(context);
        }
    }
}
