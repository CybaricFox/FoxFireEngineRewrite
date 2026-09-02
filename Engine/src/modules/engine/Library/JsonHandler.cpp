//
// Created by cmorg on 8/28/2026.
//

#include "JsonHandler.h"

#include <array>
#include <bits/locale_facets_nonio.h>

#include "StringUtils.h"

bool JsonHandler::parse(JsonHeader* header, JsonObject& object) {
    String line{};
    unsigned long outBytes = 0;

    file.readLine(line, 511, outBytes);
    if (outBytes == 0) return false;

    StringUtils::trim(line);

    switch (line[0]) {
        case '}': {
            return true;
        }
        case ']': {
            if (header && header->type == JSON_ARRAY) {
                header->bWaitingForEnd = false;
            }
            return false;
        }
        case '{': {
            if (header && header->type == JSON_ARRAY) {
                //Skip the line in this case.
                return false;
            } else {
                parseObject(nullptr, object);
            }
            return false;
        }
        case ' ': return false;
        case '"': {
            const unsigned int index = line.find(':');
            if (index == static_cast<unsigned int>(String::npos)) {
                Logger::logFatal("Json Parser ran into a fatal error! ':' was not found after \" initializer! Parser cannot continue!");
                return true;
            }
            String name = line.substr(0, index);
            String value = line.substr(index + 1);
            StringUtils::trim(name);
            name = name.substr(1, name.length() - 2);
            StringUtils::trim(value);

            JsonHeader& newHeader = *object.keys.emplace(name);

            switch (value[0]) {
                case '{': {
                    parseObject(&newHeader, object);
                    return false;
                }
                case '[': {
                    parseArray(newHeader, object);
                    return false;
                }
                default: {
                    parseValue(newHeader, object, value);
                    return false;
                }
            }
        }
        default: {
            if (header && header->type == JSON_ARRAY) {
                JsonHeader& newHeader = *object.keys.emplace();
                parseValue(newHeader, object, line);
            }
            return false;
        }
    }
}

void JsonHandler::parseObject(JsonHeader* header, JsonObject& object) {
    if (!bRootEstablished) {
        root.keys.initialize();
        root.values.initialize();
        bRootEstablished = true;
        return;
    }

    if (header == nullptr) return;

    header->type = JSON_OBJECT;
    object.values.push(FF_Memory::ff_allocate_class<JsonObject>(sizeof(JsonObject), DYNAMIC_ARRAY));
    JsonObject& objectObject = *static_cast<JsonObject *>(object.values[object.values.getLength() - 1]);
    objectObject.keys.initialize();
    objectObject.values.initialize();

    while (header->bWaitingForEnd) {
        if (parse(header, objectObject)) header->bWaitingForEnd = false;
    }
}

void JsonHandler::parseArray(JsonHeader& header, JsonObject& object) {
    header.type = JSON_ARRAY;
    object.values.push(FF_Memory::ff_allocate_class<DynamicArray<JsonObject>>(sizeof(DynamicArray<JsonObject>), DYNAMIC_ARRAY));

    const auto array = static_cast<DynamicArray<JsonObject> *>(object.values[object.values.getLength() - 1]);
    array->initialize(1);
    JsonObject* arrayObject = array->emplace();
    arrayObject->values.initialize();
    arrayObject->keys.initialize();

    bool previousResult = false;
    while (true) {
        const bool result = parse(&header, *arrayObject);

        if (!header.bWaitingForEnd) break;
        if (previousResult) {
            arrayObject = array->emplace();
            arrayObject->values.initialize();
            arrayObject->keys.initialize();
        }

        previousResult = result;
    }
}

