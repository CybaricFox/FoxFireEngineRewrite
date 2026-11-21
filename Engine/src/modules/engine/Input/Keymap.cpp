//
// Created by cmorg on 11/19/2026.
//

#include "Keymap.h"

Keymap::Keymap() {
    bOverrideAll = false;

    for (unsigned int i = 0; i < MAX_KEYS; i++) {
        entries[i].key = i;
    }
}

void Keymap::destroy() {
    for (unsigned int i = 0; i < MAX_KEYS; i++) {
        const KeymapEntry& entry = entries[i];
        KeymapBinding* binding = entry.bindings;
        while (binding) {
            KeymapBinding* next = binding->next;
            FF_Memory::ff_free<KeymapBinding>(binding, GAME);
            binding = next;
        }
    }
}

bool Keymap::hasModifiers(const KeymapBinding &binding, const unsigned int flags) {
    return binding.modifierFlags == 0 || (flags & binding.modifierFlags) == binding.modifierFlags;
}

void Keymap::addBinding(Keys key, KeymapBindType type, unsigned int modifierFlags, void *data, std::function<void(Keys key, KeymapBindType type, unsigned int modifierFlags, void *data)> callback, const String &id) {
    if (id.isEmpty()) {
        Logger::logError("Key Binding id cannot be empty!");
        return;
    }

    KeymapEntry& entry = entries[key];
    KeymapBinding* node = entry.bindings;
    KeymapBinding* previous = entry.bindings;

    while (node) {
        if (node->id == id) {
            Logger::logError("Key Binding already exists with id " + id);
            return;
        }
        previous = node;
        node = node->next;
    }

    KeymapBinding* newBinding = FF_Memory::ff_allocate<KeymapBinding>(GAME);
    newBinding->id = id;
    newBinding->callback = callback;
    newBinding->modifierFlags = modifierFlags;
    newBinding->type = type;
    newBinding->next = nullptr;
    newBinding->data = data;

    if (previous) {
        previous->next = newBinding;
    } else {
        entry.bindings = newBinding;
    }
}

void Keymap::removeBinding(const Keys key, const String &id) const {
    const KeymapEntry& entry = entries[key];
    KeymapBinding* node = entry.bindings;
    KeymapBinding* previous = entry.bindings;

    while (node) {
        if (node->id == id) {
            previous->next = node->next;
            FF_Memory::ff_free<KeymapBinding>(node, GAME);
            return;
        }

        previous = node;
        node = node->next;
    }
}

bool Keymap::processBindings(const bool pressed, const unsigned int modifierFlags, const Keys key) {
    const KeymapBinding* binding = entries[key].bindings;
    bool unset = false;

    while (binding) {
        if (binding->type == KEYMAP_BIND_UNSET) {
            unset = true;
            break;
        }
        if (binding->callback != nullptr && hasModifiers(*binding, modifierFlags)) {
            if (pressed && binding->type == KEYMAP_BIND_PRESS) {
                binding->callback(key, binding->type, modifierFlags, binding->data);
            } else if (!pressed && binding->type == KEYMAP_BIND_RELEASE) {
                binding->callback(key, binding->type, modifierFlags, binding->data);
            }
        }

        binding = binding->next;
    }

    if (unset || bOverrideAll) {
        return true;
    }

    return false;
}

bool Keymap::processHold(const unsigned int modifierFlags, const Keys key) {
    const KeymapBinding* binding = entries[key].bindings;
    bool unset = false;

    while (binding) {
        if (binding->type == KEYMAP_BIND_UNSET) {
            unset = true;
            break;
        }
        if (binding->callback != nullptr || hasModifiers(*binding, modifierFlags)) {
            if (binding->type == KEYMAP_BIND_HOLD) {
                binding->callback(key, binding->type, modifierFlags, binding->data);
            }
        }

        binding = binding->next;
    }

    if (unset || bOverrideAll) {
        return true;
    }

    return false;
}
