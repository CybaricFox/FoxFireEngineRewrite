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

/**
 * @brief The type of value stored in the json file.
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

/**
 * @brief Contains data related to a field in the json file.
 */
struct JsonHeader {
    /**
     * @brief Name of the field
     */
    String name{};
    /**
     * @brief Type of the field.
     */
    JsonType type{};
    /**
     * @brief Whether this field has closed in the file or is still being iterated.
     */
    bool bWaitingForEnd = true;
};

/**
 * @brief Contains the internal data of a field.
 */
struct JsonObject {
    /**
     * @brief Contains headers for fields inside this object.
     */
    DynamicArray<JsonHeader> keys{};
    /**
     * @brief Contains the values of those fields.
     */
    DynamicArray<void*> values{};
};

/**
 * @brief This class reads a json file and converts it to c++.
 */
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

    /**
     * @brief Gets a jsonObject that is a type of array.
     * @param name Name of the field.
     * @param object The parent JsonObject.
     * @param isOptional Whether this value is important or not. If true, an error message will print. If false, it is ignored.
     * @return Dynamic array of json objects that are fields within this array.
     */
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

    /**
     * @brief Gets a jsonObject.
     * @param name Name of the field.
     * @param object Parent of this object.
     * @return Pointer to the object or nullptr if it cannot be found.
     */
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

    /**
     * @brief Gets a string from a jsonObject.
     * @param name Name of the field.
     * @param object The parent jsonObject.
     * @param isOptional Whether this value is important or not. If true, an error message will print. If false, it is ignored.
     * @return String contained within the field. Will be empty if no string is found.
     */
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

    /**
      * @brief Gets a float from a jsonObject.
      * @param name Name of the field.
      * @param object The parent jsonObject.
      * @return Float contained within the field. Will be 0 if no float is found.
      */
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
    /**
      * @brief Gets an int from a jsonObject.
      * @param name Name of the field.
      * @param object The parent jsonObject.
      * @return Int contained within the field. Will be INVALID_ID if no int is found.
      */
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

    /**
     * @brief Begins the parsing of a json file.
     */
    void beginParse();
};