void JsonHandler::parseValue(JsonHeader& header, JsonObject& object, String& value) {
    switch (value[0]) {
        case '"': {
            String* stringValue = FF_Memory::ff_allocate_class<String>(sizeof(String), DYNAMIC_ARRAY);
            *stringValue = value.substr(1, value.length() - 2);

            object.values.push(stringValue);
            header.type = JSON_STRING;
            header.bWaitingForEnd = false;
            return;
        }
        default: {
            bool boolValue = false;
            const bool result = StringUtils::stringToBool(value, boolValue);
            unsigned int index = value.find('.');

            if (result) {
                header.type = JSON_BOOL;
                const auto boolOut = static_cast<bool *>(FF_Memory::ff_allocate(sizeof(bool), DYNAMIC_ARRAY));
                *boolOut = boolValue;
                object.values.push(boolOut);
            } else if (index == static_cast<unsigned int>(String::npos)) {
                header.type = JSON_NUMBER;
                const auto intValue = static_cast<int *>(FF_Memory::ff_allocate(sizeof(int), DYNAMIC_ARRAY));
                StringUtils::stringToInt(value, *intValue);
                object.values.push(intValue);
            } else {
                header.type = JSON_FLOAT;
                const auto floatValue = static_cast<float *>(FF_Memory::ff_allocate(sizeof(float), DYNAMIC_ARRAY));
                StringUtils::stringToFloat(value, *floatValue);
                object.values.push(floatValue);
            }

            header.bWaitingForEnd = false;
        }
    }
}

void JsonHandler::logJsonObject(JsonObject &object) {
    Logger::logDebug("{");
    for (unsigned int i = 0; i < object.keys.getLength(); i++) {
        String outputString = object.keys[i].name + ": ";

        switch (object.keys[i].type) {
            case JSON_NUMBER: {
                outputString.append(std::to_string(*static_cast<int*>(object.values[i])));
                break;
            }
            case JSON_FLOAT: {
                outputString.append(std::to_string(*static_cast<float*>(object.values[i])));
                break;
            }
            case JSON_STRING: {
                outputString.append(*static_cast<String*>(object.values[i]));
                break;
            }
            case JSON_OBJECT: {
                Logger::logDebug(outputString);
                outputString.clear();
                logJsonObject(*static_cast<JsonObject *>(object.values[i]));
                break;
            }
            case JSON_ARRAY: {
                Logger::logDebug(outputString + "[");
                outputString.clear();
                const auto array = static_cast<DynamicArray<JsonObject> *>(object.values[i]);

                for (JsonObject& arrayObject : *array) {
                    logJsonObject(arrayObject);
                }

                Logger::logDebug("]");
                break;
            }
            default: break;
        }

        if (outputString.empty()) continue;
        Logger::logDebug(outputString);
        outputString.clear();
    }
    Logger::logDebug("}");
}

void JsonHandler::shutdownJsonObject(JsonObject &object) {
    unsigned int i = 0;
    for (const JsonHeader& header : object.keys) {
        if (header.type == JSON_OBJECT) {
            shutdownJsonObject(*static_cast<JsonObject *>(object.values[i]));
            FF_Memory::ff_free_class<JsonObject>(object.values[i], sizeof(JsonObject), DYNAMIC_ARRAY);
        } else if (header.type == JSON_ARRAY) {
            DynamicArray<JsonObject>& array = *static_cast<DynamicArray<JsonObject>*>(object.values[i]);
            shutdownJsonArray(array);
        } else {
            shutdownJsonValue(header, object.values[i]);
        }
        i++;
    }

    object.values.shutdown();
    object.keys.shutdown();
}

void JsonHandler::shutdownJsonArray(DynamicArray<JsonObject> &array) {
    for (JsonObject& object : array) {
        shutdownJsonObject(object);
    }

    FF_Memory::ff_free_class<DynamicArray<JsonObject>>(&array, sizeof(DynamicArray<JsonObject>), DYNAMIC_ARRAY);
}

void JsonHandler::shutdownJsonValue(const JsonHeader &header, void *value) {
    switch (header.type) {
        case JSON_NUMBER: {
            FF_Memory::ff_free(value, sizeof(int), DYNAMIC_ARRAY);
            break;
        }
        case JSON_FLOAT: {
            FF_Memory::ff_free(value, sizeof(float), DYNAMIC_ARRAY);
            break;
        }
        case JSON_STRING: {
            FF_Memory::ff_free_class<String>(value, sizeof(String), DYNAMIC_ARRAY);
            break;
        }
        case JSON_BOOL: {
            FF_Memory::ff_free(value, sizeof(bool), DYNAMIC_ARRAY);
            break;
        }
        default: {
            Logger::logError("Json Shutdown Value failed to recognize type! Memory will leak!");
            break;
        }
    }
}


void JsonHandler::shutdown() {
    shutdownJsonObject(root);
}

void JsonHandler::beginParse() {
    String line{};
    unsigned long outBytes = 0;
    while (!parse(nullptr, root)) {}

    //logJsonObject(root);
}
