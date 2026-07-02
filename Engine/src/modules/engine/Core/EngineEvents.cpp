//
// Created by cmorg on 7/1/2026.
//

#include "EngineEvents.h"

#include <functional>

std::vector<EngineEventCallback> EngineEvents::subscribers[MAX_EVENT]{};

void EngineEvents::subscribe(const EngineEventCode code, const std::function<void(EngineInputContext)>& function, const String& id, Keys key) {
    if (key == MAX_KEYS) {
        subscribers[code].emplace_back(id, function);
    } else {
        subscribers[code].emplace_back(id, function, key);
    }
}

void EngineEvents::unsubscribe(const EngineEventCode code, const String& id) {
    auto& event = subscribers[code];

    std::erase_if(event, [id](const EngineEventCallback& callback) {
       return callback.id.compare(id);
    });
}

void EngineEvents::callEvent(const EngineEventCode code, const EngineInputContext context) {
    for (const auto& event = subscribers[code]; const EngineEventCallback& callback : event) {
        if (callback.expectedKey == MAX_KEYS || callback.expectedKey == context.key) {
            callback.function(context);
        }
    }
}
