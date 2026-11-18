//
// Created by cmorg on 11/17/2026.
//

#pragma once
#include "../IConsole.h"
#include "src/modules/engine/Input/EngineEvents.h"
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/UserInterface/UIText.h"
#include "foxfire_export.h"

/**
 *  @file DebugConsole.h
 *  @layer Engine
 *  @module Core
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/17/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API DebugConsole final : public IConsole {
private:
    /**
     * @brief How many lines to display
     */
    int lineDisplayCount = 0;
    /**
     * @brief How many lines offset from the most recent (Allows scrolling)
     */
    int lineOffset = 0;
    DynamicArray<String> lines{};
    bool bIsDirty = false;
    bool bIsVisible = false;
    /**
     * @brief Contains the lines actively displaying
     */
    UIText text{};
    /**
     * @brief Contains the line for input
     */
    UIText entry{};

    void onInputPress(EngineInputContext context);
    void onInputRelease(EngineInputContext context);

public:
    void initialize() override;
    void shutdown() override;

    UIText& getText() {return text;}
    UIText& getEntry() {return entry;}
    [[nodiscard]] bool isVisible() const {return bIsVisible;}

    void setVisibility(const bool visible) {bIsVisible = visible;}

    bool load();
    void update();
    bool write(LogLevel level, String message) override;
};