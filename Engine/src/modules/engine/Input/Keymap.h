//
// Created by cmorg on 11/19/2026.
//

#pragma once
#include <functional>

#include "EngineEvents.h"
#include "foxfire_export.h"

/**
 *  @file Keymap.h
 *  @layer Engine
 *  @module Input
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/19/2026
 *
 *  @copyright (c) 2026
 */

enum KeymapModifierFlags {
    KEYMAP_MODIFIER_NONE = 0x0,
    KEYMAP_MODIFIER_SHIFT = 0x1,
    KEYMAP_MODIFIER_CONTROL = 0x2,
    KEYMAP_MODIFIER_ALT = 0x4,
};

enum KeymapBindType {
    KEYMAP_BIND_UNDEFINED = 0x0,
    KEYMAP_BIND_PRESS = 0x1,
    KEYMAP_BIND_RELEASE = 0x2,
    KEYMAP_BIND_HOLD = 0x4,
    KEYMAP_BIND_UNSET = 0x8
};

#define KeymapCallback std::function<void(Keys key, KeymapBindType type, unsigned int modifierFlags, void* data)>
#define KeymapLambda (Keys key, KeymapBindType type, unsigned int modifierFlags, void* data)

struct KeymapBinding {
    KeymapBindType type{};
    unsigned int modifierFlags = 0;
    String id{};
    KeymapCallback callback = nullptr;
    void* data = nullptr;
    KeymapBinding* next = nullptr;
};

struct KeymapEntry {
    unsigned int key{};
    KeymapBinding* bindings = nullptr;
};

class FOXFIRE_API Keymap {
private:
    bool bOverrideAll = false;
    KeymapEntry entries[MAX_KEYS]{};

    bool hasModifiers(const KeymapBinding &binding, unsigned int flags);
public:
    Keymap();
    void destroy();

    void setOverride(const bool value){bOverrideAll = value;}

    void addBinding(Keys key, KeymapBindType type, unsigned int modifierFlags, void* data, KeymapCallback callback, const String &id);
    void removeBinding(Keys key, const String &id) const;

    /**
     * @brief Processes the bindings in this map.
     * @param pressed Whether the key was pressed or released.
     * @param modifierFlags What modifier flags must the binding require to be called? (SHIFT, ALT, CONTROL)
     * @param key The key that was pressed
     * @return True if this map overrides all other maps or a binding was set to UNSET. False otherwise.
     */
    bool processBindings(bool pressed, unsigned int modifierFlags, Keys key);

    /**
      * @brief Processes the bindings in this map for the HOLD type specifically.
      * @param modifierFlags What modifier flags must the binding require to be called? (SHIFT, ALT, CONTROL)
      * @param key The key that was pressed
      * @return True if this map overrides all other maps or a binding was set to UNSET. False otherwise.
      */
    bool processHold(unsigned int modifierFlags, Keys key);
};