//
// Created by cmorg on 7/1/2026.
//

#include "StringUtils.h"

#include <cstring>


bool StringUtils::equalsIgnoreCase(const String &a, const String &b) {
    if (a.length() != b.length()) return false;

    const char* ca = const_cast<char*>(a.c_str());
    const char* cb = const_cast<char*>(b.c_str());

    for (int i = 0; i <= a.length(); i++) {
        if (std::tolower(static_cast<unsigned char>(ca[i])) != std::tolower(static_cast<unsigned char>(cb[i]))) return false;
    }

    return true;
}

String StringUtils::copyLimited(String string, const long size) {
    if (string.length() <= size) return string;

    String out;

    for (int i = 0; i <= size; i++) {
        out.push_back(string[i]);
    }

    return out;
}

void StringUtils::trim(String &out) {
    if (out.empty()) return;

    String temp = "NULL";
    for (int i = 0; i <= out.length(); i++) {
        if (!isspace(out[i])) {
            temp = out.substr(i);
            break;
        }
    }

    for (unsigned long i = out.length() - 1; i > 0; i--) {
        if (!isspace(out[i])) {
            unsigned long cut = out.length() - 1 - i;

            while (cut > 0) {
                temp.pop_back();
                cut--;
            }
            break;
        }
    }

    if (temp != "NULL") {
        out = temp;
    }
}

bool StringUtils::stringToFloat(const String &string, float &out) {
    try {
        out = std::stof(string);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToDouble(const String &string, double &out) {
    try {
        out = std::stod(string);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToChar(const String &string, char &out) {
    try {
        const int temp = std::stoi(string);
        out = static_cast<char>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToShort(const String &string, short &out) {
    try {
        const int temp = std::stoi(string);
        out = static_cast<short>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToInt(const String &string, int &out) {
    try {
        out = std::stoi(string);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToUChar(const String &string, unsigned char &out) {
    try {
        const long temp = std::stol(string);
        out = static_cast<unsigned char>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToUShort(const String &string, unsigned short &out) {
    try {
        const long temp = std::stol(string);
        out = static_cast<unsigned short>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToUInt(const String &string, unsigned int &out) {
    try {
        const long temp = std::stol(string);
        out = static_cast<unsigned int>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToULong(const String &string, unsigned long &out) {
    try {
        out = stoll(string);
        return true;
    } catch (...) {
        return false;
    }
}

bool StringUtils::stringToBool(const String &string, bool &out) {
    if (equalsIgnoreCase(string, "true")) {
        out = true;
        return true;
    }
    if (equalsIgnoreCase(string, "false")) {
        out = false;
        return true;
    }
    return false;
}

unsigned int StringUtils::findAll(const String &string, const char toFind) {
    unsigned int count = 0;
    for (const char& c : string) {
        if (c == toFind) {
            count++;
        }
    }

    return count;
}

unsigned int StringUtils::recursiveSplit(const String &string, const char regex, DynamicArray<String> &array) {
    String remaining = string;

    if (array.getCapacity() == 0) {
        array.initialize();
    }

    while (!remaining.empty()) {
        const unsigned int index = remaining.find(regex);
        if (index == static_cast<unsigned int>(String::npos)) {
            trim(remaining);
            array.push(remaining);
            remaining.clear();
            continue;
        }

        String sub = remaining.substr(0, index);
        trim(sub);
        array.push(sub);
        remaining = remaining.substr(index + 1);
    }

    return array.getLength();
}

bool StringUtils::stringToLong(const String &string, long &out) {
    try {
        out = std::stol(string);
        return true;
    } catch (...) {
        return false;
    }
}
