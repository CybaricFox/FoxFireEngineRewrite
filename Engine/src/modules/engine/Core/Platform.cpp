//
// Created by cmorg on 6/30/2026.
//

#include "Platform.h"
#include "src/defines.h"

#include <iostream>
#include <ostream>

#include "src/modules/engine/Memory/DynamicArray.h"
#include "../Input/EngineEvents.h"

#include "Platforms/PlatformLinux.h"
#include "Platforms/PlatformWindows.h"

PlatformState* Platform::platformState = nullptr;


void Platform::processInputs() const {
    platformState->processInputs(*inputSystemRef);
}

bool Platform::initialize(const String &applicationName, const int x, const int y, const int width, const int height, IInputSystem *inputSystem) {
    if (!platformState) return false;

    inputSystemRef = inputSystem;

    return platformState->initialize(applicationName, x, y, width, height);
}

void Platform::setPlatform() {
#if FOXFIRE_PLATFORM_WINDOWS == 1
    platformState = new PlatformWindows{};
#endif
#if FOXFIRE_PLATFORM_LINUX == 1
    platformState = new PlatformLinux{};
#endif
}

void Platform::ff_sleep(const unsigned long ms) {
    platformState->ff_sleep(ms);
}

void Platform::getRequiredExtensions(DynamicArray<const char*>& extensions) {
    platformState->getRequiredExtensions(extensions);
}

bool Platform::createSurface() const {
    return platformState->createSurface();
}

void* Platform::platform_allocate(const unsigned long size, bool align) {
    return platformState->allocate(size, align);
}

void Platform::platform_free(void *memory, bool align) {
    platformState->freeMemory(memory, align);
}

void Platform::platform_clear(void *memory, const unsigned long size) {
    platformState->clear(memory, size);
}

void Platform::printConsoleMessage(const String& message, const unsigned char color) {
    platformState->printConsoleMessage(message, color);
}

void Platform::printConsoleError(const String& message, const unsigned char color) {
    platformState->printConsoleError(message, color);
}

double Platform::getAbsoluteTime() {
    return platformState->getAbsoluteTime();
}

Platform::~Platform() {
    delete platformState;
    platformState = nullptr;
}

void Platform::shutdown() {
    platformState->shutdown();
}

//Tells the platform to process the windows messages to OS.
bool Platform::processMessages() {
    return platformState->processMessages();
}