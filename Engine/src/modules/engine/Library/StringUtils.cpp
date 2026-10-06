//
// Created by cmorg on 7/1/2026.
//

#include "StringUtils.h"

#include <cstring>

bool StringUtils::stringToFloat(const String &string, float &out) {
    try {
        out = std::stof(string.getAsCharString());
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToDouble(const String &string, double &out) {
    try {
        out = std::stod(string.getAsCharString());
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToChar(const String &string, char &out) {
    try {
        const int temp = std::stoi(string.getAsCharString());
        out = static_cast<char>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToShort(const String &string, short &out) {
    try {
        const int temp = std::stoi(string.getAsCharString());
        out = static_cast<short>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToInt(const String &string, int &out) {
    try {
        out = std::stoi(string.getAsCharString());
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToUChar(const String &string, unsigned char &out) {
    try {
        const long temp = std::stol(string.getAsCharString());
        out = static_cast<unsigned char>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToUShort(const String &string, unsigned short &out) {
    try {
        const long temp = std::stol(string.getAsCharString());
        out = static_cast<unsigned short>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToUInt(const String &string, unsigned int &out) {
    try {
        const long temp = std::stol(string.getAsCharString());
        out = static_cast<unsigned int>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToULong(const String &string, ULong &out) {
    try {
        out = std::stoll(string.getAsCharString());
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToBool(const String &string, bool &out) {
    if (string.equalsIgnoreCase("true")) {
        out = true;
        return true;
    }
    if (string.equalsIgnoreCase("false")) {
        out = false;
        return true;
    }
    return false;
}

unsigned int StringUtils::recursiveSplit(String &string, const char regex, DynamicArray<String> &array) {
    const unsigned int count = string.findAll(',') + 1;
    if (array.getCapacity() == 0) {
        array.initialize(count);
    }
    for (unsigned int i = 0; i < count; i++) {
        array.emplace();
    }

    return string.recursiveSplit(regex, array.getData());
}

String StringUtils::getDirectoryFromPath(String &path) {
    //Do not check 0 because /directory/file would return nothing.
    unsigned int index = path.find('/', 1);
    if (index == static_cast<unsigned int>(INVALID_ID_U64)) {
        index = path.find('\\', 1);
        if (index == static_cast<unsigned int>(INVALID_ID_U64)) {
            Logger::logWarn("Failed to fetch a directory from the file path: " + path);
            return "";
        }
    }

    return path.substringLeft(index);
}

String StringUtils::getFilenameFromPath(String &path) {
    //Do not check 0 because /directory/file would return nothing.
    unsigned int index = path.find('/', 1);
    if (index == static_cast<unsigned int>(INVALID_ID_U64)) {
        index = path.find('\\', 1);
        if (index == static_cast<unsigned int>(INVALID_ID_U64)) {
            Logger::logWarn("Failed to fetch a filename from the file path: " + path);
            return "";
        }
    }

    return path.substringRight(index + 1);
}

String StringUtils::getFilenameNoExtensionFromPath(String &path) {
    String fileName = getFilenameFromPath(path);

    const unsigned int index = fileName.findLast('.');
    if (index == static_cast<unsigned int>(INVALID_ID_U64)) {
        Logger::logWarn("Failed to fetch a filename (no extension) from the file path: " + path);
        return "";
    }

    return fileName.substringLeft(index);
}

bool StringUtils::stringToLong(const String &string, long &out) {
    try {
        out = std::stol(string.getAsCharString());
        return true;
    } catch (...) {
        return false;
    }
}
