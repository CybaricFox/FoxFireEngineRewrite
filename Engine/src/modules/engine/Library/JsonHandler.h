//
// Created by cmorg on 8/28/2026.
//

#pragma once
#include <ranges>

#include "FileHandler.h"
#include "src/modules/engine/Memory/DynamicArray.h"

/**
 *  @file JsonHandler.h
 *  @layer 
 *  @module
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 8/28/2026
 *
 *  @copyright (c) 2026
 */

enum JsonType {
    JSON_UNKNOWN,
    //JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_FLOAT,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
};

struct JsonHeader {
    String name{};
    JsonType type{};
    bool bWaitingForEnd = true;
};

struct JsonObject {
    DynamicArray<JsonHeader> keys{};
    DynamicArray<void*> values{};
};

class JsonHandler {
private:
    FileHandler& file;
    JsonObject root{};
    bool bRootEstablished = false;

    bool parse(JsonHeader *header, JsonObject &object);
    void parseObject(JsonHeader *header, JsonObject &object);
    void parseArray(JsonHeader &header, JsonObject &object);
    void parseValue(JsonHeader &header, JsonObject &object, String &value);
    void logJsonObject(JsonObject& object);
    void shutdownJsonObject(JsonObject& object);
    void shutdownJsonArray(DynamicArray<JsonObject>& array);
    void shutdownJsonValue(const JsonHeader& header, void* value);

public:
    explicit JsonHandler(FileHandler& file) : file(file) {beginParse();}
    void shutdown();

    DynamicArray<JsonObject>* getArray(const String& name, JsonObject* object = nullptr, const bool isOptional = false) {
        if (object == nullptr) object = &root;
        for (unsigned int i = 0; i < object->keys.getLength(); i++) {
            if (object->keys[i].name == name && object->keys[i].type == JSON_ARRAY) {
                return static_cast<DynamicArray<JsonObject> *>(object->values[i]);
            }
        }

        if (!isOptional) Logger::logError("Cannot find a json attribute with the expected type and name: " + name);
        return nullptr;
    }
    JsonObject* getObject(const String &name, JsonObject* object = nullptr) {
        if (object == nullptr) object = &root;
        for (unsigned int i = 0; i < object->keys.getLength(); i++) {
            if (object->keys[i].name == name && object->keys[i].type == JSON_OBJECT) {
                return static_cast<JsonObject*>(object->values[i]);
            }
        }

        Logger::logError("Cannot find a json attribute with the expected type and name: " + name);
        return nullptr;
    }
    String getString(const String &name, JsonObject* object = nullptr, const bool isOptional = false) {
        if (object == nullptr) object = &root;
        for (unsigned int i = 0; i < object->keys.getLength(); i++) {
            if (object->keys[i].name == name && object->keys[i].type == JSON_STRING) {
                return *static_cast<String*>(object->values[i]);
            }
        }

        if (!isOptional) Logger::logError("Cannot find a json attribute with the expected type and name: " + name);
        return "";
    }
    float getFloat(const String &name, JsonObject* object = nullptr) {
        if (object == nullptr) object = &root;

        if (name.empty() && object->values[0]) {
            return *static_cast<float*>(object->values[0]);
        }

        for (unsigned int i = 0; i < object->keys.getLength(); i++) {
            if (object->keys[i].name == name && object->keys[i].type == JSON_FLOAT) {
                return *static_cast<float*>(object->values[i]);
            }
        }

        Logger::logError("Cannot find a json attribute with the expected type and name: " + name);
        return 0;
    }
    int getInt(const String &name, JsonObject* object = nullptr) {
        if (object == nullptr) object = &root;
        for (unsigned int i = 0; i < object->keys.getLength(); i++) {
            if (object->keys[i].name == name && object->keys[i].type == JSON_NUMBER) {
                return *static_cast<int*>(object->values[i]);
            }
        }

        return INVALID_ID_U32 / 2;
    }

    /**
     * @brief Fetches a bool value from a json object.
     * @param name Name of the json entry
     * @param boolOut The bool value if it exists
     * @param object The JsonObject to grab from
     * @return True if the value exists, false if it does not
     */
    bool getBool(const String &name, bool& boolOut, JsonObject* object = nullptr) {
        if (object == nullptr) object = &root;
        for (unsigned int i = 0; i < object->keys.getLength(); i++) {
            if (object->keys[i].name == name && object->keys[i].type == JSON_BOOL) {
                boolOut = *static_cast<bool*>(object->values[i]);
                return true;
            }
        }

        return false;
    }

    void beginParse();
};